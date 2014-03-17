/* 
 * h5ff_client_index.c: Client side test for index routines.
 */

#include <hdf5.h>

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define NTUPLES 256
static int my_rank = 0, my_size = 1;

static void
write_dataset(hid_t file_id, const char *dataset_name,
        hsize_t total, hsize_t ncomponents, hid_t datatype_id,
        hsize_t ntuples, hsize_t start, void *buf)
{
    hid_t       dataset_id;
    hid_t       file_space_id, mem_space_id;
    hid_t       tid0, rid0, trspl_id;
    hsize_t     dims[2] = {total, ncomponents};
    hsize_t     offset[2] = {start, 0};
    hsize_t     count[2] = {ntuples, ncomponents};
    int         rank = (ncomponents == 1) ? 1 : 2;
    uint64_t    version;
    herr_t      ret;
    void       *dset_token1;
    size_t      token_size1;
    MPI_Request mpi_reqs[2];

    /* acquire container version 0 - EXACT. */
    version = 0;
    rid0 = H5RCacquire(file_id, &version, H5P_DEFAULT, H5_EVENT_STACK_NULL);
    assert(0 == version);

    /* create transaction object */
    tid0 = H5TRcreate(file_id, rid0, (uint64_t)1);
    assert(tid0);

    trspl_id = H5Pcreate(H5P_TR_START);
    ret = H5Pset_trspl_num_peers(trspl_id, (unsigned int) my_size);
    assert(0 == ret);
    ret = H5TRstart(tid0, trspl_id, H5_EVENT_STACK_NULL);
    assert(0 == ret);
    ret = H5Pclose(trspl_id);
    assert(0 == ret);

    /* Create the data space for the first dataset. */
    file_space_id = H5Screate_simple(rank, dims, NULL);
    assert(file_space_id);

    if(0 == my_rank) {
        /* Create a dataset. */
        dataset_id = H5Dcreate_ff(file_id, dataset_name, datatype_id, file_space_id,
                H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, tid0, H5_EVENT_STACK_NULL);
        assert(dataset_id);

#ifdef H5_HAVE_INDEXING
        /* Add indexing information */
        ret = H5Xcreate_ff(file_id, H5X_PLUGIN_DUMMY, dataset_id, H5P_DEFAULT,
                tid0, H5_EVENT_STACK_NULL);
        assert(0 == ret);
#endif

        /* get the token size of each dset */
        ret = H5Oget_token(dataset_id, NULL, &token_size1);
        assert(0 == ret);

        /* allocate buffers for each token */
        dset_token1 = malloc(token_size1);

        /* get the token buffer */
        ret = H5Oget_token(dataset_id, dset_token1, &token_size1);
        assert(0 == ret);

        /* bcast the token sizes and the tokens */ 
        MPI_Ibcast(&token_size1, sizeof(size_t), MPI_BYTE, 0, MPI_COMM_WORLD,
                &mpi_reqs[0]);
        MPI_Ibcast(dset_token1, (int) token_size1, MPI_BYTE, 0, MPI_COMM_WORLD,
                &mpi_reqs[1]);
        MPI_Waitall(2, mpi_reqs, MPI_STATUS_IGNORE);
    }
    else {
        /* recieve the token size */
        MPI_Ibcast(&token_size1, sizeof(size_t), MPI_BYTE, 0, MPI_COMM_WORLD,
                &mpi_reqs[0]);
        MPI_Waitall(1, mpi_reqs, MPI_STATUS_IGNORE);

        /* allocate buffer for token */
        dset_token1 = malloc(token_size1);

        /* recieve the token */
        MPI_Ibcast(dset_token1, (int) token_size1, MPI_BYTE, 0, MPI_COMM_WORLD,
                &mpi_reqs[0]);
        MPI_Waitall(1, mpi_reqs, MPI_STATUS_IGNORE);

        dataset_id = H5Oopen_by_token(dset_token1, tid0, H5_EVENT_STACK_NULL);
    }
    free(dset_token1);

    mem_space_id = H5Screate_simple(rank, count, NULL);
    assert(mem_space_id);

    /* write data to datasets */
    ret = H5Sselect_hyperslab(file_space_id, H5S_SELECT_SET, offset,
           NULL, count, NULL);
    assert(0 == ret);

    /* Write the first dataset. */
    ret = H5Dwrite_ff(dataset_id, datatype_id, mem_space_id, file_space_id,
            H5P_DEFAULT, buf, tid0, H5_EVENT_STACK_NULL);
    assert(0 == ret);

    /* Close the data space for the first dataset. */
    ret = H5Sclose(mem_space_id);
    assert(0 == ret);

    /* Close the first dataset. */
    ret = H5Dclose_ff(dataset_id, H5_EVENT_STACK_NULL);
    assert(0 == ret);

    ret = H5Sclose(file_space_id);
    assert(0 == ret);

    /* Finish transaction 0. */
    ret = H5TRfinish(tid0, H5P_DEFAULT, NULL, H5_EVENT_STACK_NULL);
    assert(0 == ret);

    /* release container version 0. */
    ret = H5RCrelease(rid0, H5_EVENT_STACK_NULL);
    assert(0 == ret);

    ret = H5RCclose(rid0);
    assert(0 == ret);

    ret = H5TRclose(tid0);
    assert(0 == ret);
}

static hid_t
query_and_view(hid_t file_id, const char *dataset_name)
{
    double lower_bound1 = 39.1, upper_bound1 = 42.1;
    int lower_bound2 = 295, upper_bound2 = 298;
    hid_t  query_id1, query_id2, query_id3, query_id4, query_id5, query_id6;
    hid_t query_id, view_id;
    hid_t dataset_id;
    hid_t rid1;
    uint64_t version;
    herr_t ret;

    /* Create a simple query */
    /* query = (39.1 < x < 42.1) || (295 < x < 298) */
    query_id1 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_GREATER_THAN,
            H5T_NATIVE_DOUBLE, &lower_bound1);
    assert(query_id1);

    query_id2 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_LESS_THAN,
            H5T_NATIVE_DOUBLE, &upper_bound1);
    assert(query_id2);

    query_id3 = H5Qcombine(query_id1, H5Q_COMBINE_AND, query_id2);
    assert(query_id3);

    query_id4 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_GREATER_THAN,
            H5T_NATIVE_INT, &lower_bound2);
    assert(query_id4);

    query_id5 = H5Qcreate(H5Q_TYPE_DATA_ELEM, H5Q_MATCH_LESS_THAN,
            H5T_NATIVE_INT, &upper_bound2);
    assert(query_id5);

    query_id6 = H5Qcombine(query_id4, H5Q_COMBINE_AND, query_id5);
    assert(query_id6);

    query_id = H5Qcombine(query_id3, H5Q_COMBINE_OR, query_id6);
    assert(query_id);

    /* acquire container version 1 - EXACT. */
    version = 1;
    rid1 = H5RCacquire(file_id, &version, H5P_DEFAULT, H5_EVENT_STACK_NULL);
    assert(rid1 > 0);
    assert(1 == version);

    MPI_Barrier(MPI_COMM_WORLD);

    dataset_id = H5Dopen_ff(file_id, dataset_name, H5P_DEFAULT, rid1,
            H5_EVENT_STACK_NULL);

    view_id = H5Vcreate_ff(dataset_id, query_id, H5P_DEFAULT, rid1,
            H5_EVENT_STACK_NULL);
    assert(view_id > 0);

    /* TODO do stuff here */

    H5Vclose(view_id);

    ret = H5Dclose_ff(dataset_id, H5_EVENT_STACK_NULL);
    assert(0 == ret);

    /* release container version 1. */
    ret = H5RCrelease(rid1, H5_EVENT_STACK_NULL);
    assert(0 == ret);

    ret = H5RCclose(rid1);
    assert(0 == ret);

    H5Qclose(query_id);
    H5Qclose(query_id6);
    H5Qclose(query_id5);
    H5Qclose(query_id4);
    H5Qclose(query_id3);
    H5Qclose(query_id2);
    H5Qclose(query_id1);

    return query_id;
}

int
main(int argc, char **argv)
{
    const char *file_name="eff_file_view.h5";
    const char *dataset_name="D1";
    hsize_t ntuples = NTUPLES;
    hsize_t ncomponents = 3;
    hsize_t start, total;
    int *data;
    hid_t file_id, fapl_id;
    herr_t ret;
    hsize_t i, j;

    int provided;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if(MPI_THREAD_MULTIPLE != provided) {
        fprintf(stderr, "MPI does not have MPI_THREAD_MULTIPLE support\n");
        exit(1);
    }

    /* Call EFF_init to initialize the EFF stack. */
    EFF_init(MPI_COMM_WORLD, MPI_INFO_NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &my_size);
    fprintf(stderr, "APP processes = %d, my rank is %d\n", my_size, my_rank);

    start = ntuples * (hsize_t) my_rank;
    total = ntuples * (hsize_t) my_size;

    /* Initialize the dataset. */
    data = (int *) malloc(sizeof(int) * ncomponents * ntuples);
    for (i = 0; i < ntuples; i++) {
       for (j = 0; j < ncomponents; j++) {
          data[ncomponents * i + j] = my_rank * ntuples + i;
       }
    }

    MPI_Barrier(MPI_COMM_WORLD);

    /* Choose the IOD VOL plugin to use with this file. */
    fapl_id = H5Pcreate(H5P_FILE_ACCESS);
    H5Pset_fapl_iod(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL);

    /* Open an existing file. */
    file_id = H5Fcreate_ff(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id,
            H5_EVENT_STACK_NULL);

    ret = H5Pclose(fapl_id);
    assert(0 == ret);

    write_dataset(file_id, dataset_name, total, ncomponents, H5T_NATIVE_INT,
            ntuples, start, data);

    MPI_Barrier(MPI_COMM_WORLD);

    query_and_view(file_id, dataset_name);

    MPI_Barrier(MPI_COMM_WORLD);

    /* Close the file. */
    ret = H5Fclose_ff(file_id, 1, H5_EVENT_STACK_NULL);
    assert(0 == ret);

    free(data);

    MPI_Barrier(MPI_COMM_WORLD);
    EFF_finalize();
    MPI_Finalize();

    return 0;
}
