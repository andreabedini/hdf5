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

#include "H5VLiod_server.h"

#ifdef H5_HAVE_EFF

static herr_t
setup_eff_container(iod_handle_t coh, uint32_t cs_scope, unsigned num_peers, hbool_t acquire,
                    hid_t fcpl_id, iod_obj_id_t root_id, iod_obj_id_t mdkv_id, 
                    iod_obj_id_t attrkv_id, iod_obj_id_t oidkv_id, iod_handles_t *_root_oh);
/*
 * Programmer:  Mohamad Chaarawi <chaarawi@hdfgroup.gov>
 *              February, 2013
 *
 * Purpose:	The IOD plugin server side file routines.
 */


/*-------------------------------------------------------------------------
 * Function:	H5VL_iod_server_file_create_cb
 *
 * Purpose:	Creates a file as a iod HDF5 file.
 *
 * Return:	Success:	SUCCEED 
 *		Failure:	Negative
 *
 * Programmer:  Mohamad Chaarawi
 *              January, 2013
 *
 *-------------------------------------------------------------------------
 */
void
H5VL_iod_server_file_create_cb(AXE_engine_t UNUSED axe_engine, 
                               size_t UNUSED num_n_parents, AXE_task_t UNUSED n_parents[], 
                               size_t UNUSED num_s_parents, AXE_task_t UNUSED s_parents[], 
                               void *_op_data)
{
    op_data_t *op_data = (op_data_t *)_op_data;
    file_create_in_t *input = (file_create_in_t *)op_data->input;
    file_create_out_t output;
    unsigned num_peers = input->num_peers; /* the number of peers participating in creation */
    iod_obj_id_t root_id = input->root_id;
    iod_obj_id_t mdkv_id = input->mdkv_id;
    iod_obj_id_t attrkv_id = input->attrkv_id;
    iod_obj_id_t oidkv_id = input->oidkv_id;
    hid_t fcpl_id;
    unsigned int mode; /* create mode */
    iod_handle_t coh; /* container handle */
    iod_handles_t root_oh; /* root object handle */
    iod_handle_t mdkv_oh; /* metadata object handle for KV to store file's metadata */
    iod_ret_t ret, root_ret;
    iod_trans_id_t first_tid = 0;
    uint32_t cs_scope = 0;
    iod_hint_list_t *con_open_hint = NULL;
    iod_hint_list_t *obj_create_hint = NULL;
    hbool_t enable_checksum = FALSE;
    herr_t ret_value = SUCCEED;

#if H5_EFF_DEBUG
    fprintf(stderr, "Start file create %s ", input->name);
    fprintf(stderr, "with MDKV %"PRIx64" ", mdkv_id);
    fprintf(stderr, "with attrKV %"PRIx64"\n", attrkv_id);
    fprintf(stderr, "with OIDKV %"PRIx64"\n", oidkv_id);
#endif
    coh.cookie = IOD_OH_UNDEFINED;

    /* convert HDF5 flags to IOD flags */
    mode = (input->flags&H5F_ACC_RDWR) ? IOD_CONT_RW : IOD_CONT_R;
    if (input->flags&H5F_ACC_CREAT) 
        mode |= IOD_CONT_CREATE;

    if(H5P_DEFAULT == input->fcpl_id)
        input->fcpl_id = H5Pcopy(H5P_FILE_CREATE_DEFAULT);
    fcpl_id = input->fcpl_id;

    if(H5Pget_metadata_integrity_scope(input->fapl_id, &cs_scope) < 0)
        HGOTO_ERROR_FF(FAIL, "can't get scope for data integrity checks");

    if(H5Pget_ocpl_enable_checksum(fcpl_id, &enable_checksum) < 0)
        HGOTO_ERROR_FF(FAIL, "can't get scope for data integrity checks");

    /* scratch pad integrity in the container */
    if(cs_scope & H5_CHECKSUM_IOD) {
        con_open_hint = (iod_hint_list_t *)malloc(sizeof(iod_hint_list_t) + sizeof(iod_hint_t));
        con_open_hint->num_hint = 1;
        con_open_hint->hint[0].key = "iod_hint_co_scratch_cksum";
    }

    /* root group integrity */
    if((cs_scope & H5_CHECKSUM_IOD) && enable_checksum) {
        obj_create_hint = (iod_hint_list_t *)malloc(sizeof(iod_hint_list_t) + sizeof(iod_hint_t));
        obj_create_hint->num_hint = 1;
        obj_create_hint->hint[0].key = "iod_hint_obj_enable_cksum";
    }

    /* Create the Container */
    ret = iod_container_open(input->name, con_open_hint, mode, &coh, NULL);
    if(ret < 0) {
        coh.cookie = IOD_OH_UNDEFINED;
        HGOTO_ERROR_FF(ret, "can't create container");
    }

    /* MSC - skip transaction 0 since it can't be persisted */
    ret = iod_trans_start(coh, &first_tid, NULL, num_peers, IOD_TRANS_W, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't start transaction 0");

    /* Finish  the transaction */
    ret = iod_trans_finish(coh, first_tid, NULL, 0, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't finish transaction 0");

    first_tid = 1;

    /* Take transaction 1 to create root group */
    ret = iod_trans_start(coh, &first_tid, NULL, num_peers, IOD_TRANS_W, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't start transaction 1");

    /* create the root group */
    root_ret = iod_obj_create(coh, first_tid, obj_create_hint, IOD_OBJ_KV, 
                              NULL, NULL, &root_id, NULL);
    if(0 == root_ret || -EEXIST == root_ret) {
        /* root group has been created, open it */
        ret = iod_obj_open_write(coh, root_id, first_tid, NULL, &root_oh.wr_oh, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't open root group for write");
        ret = iod_obj_open_read(coh, root_id, first_tid, NULL, &root_oh.rd_oh, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't open root group for read");
    }
    else {
        HGOTO_ERROR_FF(root_ret, "can't create root group");
    }

    /* for the process that succeeded in creating the group, create
       the scratch pad for it too. */
    if(0 == root_ret) {
        scratch_pad sp;
        iod_kv_t kv;
        uint64_t value = 1;

        /* create the metadata KV object for the root group */
        ret = iod_obj_create(coh, first_tid, obj_create_hint, IOD_OBJ_KV, 
                             NULL, NULL, &mdkv_id, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't create metadata KV object");

        /* create the attribute KV object for the root group */
        ret = iod_obj_create(coh, first_tid, obj_create_hint, IOD_OBJ_KV, 
                             NULL, NULL, &attrkv_id, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't create attribute KV object");

        /* create the KV object to hold each client's indexes for
           object OIDs after each trans_finish and file_close */
        ret = iod_obj_create(coh, first_tid, obj_create_hint, IOD_OBJ_KV, 
                             NULL, NULL, &oidkv_id, NULL);
        if(ret != 0)
            HGOTO_ERROR_FF(ret, "can't create array for OID indexes");

        /* set values for the scratch pad object */
        sp[0] = mdkv_id;
        sp[1] = attrkv_id;
        sp[2] = oidkv_id;
        sp[3] = IOD_OBJ_INVALID;

        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t sp_cs;

            sp_cs = H5_checksum_crc64(&sp, sizeof(sp));

            /* set scratch pad in root group */
            ret = iod_obj_set_scratch(root_oh.wr_oh, first_tid, &sp, &sp_cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set scratch pad");
        }
        else {
            ret = iod_obj_set_scratch(root_oh.wr_oh, first_tid, &sp, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set scratch pad");
        }

        /* Store Metadata in scratch pad */
        ret = iod_obj_open_write(coh, input->mdkv_id, first_tid, NULL, &mdkv_oh, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't open metadata KV");

        /* insert plist metadata */
        ret = H5VL_iod_insert_plist(mdkv_oh, first_tid, fcpl_id, cs_scope, NULL, NULL);
        if(SUCCEED != ret)
            HGOTO_ERROR_FF(ret, "can't insert link count KV value");

        kv.value = &value;
        kv.value_len = sizeof(uint64_t);

        kv.key = (void *)H5VL_IOD_KEY_KV_IDS_INDEX;
        kv.key_len = 1 + strlen(H5VL_IOD_KEY_KV_IDS_INDEX);
        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t cs[2];

            cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
            cs[1] = H5_checksum_crc64(kv.value, kv.value_len);
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }
        else {
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }

        kv.key = (void *)H5VL_IOD_KEY_ARRAY_IDS_INDEX;
        kv.key_len = 1 + strlen(H5VL_IOD_KEY_ARRAY_IDS_INDEX);
        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t cs[2];

            cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
            cs[1] = H5_checksum_crc64(kv.value, kv.value_len);
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }
        else {
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }

        kv.key = (void *)H5VL_IOD_KEY_BLOB_IDS_INDEX;
        kv.key_len = 1 + strlen(H5VL_IOD_KEY_BLOB_IDS_INDEX);
        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t cs[2];

            cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
            cs[1] = H5_checksum_crc64(kv.value, kv.value_len);
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }
        else {
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }

        ret = iod_obj_close(mdkv_oh, NULL, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't close root object handle");
    }

    /* Finish  the transaction */
    ret = iod_trans_finish(coh, first_tid, NULL, 0, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't finish transaction 1");

    output.coh.cookie = coh.cookie;
    output.root_oh.rd_oh = root_oh.rd_oh;
    output.root_oh.wr_oh = root_oh.wr_oh;

#if H5_EFF_DEBUG
    fprintf(stderr, "Done with file create coh: %"PRIu64" root rd: %"PRIu64"  wr: %"PRIu64"\n",
            coh.cookie, root_oh.rd_oh.cookie, root_oh.wr_oh.cookie);
#endif

    HG_Handler_start_output(op_data->hg_handle, &output);

done:
    if(ret_value < 0) {
        if(coh.cookie != IOD_OH_UNDEFINED) {
            if((ret = iod_container_close(coh, NULL, NULL)) < 0)
                HDONE_ERROR_FF(ret, "can't close container");
        }
        output.coh.cookie = IOD_OH_UNDEFINED;
        output.root_oh.rd_oh.cookie = IOD_OH_UNDEFINED;
        output.root_oh.wr_oh.cookie = IOD_OH_UNDEFINED;
        HG_Handler_start_output(op_data->hg_handle, &ret_value);
    }

    HG_Handler_free_input(op_data->hg_handle, input);
    HG_Handler_free(op_data->hg_handle);
    input = (file_create_in_t *)H5MM_xfree(input);
    op_data = (op_data_t *)H5MM_xfree(op_data);

    if(con_open_hint) {
        free(con_open_hint);
        con_open_hint = NULL;
    }

    if(obj_create_hint) {
        free(obj_create_hint);
        obj_create_hint = NULL;
    }
} /* end H5VL_iod_server_file_create_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5VL_iod_server_file_open_cb
 *
 * Purpose:	Opens a file as a iod HDF5 file.
 *
 * Return:	Success:	SUCCEED 
 *		Failure:	Negative
 *
 * Programmer:  Mohamad Chaarawi
 *              January, 2013
 *
 *-------------------------------------------------------------------------
 */
void
H5VL_iod_server_file_open_cb(AXE_engine_t UNUSED axe_engine, 
                             size_t UNUSED num_n_parents, AXE_task_t UNUSED n_parents[], 
                             size_t UNUSED num_s_parents, AXE_task_t UNUSED s_parents[], 
                             void *_op_data)
{
    op_data_t *op_data = (op_data_t *)_op_data;
    file_open_in_t *input = (file_open_in_t *)op_data->input;
    file_open_out_t output;
    unsigned int mode = input->flags; /* File Open mode */
    hbool_t acquire = input->acquire;
    unsigned num_peers = input->num_peers; /* the number of peers participating in creation */
    iod_handle_t coh; /* container handle */
    iod_handles_t root_oh; /* root object handle */
    iod_handle_t mdkv_oh; /* metadata object handle for KV to store file's metadata */
    iod_handle_t oidkv_oh; /* object handle for KV to store file's OID indexes*/
    int num_entries;
    scratch_pad sp;
    iod_checksum_t sp_cs = 0;
    iod_cont_trans_stat_t *tids = NULL;
    iod_trans_id_t rtid;
    iod_size_t key_size = 0, val_size = 0;
    uint32_t cs_scope = 0;
    iod_ret_t ret;
    iod_hint_list_t *con_open_hint = NULL;
    herr_t ret_value = SUCCEED;

#if H5_EFF_DEBUG
    fprintf(stderr, "Start file open %s %d %d\n", input->name, input->flags, input->fapl_id);
#endif
    coh.cookie = IOD_OH_UNDEFINED;
    output.fcpl_id = FAIL;

    if(H5F_ACC_RDWR == mode)
        mode = IOD_CONT_RW;
    else if(H5F_ACC_RDONLY == mode)
        mode = IOD_CONT_R;
    else
        HGOTO_ERROR_FF(FAIL, "invalid mode");

    /* MSC - can't open file read only since IOD will fail when object
       are opened for write */
    if(mode == IOD_CONT_R)
        mode = IOD_CONT_RW;

    if(H5Pget_metadata_integrity_scope(input->fapl_id, &cs_scope) < 0)
        HGOTO_ERROR_FF(FAIL, "can't get scope for data integrity checks");

    /* scratch pad integrity in the container */
    if(cs_scope & H5_CHECKSUM_IOD) {
        con_open_hint = (iod_hint_list_t *)malloc(sizeof(iod_hint_list_t) + sizeof(iod_hint_t));
        con_open_hint->num_hint = 1;
        con_open_hint->hint[0].key = "iod_hint_co_scratch_cksum";
    }

    /* open the container */
    ret = iod_container_open(input->name, con_open_hint, mode, &coh, NULL);
    if(ret < 0) {
        coh.cookie = IOD_OH_UNDEFINED;
        HGOTO_ERROR_FF(ret, "can't open file");
    }
    ret = iod_query_cont_trans_stat(coh, &tids, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't get container tids status");

    rtid = tids->latest_rdable;

    ret = iod_free_cont_trans_stat(coh, tids);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't free container transaction status object");

    if(rtid == -1) {

        output.fcpl_id = H5Pcopy(H5P_FILE_CREATE_DEFAULT);
        output.kv_oid_index = 4;
        output.array_oid_index = 0;
        output.blob_oid_index = 0;

        output.root_id = 0;
        output.mdkv_id = 1*num_peers;
        output.attrkv_id = 2*num_peers;
        output.oidkv_id = 3*num_peers;

        IOD_OBJID_SETTYPE(output.root_id, IOD_OBJ_KV)
        IOD_OBJID_SETOWNER_APP(output.root_id)
        IOD_OBJID_SETTYPE(output.mdkv_id, IOD_OBJ_KV)
        IOD_OBJID_SETOWNER_APP(output.mdkv_id)
        IOD_OBJID_SETTYPE(output.attrkv_id, IOD_OBJ_KV)
        IOD_OBJID_SETOWNER_APP(output.attrkv_id)
        IOD_OBJID_SETTYPE(output.oidkv_id, IOD_OBJ_KV)
        IOD_OBJID_SETOWNER_APP(output.oidkv_id)

        output.coh.cookie = coh.cookie;

        fprintf(stderr, "Recreating Container HDF5 metadata ");
        fprintf(stderr, "with MDKV %"PRIx64" ", output.mdkv_id);
        fprintf(stderr, "with attrKV %"PRIx64" ", output.attrkv_id);
        fprintf(stderr, "with OIDKV %"PRIx64"\n", output.oidkv_id);

        if(setup_eff_container(coh, cs_scope, num_peers, acquire, output.fcpl_id,
                               output.root_id, output.mdkv_id, 
                               output.attrkv_id, output.oidkv_id, &root_oh) < 0)
            HGOTO_ERROR_FF(FAIL, "can't create container metadata");


        output.root_oh.rd_oh = root_oh.rd_oh;
        output.root_oh.wr_oh = root_oh.wr_oh;
        if(acquire == TRUE)
            output.c_version = 1;
        else
            output.c_version = IOD_TID_UNKNOWN;

        HG_Handler_start_output(op_data->hg_handle, &output);
        goto done;
    }

    ret = iod_trans_start(coh, &rtid, NULL, num_peers, IOD_TRANS_R, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't start transaction");

    /* open the root group */
    if ((ret = iod_obj_open_read(coh, ROOT_ID, rtid, NULL, &root_oh.rd_oh, NULL)) < 0)
        HGOTO_ERROR_FF(ret, "can't open root object for read");
    if ((ret = iod_obj_open_write(coh, ROOT_ID, rtid, NULL, &root_oh.wr_oh, NULL)) < 0)
        HGOTO_ERROR_FF(ret, "can't open root object for write");

    /* get scratch pad of root group */
    if((ret = iod_obj_get_scratch(root_oh.rd_oh, rtid, &sp, &sp_cs, NULL)) < 0)
        HGOTO_ERROR_FF(ret, "can't get scratch pad for root object");

    if(sp_cs && (cs_scope & H5_CHECKSUM_IOD)) {
        /* verify scratch pad integrity */
        if(H5VL_iod_verify_scratch_pad(&sp, sp_cs) < 0)
            HGOTO_ERROR_FF(FAIL, "Scratch Pad failed integrity check");
    }

    /* open the metadata KV object */
    ret = iod_obj_open_read(coh, sp[0], rtid, NULL, &mdkv_oh, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't open MD KV");

    /* retrieve all metadata from scratch pad */
    ret = H5VL_iod_get_metadata(mdkv_oh, rtid, H5VL_IOD_PLIST, H5VL_IOD_KEY_OBJ_CPL,
                                cs_scope, NULL, &output.fcpl_id);
    if(SUCCEED != ret)
        HGOTO_ERROR_FF(ret, "failed to retrieve fcpl");

    /* open the OID indexes KV object */
    ret = iod_obj_open_read(coh, sp[2], rtid, NULL, &oidkv_oh, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't open OID KV");

    ret = iod_kv_get_num(oidkv_oh, rtid, &num_entries, NULL);
    if(ret != 0)
        HGOTO_ERROR_FF(ret, "can't get number of KV entries");

    val_size = sizeof(uint64_t);

    /* This was not a clean shutdown, so gather all the oid indexes
       and compute the max */
    if(0 != num_entries) {
        iod_kv_params_t *kvs = NULL;
        iod_kv_t *kv = NULL;
        iod_checksum_t *oid_cs = NULL;
        iod_ret_t *oid_ret = NULL;
        int i;

        kvs = (iod_kv_params_t *)malloc(sizeof(iod_kv_params_t) * (size_t)num_entries);
        kv = (iod_kv_t *)malloc(sizeof(iod_kv_t) * (size_t)num_entries);
        oid_cs = (iod_checksum_t *)malloc(sizeof(iod_checksum_t) * (size_t)num_entries);
        oid_ret = (iod_ret_t *)malloc(sizeof(iod_ret_t) * (size_t)num_entries);

        for(i=0 ; i<num_entries ; i++) {
            kv[i].key = malloc(sizeof(uint32_t));
            kv[i].key_len = sizeof(uint32_t);
            kv[i].value = malloc(val_size * 3);
            kv[i].value_len = val_size * 3;
            kvs[i].kv = &kv[i];
            kvs[i].cs = &oid_cs[i];
            kvs[i].ret = &oid_ret[i];
        }

        ret = iod_kv_get_list(oidkv_oh, rtid, NULL, 0, &num_entries, kvs, NULL);
        if(ret != 0)
            HGOTO_ERROR_FF(ret, "can't get KV list from OID KV");

        output.kv_oid_index = 0;
        output.array_oid_index = 0;
        output.blob_oid_index = 0;

        for(i=0 ; i<num_entries ; i++) {
            uint64_t *oid_index = (uint64_t *)kv[i].value;

            if(output.kv_oid_index < oid_index[0])
                output.kv_oid_index = oid_index[0];
            if(output.array_oid_index < oid_index[1])
                output.array_oid_index = oid_index[1];
            if(output.blob_oid_index < oid_index[2])
                output.blob_oid_index = oid_index[2];
        }

        for(i=0 ; i<num_entries ; i++) {
            free(kv[i].key);
            free(kv[i].value);
        }

        fprintf(stderr, "OID KV index: %d\n", (int)output.kv_oid_index);
        fprintf(stderr, "OID ARRAY index: %d\n", (int)output.array_oid_index);
        fprintf(stderr, "OID BLOB index: %d\n", (int)output.blob_oid_index);

        free(kv);
        free(oid_cs);
        free(oid_ret);
        free(kvs);
    }
    /* This was a clean shutdown, so the maximum is already computed
       and stored in the metadata KV */
    else {
        iod_checksum_t *iod_cs = NULL;

        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_cs = (iod_checksum_t *)malloc(sizeof(iod_checksum_t) * 2);
        }

        key_size = 1 + strlen(H5VL_IOD_KEY_KV_IDS_INDEX);
        ret = iod_kv_get_value(mdkv_oh, rtid, H5VL_IOD_KEY_KV_IDS_INDEX, key_size,
                               &output.kv_oid_index, &val_size, iod_cs, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "KV index lookup failed");
        if(cs_scope & H5_CHECKSUM_IOD) {
            if(H5VL_iod_verify_kv_pair(H5VL_IOD_KEY_KV_IDS_INDEX, key_size, 
                                       &output.kv_oid_index, val_size, iod_cs) < 0)
                HGOTO_ERROR_FF(FAIL, "Corruption detected when reading metadata from IOD");
        }

        key_size = 1 + strlen(H5VL_IOD_KEY_ARRAY_IDS_INDEX);
        ret = iod_kv_get_value(mdkv_oh, rtid, H5VL_IOD_KEY_ARRAY_IDS_INDEX, key_size,
                               &output.array_oid_index, &val_size, iod_cs, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "Array index lookup failed");
        if(cs_scope & H5_CHECKSUM_IOD) {
            if(H5VL_iod_verify_kv_pair(H5VL_IOD_KEY_ARRAY_IDS_INDEX, key_size, 
                                       &output.array_oid_index, val_size, iod_cs) < 0)
                HGOTO_ERROR_FF(FAIL, "Corruption detected when reading metadata from IOD");
        }

        key_size = 1 + strlen(H5VL_IOD_KEY_BLOB_IDS_INDEX);
        ret = iod_kv_get_value(mdkv_oh, rtid, H5VL_IOD_KEY_BLOB_IDS_INDEX, key_size,
                               &output.blob_oid_index, &val_size, iod_cs, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "BLOB index lookup failed");
        if(cs_scope & H5_CHECKSUM_IOD) {
            if(H5VL_iod_verify_kv_pair(H5VL_IOD_KEY_BLOB_IDS_INDEX, key_size, 
                                       &output.blob_oid_index, val_size, iod_cs) < 0)
                HGOTO_ERROR_FF(FAIL, "Corruption detected when reading metadata from IOD");
        }

        if(iod_cs) {
            free(iod_cs);
            iod_cs = NULL;
        }
    }

    /* close the oid KV */
    ret = iod_obj_close(oidkv_oh, NULL, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't close OID KV");
    /* close the metadata KV */
    ret = iod_obj_close(mdkv_oh, NULL, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't close MD KV");

    output.coh.cookie = coh.cookie;
    output.root_id = ROOT_ID;
    output.mdkv_id = sp[0];
    output.attrkv_id = sp[1];
    output.oidkv_id = sp[2];
    output.root_oh.rd_oh = root_oh.rd_oh;
    output.root_oh.wr_oh = root_oh.wr_oh;
    output.c_version = rtid;

    /* If the user did not ask to acquire the latest readable version, finish it here */
    if(TRUE != acquire) {
        output.c_version = IOD_TID_UNKNOWN;
        ret = iod_trans_finish(coh, rtid, NULL, 0, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't finish transaction 0");
    }

#if H5_EFF_DEBUG
    fprintf(stderr, "Done with file open coh: %"PRIu64" root rd: %"PRIu64"  wr: %"PRIu64" CV: %"PRIu64"\n",
            coh.cookie, root_oh.rd_oh.cookie, root_oh.wr_oh.cookie, rtid);
#endif

    HG_Handler_start_output(op_data->hg_handle, &output);

done:
    if(FAIL != output.fcpl_id)
        H5Pclose(output.fcpl_id);

    if(ret_value < 0) {
        if(coh.cookie != IOD_OH_UNDEFINED) {
            if((ret = iod_container_close(coh, NULL, NULL)) < 0)
                HDONE_ERROR_FF(ret, "can't close container");
        }
        output.coh.cookie = IOD_OH_UNDEFINED;
        output.root_id = IOD_OBJ_INVALID;
        output.root_oh.rd_oh.cookie = IOD_OH_UNDEFINED;
        output.root_oh.wr_oh.cookie = IOD_OH_UNDEFINED;
        output.fcpl_id = FAIL;
        output.kv_oid_index = 0;
        output.array_oid_index = 0;
        output.blob_oid_index = 0;
        output.c_version = IOD_TID_UNKNOWN;
        HG_Handler_start_output(op_data->hg_handle, &output);
    }

    HG_Handler_free_input(op_data->hg_handle, input);
    HG_Handler_free(op_data->hg_handle);
    input = (file_open_in_t *)H5MM_xfree(input);
    op_data = (op_data_t *)H5MM_xfree(op_data);

    if(con_open_hint) {
        free(con_open_hint);
        con_open_hint = NULL;
    }

} /* end H5VL_iod_server_file_open_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5VL_iod_server_file_close_cb
 *
 * Purpose:	Closes iod HDF5 file.
 *
 * Return:	Success:	SUCCEED 
 *		Failure:	Negative
 *
 * Programmer:  Mohamad Chaarawi
 *              January, 2013
 *
 *-------------------------------------------------------------------------
 */
void
H5VL_iod_server_file_close_cb(AXE_engine_t UNUSED axe_engine, 
                              size_t UNUSED num_n_parents, AXE_task_t UNUSED n_parents[], 
                              size_t UNUSED num_s_parents, AXE_task_t UNUSED s_parents[], 
                              void *_op_data)
{
    op_data_t *op_data = (op_data_t *)_op_data;
    file_close_in_t *input = (file_close_in_t *)op_data->input;
    iod_handle_t coh = input->coh;
    iod_handles_t root_oh = input->root_oh;
    iod_ret_t ret;
    herr_t ret_value = SUCCEED;

#if H5_EFF_DEBUG
    fprintf(stderr, "Start file close\n");
#endif

    /* The root client request will create a transaction and store the
       final indexes for used up IDs */
    if(input->max_kv_index || input->max_array_index || input->max_blob_index) {
        iod_cont_trans_stat_t *tids = NULL;
        iod_trans_id_t trans_num, rtid;
        scratch_pad sp;
        iod_checksum_t sp_cs = 0;
        iod_kv_t kv;
        iod_handle_t mdkv_oh; /* metadata object handle for KV to store file's metadata */
        uint32_t cs_scope = input->cs_scope;

        ret = iod_query_cont_trans_stat(coh, &tids, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't get container tids status");

        trans_num = tids->latest_wrting + 1;
        rtid = tids->latest_rdable;

        ret = iod_free_cont_trans_stat(coh, tids);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't free container transaction status object");

#if H5_EFF_DEBUG
        fprintf(stderr, "File Close starting transaction %"PRIu64" rcxt %"PRIu64"\n", 
                trans_num, rtid);
#endif

        if((ret = iod_trans_start(coh, &rtid, NULL, 1, IOD_TRANS_R, NULL)) < 0)
            HGOTO_ERROR_FF(ret, "can't start READ transaction");

        if((ret = iod_trans_start(coh, &trans_num, NULL, 1, IOD_TRANS_W, NULL)) < 0)
            HGOTO_ERROR_FF(ret, "can't start WRITE transaction");

        /* get scratch pad of root group */
        if((ret = iod_obj_get_scratch(root_oh.rd_oh, rtid, &sp, &sp_cs, NULL)) < 0)
            HGOTO_ERROR_FF(ret, "can't get scratch pad for root object");

        if(sp_cs && (cs_scope & H5_CHECKSUM_IOD)) {
            /* verify scratch pad integrity */
            if(H5VL_iod_verify_scratch_pad(&sp, sp_cs) < 0)
                HGOTO_ERROR_FF(FAIL, "Scratch Pad failed integrity check");
        }

        /* open the metadata KV object */
        ret = iod_obj_open_write(coh, sp[0], trans_num, NULL, &mdkv_oh, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't open metadata KV");

        /* insert current indexes in the metadata KV object */
        kv.value = &input->max_kv_index;
        kv.value_len = sizeof(iod_obj_id_t);
        kv.key = (void *)H5VL_IOD_KEY_KV_IDS_INDEX;
        kv.key_len = 1 + strlen(H5VL_IOD_KEY_KV_IDS_INDEX);
        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t cs[2];

            cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
            cs[1] = H5_checksum_crc64(kv.value, kv.value_len);

            ret = iod_kv_set(mdkv_oh, trans_num, NULL, &kv, cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }
        else {
            ret = iod_kv_set(mdkv_oh, trans_num, NULL, &kv, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }


        kv.value = &input->max_array_index;
        kv.value_len = sizeof(iod_obj_id_t);
        kv.key = (void *)H5VL_IOD_KEY_ARRAY_IDS_INDEX;
        kv.key_len = 1 + strlen(H5VL_IOD_KEY_ARRAY_IDS_INDEX);
        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t cs[2];

            cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
            cs[1] = H5_checksum_crc64(kv.value, kv.value_len);

            ret = iod_kv_set(mdkv_oh, trans_num, NULL, &kv, cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }
        else {
            ret = iod_kv_set(mdkv_oh, trans_num, NULL, &kv, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }

        kv.value = &input->max_blob_index;
        kv.value_len = sizeof(iod_obj_id_t);
        kv.key = (void *)H5VL_IOD_KEY_BLOB_IDS_INDEX;
        kv.key_len = 1 + strlen(H5VL_IOD_KEY_BLOB_IDS_INDEX);
        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t cs[2];

            cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
            cs[1] = H5_checksum_crc64(kv.value, kv.value_len);

            ret = iod_kv_set(mdkv_oh, trans_num, NULL, &kv, cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }
        else {
            ret = iod_kv_set(mdkv_oh, trans_num, NULL, &kv, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }

        ret = iod_obj_close(mdkv_oh, NULL, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't close root object handle");

        /* open the OID kv object and remove all entries since this is
           a clean shutdown and the summary is stored in the metadata
           KV. */
        {
            iod_handles_t oidkv_oh;
            int num_entries = 0, i;
            iod_kv_params_t *kvs = NULL;
            iod_kv_t *oid_kv = NULL;
            iod_checksum_t *oid_cs = NULL;
            iod_ret_t *oid_ret = NULL;

            ret = iod_obj_open_write(coh, sp[2], trans_num, NULL, &oidkv_oh.wr_oh, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't open oid KV");
            ret = iod_obj_open_read(coh, sp[2], rtid, NULL, &oidkv_oh.rd_oh, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't open oid KV");

            ret = iod_kv_get_num(oidkv_oh.rd_oh, rtid, &num_entries, NULL);
            if(ret != 0)
                HGOTO_ERROR_FF(ret, "can't get number of KV entries");

#if H5_EFF_DEBUG
            fprintf(stderr, "NUM entries in OID index KV = %d\n", num_entries);
#endif

            if(num_entries) {
                kvs = (iod_kv_params_t *)malloc(sizeof(iod_kv_params_t) * (size_t)num_entries);
                oid_kv = (iod_kv_t *)malloc(sizeof(iod_kv_t) * (size_t)num_entries);
                oid_cs = (iod_checksum_t *)malloc(sizeof(iod_checksum_t) * (size_t)num_entries);
                oid_ret = (iod_ret_t *)malloc(sizeof(iod_ret_t) * (size_t)num_entries);

                for(i=0 ; i<num_entries ; i++) {
                    oid_kv[i].key = malloc(sizeof(uint32_t));
                    oid_kv[i].key_len = sizeof(uint32_t);
                    kvs[i].kv = &oid_kv[i];
                    kvs[i].cs = &oid_cs[i];
                    kvs[i].ret = &oid_ret[i];
                }

                ret = iod_kv_list_key(oidkv_oh.rd_oh, rtid, NULL, 0, &num_entries, kvs, NULL);
                if(ret != 0)
                    HGOTO_ERROR_FF(ret, "can't get list of keys");

                ret = iod_kv_unlink_keys(oidkv_oh.wr_oh, trans_num, NULL, num_entries, kvs, NULL);
                if(ret != 0)
                    HGOTO_ERROR_FF(ret, "can't unlink keys in OID index KV");

                for(i=0 ; i<num_entries ; i++)
                    free(oid_kv[i].key);
                free(oid_kv);
                free(oid_cs);
                free(oid_ret);
                free(kvs);
            }

            ret = iod_obj_close(oidkv_oh.rd_oh, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't close object handle");
            ret = iod_obj_close(oidkv_oh.wr_oh, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't close object handle");
        }

        /* finish the transaction */
        ret = iod_trans_finish(coh, rtid, NULL, 0, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't finish transaction");

        /* finish the transaction */
        ret = iod_trans_finish(coh, trans_num, NULL, 0, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't finish transaction");

        if(TRUE == input->persist_on_close) {
#if H5_EFF_DEBUG
            fprintf(stderr, "Persisting Last TID (%"PRIu64") before closing\n", trans_num);
#endif
            /* persist the last transaction */
            if((ret = iod_trans_persist(coh, trans_num, NULL, NULL)) < 0)
                HGOTO_ERROR_FF(ret, "can't persist before closing container");
        }
    }

#if H5_EFF_DEBUG
    fprintf(stderr, "Closing ROOT Group: R: %"PRIu64"  W: %"PRIu64"\n", 
            root_oh.rd_oh.cookie, root_oh.wr_oh.cookie);
#endif

    /* close the root group */
    ret = iod_obj_close(root_oh.rd_oh, NULL, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't close root object handle");
    ret = iod_obj_close(root_oh.wr_oh, NULL, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't close root object handle");

#if H5_EFF_DEBUG
    fprintf(stderr, "Closing Container: %"PRIu64"\n", coh.cookie);
#endif

    /* close the container */
    if((ret = iod_container_close(coh, NULL, NULL)) < 0)
        HGOTO_ERROR_FF(ret, "can't close container");

done:
#if H5_EFF_DEBUG
    fprintf(stderr, "Done with file close, sending response %d to client\n", ret_value);
#endif
    if(HG_SUCCESS != HG_Handler_start_output(op_data->hg_handle, &ret_value))
        HDONE_ERROR_FF(FAIL, "can't send result of file close to client");

    HG_Handler_free_input(op_data->hg_handle, input);
    HG_Handler_free(op_data->hg_handle);
    input = (file_close_in_t *)H5MM_xfree(input);
    op_data = (op_data_t *)H5MM_xfree(op_data);

} /* end H5VL_iod_server_file_close_cb() */

#if 1
static herr_t
setup_eff_container(iod_handle_t coh, uint32_t cs_scope, unsigned num_peers, hbool_t acquire,
                    hid_t fcpl_id, iod_obj_id_t root_id, iod_obj_id_t mdkv_id, 
                    iod_obj_id_t attrkv_id, iod_obj_id_t oidkv_id, iod_handles_t *_root_oh)
{
    iod_handles_t root_oh; /* root object handle */
    iod_handle_t mdkv_oh; /* metadata object handle for KV to store file's metadata */
    iod_ret_t ret, root_ret;
    iod_trans_id_t first_tid = 0;
    iod_hint_list_t *obj_create_hint = NULL;
    hbool_t enable_checksum = FALSE;
    herr_t ret_value = SUCCEED;

    if(H5Pget_ocpl_enable_checksum(fcpl_id, &enable_checksum) < 0)
        HGOTO_ERROR_FF(FAIL, "can't get scope for data integrity checks");

    /* root group integrity */
    if((cs_scope & H5_CHECKSUM_IOD) && enable_checksum) {
        obj_create_hint = (iod_hint_list_t *)malloc(sizeof(iod_hint_list_t) + sizeof(iod_hint_t));
        obj_create_hint->num_hint = 1;
        obj_create_hint->hint[0].key = "iod_hint_obj_enable_cksum";
    }

    /* MSC - skip transaction 0 since it can't be persisted */
    ret = iod_trans_start(coh, &first_tid, NULL, num_peers, IOD_TRANS_W, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't start transaction 0");

    /* Finish  the transaction */
    ret = iod_trans_finish(coh, first_tid, NULL, 0, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't finish transaction 0");

    first_tid = 1;

    /* Take transaction 1 to create root group */
    ret = iod_trans_start(coh, &first_tid, NULL, num_peers, IOD_TRANS_W, NULL);
    if(ret < 0)
        HGOTO_ERROR_FF(ret, "can't start transaction 1");

    /* create the root group */
    root_ret = iod_obj_create(coh, first_tid, obj_create_hint, IOD_OBJ_KV, 
                              NULL, NULL, &root_id, NULL);
    if(0 == root_ret || -EEXIST == root_ret) {
        /* root group has been created, open it */
        ret = iod_obj_open_write(coh, root_id, first_tid, NULL, &root_oh.wr_oh, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't open root group for write");
        ret = iod_obj_open_read(coh, root_id, first_tid, NULL, &root_oh.rd_oh, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't open root group for read");
    }
    else {
        HGOTO_ERROR_FF(root_ret, "can't create root group");
    }

    /* for the process that succeeded in creating the group, create
       the scratch pad for it too. */
    if(0 == root_ret) {
        scratch_pad sp;
        iod_kv_t kv;
        uint64_t value = 1;

        /* create the metadata KV object for the root group */
        ret = iod_obj_create(coh, first_tid, obj_create_hint, IOD_OBJ_KV, 
                             NULL, NULL, &mdkv_id, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't create metadata KV object");

        /* create the attribute KV object for the root group */
        ret = iod_obj_create(coh, first_tid, obj_create_hint, IOD_OBJ_KV, 
                             NULL, NULL, &attrkv_id, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't create attribute KV object");

        /* create the KV object to hold each client's indexes for
           object OIDs after each trans_finish and file_close */
        ret = iod_obj_create(coh, first_tid, obj_create_hint, IOD_OBJ_KV, 
                             NULL, NULL, &oidkv_id, NULL);
        if(ret != 0)
            HGOTO_ERROR_FF(ret, "can't create array for OID indexes");

        /* set values for the scratch pad object */
        sp[0] = mdkv_id;
        sp[1] = attrkv_id;
        sp[2] = oidkv_id;
        sp[3] = IOD_OBJ_INVALID;

        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t sp_cs;

            sp_cs = H5_checksum_crc64(&sp, sizeof(sp));

            /* set scratch pad in root group */
            ret = iod_obj_set_scratch(root_oh.wr_oh, first_tid, &sp, &sp_cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set scratch pad");
        }
        else {
            ret = iod_obj_set_scratch(root_oh.wr_oh, first_tid, &sp, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set scratch pad");
        }

        /* Store Metadata in scratch pad */
        ret = iod_obj_open_write(coh, mdkv_id, first_tid, NULL, &mdkv_oh, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't open metadata KV");

        /* insert plist metadata */
        ret = H5VL_iod_insert_plist(mdkv_oh, first_tid, fcpl_id, cs_scope, NULL, NULL);
        if(SUCCEED != ret)
            HGOTO_ERROR_FF(ret, "can't insert link count KV value");

        kv.value = &value;
        kv.value_len = sizeof(uint64_t);

        kv.key = (void *)H5VL_IOD_KEY_KV_IDS_INDEX;
        kv.key_len = 1 + strlen(H5VL_IOD_KEY_KV_IDS_INDEX);
        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t cs[2];

            cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
            cs[1] = H5_checksum_crc64(kv.value, kv.value_len);
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }
        else {
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }

        kv.key = (void *)H5VL_IOD_KEY_ARRAY_IDS_INDEX;
        kv.key_len = 1 + strlen(H5VL_IOD_KEY_ARRAY_IDS_INDEX);
        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t cs[2];

            cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
            cs[1] = H5_checksum_crc64(kv.value, kv.value_len);
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }
        else {
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }

        kv.key = (void *)H5VL_IOD_KEY_BLOB_IDS_INDEX;
        kv.key_len = 1 + strlen(H5VL_IOD_KEY_BLOB_IDS_INDEX);
        if(cs_scope & H5_CHECKSUM_IOD) {
            iod_checksum_t cs[2];

            cs[0] = H5_checksum_crc64(kv.key, kv.key_len);
            cs[1] = H5_checksum_crc64(kv.value, kv.value_len);
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, cs, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }
        else {
            ret = iod_kv_set(mdkv_oh, first_tid, NULL, &kv, NULL, NULL);
            if(ret < 0)
                HGOTO_ERROR_FF(ret, "can't set KV pair in parent");
        }

        ret = iod_obj_close(mdkv_oh, NULL, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't close root object handle");
    }

    _root_oh->rd_oh.cookie = root_oh.rd_oh.cookie;
    _root_oh->wr_oh.cookie = root_oh.wr_oh.cookie;

    /* If the user did not ask to acquire the latest readable version, finish it here */
    if(TRUE != acquire) {
        /* Finish  the transaction */
        ret = iod_trans_finish(coh, first_tid, NULL, 0, NULL);
        if(ret < 0)
            HGOTO_ERROR_FF(ret, "can't finish transaction 1");
    }

done:
    if(obj_create_hint) {
        free(obj_create_hint);
        obj_create_hint = NULL;
    }
    return ret_value;
}
#endif
#endif /* H5_HAVE_EFF */
