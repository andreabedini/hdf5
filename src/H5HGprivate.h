/*
 * Copyright (C) 1998 NCSA
 *                    All rights reserved.
 *
 * Programmer:  Robb Matzke <matzke@llnl.gov>
 *              Friday, March 27, 1998
 */
#ifndef _H5HGprivate_H
#define _H5HGprivate_H

#include <H5HGpublic.h>

/*
 * Each collection has a magic number for some redundancy.
 */
#define H5HG_MAGIC	"GCOL"
#define H5HG_SIZEOF_MAGIC 4

/*
 * Global heap collection version.
 */
#define H5HG_VERSION	1

/*
 * All global heap collections are at least this big.  This allows us to read
 * most collections with a single read() since we don't have to read a few
 * bytes of header to figure out the size.  If the heap is larger than this
 * then a second read gets the rest after we've decoded the header.
 */
#define H5HG_MINSIZE	4096

/*
 * Maximum length of the CWFS list, the list of remembered collections that
 * have free space.
 */
#define H5HG_NCWFS	16

/*
 * The maximum number of links allowed to a global heap object.
 */
#define H5HG_MAXLINK	65535

/*
 * The size of the collection header.
 */
#define H5HG_SIZEOF_HDR(f) (4 +			/*magic number		*/    \
			    1 +			/*version number	*/    \
			    3 +			/*reserved		*/    \
			    H5F_SIZEOF_SIZE(f)) /*collection size	*/

/*
 * The overhead associated with each object in the heap.
 */
#define H5HG_SIZEOF_OBJHDR(f) (2 +		/*reference count	*/    \
			       2 +		/*reserved		*/    \
			       H5F_SIZEOF_SIZE(f)) /*object data size	*/

/*
 * The initial guess for the number of messages in a collection.  We assume
 * that all objects in that collection are zero length, giving the maximum
 * possible number of objects in the collection.  The collection itself has
 * some overhead and each message has some overhead.
 */
#define H5HG_NOBJS(f,z) (int)((((z)-H5HG_SIZEOF_HDR(f))/		      \
			       H5HG_SIZEOF_OBJHDR(f)+1))

typedef struct H5HG_t H5HG_t;

herr_t H5HG_create (H5F_t *f, size_t size);
herr_t H5HG_insert (H5F_t *f, size_t size, void *obj, H5HG_t *hobj/*out*/);
void *H5HG_peek (H5F_t *f, H5HG_t *hobj);
void *H5HG_read (H5F_t *f, H5HG_t *hobj, void *object);
intn H5HG_link (H5F_t *f, H5HG_t *hobj, intn adjust);
herr_t H5HG_remove (H5F_t *f, H5HG_t *hobj);
herr_t H5HG_debug(H5F_t *f, const haddr_t *addr, FILE *stream, intn indent,
		  intn fwidth);

#endif
