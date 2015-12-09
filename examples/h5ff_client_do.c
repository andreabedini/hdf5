/* 
 * test_client_acg.c: Client side of ACG Dynamic Data Structure Support
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "mpi.h"
#include "hdf5.h"

int main(int argc, char **argv) {
    char file_name[]="acg_file.h5";
    hid_t file_id;
    hid_t       dsid = -1;      /* Dataset ID */
    hid_t       sid = -1;       /* Dataspace ID */
    hsize_t     dim, max_dim, chunk_dim; /* Dataset and chunk dimensions */
    hsize_t	curr_size;
    unsigned    u;              /* Local index variable */
    unsigned    write_elem[60], read_elem[60];  /* Element written/read */
    hid_t fapl_id, dxpl_id;
    int my_rank, my_size;
    int provided;
    hid_t e_stack;
    H5ES_status_t *status = NULL;
    int num_requests = 0, i;
    herr_t ret;
    H5_request_t req1;
    H5ES_status_t status;

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

    /* create the file. This is asynchronous, but the file_id can be used. */
    file_id = H5Fcreate_ff(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id, e_stack);
    assert(file_id);

    /* Create 1-D dataspace */
    dim = 0;
    max_dim = H5S_UNLIMITED;
    if((sid = H5Screate_simple(1, &dim, &max_dim)) < 0) {
        fprintf(stderr, "Failed\n");
        exit(1);
    }

    /* Create 1-D chunked dataset */
    if((dsid = H5Dcreate_ff(file_id, "dset", H5T_NATIVE_UINT, sid, 
                            H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, 0 , e_stack)) < 0) {
        fprintf(stderr, "Failed\n");
        exit(1);
    }

    /* Close dataspace */
    H5Sclose(sid);

    /* Initialize data elements */
    for(u = 0; u < 60; u++)        
        write_elem[u] = u;

    printf("App Dataset Id = %d  File id = %d\n", dsid, file_id);
    ret = H5DOappend_ff(dsid, H5P_DEFAULT, 1, 10, H5T_NATIVE_UINT, 
                        write_elem, 0 , e_stack);
    assert(ret<0);

    /* Append 60 elements to dataset, along proper axis */
    if(H5DOappend_ff(dsid, H5P_DEFAULT, 0, 60, H5T_NATIVE_UINT, 
                     write_elem, 0 , e_stack) < 0) {
        fprintf(stderr, "Failed\n");
        exit(1);
    }

    /* Get the dataset's dataspace now */
    if((sid = H5Dget_space(dsid)) < 0) {
        fprintf(stderr, "Failed\n");
        exit(1);
    }

    if(H5Sget_simple_extent_dims(sid, &curr_size, NULL) < 0) {
        fprintf(stderr, "Failed\n");
        exit(1);
    }

    /* Verify dataset is correct size */
    if(curr_size != 60) {
        fprintf(stderr, "Failed\n");
        exit(1);
    }

    /* Close dataspace */
    if(H5Sclose(sid) < 0) {
        fprintf(stderr, "Failed\n");
        exit(1);
    }

    /* Read elements back, with sequence operation */
    memset(read_elem, 0, sizeof(read_elem));

    /* Sequence 10 elements from dataset, along bad axis */
    ret = H5DOsequence_ff(dsid, H5P_DEFAULT, 1, 0, 60, H5T_NATIVE_UINT, 
                          read_elem, 0 , e_stack);

    /* Sequence first 60 elements from dataset, along proper axis */
    ret = H5DOsequence_ff(dsid, H5P_DEFAULT, 0, 0, 60, H5T_NATIVE_UINT, 
                          read_elem, 0 , e_stack);
    assert(ret == 0);

    /* close dataset */
    assert(H5Dclose(dsid) == 0);


    {
        hvl_t wdata[5];   /* Information to write */
        hvl_t rdata[5];   /* Information to write */
        hid_t tid;
        hsize_t dims[1];
        int increment, j, n;

        n = 0;
        increment = 4;
        /* Allocate and initialize VL data to write */
        for(i = 0; i < 5; i++) {
            int temp = i*increment + increment;

            wdata[i].p = malloc(temp * sizeof(unsigned int));
            wdata[i].len = temp;
            for(j = 0; j < temp; j++)
                ((unsigned int *)wdata[i].p)[j] = n ++;
        } /* end for */

        /* Create a datatype to refer to */
        tid = H5Tvlen_create (H5T_NATIVE_UINT);

        /* create a dataspace. This is a local Bookeeping operation that 
           does not touch the file */
        dims [0] = 5;
        sid = H5Screate_simple(1, dims, NULL);

        /* Create Dataset */
        if((dsid = H5Dcreate_ff(file_id, "dset_vl", tid, sid, 
                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, 0 , e_stack)) < 0) {
            fprintf(stderr, "Failed\n");
            exit(1);
        }

        ret = H5Dwrite(dsid, tid, sid, sid, H5P_DEFAULT, wdata);
        assert(ret == 0);

        ret = H5Dread(dsid, tid, sid, sid, H5P_DEFAULT, rdata);
        assert(ret == 0);

        /* Print VL DATA */
        for(i = 0; i < 5; i++) {
            int temp = i*increment + increment;

            fprintf(stderr, "Element %d  size %zu: ", i, rdata[i].len);
            for(j = 0; j < temp; j++)
                fprintf(stderr, "%d ",((unsigned int *)rdata[i].p)[j]);
            fprintf(stderr, "\n");
        } /* end for */

        H5Dvlen_reclaim(tid, sid, H5P_DEFAULT, rdata);
        H5Dvlen_reclaim(tid, sid, H5P_DEFAULT, wdata);

        H5Sclose(sid);
        H5Tclose(tid);

        /* close dataset */
        assert(H5Dclose_ff(dsid, e_stack) == 0);

    }

    {
        hid_t tid;
        hsize_t dims[1];
        const char *str_wdata[4]= {
            "Four score and seven years ago our forefathers brought forth on this continent a new nation,",
            "conceived in liberty and dedicated to the proposition that all men are created equal.",
            "Now we are engaged in a great civil war,",
            "testing whether that nation or any nation so conceived and so dedicated can long endure."
        };   /* Information to write */
        char *str_rdata[4];   /* Information read in */

        /* create a dataspace. This is a local Bookeeping operation that 
           does not touch the file */
        dims [0] = 4;
        sid = H5Screate_simple(1, dims, NULL);

        /* Create a datatype to refer to */
        tid = H5Tcopy(H5T_C_S1);
        H5Tset_size(tid,H5T_VARIABLE);

        /* Create Dataset */
        if((dsid = H5Dcreate_ff(file_id, "dset_vl_str", tid, sid, 
                                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, 0 , e_stack)) < 0) {
            fprintf(stderr, "Failed\n");
            exit(1);
        }

        ret = H5Dwrite(dsid, tid, sid, sid, H5P_DEFAULT, str_wdata);
        assert(ret == 0);

        ret = H5Dread(dsid, tid, sid, sid, H5P_DEFAULT, str_rdata);
        assert(ret == 0);

        fprintf(stderr, "Reading VL Strings: \n");
        for(i=0 ; i<4 ; i++) {
            fprintf(stderr, "%s\n", str_rdata[i]);
        }

        H5Dvlen_reclaim(tid, sid, H5P_DEFAULT, str_rdata);

        H5Sclose(sid);
        H5Tclose(tid);

        /* close dataset */
        assert(H5Dclose_ff(dsid, e_stack) == 0);
    }
    /* closing the container also acts as a wait all on all pending requests 
       on the container. */
    assert(H5Fclose_ff(file_id, e_stack) == 0);

    fprintf(stderr, "\n*****************************************************************************************************************\n");
    fprintf(stderr, "Wait on everything in EQ and check Results of operations in EQ\n");
    fprintf(stderr, "*****************************************************************************************************************\n");

    /* wait on all requests and print completion status */
    H5EQwait(e_stack, &num_requests, &status);
    fprintf(stderr, "%d requests in event queue. Completions: ", num_requests);
    for(i=0 ; i<num_requests; i++)
        fprintf(stderr, "%d ",status[i]);
    fprintf(stderr, "\n");
    free(status);

    /* Verify data read */
    for(u = 0; u < 60; u++) {
        if(read_elem[u] != write_elem[u]) {
            fprintf(stderr, "Failed\n");
            exit(1);
        }
    }

    fprintf(stderr, "\n*****************************************************************************************************************\n");
    fprintf(stderr, "Finalize EFF stack\n");
    fprintf(stderr, "*****************************************************************************************************************\n");

    H5ESclose(e_stack);
    H5Pclose(fapl_id);

    /* This finalizes the EFF stack. ships a terminate and IOD finalize to the server 
       and shutsdown the FS server (when all clients send the terminate request) 
       and client */
    EFF_finalize();

    MPI_Finalize();
    return 0;
}
