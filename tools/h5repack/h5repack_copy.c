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


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "H5private.h"
#include "h5repack.h"

/*-------------------------------------------------------------------------
 * Function: copy_file
 *
 * Purpose: duplicate all HDF5 objects in the file 
 *
 * Return: 0, ok, -1 no
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: October, 23, 2003
 *
 *-------------------------------------------------------------------------
 */

int copy_file(const char* fnamein, 
              const char* fnameout,
              pack_opt_t *options)
{
 hid_t       fidin; 
 hid_t       fidout; 
 int         nobjects;
 trav_info_t *travi=NULL;

/*-------------------------------------------------------------------------
 * open the files 
 *-------------------------------------------------------------------------
 */

 /* disable error reporting */
 H5E_BEGIN_TRY {
 
 /* Open the files */
 if ((fidin=H5Fopen(fnamein,H5F_ACC_RDONLY,H5P_DEFAULT))<0 ){
  printf("h5repack: <%s>: No such file or directory\n", fnamein );
  exit(1);
 }
 if ((fidout=H5Fcreate(fnameout,H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))<0 ){
  printf("h5repack: <%s>: Could not create file\n", fnameout );
  exit(1);
 }
 /* enable error reporting */
 } H5E_END_TRY;

 
 if (options->verbose)
  printf("Making file <%s>...\n",fnameout);


/*-------------------------------------------------------------------------
 * get the number of objects in the file
 *-------------------------------------------------------------------------
 */

 if ((nobjects = h5trav_getinfo(fidin, NULL ))<0) {
  printf("h5repack: <%s>: Could not obtain object list\n", fnamein );
  return -1;
 }

/*-------------------------------------------------------------------------
 * get the list of objects in the file
 *-------------------------------------------------------------------------
 */

 if ((travi = (trav_info_t*) malloc( nobjects * sizeof(trav_info_t)))==NULL){
  printf("h5repack: <%s>: Could not allocate object list\n", fnamein );
  return -1;
 }
 if (h5trav_getinfo(fidin, travi )<0) {
  printf("h5repack: <%s>: Could not obtain object list\n", fnamein );
  return -1;
 }

/*-------------------------------------------------------------------------
 * do the copy
 *-------------------------------------------------------------------------
 */
 if(do_copy_file(fidin,fidout,nobjects,travi,options)<0) {
  printf("h5repack: <%s>: Could not copy data to: %s\n", fnamein, fnameout);
  return -1;
 }

/*-------------------------------------------------------------------------
 * do the copy of referenced objects
 *-------------------------------------------------------------------------
 */
 if(do_copy_refobjs(fidin,fidout,nobjects,travi,options)<0) {
  printf("h5repack: <%s>: Could not copy data to: %s\n", fnamein, fnameout);
  return -1;
 }

#if defined (H5_REPACK_DEBUG)
 h5trav_printinfo(nobjects,travi);
#endif


/*-------------------------------------------------------------------------
 * free
 *-------------------------------------------------------------------------
 */
 H5Fclose(fidin);
 H5Fclose(fidout);
 h5trav_freeinfo(travi,nobjects);
 return 0;
}



/*-------------------------------------------------------------------------
 * Function: do_copy_file
 *
 * Purpose: duplicate all HDF5 objects in the file 
 *
 * Return: 0, ok, -1 no
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: October, 23, 2003
 *
 *-------------------------------------------------------------------------
 */

int do_copy_file(hid_t fidin, 
                 hid_t fidout, 
                 int nobjects,        /* number of objects */
                 trav_info_t *travi,  /* array of object names */
                 pack_opt_t *options) /* repack options */
{
 hid_t     grp_in;       /* group ID */ 
 hid_t     grp_out;      /* group ID */ 
 hid_t     dset_in;      /* read dataset ID */ 
 hid_t     dset_out;     /* write dataset ID */ 
 hid_t     type_in;      /* named type ID */ 
 hid_t     type_out;     /* named type ID */ 
 hid_t     dcpl_id;      /* dataset creation property list ID */ 
 hid_t     space_id;     /* space ID */ 
 hid_t     ftype_id;     /* file data type ID */ 
 hid_t     mtype_id;     /* memory data type ID */
 size_t    msize;        /* memory size of memory type */
 void      *buf=NULL;    /* data buffer */
 hsize_t   nelmts;       /* number of elements in dataset */
 int       rank;         /* rank of dataset */
 hsize_t   dims[H5S_MAX_RANK];/* dimensions of dataset */
 int       i, j;

/*-------------------------------------------------------------------------
 * copy the suppplied object list
 *-------------------------------------------------------------------------
 */

 for ( i = 0; i < nobjects; i++)
 {

  switch ( travi[i].type )
  {
/*-------------------------------------------------------------------------
 * H5G_GROUP
 *-------------------------------------------------------------------------
 */
  case H5G_GROUP:
   if (options->verbose)
    printf(" %-10s %s\n", "group",travi[i].name );

   if ((grp_out=H5Gcreate(fidout,travi[i].name, 0))<0) 
    goto error;

   if((grp_in = H5Gopen (fidin,travi[i].name))<0) 
    goto error;

/*-------------------------------------------------------------------------
 * copy attrs
 *-------------------------------------------------------------------------
 */
   if (copy_attr(grp_in,grp_out,options,nobjects,travi,fidout)<0) 
    goto error;
   
   if (H5Gclose(grp_out)<0) 
    goto error;
   if (H5Gclose(grp_in)<0) 
    goto error;

   break;

/*-------------------------------------------------------------------------
 * H5G_DATASET
 *-------------------------------------------------------------------------
 */
  case H5G_DATASET:
   if (options->verbose)
    printf(" %-10s %s\n", "dataset",travi[i].name );

   if ((dset_in=H5Dopen(fidin,travi[i].name))<0) 
    goto error;
   if ((space_id=H5Dget_space(dset_in))<0) 
    goto error;
   if ((ftype_id=H5Dget_type (dset_in))<0) 
    goto error;
   if ((dcpl_id=H5Dget_create_plist(dset_in))<0) 
    goto error;
   if ( (rank=H5Sget_simple_extent_ndims(space_id))<0)
    goto error;
   if ( H5Sget_simple_extent_dims(space_id,dims,NULL)<0)
    goto error;
   nelmts=1;
   for (j=0; j<rank; j++) 
    nelmts*=dims[j];
   if ((mtype_id=H5Tget_native_type(ftype_id,H5T_DIR_DEFAULT))<0)
    goto error;
   if ((msize=H5Tget_size(mtype_id))==0)
    goto error;


/*-------------------------------------------------------------------------
 * object references are a special case
 * we cannot just copy the buffers, but instead we recreate the reference
 * in a second traversal of the output file
 *-------------------------------------------------------------------------
 */
   if ( ! H5Tequal(mtype_id, H5T_STD_REF_OBJ) ) 
   {
  
  /*-------------------------------------------------------------------------
   * read to memory
   *-------------------------------------------------------------------------
   */
    
    buf=(void *) HDmalloc((unsigned)(nelmts*msize));
    if ( buf==NULL){
     printf( "cannot read into memory\n" );
     goto error;
    }
    if (H5Dread(dset_in,mtype_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,buf)<0)
     goto error;
    
    /*-------------------------------------------------------------------------
     * create/write dataset/close
     *-------------------------------------------------------------------------
     */
    if ((dset_out=H5Dcreate(fidout,travi[i].name,ftype_id,space_id,dcpl_id))<0) 
     goto error;
    if (H5Dwrite(dset_out,mtype_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,buf)<0)
     goto error;
    
    /*-------------------------------------------------------------------------
     * copy attrs
     *-------------------------------------------------------------------------
    */
    if (copy_attr(dset_in,dset_out,options,nobjects,travi,fidout)<0) 
     goto error;
    
    /*close */
    if (H5Dclose(dset_out)<0) 
     goto error;
    
    if (buf)
     free(buf);
    
   }/*H5T_STD_REF_OBJ*/
  

/*-------------------------------------------------------------------------
 * close
 *-------------------------------------------------------------------------
 */

   if (H5Tclose(ftype_id)<0) 
    goto error;
   if (H5Tclose(mtype_id)<0) 
    goto error;
   if (H5Pclose(dcpl_id)<0) 
    goto error;
   if (H5Sclose(space_id)<0) 
    goto error;
   if (H5Dclose(dset_in)<0) 
    goto error;
  

   
   break;

/*-------------------------------------------------------------------------
 * H5G_TYPE
 *-------------------------------------------------------------------------
 */
  case H5G_TYPE:

   if ((type_in = H5Topen (fidin,travi[i].name))<0) 
    goto error;

   if ((type_out = H5Tcopy(type_in))<0) 
    goto error;

   if ((H5Tcommit(fidout,travi[i].name,type_out))<0) 
    goto error;

/*-------------------------------------------------------------------------
 * copy attrs
 *-------------------------------------------------------------------------
 */
   if (copy_attr(type_in,type_out,options,nobjects,travi,fidout)<0) 
    goto error;
   
   if (H5Tclose(type_in)<0) 
    goto error;
   if (H5Tclose(type_out)<0) 
    goto error;

   if (options->verbose)
    printf(" %-10s %s\n","datatype",travi[i].name );

   break;


/*-------------------------------------------------------------------------
 * H5G_LINK
 *-------------------------------------------------------------------------
 */

  case H5G_LINK:

   {

    H5G_stat_t  statbuf;
    char        *targbuf=NULL;
    
    if (H5Gget_objinfo(fidin,travi[i].name,FALSE,&statbuf)<0)
     goto error;
    
    targbuf = malloc(statbuf.linklen);
    
    if (H5Gget_linkval(fidin,travi[i].name,statbuf.linklen,targbuf)<0)
     goto error;

    if (H5Glink(fidout,
     H5G_LINK_SOFT,
     targbuf,        /* current name of object */
     travi[i].name   /* new name of object */
     )<0) 
     goto error;
    
    free(targbuf);
      
    if (options->verbose)
     printf(" %-10s %s\n","link",travi[i].name );

   }
   break;

  default:
   if (options->verbose)
    printf(" %-10s %s\n","User defined object",travi[i].name);
   break;
  }
 }


  
/*-------------------------------------------------------------------------
 * the root is a special case, we get an ID for the root group 
 * and copy its attributes using that ID
 * it must be done last, because the attributes might contain references to
 * objects in the object list
 *-------------------------------------------------------------------------
 */

 if ((grp_out = H5Gopen(fidout,"/"))<0) 
  goto error;
 
 if ((grp_in  = H5Gopen(fidin,"/"))<0) 
  goto error;
 
 if (copy_attr(grp_in,grp_out,options,nobjects,travi,fidout)<0) 
  goto error;
 
 if (H5Gclose(grp_out)<0) 
  goto error;
 if (H5Gclose(grp_in)<0) 
  goto error;
 
 return 0;
 
error:
 H5E_BEGIN_TRY {
  H5Gclose(grp_in);
  H5Gclose(grp_out);
  H5Pclose(dcpl_id);
  H5Sclose(space_id);
  H5Dclose(dset_in);
  H5Dclose(dset_out);
  H5Tclose(ftype_id);
  H5Tclose(mtype_id);
  H5Tclose(type_in);
  H5Tclose(type_out);
  if (buf)
   free(buf);
 } H5E_END_TRY;
 return -1;

}


/*-------------------------------------------------------------------------
 * Function: copy_attr
 *
 * Purpose: copy attributes located in LOC_IN, which is obtained either from 
 * loc_id = H5Gopen( fid, name);
 * loc_id = H5Dopen( fid, name);
 * loc_id = H5Topen( fid, name);
 *
 * Return: 0, ok, -1 no
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: October, 28, 2003
 *
 *-------------------------------------------------------------------------
 */

int copy_attr(hid_t loc_in, 
              hid_t loc_out, 
              pack_opt_t *options,
              int nobjects,        /* number of objects */
              trav_info_t *travi,  /* array of object names */
              hid_t fidout         /* for saving references */
              )
{
 hid_t      attr_id;      /* attr ID */ 
 hid_t      attr_out;     /* attr ID */ 
 hid_t      space_id;     /* space ID */ 
 hid_t      ftype_id;     /* file data type ID */ 
 hid_t      mtype_id;     /* memory data type ID */
 size_t     msize;        /* memory size of type */
 void       *buf;         /* data buffer */
 hsize_t    nelmts;       /* number of elements in dataset */
 int        rank;         /* rank of dataset */
 hsize_t    dims[H5S_MAX_RANK];/* dimensions of dataset */
 char       name[255];
 int        n, j;
 unsigned   u;
 int        have_ref=0;

 if ((n = H5Aget_num_attrs(loc_in))<0) 
  goto error;
 
 for ( u = 0; u < (unsigned)n; u++)
 {

  /* set data buffer to NULL each iteration 
     we might not use it in the case of references 
   */
   buf=NULL;    
/*-------------------------------------------------------------------------
 * open
 *-------------------------------------------------------------------------
 */
  /* open attribute */
  if ((attr_id = H5Aopen_idx(loc_in, u))<0) 
   goto error;
  
  /* get name */
  if (H5Aget_name( attr_id, 255, name )<0) 
   goto error;

  /* get the file datatype  */
  if ((ftype_id = H5Aget_type( attr_id )) < 0 )
   goto error;
  
  /* get the dataspace handle  */
  if ((space_id = H5Aget_space( attr_id )) < 0 )
   goto error;
  
  /* get dimensions  */
  if ( (rank = H5Sget_simple_extent_dims(space_id, dims, NULL)) < 0 )
   goto error;
  
  nelmts=1;
  for (j=0; j<rank; j++) 
   nelmts*=dims[j];
  if ((mtype_id=H5Tget_native_type(ftype_id,H5T_DIR_DEFAULT))<0)
   goto error;
  if ((msize=H5Tget_size(mtype_id))==0)
   goto error;

/*-------------------------------------------------------------------------
 * object references are a special case
 * we cannot just copy the buffers, but instead we recreate the reference 
 * this is done on a second sweep of the file that just copies 
 * the referenced objects
 *-------------------------------------------------------------------------
 */
  if ( ! H5Tequal(mtype_id, H5T_STD_REF_OBJ)) 
  {
   
   
 /*-------------------------------------------------------------------------
  * read to memory
  *-------------------------------------------------------------------------
  */
   
   buf=(void *) HDmalloc((unsigned)(nelmts*msize));
   if ( buf==NULL){
    printf( "cannot read into memory\n" );
    goto error;
   }
   if (H5Aread(attr_id,mtype_id,buf)<0)
    goto error;
   
   /*-------------------------------------------------------------------------
    * copy 
    *-------------------------------------------------------------------------
    */
   
   if ((attr_out=H5Acreate(loc_out,name,ftype_id,space_id,H5P_DEFAULT))<0)
    goto error;
   if(H5Awrite(attr_out,mtype_id,buf)<0)
    goto error;
   
   /*close*/
   if (H5Aclose(attr_out)<0) 
    goto error;
   
   
   if (buf)
    free(buf);
   
   
  } /*H5T_STD_REF_OBJ*/

      
  if (options->verbose)
   printf("   %-13s %s\n", "attr", name);
  
/*-------------------------------------------------------------------------
 * close
 *-------------------------------------------------------------------------
 */

  if (H5Tclose(ftype_id)<0) goto error;
  if (H5Tclose(mtype_id)<0) goto error;
  if (H5Sclose(space_id)<0) goto error;
  if (H5Aclose(attr_id)<0) goto error;

 } /* u */
 
  return 0;

error:
 H5E_BEGIN_TRY {
  H5Tclose(ftype_id);
  H5Tclose(mtype_id);
  H5Sclose(space_id);
  H5Aclose(attr_id);
  H5Aclose(attr_out);
  if (buf)
    free(buf);
 } H5E_END_TRY;
 return -1;
}


