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

/*
 * Programmer:  Mohamad Chaarawi <chaarawi@hdfgroup.gov>
 *              June, 2013
 *
 * Purpose:	The IOD plugin server side group routines.
 */


/*-------------------------------------------------------------------------
 * Function:	H5VL_iod_server_group_create_cb
 *
 * Purpose:	Creates a group as a iod object.
 *
 * Return:	Success:	SUCCEED 
 *		Failure:	Negative
 *
 * Programmer:  Mohamad Chaarawi
 *              February, 2013
 *
 *-------------------------------------------------------------------------
 */
void
H5VL_iod_server_group_create_cb(AXE_engine_t UNUSED axe_engine, 
                                size_t UNUSED num_n_parents, AXE_task_t UNUSED n_parents[], 
                                size_t UNUSED num_s_parents, AXE_task_t UNUSED s_parents[], 
                                void *_op_data)
{
    op_data_t *op_data = (op_data_t *)_op_data;
    group_create_in_t *input = (group_create_in_t *)op_data->input;
    group_create_out_t output;
    iod_handle_t coh = input->coh; /* the container handle */
    iod_handles_t loc_handle = input->loc_oh; /* The handle for current object - could be undefined */
    iod_obj_id_t loc_id = input->loc_id; /* The ID of the current location object */
    iod_obj_id_t grp_id = input->grp_id; /* The ID of the group that needs to be created */
    iod_obj_id_t mdkv_id = input->mdkv_id; /* The ID of the metadata KV to be created */
    iod_obj_id_t attrkv_id = input->attrkv_id; /* The ID of the attirbute KV to be created */
    const char *name = input->name; /* path relative to loc_id and loc_oh  */
    iod_trans_id_t wtid = input->trans_num;
    iod_trans_id_t rtid = input->rcxt_num;
    uint32_t cs_scope = input->cs_scope;
    iod_handles_t grp_oh, cur_oh;
    iod_handle_t mdkv_oh;
    iod_obj_id_t cur_id;
    char *last_comp = NULL; /* the name of the group obtained from traversal function */
    hid_t gcpl_id;
    scratch_pad sp;
    iod_ret_t ret;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT

#if H5VL_IOD_DEBUG
        fprintf(stderr, "Start group create %s at %"PRIu64"\n", name, loc_handle.wr_oh);
#endif

    /* the traversal will retrieve the location where the group needs
       to be created. The traversal will fail if an intermediate group
       does not exist. */
    if(H5VL_iod_server_traverse(coh, loc_id, loc_handle, name, wtid, rtid, FALSE, 
                                &last_comp, &cur_id, &cur_oh) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_NOSPACE, FAIL, "can't traverse path");

    fprintf(stderr, "Creating Group ID %"PRIx64" (CV %"PRIu64", TR %"PRIu64") ", 
            grp_id, rtid, wtid);
    fprintf(stderr, "at (OH %"PRIu64" ID %"PRIx64")\n", cur_oh.wr_oh, cur_id);

    /* create the group */
    if(iod_obj_create(coh, wtid, NULL, IOD_OBJ_KV, NULL, NULL, &grp_id, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't create Group");

    if((ret = iod_obj_open_read(coh, grp_id, wtid, NULL, &grp_oh.rd_oh, NULL)) < 0) {
        fprintf(stderr, "%d (%s).\n", ret, strerror(-ret));
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't open Group for read");
    }

    if((ret = iod_obj_open_write(coh, grp_id, wtid, NULL, &grp_oh.wr_oh, NULL)) < 0) {
        fprintf(stderr, "%d (%s).\n", ret, strerror(-ret));
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't open Group for write");
    }

    /* create the metadata KV object for the group */
    if(iod_obj_create(coh, wtid, NULL, IOD_OBJ_KV, NULL, NULL, &mdkv_id, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't create metadata KV object");

    /* create the attribute KV object for the group */
    if(iod_obj_create(coh, wtid, NULL, IOD_OBJ_KV, NULL, NULL, &attrkv_id, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't create metadata KV object");

    /* set values for the scratch pad object */
    sp[0] = mdkv_id;
    sp[1] = attrkv_id;
    sp[2] = IOD_OBJ_INVALID;
    sp[3] = IOD_OBJ_INVALID;

    /* set scratch pad in group */
    if(cs_scope & H5_CHECKSUM_IOD) {
        iod_checksum_t sp_cs;

        sp_cs = H5_checksum_crc64(&sp, sizeof(sp));
        if (iod_obj_set_scratch(grp_oh.wr_oh, wtid, &sp, &sp_cs, NULL) < 0)
            HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't set scratch pad");
    }
    else {
        if (iod_obj_set_scratch(grp_oh.wr_oh, wtid, &sp, NULL, NULL) < 0)
            HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't set scratch pad");
    }

    /* store metadata */
    /* Open Metadata KV object for write */
    if (iod_obj_open_write(coh, mdkv_id, wtid, NULL, &mdkv_oh, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't create scratch pad");

    if(H5P_DEFAULT == input->gcpl_id)
        input->gcpl_id = H5Pcopy(H5P_GROUP_CREATE_DEFAULT);
    gcpl_id = input->gcpl_id;

    /* insert plist metadata */
    if(H5VL_iod_insert_plist(mdkv_oh, wtid, gcpl_id, 
                             NULL, NULL, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't insert KV value");

    /* insert link count metadata */
    if(H5VL_iod_insert_link_count(mdkv_oh, wtid, (uint64_t)1, 
                                  NULL, NULL, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't insert KV value");

    /* insert object type metadata */
    if(H5VL_iod_insert_object_type(mdkv_oh, wtid, H5I_GROUP, 
                                   NULL, NULL, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't insert KV value");

    /* close Metadata KV object */
    if(iod_obj_close(mdkv_oh, NULL, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't close object");

    /* add link in parent group to current object */
    if(H5VL_iod_insert_new_link(cur_oh.wr_oh, wtid, last_comp, 
                                H5L_TYPE_HARD, &grp_id, NULL, NULL, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't insert KV value");

    /* close parent group if it is not the location we started the
       traversal into */
    if(loc_handle.rd_oh.cookie != cur_oh.rd_oh.cookie) {
        if(iod_obj_close(cur_oh.rd_oh, NULL, NULL) < 0)
            HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't close current object handle");
    }
    if(loc_handle.wr_oh.cookie != cur_oh.wr_oh.cookie) {
        if(iod_obj_close(cur_oh.wr_oh, NULL, NULL) < 0)
            HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't close current object handle");
    }

#if H5VL_IOD_DEBUG
    fprintf(stderr, "Done with group create, sending response to client\n");
#endif

    /* return the object handle for the group to the client */
    output.iod_oh.rd_oh.cookie = grp_oh.rd_oh.cookie;
    output.iod_oh.wr_oh.cookie = grp_oh.wr_oh.cookie;
    HG_Handler_start_output(op_data->hg_handle, &output);

    fprintf(stderr, "Created group RD_OH %"PRIu64" WR_OH %"PRIu64"\n", 
            grp_oh.rd_oh.cookie, grp_oh.wr_oh.cookie);

 done:
    /* return an UNDEFINED oh to the client if the operation failed */
    if(ret_value < 0) {
        fprintf(stderr, "Failed Group Create\n");
        output.iod_oh.rd_oh.cookie = IOD_OH_UNDEFINED;
        output.iod_oh.wr_oh.cookie = IOD_OH_UNDEFINED;
        HG_Handler_start_output(op_data->hg_handle, &output);
    }

    last_comp = (char *)H5MM_xfree(last_comp);
    input = (group_create_in_t *)H5MM_xfree(input);
    op_data = (op_data_t *)H5MM_xfree(op_data);

    FUNC_LEAVE_NOAPI_VOID
} /* end H5VL_iod_server_group_create_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5VL_iod_server_group_open_cb
 *
 * Purpose:	Opens a group as a iod object.
 *
 * Return:	Success:	SUCCEED 
 *		Failure:	Negative
 *
 * Programmer:  Mohamad Chaarawi
 *              February, 2013
 *
 *-------------------------------------------------------------------------
 */
void
H5VL_iod_server_group_open_cb(AXE_engine_t UNUSED axe_engine, 
                              size_t UNUSED num_n_parents, AXE_task_t UNUSED n_parents[], 
                              size_t UNUSED num_s_parents, AXE_task_t UNUSED s_parents[], 
                              void *_op_data)
{
    op_data_t *op_data = (op_data_t *)_op_data;
    group_open_in_t *input = (group_open_in_t *)op_data->input;
    group_open_out_t output;
    iod_handle_t coh = input->coh;
    iod_handles_t loc_handle = input->loc_oh; /* location handle to start lookup */
    iod_obj_id_t loc_id = input->loc_id; /* The ID of the current location object */
    const char *name = input->name; /* group name including path to open */
    iod_trans_id_t rtid = input->rcxt_num;
    uint32_t cs_scope = input->cs_scope;
    iod_obj_id_t grp_id; /* The ID of the group that needs to be opened */
    iod_handles_t grp_oh; /* The group handle */
    iod_handle_t mdkv_oh; /* The metadata KV handle */
    scratch_pad sp;
    iod_checksum_t sp_cs = 0;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT

#if H5VL_IOD_DEBUG
    fprintf(stderr, "Start group open %s at (OH %"PRIu64" ID %"PRIx64")\n", 
            name, loc_handle.rd_oh.cookie, loc_id);
#endif

    /* Traverse Path and open group */
    if(H5VL_iod_server_open_path(coh, loc_id, loc_handle, name, rtid, &grp_id, &grp_oh) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_NOSPACE, FAIL, "can't open object");

    /* open a write handle on the ID. */
    if(iod_obj_open_write(coh, grp_id, rtid, NULL, &grp_oh.wr_oh, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't open current group");

    /* get scratch pad of group */
    if(iod_obj_get_scratch(grp_oh.rd_oh, rtid, &sp, &sp_cs, NULL) < 0)
        HGOTO_ERROR2(H5E_FILE, H5E_CANTINIT, FAIL, "can't get scratch pad for object");

    if(sp_cs && (cs_scope & H5_CHECKSUM_IOD)) {
        /* verify scratch pad integrity */
        if(H5VL_iod_verify_scratch_pad(sp, sp_cs) < 0)
            HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "Scratch Pad failed integrity check");
    }

    /* open the metadata scratch pad */
    if (iod_obj_open_read(coh, sp[0], rtid, NULL, &mdkv_oh, NULL) < 0)
        HGOTO_ERROR2(H5E_FILE, H5E_CANTINIT, FAIL, "can't open scratch pad");

    if(H5VL_iod_get_metadata(mdkv_oh, rtid, H5VL_IOD_PLIST, 
                             H5VL_IOD_KEY_OBJ_CPL, NULL, NULL, &output.gcpl_id) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTGET, FAIL, "failed to retrieve gcpl");

    /* close the metadata scratch pad */
    if(iod_obj_close(mdkv_oh, NULL, NULL) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't close meta data KV handle");

    output.iod_id = grp_id;
    output.iod_oh.rd_oh.cookie = grp_oh.rd_oh.cookie;
    output.iod_oh.wr_oh.cookie = grp_oh.wr_oh.cookie;
    output.mdkv_id = sp[0];
    output.attrkv_id = sp[1];

#if H5VL_IOD_DEBUG
    fprintf(stderr, "Done with group open, sending response to client\n");
#endif

    HG_Handler_start_output(op_data->hg_handle, &output);

done:
    if(ret_value < 0) {
        output.iod_oh.rd_oh.cookie = IOD_OH_UNDEFINED;
        output.iod_oh.wr_oh.cookie = IOD_OH_UNDEFINED;
        output.iod_id = IOD_OBJ_INVALID;
        output.gcpl_id = H5P_GROUP_CREATE_DEFAULT;
        HG_Handler_start_output(op_data->hg_handle, &output);
    }

    input = (group_open_in_t *)H5MM_xfree(input);
    op_data = (op_data_t *)H5MM_xfree(op_data);

    FUNC_LEAVE_NOAPI_VOID
} /* end H5VL_iod_server_group_open_cb() */


/*-------------------------------------------------------------------------
 * Function:	H5VL_iod_server_group_close_cb
 *
 * Purpose:	Closes iod HDF5 group.
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
H5VL_iod_server_group_close_cb(AXE_engine_t UNUSED axe_engine, 
                               size_t UNUSED num_n_parents, AXE_task_t UNUSED n_parents[], 
                               size_t UNUSED num_s_parents, AXE_task_t UNUSED s_parents[], 
                               void *_op_data)
{
    op_data_t *op_data = (op_data_t *)_op_data;
    group_close_in_t *input = (group_close_in_t *)op_data->input;
    iod_handles_t iod_oh = input->iod_oh;
    herr_t ret_value = SUCCEED;

    FUNC_ENTER_NOAPI_NOINIT

#if H5VL_IOD_DEBUG
    fprintf(stderr, "Start group close\n");
#endif

    if(IOD_OH_UNDEFINED == iod_oh.wr_oh.cookie ||
       IOD_OH_UNDEFINED == iod_oh.rd_oh.cookie) {
        HGOTO_ERROR2(H5E_SYM, H5E_CANTCLOSEOBJ, FAIL, "can't close object with invalid handle");
    }

    if((iod_obj_close(iod_oh.rd_oh, NULL, NULL)) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't close object");
    if((iod_obj_close(iod_oh.wr_oh, NULL, NULL)) < 0)
        HGOTO_ERROR2(H5E_SYM, H5E_CANTINIT, FAIL, "can't close object");

done:
#if H5VL_IOD_DEBUG
    fprintf(stderr, "Done with group close, sending response to client\n");
#endif

    HG_Handler_start_output(op_data->hg_handle, &ret_value);

    input = (group_close_in_t *)H5MM_xfree(input);
    op_data = (op_data_t *)H5MM_xfree(op_data);

    FUNC_LEAVE_NOAPI_VOID
} /* end H5VL_iod_server_group_close_cb() */

#endif /* H5_HAVE_EFF */
