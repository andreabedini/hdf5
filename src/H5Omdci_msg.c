/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdfgroup.org/HDF5/doc/Copyright.html.  If you do not have          *
 * access to either file, you may request a copy from help@hdfgroup.org.     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*-------------------------------------------------------------------------
 *
 * Created:     H5Omdci_msg.c
 *              June 21, 2015
 *              John Mainzer
 *
 * Purpose:     A message indicating that a metadata cache image block
 *		of the indicated length exists at the specified offset
 *		in the HDF5 file.
 *
 * 		The mdci_msg only appears in the superblock extension.
 *
 * Modifications:
 *
 *              None.
 *
 *-------------------------------------------------------------------------
 */

#define H5O_PACKAGE             /* suppress error about including H5Opkg */

#include "H5private.h"          /* Generic Functions                     */
#include "H5Eprivate.h"         /* Error handling                        */
#include "H5Opkg.h"             /* Object headers                        */
#include "H5MMprivate.h"        /* Memory management                     */

static void * H5O_mdci_msg_decode(H5F_t *f, 
                                  hid_t H5_ATTR_UNUSED dxpl_id, 
                                  H5O_t H5_ATTR_UNUSED *open_oh,
                                  unsigned H5_ATTR_UNUSED mesg_flags, 
                                  unsigned H5_ATTR_UNUSED *ioflags, 
                                  const uint8_t *p);

static herr_t H5O_mdci_msg_encode(H5F_t *f, 
                                  hbool_t H5_ATTR_UNUSED disable_shared, 
                                  uint8_t *p, 
                                  const void *_mesg);

static void * H5O_mdci_msg_copy(const void *_mesg, void *_dest);

static size_t H5O_mdci_msg_size(const H5F_t *f, 
                                hbool_t H5_ATTR_UNUSED disable_shared, 
                                const void H5_ATTR_UNUSED *_mesg);

static herr_t H5O_mdci_msg_free(void *mesg);

static herr_t H5O_mdci_msg_debug(H5F_t H5_ATTR_UNUSED *f, 
                                 hid_t H5_ATTR_UNUSED dxpl_id, 
                                 const void *_mesg, FILE * stream,
                                 int indent, 
                                 int fwidth);

/* This message derives from H5O message class */
const H5O_msg_class_t H5O_MSG_MDCI[1] = {{
    H5O_MDCI_MSG_ID,            /* message id number              */
    "mdci",                     /* message name for debugging     */
    sizeof(H5O_mdci_msg_t),     /* native message size            */
    0,                          /* messages are sharable?         */
    H5O_mdci_msg_decode,        /* decode message                 */
    H5O_mdci_msg_encode,        /* encode message                 */
    H5O_mdci_msg_copy,          /* copy method                    */
    H5O_mdci_msg_size,          /* size of mdc image message      */
    NULL,                       /* reset method                   */
    H5O_mdci_msg_free,          /* free method                    */
    NULL,                       /* file delete method             */
    NULL,                       /* link method                    */
    NULL,                       /* set share method               */
    NULL,                       /* can share method               */
    NULL,                       /* pre copy native value to file  */
    NULL,                       /* copy native value to file      */
    NULL,                       /* post copy native value to file */
    NULL,                       /* get creation index             */
    NULL,                       /* set creation index             */
    H5O_mdci_msg_debug          /* debugging                      */
}};

/* Declare the free list for H5O_mdci_t's */
H5FL_DEFINE(H5O_mdci_msg_t);



/*-------------------------------------------------------------------------
 * Function:    H5O_mdci_msg_decode
 *
 * Purpose:     Decode a metadata cache image  message and return a
 * 		pointer to a newly allocated H5O_mdci_msg_t struct.
 *
 * Return:      Success:        Ptr to new message in native struct.
 *              Failure:        NULL
 *
 * Programmer:  John Mainzer
 *              6/22/15
 *
 *-------------------------------------------------------------------------
 */

static void *
H5O_mdci_msg_decode(H5F_t *f, 
                    hid_t H5_ATTR_UNUSED dxpl_id, 
                    H5O_t H5_ATTR_UNUSED *open_oh,
                    unsigned H5_ATTR_UNUSED mesg_flags, 
                    unsigned H5_ATTR_UNUSED *ioflags, 
                    const uint8_t *p)
{
    H5O_mdci_msg_t      *mesg;           /* Native message        */
    void                *ret_value;      /* Return value          */

    FUNC_ENTER_NOAPI_NOINIT

    /* Sanity check */
    HDassert(f);
    HDassert(p);

    /* Version of message */
    if ( *p++ != H5O_MDCI_VERSION_0 ) {

        HGOTO_ERROR(H5E_OHDR, H5E_CANTLOAD, NULL, \
		    "bad version number for message")
    }

    /* Allocate space for message */

    if ( NULL == (mesg = (H5O_mdci_msg_t *)H5FL_MALLOC(H5O_mdci_msg_t)) ) {

        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, \
	       "memory allocation failed for metadata cache image message.");

    }

    /* Decode */
    H5F_addr_decode(f, &p, &(mesg->addr));
    H5F_DECODE_LENGTH(f, p, mesg->size);

    /* Set return value */
    ret_value = (void *)mesg;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* end H5O_mdci_msg_decode() */


/*-------------------------------------------------------------------------
 * Function:    H5O_mdci_msg_encode
 *
 * Purpose:     Encode metadata cache image message
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *              6/22/15
 *
 *-------------------------------------------------------------------------
 */

static herr_t
H5O_mdci_msg_encode(H5F_t *f, 
                    hbool_t H5_ATTR_UNUSED disable_shared, 
                    uint8_t *p, 
                    const void *_mesg)
{
    const H5O_mdci_msg_t *mesg = (const H5O_mdci_msg_t *)_mesg;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* Sanity check */
    HDassert(f);
    HDassert(p);
    HDassert(mesg);

    /* encode */
    *p++ = H5O_MDCI_VERSION_0;
    H5F_addr_encode(f, &p, mesg->addr);
    H5F_ENCODE_LENGTH(f, p, mesg->size);

    FUNC_LEAVE_NOAPI(SUCCEED)

} /* H5O_mdci_msg_encode() */


/*-------------------------------------------------------------------------
 * Function:    H5O_mdci_msg_copy
 *
 * Purpose:     Copies a message from _MESG to _DEST, allocating _DEST if
 *              necessary.
 *
 * Return:      Success:        Ptr to _DEST
 *              Failure:        NULL
 *
 * Programmer:  John Mainzer
 *              6/22/15
 *
 *-------------------------------------------------------------------------
 */
static void *
H5O_mdci_msg_copy(const void *_mesg, void *_dest)
{
    const H5O_mdci_msg_t   *mesg = (const H5O_mdci_msg_t *)_mesg;
    H5O_mdci_msg_t         *dest = (H5O_mdci_msg_t *) _dest;
    void                   *ret_value;     /* Return value */

    FUNC_ENTER_NOAPI_NOINIT

    /* check args */
    HDassert(mesg);
    if(!dest && NULL == (dest = H5FL_CALLOC(H5O_mdci_msg_t)))
        HGOTO_ERROR(H5E_RESOURCE, H5E_NOSPACE, NULL, "memory allocation failed")

    /* copy */
    *dest = *mesg;

    /* Set return value */
    ret_value = dest;

done:

    FUNC_LEAVE_NOAPI(ret_value)

} /* end H5O_mdci_msg__copy() */


/*-------------------------------------------------------------------------
 * Function:    H5O_mdci_msg_size
 *
 * Purpose:     Returns the size of the raw message in bytes not counting
 *              the message type or size fields, but only the data fields.
 *              This function doesn't take into account alignment.
 *
 * Return:      Success:        Message data size in bytes without alignment.
 *
 *              Failure:        zero
 *
 * Programmer:  John Mainzer
 *              6/22/15
 *
 *-------------------------------------------------------------------------
 */
static size_t
H5O_mdci_msg_size(const H5F_t *f, 
                  hbool_t H5_ATTR_UNUSED disable_shared, 
                  const void H5_ATTR_UNUSED *_mesg)
{
    size_t ret_value;   /* Return value */

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* Set return value */
    ret_value = (size_t)( 1 +                   /* Version number           */ 
                          H5F_SIZEOF_ADDR(f) +  /* addr of metadata cache   */
                                                /* image block              */
                          H5F_SIZEOF_SIZE(f) ); /* length of metadata cache */
                                                /* image block              */

    FUNC_LEAVE_NOAPI(ret_value)

} /* H5O_mdci_msg_size() */


/*-------------------------------------------------------------------------
 * Function:    H5O_mdci_msg_free
 *
 * Purpose:     Free the message
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *              6/22/15
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_mdci_msg_free(void *mesg)
{
    FUNC_ENTER_NOAPI_NOINIT_NOERR

    HDassert(mesg);

    mesg = H5FL_FREE(H5O_mdci_msg_t, mesg);

    FUNC_LEAVE_NOAPI(SUCCEED)

} /* H5O_mdci_msg_free() */


/*-------------------------------------------------------------------------
 * Function:    H5O_mdci_msg_debug
 *
 * Purpose:     Prints debugging info.
 *
 * Return:      Non-negative on success/Negative on failure
 *
 * Programmer:  John Mainzer
 *              6/22/15
 *
 *-------------------------------------------------------------------------
 */
static herr_t
H5O_mdci_msg_debug(H5F_t H5_ATTR_UNUSED *f, 
                   hid_t H5_ATTR_UNUSED dxpl_id, 
                   const void *_mesg, FILE * stream,
                   int indent, 
                   int fwidth)
{
    const H5O_mdci_msg_t   *cont = (const H5O_mdci_msg_t *) _mesg;

    FUNC_ENTER_NOAPI_NOINIT_NOERR

    /* check args */
    HDassert(f);
    HDassert(cont);
    HDassert(stream);
    HDassert(indent >= 0);
    HDassert(fwidth >= 0);

    HDfprintf(stream, "%*s%-*s %a\n", indent, "", fwidth,
              "Metadata Cache Image Block address:", cont->addr);

    HDfprintf(stream, "%*s%-*s %lu\n", indent, "", fwidth,
              "Metadata Cache Image Block size in bytes:",
              (unsigned long) (cont->size));

    FUNC_LEAVE_NOAPI(SUCCEED)

} /* end H5O_mdci_msg_debug() */

