#include "H5f90.h"

/*----------------------------------------------------------------------------
 * Name:        h5fcreate_c
 * Purpose:     Call H5Fcreate to create the file
 * Inputs:      name - name of the file     
 *              namelen - name length
 *              access_flags - file access  flags
 *              crt_pr  - identifier of creation property list
 *              acc_prp - identifier of access property list 
 * Outputs:     file_id - file identifier
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, July 26, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5fcreate_c(_fcd name, int_f *namelen, int_f *access_flags, hid_t_f* crt_prp, hid_t_f *acc_prp, hid_t_f *file_id)
{
     int ret_value = -1;
     char *c_name;
     int c_namelen;
     hid_t c_file_id;
     unsigned c_access_flags;
     hid_t c_crt_prp;
     hid_t c_acc_prp;
     int CASE;
     hid_t CASE_prp;

     /*
      * Define access flags
      */
     c_access_flags = (unsigned) *access_flags;
/*
     CASE = (int)*access_flags;
     switch (CASE) {

	case H5F_ACC_RDWR_F:
	  c_access_flags = H5F_ACC_RDWR;
          break;

	case H5F_ACC_RDONLY_F:
          c_access_flags = H5F_ACC_RDONLY;
          break;

        case H5F_ACC_TRUNC_F:
          c_access_flags = H5F_ACC_TRUNC;
          break;

        case H5F_ACC_EXCL_F:
          c_access_flags = H5F_ACC_EXCL;
          break;

        case H5F_ACC_DEBUG_F:
          c_access_flags = H5F_ACC_DEBUG;
          break;

        default:
          return ret_value;
     }
*/     
     /*
      * Define creation property
      */
     c_crt_prp = *crt_prp;
/*
     if ( H5P_DEFAULT_F == c_crt_prp ) c_crt_prp = H5P_DEFAULT;
*/
     /*
      * Define access property
      */
     c_acc_prp = *acc_prp;
/*
     if ( H5P_DEFAULT_F == c_acc_prp ) c_acc_prp = H5P_DEFAULT;

*/
     /*
      * Convert FORTRAN name to C name
      */
     c_namelen = *namelen;
     c_name = (char *)HD5f2cstring(name, c_namelen); 
     if (c_name == NULL) return ret_value;

     /*
      * Call H5Fcreate function.
      */
     c_file_id = H5Fcreate(c_name, c_access_flags, c_crt_prp, c_acc_prp);

     if (c_file_id < 0) return ret_value;
     *file_id = c_file_id;
     HDfree(c_name);
     ret_value = 0;
     return ret_value;
} 
     
/*----------------------------------------------------------------------------
 * Name:        h5fflush_c
 * Purpose:     Call H5Fflush to flush the object
 * Inputs:      object_id - identifier of either a file, a dataset,
 *                          a group, an attribute or a named data type       
 *              scope - integer to specify the flushing action, either
 *                      H5F_SCOPE_GLOBAL or H5F_SCOPE_LOCAL 
 * Returns:     0 on success, -1 on failure
 * Programmer:  Xiangyang Su
 *              Friday, November 5, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5fflush_c (hid_t_f *object_id, int_f *scope)
{
     int ret_value = -1;
     hid_t c_file_id;
     int CASE;
     H5F_scope_t  c_scope;
     htri_t status;
     c_scope = (H5F_scope_t)*scope;

     /*
      * Define scope flags
      */
/*
     CASE = (int)*scope;
     switch (CASE) {

	case H5F_SCOPE_LOCAL_F:
	  c_scope = H5F_SCOPE_LOCAL;
          break;

	case H5F_SCOPE_GLOBAL_F:
	  c_scope = H5F_SCOPE_GLOBAL;
          break;

        default:
          return ret_value;
     }
*/     
     /*
      * Call H5Fflush function.
      */

     c_file_id = *object_id;

     status = H5Fflush(c_file_id, c_scope);

     if (status >= 0)  ret_value = 0;

     return ret_value;
}      


/*----------------------------------------------------------------------------
 * Name:        h5fmount_c
 * Purpose:     Call H5Fmount to mount the file
 * Inputs:      loc_id - Identifier for file or group
 *              dsetname - name of dataset    
 *              namelen - dsetname length
 *              file_id - file identifier for the file to be mounted
 *              acc_prp - identifier of access property list 
 * Returns:     0 on success, -1 on failure
 * Programmer:  Xiangyang Su
 *              Monday, October 25, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5fmount_c (hid_t_f *loc_id, _fcd dsetname, int_f *namelen, hid_t_f *file_id, hid_t_f *acc_prp)
{
     int ret_value = -1;
     char *c_name;
     int c_namelen;
     hid_t c_loc_id;
     hid_t c_file_id;
     hid_t c_acc_prp;
     htri_t status;

     /*
      * Define access property
      */
     c_acc_prp = *acc_prp;
/*
     if ( H5P_DEFAULT_F == c_acc_prp ) c_acc_prp = H5P_DEFAULT;
*/

     c_loc_id = *loc_id;
     c_file_id = *file_id;
     /*
      * Convert FORTRAN name to C name
      */
     c_namelen = *namelen;
     c_name = (char *)HD5f2cstring(dsetname, c_namelen); 
     if (c_name == NULL) return ret_value;

     /*
      * Call H5Fmount function.
      */
     status = H5Fmount(c_loc_id, c_name, c_file_id, c_acc_prp);

     if (status >= 0)  ret_value = 0;

     HDfree(c_name);
     return ret_value;
}      

/*----------------------------------------------------------------------------
 * Name:        h5funmount_c
 * Purpose:     Call H5Funmount to unmount the file
 * Inputs:      loc_id - Identifier for file or group
 *              dsetname - name of dataset    
 *              namelen - dsetname length
 * Returns:     0 on success, -1 on failure
 * Programmer:  Xiangyang Su
 *              Monday, October 25, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5funmount_c (hid_t_f *loc_id, _fcd dsetname, int_f *namelen)
{
     int ret_value = -1;
     char *c_name;
     int c_namelen;
     hid_t c_loc_id;
     htri_t status;

     c_loc_id = *loc_id;

     /*
      * Convert FORTRAN name to C name
      */
     c_namelen = *namelen;
     c_name = (char *)HD5f2cstring(dsetname, c_namelen); 
     if (c_name == NULL) return ret_value;

     /*
      * Call H5Fmount function.
      */
     status = H5Funmount(c_loc_id, c_name);

     if (status >= 0)  ret_value = 0;

     HDfree(c_name);
     return ret_value;
}      



/*----------------------------------------------------------------------------
 * Name:        h5fopen_c
 * Purpose:     Call H5Fopen to open the file
 * Inputs:      name - name of the file     
 *              namelen - name length
 *              access_flags - file access  flags
 *              acc_prp - identifier of access property list 
 * Outputs:     file_id - file identifier
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Tuesday, August 3, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5fopen_c (_fcd name, int_f *namelen, int_f *access_flags, hid_t_f *acc_prp, hid_t_f *file_id)
{
     int ret_value = -1;
     char *c_name;
     int c_namelen;
     hid_t c_file_id;
     unsigned c_access_flags;
     hid_t c_acc_prp;
     int CASE;
     hid_t CASE_prp;
     c_acc_prp = (hid_t)*acc_prp;

     /*
      * Define access flags
      */
     c_access_flags = (unsigned) *access_flags;
/*
     CASE = (int)*access_flags;
     switch (CASE) {

	case H5F_ACC_RDWR_F:
	  c_access_flags = H5F_ACC_RDWR;
          break;

	case H5F_ACC_RDONLY_F:
          c_access_flags = H5F_ACC_RDONLY;
          break;

        case H5F_ACC_TRUNC_F:
          c_access_flags = H5F_ACC_TRUNC;
          break;

        case H5F_ACC_EXCL_F:
          c_access_flags = H5F_ACC_EXCL;
          break;

        case H5F_ACC_DEBUG_F:
          c_access_flags = H5F_ACC_DEBUG;
          break;

        default:
          return ret_value;
     }
 */    
     /*
      * Define access property
      */
     c_acc_prp = *acc_prp;
/*
     if ( H5P_DEFAULT_F == c_acc_prp ) c_acc_prp = H5P_DEFAULT;
*/

     /*
      * Convert FORTRAN name to C name
      */
     c_namelen = *namelen;
     c_name = (char *)HD5f2cstring(name, c_namelen); 
     if (c_name == NULL) return ret_value;

     /*
      * Call H5Fopen function.
      */
     c_file_id = H5Fopen(c_name, c_access_flags, c_acc_prp);

     if (c_file_id < 0) return ret_value;
     *file_id = (hid_t_f)c_file_id;

     HDfree(c_name);
     ret_value = 0;
     return ret_value;
}      

/*----------------------------------------------------------------------------
 * Name:        h5freopen_c
 * Purpose:     Call H5Freopen to open the file
 * Inputs:      file_id1 - file identifier
 * Outputs:     file_id2 - file identifier
 * Returns:     0 on success, -1 on failure
 * Programmer:  Xiangyang Su
 *              Wednesday, November 3, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5freopen_c (hid_t_f *file_id1, hid_t_f *file_id2)
{
     int ret_value = -1;
     hid_t c_file_id1, c_file_id2;

     c_file_id1 = *file_id1;
     c_file_id2 = H5Freopen(c_file_id1);

     if (c_file_id2 < 0) return ret_value;
     *file_id2 = (hid_t_f)c_file_id2;

     ret_value = 0;
     return ret_value;
}      

/*----------------------------------------------------------------------------
 * Name:        h5fget_create_plist_c
 * Purpose:     Call H5Fget_create_plist to get the file creation property list
 * Inputs:      file_id - file identifier
 * Outputs:     prop_id - creation property list identifier
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal, Xiangyang Su
 *              Wednesday, November 3, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5fget_create_plist_c (hid_t_f *file_id, hid_t_f *prop_id)
{
     int ret_value = -1;
     hid_t c_file_id, c_prop_id;

     c_file_id = (hid_t)*file_id;
     c_prop_id = H5Fget_create_plist(c_file_id);

     if (c_prop_id < 0) return ret_value;
     *prop_id = (hid_t_f)c_prop_id;

     ret_value = 0;
     return ret_value;
}      

/*----------------------------------------------------------------------------
 * Name:        h5fget_access_plist_c
 * Purpose:     Call H5Fget_access_plist to get the file access property list
 * Inputs:      file_id - file identifier
 * Outputs:     access_id - access property list identifier
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, September 30, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5fget_access_plist_c (hid_t_f *file_id, hid_t_f *access_id)
{
     int ret_value = -1;
     hid_t c_file_id, c_access_id;

     c_file_id = (hid_t)*file_id;
     c_access_id = H5Fget_access_plist(c_file_id);

     if (c_access_id < 0) return ret_value;
     *access_id = (hid_t_f)c_access_id;

     ret_value = 0;
     return ret_value;
}      
      
/*----------------------------------------------------------------------------
 * Name:        h5fis_hdf5_c
 * Purpose:     Call H5Fis_hdf5 to determone if the file is an HDF5 file
 * Inputs:      name - name of the file     
 *              namelen - name length
 * Outputs:     flag - 0 if file is not HDF5 file , positive if a file
 *                     is an HDF5 file, and negative on failure. 
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Tuesday, August 3, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/
int_f
nh5fis_hdf5_c (_fcd name, int_f *namelen, int_f *flag)
{
     int ret_value = -1;
     char *c_name;
     int c_namelen;
     htri_t status;

     /*
      * Convert FORTRAN name to C name
      */
     c_namelen = *namelen;
     c_name = (char *)HD5f2cstring(name, c_namelen); 
     if (c_name == NULL) return ret_value;

     /*
      * Call H5Fopen function.
      */
     status = H5Fis_hdf5(c_name);
     *flag = (int_f)status;
     if (status >= 0) ret_value = 0;

     HDfree(c_name);
     return ret_value;
}      
/*----------------------------------------------------------------------------
 * Name:        h5fclose_c
 * Purpose:     Call H5Fclose to close the file
 * Inputs:      file_id - identifier of the file to be closed
 * Returns:     0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, July 26, 1999
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5fclose_c ( hid_t_f *file_id )
{
  int ret_value = 0;
  hid_t c_file_id;

  c_file_id = *file_id;
  if ( H5Fclose(c_file_id) < 0  ) ret_value = -1;
  return ret_value;
}
/*----------------------------------------------------------------------------
 * Name:        h5fget_obj_count_c
 * Purpose:     Call H5Fget_obj_count to get number of open objects within a file
 * Inputs:      file_id - identifier of the file to be closed
 *              obj_type - type of the object
 * Returns:     obj_count - number of objects 
 *              0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, September 30, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5fget_obj_count_c ( hid_t_f *file_id , int_f *obj_type, int_f * obj_count)
{
  int ret_value = 0;
  hid_t c_file_id;
  unsigned c_obj_type;
  unsigned c_obj_count;
  

  c_file_id = (hid_t)*file_id;
  c_obj_type = (unsigned) *obj_type;
  if ( H5Fget_obj_count(c_file_id, c_obj_type, &c_obj_count) < 0  ) ret_value = -1;
  *obj_count = (int_f)c_obj_count;
  return ret_value;
}
/*----------------------------------------------------------------------------
 * Name:        h5fget_obj_ids_c
 * Purpose:     Call H5Fget_obj_count to get number of open objects within a file
 * Inputs:      file_id - identifier of the file to be closed
 *              obj_type - type of the object
 * Returns:     obj_ids  - iarray of open objects identifiers
 *              0 on success, -1 on failure
 * Programmer:  Elena Pourmal
 *              Monday, September 30, 2002
 * Modifications:
 *---------------------------------------------------------------------------*/

int_f 
nh5fget_obj_ids_c ( hid_t_f *file_id , int_f *obj_type, hid_t_f *obj_ids)
{
  int ret_value = 0;
  hid_t c_file_id;
  unsigned c_obj_type;

  c_file_id = (hid_t)*file_id;
  c_obj_type = (unsigned) *obj_type;
  if ( H5Fget_obj_ids(c_file_id, c_obj_type, (hid_t *)obj_ids) < 0  ) ret_value = -1;
  return ret_value;
}
