/* 
 * test_client_obj.c: Client side of H5O routines
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mpi.h"
#include "hdf5.h"

int main(int argc, char **argv) {
    const char file_name[]="eff_file_deltas.h5";
    hid_t file_id;
    hid_t gid;
    hid_t did, map;
    hid_t sid, dtid;
    hid_t tid1, tid2, rid1, rid2;
    hid_t fapl_id, dxpl_id;
    hid_t e_stack;
    hbool_t exists = -1;

    const unsigned int nelem=60;
    hsize_t dims[1];

    uint64_t version;
    uint64_t trans_num;

    int my_rank, my_size;
    int provided;
    MPI_Request mpi_req;

    H5ES_status_t status;
    size_t num_events = 0;
    herr_t ret;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if(MPI_THREAD_MULTIPLE != provided) {
        fprintf(stderr, "MPI does not have MPI_THREAD_MULTIPLE support\n");
        exit(1);
    }

    /* Call EFF_init to initialize the EFF stack.  
       As a result of this call, the Function Shipper client is started, 
       and HDF5 VOL calls are registered with the function shipper.
       An "IOD init" call is forwarded from the FS client to the FS server 
       which should already be running. */
    EFF_init(MPI_COMM_WORLD, MPI_INFO_NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &my_size);
    fprintf(stderr, "APP processes = %d, my rank is %d\n", my_size, my_rank);

    fprintf(stderr, "Create the FAPL to set the IOD VOL plugin and create the file\n");
    /* Choose the IOD VOL plugin to use with this file. 
       First we create a file access property list. Then we call a new routine to set
       the IOD plugin to use with this fapl */
    fapl_id = H5Pcreate (H5P_FILE_ACCESS);
    H5Pset_fapl_iod(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL);

    /* create an event Queue for managing asynchronous requests.

       Event Queues will releive the use from managing and completing
       individual requests for every operation. Instead of passing a
       request for every operation, the event queue is passed and
       internally the HDF5 library creates a request and adds it to
       the event queue.

       Multiple Event queue can be created used by the application. */
    e_stack = H5EScreate();
    assert(e_stack);

    /* create the file. */
    file_id = H5Fcreate_ff(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id, H5_EVENT_STACK_NULL);
    assert(file_id > 0);

    /* create 1-D dataspace with 60 elements */
    dims [0] = nelem;
    sid = H5Screate_simple(1, dims, NULL);
    dtid = H5Tcopy(H5T_STD_I32LE);

    /* acquire container version 0 - EXACT.  
       This can be asynchronous, but here we need the acquired ID 
       right after the call to start the transaction so we make synchronous. */
    version = 0;
    rid1 = H5RCacquire(file_id, &version, H5P_DEFAULT, H5_EVENT_STACK_NULL);
    assert(0 == version);

    /* start transaction 1 with default Leader/Delegate model. Leader
       which is rank 0 here starts the transaction. It can be
       asynchronous, but we make it synchronous here so that the
       Leader can tell its delegates that the transaction is
       started. */
    if(0 == my_rank) {
        hid_t rid_temp;

        /* create transaction object */
        tid1 = H5TRcreate(file_id, rid1, (uint64_t)1);
        assert(tid1);
        ret = H5TRstart(tid1, H5P_DEFAULT, e_stack);
        assert(0 == ret);

        /* create objects */
        gid = H5Gcreate_ff(file_id, "G1", H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(gid >= 0);

        did = H5Dcreate_ff(gid, "D1", dtid, sid, H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(did >= 0);

        ret = H5Tcommit_ff(file_id, "DT1", dtid, H5P_DEFAULT, H5P_DEFAULT, 
                           H5P_DEFAULT, tid1, e_stack);
        assert(ret == 0);

        map = H5Mcreate_ff(file_id, "MAP1", H5T_STD_I32LE, H5T_STD_I32LE, 
                           H5P_DEFAULT,H5P_DEFAULT,H5P_DEFAULT, tid1, e_stack);
        assert(map >= 0);

        ret = H5TRfinish(tid1, H5P_DEFAULT, &rid_temp, e_stack);
        assert(0 == ret);

        /* wait on all requests and print completion status */
        H5ESget_count(e_stack, &num_events);
        H5ESwait_all(e_stack, &status);
        H5ESclear(e_stack);
        printf("%d events in event stack. Completion status = %d\n", num_events, status);

        /* Close transaction object. Local op */
        ret = H5TRclose(tid1);
        assert(0 == ret);

        /* create transaction object */
        tid2 = H5TRcreate(file_id, rid_temp, (uint64_t)2);
        assert(tid2);
        ret = H5TRstart(tid2, H5P_DEFAULT, e_stack);
        assert(0 == ret);

        ret = H5Oset_comment_ff(gid, "Testing Object Comment", tid2, e_stack);
        assert(ret == 0);

        ret = H5TRfinish(tid2, H5P_DEFAULT, &rid2, e_stack);
        assert(0 == ret);

        assert(H5Gclose_ff(gid, e_stack) == 0);
        assert(H5Mclose_ff(map, e_stack) == 0);
        assert(H5Tclose_ff(dtid, e_stack) == 0);
        assert(H5Dclose_ff(did, e_stack) == 0);

        /* release container version 1. This is async. */
        ret = H5RCrelease(rid_temp, e_stack);
        assert(0 == ret);
        ret = H5RCclose(rid_temp);
        assert(0 == ret);

        version = 2;
    }

    /* release container version 0. This is async. */
    ret = H5RCrelease(rid1, e_stack);
    assert(0 == ret);

    /* wait on all requests and print completion status */
    H5ESget_count(e_stack, &num_events);
    H5ESwait_all(e_stack, &status);
    H5ESclear(e_stack);
    printf("%d events in event stack. Completion status = %d\n", num_events, status);

    if(0 == my_rank) {
        /* Close transaction object. Local op */
        ret = H5TRclose(tid2);
        assert(0 == ret);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /* Process 0 tells other procs that container version 2 is acquired */
    MPI_Bcast(&version, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    assert(2 == version);

    /* other processes just create a read context object; no need to
       acquire it */
    if(0 != my_rank) {
        rid2 = H5RCcreate(file_id, version);
        assert(rid2 > 0);
    }

    gid = H5Oopen_ff(file_id, "G1", H5P_DEFAULT, rid2);
    assert(gid);
    dtid = H5Oopen_ff(file_id, "DT1", H5P_DEFAULT, rid2);
    assert(dtid);
    did = H5Oopen_ff(gid,"D1", H5P_DEFAULT, rid2);
    assert(did);
    map = H5Oopen_ff(file_id,"MAP1", H5P_DEFAULT, rid2);
    assert(did);

    MPI_Barrier(MPI_COMM_WORLD);

    if(0 == my_rank) {
        ret = H5RCpersist(rid2, H5_EVENT_STACK_NULL);
        assert(ret == 0);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if(my_rank == 0) {
        /* release container version 2. This is async. */
        ret = H5RCrelease(rid2, e_stack);
        assert(0 == ret);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    if((my_size > 1 && 1 == my_rank) || 
       (my_size == 1 && 0 == my_rank)) {
        ret = H5Tevict_ff(dtid, 2, H5P_DEFAULT, H5_EVENT_STACK_NULL);
        assert(0 == ret);

        ret = H5Devict_ff(did, 2, H5P_DEFAULT, H5_EVENT_STACK_NULL);
        assert(0 == ret);

        ret = H5Mevict_ff(map, 2, H5P_DEFAULT, H5_EVENT_STACK_NULL);
        assert(0 == ret);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    assert(H5Oclose_ff(gid, e_stack) == 0);
    assert(H5Oclose_ff(did, e_stack) == 0);
    assert(H5Oclose_ff(dtid, e_stack) == 0);
    assert(H5Oclose_ff(map, e_stack) == 0);

    ret = H5RCclose(rid1);
    assert(0 == ret);
    ret = H5RCclose(rid2);
    assert(0 == ret);

    /* wait on all requests and print completion status */
    H5ESget_count(e_stack, &num_events);
    H5ESwait_all(e_stack, &status);
    H5ESclear(e_stack);
    printf("%d events in event stack. Completion status = %d\n", num_events, status);

    /* closing the container also acts as a wait all on all pending requests 
       on the container. */
    assert(H5Fclose_ff(file_id, 1, H5_EVENT_STACK_NULL) == 0);

    H5Sclose(sid);
    H5Pclose(fapl_id);
    H5ESclose(e_stack);

    /* This finalizes the EFF stack. ships a terminate and IOD finalize to the server 
       and shutsdown the FS server (when all clients send the terminate request) 
       and client */
    MPI_Barrier(MPI_COMM_WORLD);
    ret = EFF_finalize();
    assert(ret >= 0);

    MPI_Finalize();
    return 0;
}
