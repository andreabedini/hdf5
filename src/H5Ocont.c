/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------
 *
 * Created:             H5Ocont.c
 *                      Aug  6 1997
 *                      Robb Matzke <matzke@llnl.gov>
 *
 * Purpose:             The object header continuation message.  This
 *                      message is only generated and read from within
 *                      the H5O package.  Therefore, do not change
 *                      any definitions in this file!
 *
 *-------------------------------------------------------------------------
 */

#define H5O_PACKAGE		/*suppress error about including H5Opkg	  */

#include "H5private.h"		/* Generic Functions			*/
#include "H5Eprivate.h"		/* Error handling		  	*/
#include "H5FLprivate.h"	/* Free Lists				*/
#include "H5MFprivate.h"	/* File memory management		*/
#include "H5Opkg.h"             /* Object headers			*/


/* PRIVATE PROTOTYPES */
static void *H5O_cont_decode(H5F_t *f, hid_t dxpl_id, const uint8_t *p);
static herr_t H5O_cont_encode(H5F_t *f, uint8_t *p, const void *_mesg);
static size_t H5O_cont_size(const H5F_t *f, const void *_mesg);
static herr_t H5O_cont_free(void *mesg);
static herr_t H5O_cont_delete(H5F_t *f, hid_t dxpl_id, const void *_mesg, hbool_t adj_link);
static void *H5O_cont_copy_file(H5F_t *file_src, void *mesg_src,
    H5F_t *file_dst, hid_t dxpl_id, H5SL_t *map_list, void *udata);
static herr_t H5O_cont_debug(H5F_t *f, hid_t dxpl_id, const void *_mesg, FILE * stream,
			     int indent, int fwidth);

/* This message derives from H5O */
const H5O_class_t H5O_CONT[1] = {{
    H5O_CONT_ID,            	/*message id number             */
    "hdr continuation",     	/*message name for debugging    */
    sizeof(H5O_cont_t),     	/*native message size           */
    H5O_cont_decode,        	/*decode message                */
    H5O_cont_encode,        	/*encode message                */
    NULL,                   	/*no copy method                */
    H5O_cont_size,          	/*size of header continuation   */
    NULL,                   	/*reset method			*/
    H5O_cont_free,	        /* free method			*/
    H5O_cont_delete,		/* file delete method		*/
    NULL,			/* link method			*/
    NULL, 		    	/*get share method		*/
    NULL,		    	/*set share method		*/
    H5O_cont_copy_file,  	/* copy native value to file    */
    NULL, 		 	/* post copy native value to file    */
    H5O_cont_debug         	/*debugging                     */
}};

/* Declare the free list for H5O_cont_t's */
H5FL_DEFINE(H5O_cont_t);


/*-------------------------------------------------------------------------
 * Function:    H5O_cont_decode
 *
 * Purpose:     Decode the raw header continuation message.
 *
 * Return:      Success:        Ptr to the new native message
 *
 *              Failure:        NULL
 *
 * Programmer:  Robb Matzke
 *              matzke@llnl.gov
 *              Aug  6 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void *
H5O_cont_decode(H5F_t *f, hid_t UNUSED dxpl_id, const uint8_t *p)
{
    H5O_cont_t             *cont = NULL;
    void                   *ret_value;

    FUNC_ENTER_NOAPI_NOINIT(H5O_cont_decode);

    /* check args */
    assert(f);
    assert(p);

    /* decode */
    if (NULL==(cont = H5FL_MALLOC(H5O_cont_t)))
	HGOTO_ERROR (H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed");
    H5F_addr_decode(f, &p, &(cont->addr));
    H5F_DECODE_LENGTH(f, p, cont->size);
    cont->chunkno=0;

    /* Set return value */
    ret_value=cont;

done:
    FUNC_LEAVE_NOAPI(ret_value);
}


/*-------------------------------------------------------------------------
 * Function:    H5O_cont_encode
 *
 * Purpose:     Encodes a continuation message.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Robb Matzke
 *              matzke@llnl.gov
 *              Aug  7 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_cont_encode(H5F_t *f, uint8_t *p, const void *_mesg)
{
    const H5O_cont_t       *cont = (const H5O_cont_t *) _mesg;

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5O_cont_encode);

    /* check args */
    assert(f);
    assert(p);
    assert(cont);

    /* encode */
    H5F_addr_encode(f, &p, cont->addr);
    H5F_ENCODE_LENGTH(f, p, cont->size);

    FUNC_LEAVE_NOAPI(SUCCEED);
}


/*-------------------------------------------------------------------------
 * Function:    H5O_cont_size
 *
 * Purpose:     Returns the size of the raw message in bytes not counting
 *              the message type or size fields, but only the data fields.
 *              This function doesn't take into account alignment.
 *
 * Return:      Success:        Message data size in bytes without alignment.
 *
 *              Failure:        zero
 *
 * Programmer:  Quincey Koziol
 *              koziol@ncsa.uiuc.edu
 *              Sep  6 2005
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static size_t
H5O_cont_size(const H5F_t *f, const void UNUSED *_mesg)
{
    size_t ret_value;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5O_cont_size)

    /* Set return value */
    ret_value = H5F_SIZEOF_ADDR(f) +    /* Continuation header address */
                H5F_SIZEOF_SIZE(f);     /* Continuation header length */

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_cont_size() */


/*-------------------------------------------------------------------------
 * Function:	H5O_cont_free
 *
 * Purpose:	Free's the message
 *
 * Return:	Non-negative on success/Negative on failure
 *
 * Programmer:	Quincey Koziol
 *              Monday, November 15, 2004
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_cont_free (void *mesg)
{
    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5O_cont_free);

    assert (mesg);

    H5FL_FREE(H5O_cont_t,mesg);

    FUNC_LEAVE_NOAPI(SUCCEED);
} /* end H5O_cont_free() */


/*-------------------------------------------------------------------------
 * Function:    H5O_cont_delete
 *
 * Purpose:     Free file space referenced by message
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Quincey Koziol
 *              Monday, October 10, 2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_cont_delete(H5F_t *f, hid_t dxpl_id, const void *_mesg, hbool_t UNUSED adj_link)
{
    const H5O_cont_t     *mesg = (const H5O_cont_t *) _mesg;
    herr_t ret_value = SUCCEED;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_cont_delete)

    /* check args */
    HDassert(f);
    HDassert(mesg);

    /* Release space for chunk */
    if(H5MF_xfree(f, H5FD_MEM_OHDR, dxpl_id, mesg->addr, (hsize_t)mesg->size) < 0)
        HGOTO_ERROR(H5E_OHDR, H5E_CANTFREE, FAIL, "unable to free object header chunk")

done:
    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_cont_delete() */


/*-------------------------------------------------------------------------
 * Function:    H5O_cont_copy_file
 *
 * Purpose:     Copies a continuation block message from _MESG to _DEST in file
 *
 * Return:      Success:        Ptr to _DEST
 *
 *              Failure:        NULL
 *
 * Programmer:  Peter Cao 
 *              September 22, 2005 
 *
 *-------------------------------------------------------------------------
 */
static void *
H5O_cont_copy_file(H5F_t UNUSED *file_src, void *mesg_src,
        H5F_t UNUSED *file_dst, hid_t UNUSED dxpl_id, H5SL_t UNUSED *map_list, void *udata)
{
    H5O_cont_t  *cont_src = (H5O_cont_t *) mesg_src;
    H5O_chunk_t *chunk = (H5O_chunk_t *)udata;
    H5O_cont_t  *cont_dst = NULL;
    void        *ret_value;          /* Return value */

    FUNC_ENTER_NOAPI_NOINIT(H5O_cont_copy_file)

    /* check args */
    HDassert(cont_src);
    HDassert(file_dst);

    /* Allocate space for the destination cont */
    if(NULL == (cont_dst = H5FL_MALLOC(H5O_cont_t)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed")

    HDmemcpy(cont_dst, cont_src, sizeof(H5O_cont_t));
    cont_dst->addr = chunk[cont_src->chunkno].addr;

    /* Set return value */
    ret_value = cont_dst;

done:
    if(!ret_value)
        if(cont_dst)
            H5FL_FREE(H5O_cont_t, cont_dst);

    FUNC_LEAVE_NOAPI(ret_value)
} /* end H5O_cont_copy_file() */


/*-------------------------------------------------------------------------
 * Function:    H5O_cont_debug
 *
 * Purpose:     Prints debugging info.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  Robb Matzke
 *              matzke@llnl.gov
 *              Aug  6 1997
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_cont_debug(H5F_t UNUSED *f, hid_t UNUSED dxpl_id, const void *_mesg, FILE * stream,
	       int indent, int fwidth)
{
    const H5O_cont_t       *cont = (const H5O_cont_t *) _mesg;

    FUNC_ENTER_NOAPI_NOINIT_NOFUNC(H5O_cont_debug);

    /* check args */
    assert(f);
    assert(cont);
    assert(stream);
    assert(indent >= 0);
    assert(fwidth >= 0);

    HDfprintf(stream, "%*s%-*s %a\n", indent, "", fwidth,
	      "Continuation address:", cont->addr);

    HDfprintf(stream, "%*s%-*s %lu\n", indent, "", fwidth,
	      "Continuation size in bytes:",
	      (unsigned long) (cont->size));
    HDfprintf(stream, "%*s%-*s %d\n", indent, "", fwidth,
	      "Points to chunk number:",
	      (int) (cont->chunkno));

    FUNC_LEAVE_NOAPI(SUCCEED);
}
