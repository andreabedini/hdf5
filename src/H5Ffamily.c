/*
 * Copyright (C) 1997 Spizella Software
 *		      All rights reserved.
 *
 * Programmer: Robb Matzke <matzke@llnl.gov>
 *	       Monday, November 10, 1997
 *
 * Purpose:	Implements a family of files that acts as a single hdf5
 *		file.  The purpose is to be able to split a huge file on a
 *		64-bit platform, transfer all the <2GB members to a 32-bit
 *		platform, and then access the entire huge file on the 32-bit
 *		platform.
 *
 *		All family members are logically the same size although their
 *		physical sizes may vary.  The logical member size is
 *		determined by looking at the physical size of the first
 *		member and rounding that up to the next power of two.  When
 *		creating a file family, the first member is created with a
 *		predefined physical size (actually, this happens when the
 *		file family is flushed, and can be quite time consuming on
 *		file systems that don't implement holes, like nfs).
 *		
 */
#include <H5private.h>
#include <H5Eprivate.h>
#include <H5Fprivate.h>
#include <H5MMprivate.h>

#define PABLO_MASK H5F_family
static hbool_t		interface_initialize_g = FALSE;
#define INTERFACE_INIT NULL

#define H5F_FAM_MASK(N)		(((uint64)1<<(N))-1)
#define H5F_FAM_OFFSET(ADDR,N)	((off_t)((ADDR)->offset & H5F_FAM_MASK(N)))
#define H5F_FAM_MEMBNO(ADDR,N)	((intn)((ADDR)->offset >> (N)))

static hbool_t H5F_fam_access(const char *name,
			      const H5F_access_t *access_parms, int mode,
			      H5F_search_t *key/*out*/);
static H5F_low_t *H5F_fam_open(const char *name,
			       const H5F_access_t *access_parms, uintn flags,
			       H5F_search_t *key/*out*/);
static herr_t H5F_fam_close(H5F_low_t *lf, const H5F_access_t *access_parms);
static herr_t H5F_fam_read(H5F_low_t *lf, const H5F_access_t *access_parms,
			   const haddr_t *addr, size_t size, uint8 *buf);
static herr_t H5F_fam_write(H5F_low_t *lf, const H5F_access_t *access_parms,
			    const haddr_t *addr, size_t size,
			    const uint8 *buf);
static herr_t H5F_fam_flush(H5F_low_t *lf, const H5F_access_t *access_parms);

const H5F_low_class_t H5F_LOW_FAMILY_g[1] = {{
    H5F_fam_access,	    /* access method			    */
    H5F_fam_open,	    /* open method			    */
    H5F_fam_close,	    /* close method			    */
    H5F_fam_read,	    /* read method			    */
    H5F_fam_write,	    /* write method			    */
    H5F_fam_flush,	    /* flush method			    */
    NULL,		    /* extend method			    */
}};


/*-------------------------------------------------------------------------
 * Function:	H5F_fam_open
 *
 * Purpose:	Opens a file family with the specified base name.  The name
 *		should contain a printf-style "%d" field which will be
 *		expanded with a zero-origin family member number.
 *
 * Bugs:	We don't check for overflow on the name, so keep it under
 *		4kb, please.  Also, we don't actually check for the `%d'
 *		field because we assume that the caller already did.  Who
 *		knows what happens when all the family member names are the
 *		same!
 *
 * Return:	Success:	Low-level file pointer
 *
 *		Failure:	NULL
 *
 * Programmer:	Robb Matzke
 *		Monday, November 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static H5F_low_t *
H5F_fam_open(const char *name, const H5F_access_t *access_parms,
	     uintn flags, H5F_search_t *key/*out*/)
{
    H5F_low_t	*ret_value = NULL;
    H5F_low_t	*lf = NULL;
    H5F_low_t	*member = NULL;		/*a family member		*/
    char	member_name[4096];	/*name of family member		*/
    intn	membno;			/*member number (zero-origin)	*/
    size_t	nbits; 			/*num bits in an offset     */
    haddr_t	tmp_addr;		/*temporary address		*/
    const H5F_low_class_t *memb_type;	/*type of family member		*/

    FUNC_ENTER(H5F_fam_open, NULL);

    assert (access_parms);
    assert (H5F_LOW_FAMILY==access_parms->driver);

    /*
     * Use the default file driver or the specified driver for each of the
     * family members.
     */
    if (access_parms->u.fam.memb_access) {
	memb_type = H5F_low_class (access_parms->u.fam.memb_access->driver);
    } else {
	memb_type = H5F_low_class (H5F_LOW_DFLT);
    }

    /*
     * If we're truncating the file then delete all but the first family
     * member.
     */
    if ((flags & H5F_ACC_RDWR) && (flags & H5F_ACC_TRUNC)) {
	for (membno=1; /*void*/; membno++) {
	    sprintf(member_name, name, membno);
	    if (!H5F_low_access(memb_type, member_name,
				access_parms->u.fam.memb_access,
				F_OK, NULL)) {
		break;
	    } else if (unlink(member_name) < 0) {
		HGOTO_ERROR(H5E_IO, H5E_CANTINIT, NULL, "can't delete member");
	    }
	}
    }

    /* Create the file descriptor */
    lf = H5MM_xcalloc(1, sizeof(H5F_low_t));
    lf->u.fam.name = H5MM_xstrdup(name);
    lf->u.fam.flags = (flags & ~H5F_ACC_CREAT);

    /* Open all existing members */
    for (membno = 0; /*void*/; membno++) {
	sprintf(member_name, name, membno);

	/*
	 * Open the family member.  After the first member is opened or
	 * created, turn off the creation flag so we don't create a zillion
	 * family members.
	 */
	member = H5F_low_open(memb_type, member_name,
			      access_parms->u.fam.memb_access, flags,
			      0 == membno ? key : NULL);
	if (!member) {
	    if (0 == membno) {
		HGOTO_ERROR(H5E_IO, H5E_CANTOPENFILE, NULL,
			    "can't open first family member");
	    }
	    break;
	}
	flags &= ~H5F_ACC_CREAT;

	/* Add the member to the family */
	if (lf->u.fam.nmemb >= lf->u.fam.nalloc) {
	    lf->u.fam.nalloc = MAX(100, 2 * lf->u.fam.nalloc);
	    lf->u.fam.memb = H5MM_xrealloc(lf->u.fam.memb,
				       lf->u.fam.nalloc * sizeof(H5F_low_t *));
	}
	lf->u.fam.memb[lf->u.fam.nmemb++] = member;
	member = NULL;
    }

    /* Calculate member size */
    if (lf->u.fam.nmemb >= 2) {
	/*
	 * If the first and second files exists then round the first file size
	 * up to the next power of two and use that as the number of bits per
	 * family member.
	 */
	size_t size = H5F_low_size(lf->u.fam.memb[0], &tmp_addr);
	for (nbits=8*sizeof(size_t)-1; nbits>0; --nbits) {
	    size_t mask = (size_t)1 << nbits;
	    if (size & mask) {
		if (size != mask) {
		    size++;
#ifdef H5F_DEBUG
		    fprintf(stderr, "H5F: family member size was "
			    "rounded up to a power of 2");
#endif
		}
		break;
	    }
	}
    } else {
	/*
	 * Typically, the number of bits in the member offset can be up to two
	 * less than the number of bits in an `off_t'.  On a 32-bit machine,
	 * for instance, files can be up to 2GB-1byte in size, but since HDF5
	 * must have a power of two we're restricted to just 1GB.  Smaller
	 * values result in files of a more manageable size (from a human
	 * perspective) but also limit the total logical size of the hdf5 file
	 * since most OS's only allow a certain number of open file
	 * descriptors (all family members are open at once).
	 */
#ifdef H5F_DEBUG
	if (access_parms->u.fam.offset_bits+2>=8*sizeof(off_t)) {
	    fprintf (stderr, "H5F: family member size may be too large.\n");
	}
#endif
	nbits = MAX (access_parms->u.fam.offset_bits, 10); /*1K min*/
    }
    lf->u.fam.offset_bits = nbits;

#ifdef H5F_DEBUG
    if (nbits >= 30) {
	fprintf(stderr, "H5F: family members are %dGB\n",
		1 << (nbits-30));
    } else if (nbits >= 20) {
	fprintf(stderr, "H5F: family members are %dMB\n",
		1 << (nbits-20));
    } else if (nbits >= 10) {
	fprintf(stderr, "H5F: family members are %dkB\n",
		1 << (nbits-10));
    } else {
	fprintf(stderr, "H5F: family members are %d bytes\n",
		1 << nbits);
    }
#endif

    /*
     * Get the total family size and store it in the max_addr field.
     */
    assert(lf->u.fam.nmemb >= 1);
    lf->eof.offset = (size_t)1 << lf->u.fam.offset_bits;
    lf->eof.offset *= (lf->u.fam.nmemb-1);
    lf->eof.offset += lf->u.fam.memb[lf->u.fam.nmemb-1]->eof.offset;

    HRETURN(lf);

  done:
    if (!ret_value) {
	if (lf) {
	    H5F_fam_close(lf, access_parms);
	    H5MM_xfree(lf);
	}
    }
    FUNC_LEAVE(ret_value);
}

/*-------------------------------------------------------------------------
 * Function:	H5F_fam_close
 *
 * Purpose:	Closes all members of a file family and releases resources
 *		used by the file descriptor.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Monday, November 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5F_fam_close(H5F_low_t *lf, const H5F_access_t *access_parms)
{
    intn		    membno;

    FUNC_ENTER(H5F_fam_close, FAIL);

    assert(lf);

    for (membno = 0; membno < lf->u.fam.nmemb; membno++) {
	lf->u.fam.memb[membno] = H5F_low_close(lf->u.fam.memb[membno],
					      access_parms->u.fam.memb_access);
    }
    H5MM_xfree(lf->u.fam.memb);
    H5MM_xfree(lf->u.fam.name);

    FUNC_LEAVE(SUCCEED);
}

/*-------------------------------------------------------------------------
 * Function:	H5F_fam_read
 *
 * Purpose:	Reads a chunk of contiguous data from the file family.
 *		Reading past the physical end of a file returns zeros instead
 *		of failing. We must insure that if the logical end of file is
 *		before the physical end of file that we will read zeros there
 *		also (the only time this can happen is if we create a family
 *		and then close it before the first member is filled, since
 *		flushing the file causes the first member to be physically
 *		extended to it's maximum size).
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Monday, November 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5F_fam_read(H5F_low_t *lf, const H5F_access_t *access_parms,
	     const haddr_t *addr, size_t size, uint8 *buf)
{
    size_t		    nbytes;
    haddr_t		    cur_addr;
    intn		    membno;
    off_t		    offset;
    hsize_t		    member_size;

    FUNC_ENTER(H5F_fam_read, FAIL);

    assert(lf);
    assert(addr && H5F_addr_defined(addr));
    assert(buf);

    member_size = (hsize_t) 1 << lf->u.fam.offset_bits;
    membno = H5F_FAM_MEMBNO(addr, lf->u.fam.offset_bits);
    offset = H5F_FAM_OFFSET(addr, lf->u.fam.offset_bits);
    cur_addr = *addr;

    while (size > 0) {
	if (membno >= lf->u.fam.nmemb) {
	    HDmemset(buf, 0, size);
	    break;
	} else {
	    nbytes = MIN(size, member_size-offset);
	    cur_addr.offset = offset;
	    if (H5F_low_read(lf->u.fam.memb[membno],
			     access_parms->u.fam.memb_access,
			     &cur_addr, nbytes, buf) < 0) {
		HRETURN_ERROR(H5E_IO, H5E_READERROR, FAIL,
			      "can't read from family member");
	    }
	    buf += nbytes;
	    size -= nbytes;
	    membno++;
	    offset = 0;
	}
    }

    FUNC_LEAVE(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:	H5F_fam_write
 *
 * Purpose:	Writes BUF to the family of files.  The superclass has
 *		already insured that we aren't writing past the logical end
 *		of file, so this function will extend the physical file to
 *		accommodate the new data if necessary.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Monday, November 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5F_fam_write(H5F_low_t *lf, const H5F_access_t *access_parms,
	      const haddr_t *addr, size_t size, const uint8 *buf)
{
    size_t		   	nbytes;
    haddr_t		    	cur_addr, max_addr;
    intn		    	membno;
    off_t		    	offset;
    H5F_low_t		   	*member = NULL;
    char		    	member_name[4096];
    intn		    	i;
    hsize_t		    	member_size;
    const H5F_low_class_t	*memb_type = NULL;
    
    FUNC_ENTER(H5F_fam_write, FAIL);

    /* Check args */
    assert(lf);
    assert(addr && H5F_addr_defined(addr));
    assert(buf);
    assert (access_parms);
    assert (H5F_LOW_FAMILY==access_parms->driver);

    /* Get the member driver */
    if (access_parms->u.fam.memb_access) {
	memb_type = H5F_low_class (access_parms->u.fam.memb_access->driver);
    } else {
	memb_type = H5F_low_class (H5F_LOW_DFLT);
    }
    
    member_size = (hsize_t) 1 << lf->u.fam.offset_bits;
    membno = H5F_FAM_MEMBNO(addr, lf->u.fam.offset_bits);
    offset = H5F_FAM_OFFSET(addr, lf->u.fam.offset_bits);
    cur_addr = *addr;

    while (size > 0) {
	nbytes = MIN(size, member_size - offset);
	cur_addr.offset = offset;

	if (membno >= lf->u.fam.nmemb) {
	    /*
	     * We're writing past the end of the last family member--create the
	     * new family member(s)
	     */
	    for (i = lf->u.fam.nmemb; i <= membno; i++) {
		sprintf(member_name, lf->u.fam.name, i);
		member = H5F_low_open(memb_type, member_name,
				      access_parms->u.fam.memb_access, 
				      lf->u.fam.flags | H5F_ACC_CREAT,
				      NULL);
		if (!member) {
		    HRETURN_ERROR(H5E_IO, H5E_CANTOPENFILE, FAIL,
				  "can't create a new member");
		}
		/*
		 * For members in the middle, set their logical eof to the
		 * maximum possible value.
		 */
		if (i < membno) {
		    H5F_addr_reset(&max_addr);
		    H5F_addr_inc(&max_addr, member_size);
		    H5F_low_seteof(member, &max_addr);
		}
		if (lf->u.fam.nmemb >= lf->u.fam.nalloc) {
		    lf->u.fam.nalloc *= 2;
		    lf->u.fam.memb = H5MM_xrealloc(lf->u.fam.memb,
						   (lf->u.fam.nalloc *
						    sizeof(H5F_low_t *)));
		}
		lf->u.fam.memb[lf->u.fam.nmemb++] = member;
	    }
	}
	/*
	 * Make sure the logical eof is large enough to handle the request.
	 */
	max_addr = cur_addr;
	H5F_addr_inc(&max_addr, (hsize_t)nbytes);
	H5F_low_seteof(lf->u.fam.memb[membno], &max_addr);

	/* Write the data to the member */
	if (H5F_low_write(lf->u.fam.memb[membno],
			  access_parms->u.fam.memb_access,
			  &cur_addr, nbytes, buf) < 0) {
	    HRETURN_ERROR(H5E_IO, H5E_WRITEERROR, FAIL,
			  "can't write to family member");
	}
	buf += nbytes;
	size -= nbytes;
	membno++;
	offset = 0;
    }

    FUNC_LEAVE(SUCCEED);
}

/*-------------------------------------------------------------------------
 * Function:	H5F_fam_flush
 *
 * Purpose:	Flushes all data to disk and makes sure that the first member
 *		is as large as a member can be so we can accurately detect
 *		the member size if we open this file for read access at a
 *		later date.
 *
 * Return:	Success:	SUCCEED
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Monday, November 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5F_fam_flush(H5F_low_t *lf, const H5F_access_t *access_parms)
{
    int			    membno, nerrors = 0;
    uint8		    buf[1];
    haddr_t		    addr1, addr2, addr3;
    hsize_t		    max_offset;

    FUNC_ENTER(H5F_fam_flush, FAIL);

    /*
     * Make sure that the first family member is the maximum size because
     * H5F_fam_open() looks at the size of the first member to determine the
     * number of bits to use for each family member offset.  We do this by
     * reading the last possible byte from the member (which defaults to zero
     * if we're reading past the end of the member) and then writing it back.
     */
    max_offset = H5F_FAM_MASK(lf->u.fam.offset_bits);
    H5F_addr_reset(&addr1);
    H5F_addr_inc(&addr1, max_offset);
    H5F_low_size(lf->u.fam.memb[0], &addr2);	/*remember logical eof */
    addr3 = addr1;
    H5F_addr_inc(&addr3, (hsize_t)1);
    H5F_low_seteof(lf->u.fam.memb[0], &addr3);	/*prevent a warning */
    if (H5F_low_read(lf->u.fam.memb[0], access_parms->u.fam.memb_access,
		     &addr1, 1, buf) < 0) {
	HRETURN_ERROR(H5E_IO, H5E_READERROR, FAIL,
		      "can't read from first family member");
    }
    if (H5F_low_write(lf->u.fam.memb[0], access_parms->u.fam.memb_access,
		      &addr1, 1, buf) < 0) {
	HRETURN_ERROR(H5E_IO, H5E_WRITEERROR, FAIL,
		      "can't write to first family member");
    }
    H5F_low_seteof(lf->u.fam.memb[0], &addr2);	/*reset old eof */

    /*
     * Flush each member file.	Don't return an error status until we've
     * flushed as much as possible.
     */
    for (membno = 0; membno < lf->u.fam.nmemb; membno++) {
	if (H5F_low_flush(lf->u.fam.memb[membno],
			  access_parms->u.fam.memb_access) < 0) {
	    nerrors++;
	}
    }
    if (nerrors) {
	HRETURN_ERROR(H5E_IO, H5E_WRITEERROR, FAIL,
		      "can't flush family member");
    }
    FUNC_LEAVE(SUCCEED);
}

/*-------------------------------------------------------------------------
 * Function:	H5F_fam_access
 *
 * Purpose:	Determines if all members of the file family can be accessed
 *		and returns the key for the first member of the family.
 *
 * Return:	Success:	TRUE or FALSE
 *
 *		Failure:	FAIL
 *
 * Programmer:	Robb Matzke
 *		Monday, November 10, 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static hbool_t
H5F_fam_access(const char *name, const H5F_access_t *access_parms,
	       int mode, H5F_search_t *key/*out*/)
{
    intn			membno;
    char			member_name[4096];
    hbool_t			status;
    hbool_t			ret_value = FALSE;
    const H5F_low_class_t	*memb_type = NULL;

    FUNC_ENTER(H5F_fam_access, FAIL);

    /* Check args */
    assert (name && *name);
    assert (access_parms);
    assert (H5F_LOW_FAMILY==access_parms->driver);

    /* Get the driver for the family members */
    if (access_parms->u.fam.memb_access) {
	memb_type = H5F_low_class (access_parms->u.fam.memb_access->driver);
    } else {
	memb_type = H5F_low_class (H5F_LOW_DFLT);
    }

    /* Access the members */
    for (membno=0; /*void*/; membno++) {
	sprintf(member_name, name, membno);
	status = H5F_low_access(memb_type, member_name, NULL, mode,
				0 == membno ? key : NULL);

	if (!status) {
	    if (F_OK == mode) {
		/*
		 * If we didn't find a member then we must have gotten to the
		 * end of the family.  As long as we found the first
		 * member(s) the family exists.
		 */
		ret_value = membno > 0 ? TRUE : FALSE;
		break;
		
	    } else if (H5F_low_access(memb_type, member_name,
				      access_parms->u.fam.memb_access,
				      F_OK, NULL)) {
		/*
		 * The file exists but didn't have the write access permissions.
		 */
		ret_value = FALSE;
		break;
		
	    } else {
		/*
		 * The file doesn't exist because we got to the end of the
		 * family.
		 */
		ret_value = TRUE;
		break;
	    }
	}
	if (status < 0) {
	    HRETURN_ERROR(H5E_IO, H5E_CANTOPENFILE, FAIL,
			  "access method failed for a member file");
	}
    }

    FUNC_LEAVE(ret_value);
}
