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

/*
 * Programmer:	Raymond Lu
 *              October 14, 2001	
 *
 * Purpose:	Tests the H5Tget_native_type function.
 */

#include "h5test.h"

const char *FILENAME[] = {
    "errors",
    NULL
};

#define DIM0    100
#define DIM1    200

int	ipoints2[DIM0][DIM1], icheck2[DIM0][DIM1];

hid_t   ERR_CLS;
hid_t   ERR_STACK;

hid_t   ERR_MAJ_TEST;
hid_t   ERR_MAJ_IO;
hid_t   ERR_MAJ_API;

hid_t   ERR_MIN_SUBROUTINE;
hid_t   ERR_MIN_ERRSTACK;
hid_t   ERR_MIN_CREATE;
hid_t   ERR_MIN_WRITE;
hid_t   ERR_MIN_GETNUM;

#define DSET_NAME               "a_dataset"
#define FAKE_ID                 0

#define ERR_CLS_NAME            "Error Test"
#define PROG_NAME               "Error Program"
#define PROG_VERS               "1.0"

#define ERR_MAJ_TEST_MSG             "Error in test"
#define ERR_MAJ_IO_MSG               "Error in IO"
#define ERR_MAJ_API_MSG              "Error in API"
#define ERR_MIN_SUBROUTINE_MSG       "Error in subroutine"
#define ERR_MIN_ERRSTACK_MSG         "Error in error stack"
#define ERR_MIN_CREATE_MSG           "Error in H5Dcreate"
#define ERR_MIN_WRITE_MSG            "Error in H5Dwrite"
#define ERR_MIN_GETNUM_MSG           "Error in H5Eget_num"

#define MSG_SIZE                64
#define SPACE1_DIM1             4
#define SPACE1_RANK             1
#define SPACE2_RANK	        2
#define SPACE2_DIM1	        10
#define SPACE2_DIM2	        10

herr_t custom_print_cb(int n, H5E_error_t *err_desc, void* client_data);


/*-------------------------------------------------------------------------
 * Function:	test_error
 *
 * Purpose:	Test error API functions
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Raymond Lu
 *		July 10, 2003
 *
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
test_error(hid_t file)
{
    hid_t		dataset, space;
    hid_t               estack_id;
    hsize_t		dims[2];
    const char          *FUNC_test_error="test_error";
    H5E_auto_t          old_func;
    void                *old_data;
    
    TESTING("error API based on data I/O");

    /* Create the data space */
    dims[0] = DIM0;
    dims[1] = DIM1;
    if ((space = H5Screate_simple(2, dims, NULL))<0) TEST_ERROR;

    /* Test H5E_BEGIN_TRY */
    H5E_BEGIN_TRY {
        dataset = H5Dcreate(FAKE_ID, DSET_NAME, H5T_STD_I32BE, space, H5P_DEFAULT);
    } H5E_END_TRY;
    
    /* Create the dataset */
    if ((dataset = H5Dcreate(file, DSET_NAME, H5T_STD_I32BE, space,
			     H5P_DEFAULT))<0) {
        H5Epush(H5E_DEFAULT, __FILE__, FUNC_test_error, __LINE__, ERR_MAJ_IO, ERR_MIN_CREATE, 
                "H5Dcreate failed");
        goto error;
    }

    /* Test enabling and disabling default printing */
    if (H5Eget_auto(H5E_DEFAULT, &old_func, &old_data)<0)
	TEST_ERROR;
    if (old_data != stderr) 
	TEST_ERROR;
    if (old_func != H5Eprint)
	TEST_ERROR;

    if(H5Eset_auto(H5E_DEFAULT, NULL, NULL)<0)
        TEST_ERROR;

    /* Make H5Dwrite fail, verify default print is disabled */
    /*if (H5Dwrite(FAKE_ID, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ipoints2)>=0) {
        H5Epush(H5E_DEFAULT, __FILE__, FUNC_test_error, __LINE__, ERR_MAJ_IO, ERR_MIN_WRITE, 
                "H5Dwrite shouldn't succeed");
        goto error;
    }*/

    if(H5Eset_auto(H5E_DEFAULT, old_func, old_data)<0)
        TEST_ERROR;

    /* Test saving and restoring the current error stack */
    if (H5Dwrite(FAKE_ID, H5T_NATIVE_INT, H5S_ALL, H5S_ALL, H5P_DEFAULT, ipoints2)<0) {
        H5Epush(H5E_DEFAULT, __FILE__, FUNC_test_error, __LINE__, ERR_MAJ_IO, ERR_MIN_WRITE, 
                "H5Dwrite failed as supposed to");
        estack_id = H5Eget_current_stack();
        H5Dclose(dataset);
        H5Sclose(space);
        H5Eset_current_stack(estack_id);
        goto error; 
    }

    /* In case program comes to this point, close dataset */
    if(H5Dclose(dataset)<0) TEST_ERROR;
        
    TEST_ERROR; 

  error:
    return -1;
}


/*-------------------------------------------------------------------------
 * Function:	init_error
 *
 * Purpose:	Initialize error information.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Raymond Lu
 *		July 10, 2003
 *
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t 
init_error(void)
{
    ssize_t cls_size = strlen(ERR_CLS_NAME)+1;
    char *cls_name = malloc(strlen(ERR_CLS_NAME)+1);
    ssize_t msg_size = strlen(ERR_MIN_SUBROUTINE_MSG) + 1;
    char *msg = malloc(strlen(ERR_MIN_SUBROUTINE_MSG)+1);
    H5E_type_t *msg_type= malloc(sizeof(H5E_type_t));
    
    if((ERR_CLS = H5Eregister_class(ERR_CLS_NAME, PROG_NAME, PROG_VERS))<0)
        TEST_ERROR;
    if((ERR_CLS = H5Eregister_class(ERR_CLS_NAME, PROG_NAME, PROG_VERS))<0)
        TEST_ERROR;

    if(cls_size != H5Eget_class_name(ERR_CLS, cls_name, (size_t)cls_size) + 1) 
        TEST_ERROR;
    if(strcmp(ERR_CLS_NAME, cls_name)) 
        TEST_ERROR;
   
    if((ERR_MAJ_TEST = H5Ecreate_msg(ERR_CLS, H5E_MAJOR, ERR_MAJ_TEST_MSG))<0)
        TEST_ERROR;
    if((ERR_MAJ_IO = H5Ecreate_msg(ERR_CLS, H5E_MAJOR, ERR_MAJ_IO_MSG))<0)
        TEST_ERROR;
    if((ERR_MAJ_API = H5Ecreate_msg(ERR_CLS, H5E_MAJOR, ERR_MAJ_API_MSG))<0)
        TEST_ERROR;

    if((ERR_MIN_SUBROUTINE = H5Ecreate_msg(ERR_CLS, H5E_MINOR, ERR_MIN_SUBROUTINE_MSG))<0)
        TEST_ERROR;
    if((ERR_MIN_ERRSTACK = H5Ecreate_msg(ERR_CLS, H5E_MINOR, ERR_MIN_ERRSTACK_MSG))<0)
        TEST_ERROR;
    if((ERR_MIN_CREATE = H5Ecreate_msg(ERR_CLS, H5E_MINOR, ERR_MIN_CREATE_MSG))<0)
        TEST_ERROR;
    if((ERR_MIN_WRITE = H5Ecreate_msg(ERR_CLS, H5E_MINOR, ERR_MIN_WRITE_MSG))<0)
        TEST_ERROR;
    if((ERR_MIN_GETNUM = H5Ecreate_msg(ERR_CLS, H5E_MINOR, ERR_MIN_GETNUM_MSG))<0)
        TEST_ERROR;

    if(msg_size != H5Eget_msg(ERR_MIN_SUBROUTINE, msg_type, msg, (size_t)msg_size) + 1)
        TEST_ERROR;
    if(*msg_type != H5E_MINOR)
        TEST_ERROR;
    if(strcmp(msg, ERR_MIN_SUBROUTINE_MSG))
        TEST_ERROR;

    free(cls_name);
    free(msg);
    free(msg_type);

    return 0;

  error:
    return -1;
}


/*-------------------------------------------------------------------------
 * Function:	error_stack
 *
 * Purpose:	Manipulates current error stack.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Raymond Lu
 *		July 14, 2003
 *
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t 
error_stack(void)
{
    int err_num;
    const char          *FUNC_error_stack="error_stack";
    
    if((err_num = H5Eget_num(H5E_DEFAULT))<0)
        TEST_ERROR;
    if(err_num)
        TEST_ERROR;
    
    if((ERR_STACK = H5Eget_current_stack())<0)
        TEST_ERROR;

    /* Make it push error, force this function to fail */
    if((err_num = H5Eget_num(ERR_STACK))==0) {
        H5Epush(ERR_STACK, __FILE__, FUNC_error_stack, __LINE__, ERR_MAJ_API, ERR_MIN_GETNUM, 
                "Get number test failed, returned %d", err_num);
        goto error;
    } 
 
    /* In case program falls through here, close the stack and let it fail. */ 
    if(H5Eclose_stack(ERR_STACK)<0)
        TEST_ERROR;
    
    return -1;

  error:
    return -1;
}


/*-------------------------------------------------------------------------
 * Function:    dump_error	
 *
 * Purpose:	Prints error stack in default and customized ways.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Raymond Lu
 *		July 17, 2003
 *
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t 
dump_error(hid_t estack)
{
    /* Print errors in library default way */
    fprintf(stderr, "********* Print error stack in HDF5 default way *********\n");
    if(H5Eprint(estack, stderr)<0)
        TEST_ERROR;
    
    /* Customized way to print errors */
    fprintf(stderr, "\n********* Print error stack in customized way *********\n");
    if(H5Ewalk(estack, H5E_WALK_UPWARD, custom_print_cb, stderr)<0)
        TEST_ERROR;
    
    return 0;

  error:
    return -1;
}

/*-------------------------------------------------------------------------
 * Function:    custom_print_cb
 *
 * Purpose:	Callback function to print error stack in customized way.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Raymond Lu
 *		July 17, 2003
 *
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
herr_t 
custom_print_cb(int n, H5E_error_t *err_desc, void* client_data)
{
    FILE		*stream  = (FILE *)client_data;
    char                maj[MSG_SIZE];
    char                min[MSG_SIZE];
    char                cls[MSG_SIZE];
    const int		indent = 4;

    /* Get descriptions for the major and minor error numbers */
    if(H5Eget_class_name(err_desc->cls_id, cls, MSG_SIZE)<0)
        TEST_ERROR;

    if(H5Eget_msg(err_desc->maj_id, NULL, maj, MSG_SIZE)<0)
        TEST_ERROR;
        
    if(H5Eget_msg(err_desc->min_id, NULL, min, MSG_SIZE)<0)
        TEST_ERROR;

    fprintf (stream, "%*serror #%03d: %s in %s(): line %u\n",
	     indent, "", n, err_desc->file_name,
	     err_desc->func_name, err_desc->line);
    fprintf (stream, "%*sclass: %s\n", indent*2, "", cls);
    fprintf (stream, "%*smajor: %s\n", indent*2, "", maj);
    fprintf (stream, "%*sminor: %s\n", indent*2, "", min);

    return 0;

  error:
    return -1;
}


/*-------------------------------------------------------------------------
 * Function:	close_error
 *
 * Purpose:	Closes error information.
 *
 * Return:	Success:	0
 *
 *		Failure:	-1
 *
 * Programmer:	Raymond Lu
 *		July 10, 2003
 *
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t 
close_error(void)
{
    /* Close major errors, let H5Eunregister_class close minor errors */
    if(H5Eclose_msg(ERR_MAJ_TEST)<0)
        TEST_ERROR ;
    
    if(H5Eclose_msg(ERR_MAJ_IO)<0)
        TEST_ERROR ;
     
    if(H5Eclose_msg(ERR_MAJ_API)<0)
        TEST_ERROR ;
  
    if(H5Eunregister_class(ERR_CLS)<0)
        TEST_ERROR ;

    return 0;

  error:
    return -1;
}
    

/*-------------------------------------------------------------------------
 * Function:	main
 *
 * Purpose:	Test error API.
 *
 * Programmer:	Raymond Lu
 *		July 10, 2003
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
int
main(void)
{
    hid_t		file, fapl;
    hid_t               estack_id;
    char		filename[1024];
    const char          *FUNC_main="main";

    fprintf(stderr, "   This program tests the Error API.  There're supposed to be some error messages\n");
    /*h5_reset();*/

    /* Initialize errors */
    if(init_error()<0)
        TEST_ERROR ;
    
    fapl = h5_fileaccess();
    
    h5_fixname(FILENAME[0], fapl, filename, sizeof filename);
    if ((file=H5Fcreate(filename, H5F_ACC_TRUNC, H5P_DEFAULT, fapl))<0)
	TEST_ERROR ;

    /* Test error stack */ 
    if(error_stack()<0) {
        /* Push an error onto error stack */
        H5Epush(ERR_STACK, __FILE__, FUNC_main, __LINE__, ERR_MAJ_TEST, ERR_MIN_ERRSTACK, 
                "Error stack test failed");
        
        /* Delete an error from the top of error stack */
        H5Epop(ERR_STACK, 1);
        
        /* Print out the errors on stack */
        dump_error(ERR_STACK);

        /* Empty error stack */
        H5Eclear(ERR_STACK);

        /* Close error stack */
        H5Eclose_stack(ERR_STACK);
    }

    /* Test error API */
    if(test_error(file)<0) {
        H5Epush(H5E_DEFAULT, __FILE__, FUNC_main, __LINE__, ERR_MAJ_TEST, ERR_MIN_SUBROUTINE, 
                "Error test failed, %s", "it's wrong");
        estack_id = H5Eget_current_stack();
        H5Eprint(estack_id, stderr);
        H5Eclose_stack(estack_id);
    }
    
    if (H5Fclose(file)<0) TEST_ERROR ;
    h5_cleanup(FILENAME, fapl);

    /* Close error information */
    if(close_error()<0)
        TEST_ERROR ;

    PASSED();
    printf("All error API test based on native datatype test passed.\n");
    return 0;

 error:
    printf("***** ERROR TEST FAILED! *****\n");
    return 1;
}
