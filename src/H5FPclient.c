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

#define H5S_PACKAGE             /*suppress error about including H5Spkg */
#define H5G_PACKAGE             /*suppress error about including H5Gpkg */

#include "H5private.h"          /* Generic Functions                    */
#include "H5ACprivate.h"        /* Metadata Cache                       */
#include "H5Dprivate.h"         /* Dataset Functions                    */
#include "H5Eprivate.h"         /* Error Handling                       */
#include "H5Gpkg.h"             /* Group functions                      */
#include "H5Iprivate.h"         /* ID Functions                         */
#include "H5MMprivate.h"        /* Memory allocation                    */
#include "H5Oprivate.h"         /* Object Headers                       */
#include "H5Spkg.h"             /* Dataspace functions                  */
#include "H5TBprivate.h"        /* Threaded, Balanced, Binary Trees     */

#ifdef H5_HAVE_FPHDF5

#include "H5FPprivate.h"        /* Flexible Parallel Functions          */

/* Pablo mask */
#define PABLO_MASK      H5FPclient_mask

/* Is the interface initialized? */
static int interface_initialize_g = 0;
#define INTERFACE_INIT  NULL

/* local functions */
static unsigned H5FP_gen_request_id(void);

/** Public Library (non-API) Functions **/

/*
 * Function:    H5FP_request_open
 * Purpose:     Request an open of a file from the SAP. You pass in the
 *              metadata string (MDATA) (the filename), it's length in
 *              (MD_LEN), and the type of the object you're trying to
 *              open (OBJ_TYPE). The request ID is returned in a pointer
 *              supplied by the user.
 *
 *              The so-called "captain" process is in charge of telling
 *              the SAP that the processes opened a file. All processes
 *              opening the file, though, should call this function so
 *              that they can get the file ID that the SAP assigns to it.
 *
 * Return:      Success:    SUCCEED
 *              Failure:    FAIL
 * Programmer:  Bill Wendling, 28. August, 2002
 * Modifications:
 */
herr_t
H5FP_request_open(const char *mdata, int md_size, H5FP_obj_t obj_type,
                  unsigned *file_id, unsigned *req_id)
{
    H5FP_request req;
    MPI_Status mpi_status;
    int ret_value = SUCCEED, mrc;

    FUNC_ENTER_NOAPI(H5FP_request_open, FAIL);

    /* check args */
    assert(mdata);
    assert(file_id);
    assert(req_id);

    HDmemset(&mpi_status, 0, sizeof(MPI_Status));

    if (H5FP_my_rank == H5FP_capt_rank) {
        HDmemset(&req, 0, sizeof(req));
        req.req_type = H5FP_REQ_OPEN;
        req.req_id = H5FP_gen_request_id();
        req.proc_rank = H5FP_my_rank;
        req.md_size = md_size;
        req.obj_type = obj_type;

        if ((mrc = MPI_Send(&req, 1, H5FP_request_t, (int)H5FP_sap_rank,
                            H5FP_TAG_REQUEST, H5FP_SAP_COMM)) != MPI_SUCCESS)
            HMPI_GOTO_ERROR(FAIL, "MPI_Send failed", mrc);

        if (H5FP_send_metadata(mdata, md_size, (int)H5FP_sap_rank))
            HGOTO_ERROR(H5E_FPHDF5, H5E_CANTSENDMDATA, FAIL,
                        "can't send metadata to server");
    }

    if ((mrc = MPI_Recv(file_id, 1, MPI_UNSIGNED, (int)H5FP_sap_rank,
                        H5FP_TAG_FILE_ID, H5FP_SAP_COMM, &mpi_status)) != MPI_SUCCESS)
        HMPI_GOTO_ERROR(FAIL, "MPI_Send failed", mrc);

    printf("%u: New File ID == %u\n", H5FP_my_rank, *file_id);
    fflush(stdout);

done:
    *req_id = req.req_id;
    FUNC_LEAVE_NOAPI(ret_value);
}

/*
 * Function:    H5FP_request_lock
 * Purpose:     Request a lock on an object in a file from the SAP. The
 *              request ID is returned in a pointer supplied by the user.
 *              The status of the SAP is returned to the user in the
 *              supplied STATUS pointer.
 * Return:      Success:    SUCCEED
 *              Failure:    FAIL
 * Programmer:  Bill Wendling, 30. July, 2002
 * Modifications:
 */
herr_t
H5FP_request_lock(unsigned file_id, unsigned char *obj_oid,
                  H5FP_lock_t rw_lock, int last, unsigned *req_id,
                  H5FP_status_t *status)
{
    H5FP_request req;
    int ret_value = SUCCEED, mrc;

    FUNC_ENTER_NOAPI(H5FP_request_lock, FAIL);

    assert(obj_oid);
    assert(req_id);
    assert(status);
    HDmemset(&req, 0, sizeof(req));

    *status = H5FP_STATUS_OK;
    req.req_type = last ? H5FP_REQ_LOCK_END : H5FP_REQ_LOCK;
    req.req_id = H5FP_gen_request_id();
    req.file_id = file_id;
    req.rw_lock = rw_lock;
    req.md_size = 0;
    req.proc_rank = H5FP_my_rank;
    H5FP_COPY_OID(req.oid, obj_oid);

    if ((mrc = MPI_Send(&req, 1, H5FP_request_t, (int)H5FP_sap_rank,
                        H5FP_TAG_REQUEST, H5FP_SAP_COMM)) != MPI_SUCCESS)
        HMPI_GOTO_ERROR(FAIL, "MPI_Send failed", mrc);

    if (last) {
        /*
         * On the last lock in the lock-group to be acquired, we expect a
         * reply from the SAP
         */
        H5FP_reply sap_reply;
        MPI_Status mpi_status;

        HDmemset(&mpi_status, 0, sizeof(mpi_status));

        if ((mrc = MPI_Recv(&sap_reply, 1, H5FP_reply_t, (int)H5FP_sap_rank,
                            H5FP_TAG_REPLY, H5FP_SAP_COMM, &mpi_status)) != MPI_SUCCESS)
            HMPI_GOTO_ERROR(FAIL, "MPI_Recv failed", mrc);

        *status = sap_reply.status;

        if (sap_reply.status != H5FP_STATUS_LOCK_ACQUIRED)
            HGOTO_ERROR(H5E_RESOURCE, H5E_CANTLOCK, FAIL, "can't lock object on server");
    }

done:
    *req_id = req.req_id;
    FUNC_LEAVE_NOAPI(ret_value);
}

/*
 * Function:    H5FP_request_release_lock
 * Purpose:     Release a lock on the file from the SAP. Request a lock
 *              on an object in a file from the SAP. The request ID is
 *              returned in a pointer supplied by the user. The status
 *              of the SAP is returned to the user in the supplied STATUS
 *              pointer.
 * Return:      Success: The request ID.
 *              Failure: -1
 * Programmer:  Bill Wendling, 30. July, 2002
 * Modifications:
 */
herr_t
H5FP_request_release_lock(unsigned file_id, unsigned char *obj_oid,
                          int last, unsigned *req_id, H5FP_status_t *status)
{
    H5FP_request req;
    herr_t ret_value = SUCCEED;
    int mrc;

    FUNC_ENTER_NOAPI(H5FP_request_release_lock, FAIL);

    assert(obj_oid);
    assert(req_id);
    assert(status);

    HDmemset(&req, 0, sizeof(req));

    *status = H5FP_STATUS_OK;
    H5FP_COPY_OID(req.oid, obj_oid);
    req.req_type = last ? H5FP_REQ_RELEASE_END : H5FP_REQ_RELEASE;
    req.req_id = H5FP_gen_request_id();
    req.file_id = file_id;
    req.md_size = 0;
    req.proc_rank = H5FP_my_rank;

    if ((mrc = MPI_Send(&req, 1, H5FP_request_t, (int)H5FP_sap_rank,
                        H5FP_TAG_REQUEST, H5FP_SAP_COMM)) != MPI_SUCCESS)
        HMPI_GOTO_ERROR(FAIL, "MPI_Send failed", mrc);

    if (last) {
        /*
         * On the last lock released in this lock-group, we expect a
         * reply from the SAP
         */
        H5FP_reply sap_reply;
        MPI_Status mpi_status;

        HDmemset(&mpi_status, 0, sizeof(mpi_status));

        if ((mrc = MPI_Recv(&sap_reply, 1, H5FP_reply_t, (int)H5FP_sap_rank,
                            H5FP_TAG_REPLY, H5FP_SAP_COMM, &mpi_status)) != MPI_SUCCESS)
            HMPI_GOTO_ERROR(FAIL, "MPI_Recv failed", mrc);

        *status = sap_reply.status;

        if (sap_reply.status != H5FP_STATUS_LOCK_RELEASED)
            HGOTO_ERROR(H5E_RESOURCE, H5E_CANTUNLOCK, FAIL, "can't unlock object on server");
    }

done:
    *req_id = req.req_id;
    FUNC_LEAVE_NOAPI(ret_value);
}

/*
 * Function:    H5FP_request_write_metadata
 * Purpose:     Tell the SAP that we want to change a piece of metadata
 *              associated with the file. The request ID is returned in a
 *              pointer supplied by the user.
 * Return:      Success:    SUCCEED
 *              Failure:    FAIL
 * Programmer:  Bill Wendling, 02. August, 2002
 * Modifications:
 */
herr_t
H5FP_request_write_metadata(unsigned file_id, unsigned char *obj_oid,
                            H5FP_obj_t obj_type, H5AC_subid_t type_id,
                            haddr_t addr, int mdata_size, const char *mdata,
                            unsigned *req_id, H5FP_status_t *status)
{
    H5FP_reply sap_reply;
    MPI_Status mpi_status;
    H5FP_request req;
    herr_t ret_value = SUCCEED;
    int mrc;

    FUNC_ENTER_NOAPI(H5FP_request_change, FAIL);

    assert(mdata);
    assert(obj_oid);
    assert(req_id);
    assert(status);

    HDmemset(&req, 0, sizeof(req));

    H5FP_COPY_OID(req.oid, obj_oid);
    req.req_type = H5FP_REQ_WRITE;
    req.req_id = H5FP_gen_request_id();
    req.proc_rank = H5FP_my_rank;
    req.file_id = file_id;
    req.obj_type = obj_type;
    req.type_id = type_id;
    req.addr = addr;
    req.md_size = mdata_size;

    if ((mrc = MPI_Send(&req, 1, H5FP_request_t, (int)H5FP_sap_rank,
                        H5FP_TAG_REQUEST, H5FP_SAP_COMM)) != MPI_SUCCESS)
        HMPI_GOTO_ERROR(FAIL, "MPI_Send failed", mrc);

    /* The first MPI_Send will have been sent before this one will be read. */
    if (H5FP_send_metadata(mdata, mdata_size, (int)H5FP_sap_rank) != SUCCEED)
        HGOTO_ERROR(H5E_FPHDF5, H5E_CANTSENDMDATA, FAIL, "can't send metadata to server");

    HDmemset(&mpi_status, 0, sizeof(mpi_status));

    if ((mrc = MPI_Recv(&sap_reply, 1, H5FP_reply_t, (int)H5FP_sap_rank,
                        H5FP_TAG_REPLY, H5FP_SAP_COMM, &mpi_status)) != MPI_SUCCESS)
        HMPI_GOTO_ERROR(FAIL, "MPI_Recv failed", mrc);

    *status = sap_reply.status;

    if (sap_reply.status != H5FP_STATUS_OK)
        HGOTO_ERROR(H5E_RESOURCE, H5E_CANTCHANGE, FAIL, "can't write metadata to server");

done:
    *req_id = req.req_id;
    FUNC_LEAVE_NOAPI(ret_value);
}

/*
 * Function:    H5FP_request_read_metadata
 * Purpose:     Read a piece of metadata from the SAP. That is, if the
 *              SAP has access to that metadata. If not, then we'll need
 *              to read it from disk.
 * Return:      Success:    SUCCEED
 *              Failure:    FAIL
 * Programmer:  Bill Wendling, 02. August, 2002
 * Modifications:
 */
herr_t
H5FP_request_read_metadata(unsigned file_id, H5FP_obj_t obj_type,
                           H5AC_subid_t type_id, haddr_t addr, size_t size,
                           uint8_t **buf, unsigned *req_id,
                           H5FP_status_t *status)
{
    H5FP_request req;
    H5FP_read sap_read;     /* metadata info read from the SAP's cache */
    herr_t ret_value = SUCCEED;
    MPI_Status mpi_status;
    int mrc;

    FUNC_ENTER_NOAPI(H5FP_request_read_metadata, FAIL);

    assert(req_id);
    assert(status);

    HDmemset(&req, 0, sizeof(req));

    *status = H5FP_STATUS_OK;
    req.req_type = H5FP_REQ_READ;
    req.req_id = H5FP_gen_request_id();
    req.file_id = file_id;
    req.proc_rank = H5FP_my_rank;
    req.obj_type = obj_type;
    req.type_id = type_id;
    req.addr = addr;

    if ((mrc = MPI_Send(&req, 1, H5FP_request_t, (int)H5FP_sap_rank,
                        H5FP_TAG_REQUEST, H5FP_SAP_COMM)) != MPI_SUCCESS)
        HMPI_GOTO_ERROR(FAIL, "MPI_Send failed", mrc);

    HDmemset(&mpi_status, 0, sizeof(mpi_status));

    if ((mrc = MPI_Recv(&sap_read, 1, H5FP_read_t, (int)H5FP_sap_rank, H5FP_TAG_READ,
                        H5FP_SAP_COMM, &mpi_status)) != MPI_SUCCESS)
        HMPI_GOTO_ERROR(FAIL, "MPI_Recv failed", mrc);

    if (sap_read.status == H5FP_STATUS_OK) {
        /* use the info in the H5FP_read_t structure to update the metadata */
        memset(*buf, '\0', size);
        HDmemset(&mpi_status, 0, sizeof(mpi_status));

        if ((mrc = MPI_Recv(*buf, sap_read.md_size, MPI_BYTE, (int)H5FP_sap_rank,
                            H5FP_TAG_METADATA, H5FP_SAP_COMM,
                            &mpi_status)) != MPI_SUCCESS)
            HMPI_GOTO_ERROR(FAIL, "MPI_Recv failed", mrc);
    } else {
        /*
         * FIXME!!!
         *
         * The metadata wasn't in the SAP's cache. Should read from disk
         * now.
         */
    }

done:
    *req_id = req.req_id;
    *status = sap_read.status;
    FUNC_LEAVE_NOAPI(ret_value);
}

/*
 * Function:    H5FP_request_close
 * Purpose:     Tell the SAP that we want all of the structural changes
 *              made on the file and then close the file. The request ID
 *              is returned in a pointer passed to the function by the
 *              user.
 * Return:      Success:    SUCCEED
 *              Failure:    FAIL
 * Programmer:  Bill Wendling, 02. August, 2002
 * Modifications:
 */
herr_t
H5FP_request_close(unsigned file_id, unsigned *req_id)
{
    H5FP_request req;
    int ret_value = SUCCEED, mrc;

    FUNC_ENTER_NOAPI(H5FP_request_close, FAIL);

    assert(req_id);

    HDmemset(&req, 0, sizeof(req));

    req.req_type = H5FP_REQ_CLOSE;
    req.req_id = H5FP_gen_request_id();
    req.file_id = file_id;
    req.proc_rank = H5FP_my_rank;

    if ((mrc = MPI_Send(&req, 1, H5FP_request_t, (int)H5FP_sap_rank,
                        H5FP_TAG_REQUEST, H5FP_SAP_COMM)) != MPI_SUCCESS)
        HMPI_GOTO_ERROR(FAIL, "MPI_Send failed", mrc);

    *req_id = req.req_id;

done:
    FUNC_LEAVE_NOAPI(ret_value);
}

/*
 *===----------------------------------------------------------------------===
 *                    Functions Private to Client Module
 *===----------------------------------------------------------------------===
 */

/*
 * Function:    H5FP_gen_request_id
 * Purpose:     Generate a unique request ID to send along with a
 *              message.
 * Return:      Integer >= 0 - Doesn't fail.
 * Programmer:  Bill Wendling, 30. July, 2002
 * Modifications:
 */
static unsigned
H5FP_gen_request_id()
{
    static unsigned int i = 0;

    FUNC_ENTER_NOINIT(H5FP_gen_request_id);
    FUNC_LEAVE_NOAPI(i++);
}

#endif  /* H5_HAVE_FPHDF5 */
