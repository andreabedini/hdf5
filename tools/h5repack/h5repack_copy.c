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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "H5private.h"
#include "h5tools.h"
#include "h5tools_utils.h"
#include "h5repack.h"

extern char  *progname;
static int do_create_refs;


#define PER(A,B) { per = 0;                   \
                   if (A!=0)                  \
                    per = (double) fabs( (double)(B-A) / (double)A  ); \
                 }

#define FORMAT_OBJ      " %-27s %s\n"  /* obj type, name */
#define FORMAT_OBJ_ATTR "  %-27s %s\n"  /* obj type, name */

/* local functions */
static int   do_hardlinks(hid_t fidout,trav_table_t *travt);
static void  close_obj(H5G_obj_t obj_type, hid_t obj_id);
static int   do_copy_refobjs(hid_t fidin, hid_t fidout,trav_table_t *travt,pack_opt_t *options); 
static int   copy_refs_attr(hid_t loc_in,hid_t loc_out,pack_opt_t *options,trav_table_t *travt,hid_t fidout);
static const char* MapIdToName(hid_t refobj_id,trav_table_t *travt);

/*-------------------------------------------------------------------------
 * Function: print_dataset_info
 *
 * Purpose: print name, filters, percentage compression of a dataset
 *
 *-------------------------------------------------------------------------
 */
static void print_dataset_info(hid_t dcpl_id,
                               char *objname,
                               double per)
{
 char         strfilter[255];
#if defined (PRINT_DEBUG )
 char         temp[255];
#endif
 int          nfilters;       /* number of filters */
 unsigned     filt_flags;     /* filter flags */
 H5Z_filter_t filtn;          /* filter identification number */
 unsigned     cd_values[20];  /* filter client data values */
 size_t       cd_nelmts;      /* filter client number of values */
 char         f_objname[256];    /* filter objname */
 int          i;

 strcpy(strfilter,"\0");

 /* get information about input filters */
 if ((nfilters = H5Pget_nfilters(dcpl_id))<0)
  return;

 for ( i=0; i<nfilters; i++)
 {
  cd_nelmts = NELMTS(cd_values);

#ifdef H5_WANT_H5_V1_6_COMPAT
  filtn = H5Pget_filter(dcpl_id,
   (unsigned)i,
   &filt_flags,
   &cd_nelmts,
   cd_values,
   sizeof(f_objname),
   f_objname);
#else
  filtn = H5Pget_filter(dcpl_id,
   (unsigned)i,
   &filt_flags,
   &cd_nelmts,
   cd_values,
   sizeof(f_objname),
   f_objname,
   NULL);
#endif /* H5_WANT_H5_V1_6_COMPAT */

  switch (filtn)
  {
  default:
   break;
  case H5Z_FILTER_DEFLATE:
   strcat(strfilter,"GZIP ");

#if defined (PRINT_DEBUG)
   {
    unsigned level=cd_values[0];
    sprintf(temp,"(%d)",level);
    strcat(strfilter,temp);
   }
#endif

   break;
  case H5Z_FILTER_SZIP:
   strcat(strfilter,"SZIP ");

#if defined (PRINT_DEBUG)
   {
    unsigned options_mask=cd_values[0]; /* from dcpl, not filt*/
    unsigned ppb=cd_values[1];
    sprintf(temp,"(%d,",ppb);
    strcat(strfilter,temp);
    if (options_mask & H5_SZIP_EC_OPTION_MASK)
     strcpy(temp,"EC) ");
    else if (options_mask & H5_SZIP_NN_OPTION_MASK)
     strcpy(temp,"NN) ");
   }
   strcat(strfilter,temp);

#endif

   break;
  case H5Z_FILTER_SHUFFLE:
   strcat(strfilter,"SHUF ");
   break;
  case H5Z_FILTER_FLETCHER32:
   strcat(strfilter,"FLET ");
   break;
  case H5Z_FILTER_NBIT:
   strcat(strfilter,"NBIT ");
   break;
  case H5Z_FILTER_SCALEOFFSET:
   strcat(strfilter,"SCALEOFFSET ");
   break;
  } /* switch */
 }/*i*/

 if (strcmp(strfilter,"\0")==0)
  printf(FORMAT_OBJ,"dset",objname );
 else
 {
  char str[255], temp[20];
  strcpy(str,"dset     ");
  strcat(str,strfilter);
  sprintf(temp,"  (%.1f%%)",per);
  strcat(str,temp);
  printf(FORMAT_OBJ,str,objname);
 }
}

/*-------------------------------------------------------------------------
 * Function: copy_objects
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

int copy_objects(const char* fnamein,
                 const char* fnameout,
                 pack_opt_t *options)
{
 hid_t         fidin;
 hid_t         fidout=-1;
 trav_table_t  *travt=NULL;

/*-------------------------------------------------------------------------
 * open the files
 *-------------------------------------------------------------------------
 */
 if ((fidin=h5tools_fopen(fnamein, H5F_ACC_RDONLY, NULL, NULL, 0))<0 ){
  error_msg(progname, "<%s>: %s\n", fnamein, H5FOPENERROR );
  goto out;
 }
 if ((fidout=H5Fcreate(fnameout,H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT))<0 ){
  error_msg(progname, "<%s>: Could not create file\n", fnameout );
  goto out;
 }

 if (options->verbose)
  printf("Making file <%s>...\n",fnameout);

 /* init table */
 trav_table_init(&travt);

 /* get the list of objects in the file */
 if (h5trav_gettable(fidin,travt)<0)
  goto out;

/*-------------------------------------------------------------------------
 * do the copy
 *-------------------------------------------------------------------------
 */
 if(do_copy_objects(fidin,fidout,travt,options)<0) {
  error_msg(progname, "<%s>: Could not copy data to: %s\n", fnamein, fnameout);
  goto out;
 }

/*-------------------------------------------------------------------------
 * do the copy of referenced objects in a second sweep if needed (references 
 *  detected in the first traversal)
 *-------------------------------------------------------------------------
 */
 if (do_create_refs){
  if(do_copy_refobjs(fidin,fidout,travt,options)<0) {
   error_msg(progname, "<%s>: Could not copy data to: %s\n", fnamein, fnameout);
   goto out;
  }
 }

/*-------------------------------------------------------------------------
 * create hard links
 *-------------------------------------------------------------------------
 */

 if(do_hardlinks(fidout,travt)<0) {
  error_msg(progname, "<%s>: Could not copy data to: %s\n", fnamein, fnameout);
  goto out;
 }

 /* free table */
 trav_table_free(travt);

/*-------------------------------------------------------------------------
 * close
 *-------------------------------------------------------------------------
 */

 H5Fclose(fidin);
 H5Fclose(fidout);
 return 0;

/*-------------------------------------------------------------------------
 * out
 *-------------------------------------------------------------------------
 */

out:
 H5E_BEGIN_TRY {
  H5Fclose(fidin);
  H5Fclose(fidout);
 } H5E_END_TRY;
 if (travt)
  trav_table_free(travt);

 return -1;
}

/*-------------------------------------------------------------------------
 * Function: do_copy_objects
 *
 * Purpose: duplicate all HDF5 objects in the file
 *
 * Return: 0, ok, -1 no
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: October, 23, 2003
 *
 * Modifications: 
 *
 *  July 2004:     Introduced the extra EC or NN option for SZIP
 *
 *  December 2004: Added a check for H5Dcreate; if the dataset cannot be created
 *                  with the requested filter, use the input one
 *
 *  October 2006:  Read/write using the file type by default.
 *                 Read/write by hyperslabs for big datasets.
 *
 *  November 2006:  Use H5Ocopy in the copy of objects. The logic for using 
 *   H5Ocopy or not is if the input DCPL has filters or non default layout or these are 
 *   requested by the user then use read/write else use H5Ocopy. 
 *   A detection is made for the cases where the recreation of references is needed
 *   in a second sweep of the file
 *
 *-------------------------------------------------------------------------
 */

int do_copy_objects(hid_t fidin,
                    hid_t fidout,
                    trav_table_t *travt,
                    pack_opt_t *options) /* repack options */
{
 hid_t    grp_in=-1;         /* group ID */
 hid_t    grp_out=-1;        /* group ID */
 hid_t    dset_in=-1;        /* read dataset ID */
 hid_t    dset_out=-1;       /* write dataset ID */
 hid_t    type_in=-1;        /* named type ID */
 hid_t    type_out=-1;       /* named type ID */
 hid_t    dcpl_id=-1;        /* dataset creation property list ID */
 hid_t    dcpl_out=-1;       /* dataset creation property list ID */
 hid_t    f_space_id=-1;     /* file space ID */
 hid_t    ftype_id=-1;       /* file type ID */
 hid_t    wtype_id=-1;       /* read/write type ID */
 size_t   msize;          /* size of type */
 hsize_t  nelmts;         /* number of elements in dataset */
 int      rank;           /* rank of dataset */
 hsize_t  dims[H5S_MAX_RANK];/* dimensions of dataset */
 hsize_t  dsize_in;       /* input dataset size before filter */
 hsize_t  dsize_out;      /* output dataset size after filter */
 int      next;           /* external files */
 int      apply_s;        /* flag for apply filter to small dataset sizes */
 int      apply_f;        /* flag for apply filter to return error on H5Dcreate */
 double   per;            /* percent utilization of storage */
 void     *buf=NULL;      /* buffer for raw data */
 void     *sm_buf=NULL;   /* buffer for raw data */
 unsigned i;

/*-------------------------------------------------------------------------
 * copy the suppplied object list
 *-------------------------------------------------------------------------
 */

 if (options->verbose) {
  printf("-----------------------------------------\n");
  printf(" Type     Filter (Ratio)     Name\n");
  printf("-----------------------------------------\n");
 }

 for ( i = 0; i < travt->nobjs; i++)
 {

  buf = NULL;
  do_create_refs = 0;

  switch ( travt->objs[i].type )
  {
/*-------------------------------------------------------------------------
 * H5G_GROUP
 *-------------------------------------------------------------------------
 */
  case H5G_GROUP:
   if (options->verbose)
    printf(FORMAT_OBJ,"group",travt->objs[i].name );

   if ((grp_out=H5Gcreate(fidout,travt->objs[i].name, 0))<0)
    goto error;

   if((grp_in = H5Gopen (fidin,travt->objs[i].name))<0)
    goto error;

   /*-------------------------------------------------------------------------
    * copy attrs
    *-------------------------------------------------------------------------
    */
   if (copy_attr(grp_in,grp_out,options)<0)
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
   
/*-------------------------------------------------------------------------
 * check if we should use H5Ocopy or not
 * if there is a request for filters/layout, we read/write the object
 * otherwise we do a copy using H5Ocopy
 *-------------------------------------------------------------------------
 */
   if (options->op_tbl->nelems
       /* do we have input request for filter/chunk ? */ 
       ||
       options->all_filter==1 || options->all_layout==1 
       )
   {
    int j;

    if ((dset_in=H5Dopen(fidin,travt->objs[i].name))<0)
     goto error;
    if ((f_space_id=H5Dget_space(dset_in))<0)
     goto error;
    if ((ftype_id=H5Dget_type (dset_in))<0)
     goto error;
    if ((dcpl_id=H5Dget_create_plist(dset_in))<0)
     goto error;
    if ((dcpl_out = H5Pcopy (dcpl_id))<0)
     goto error;
    if ( (rank=H5Sget_simple_extent_ndims(f_space_id))<0)
     goto error;
    HDmemset(dims, 0, sizeof dims);
    if ( H5Sget_simple_extent_dims(f_space_id,dims,NULL)<0)
     goto error;
    nelmts=1;
    for (j=0; j<rank; j++)
     nelmts*=dims[j];
    
    if (options->use_native==1)
     wtype_id = h5tools_get_native_type(ftype_id);
    else
     wtype_id = H5Tcopy(ftype_id); 
    
    if ((msize=H5Tget_size(wtype_id))==0)
     goto error;
    
    /*-------------------------------------------------------------------------
     * check for external files
     *-------------------------------------------------------------------------
     */
    if ((next=H5Pget_external_count (dcpl_id))<0)
     goto error;
    
    if (next)
     fprintf(stderr," <warning: %s has external files, ignoring read...>\n",
     travt->objs[i].name );
    
     /*-------------------------------------------------------------------------
     * check if the dataset creation property list has filters that
     * are not registered in the current configuration
     * 1) the external filters GZIP and SZIP might not be available
     * 2) the internal filters might be turned off
     *-------------------------------------------------------------------------
    */
    if (next==0 && h5tools_canreadf((travt->objs[i].name),dcpl_id)==1)
    {
     apply_s=1;
     apply_f=1;
     
    /*-------------------------------------------------------------------------
     * references are a special case
     * we cannot just copy the buffers, but instead we recreate the reference
     * in a second traversal of the output file
     *-------------------------------------------------------------------------
     */
     if (H5T_REFERENCE==H5Tget_class(wtype_id))
     {
         do_create_refs = 1;
     }
     else /* H5T_REFERENCE */
     {
      /* get the storage size of the input dataset */
      dsize_in=H5Dget_storage_size(dset_in);
      
      /* check for datasets too small */
      if (nelmts*msize < options->threshold )
       apply_s=0;
      
      /* apply the filter */
      if (apply_s){
       if (apply_filters(travt->objs[i].name,rank,dims,dcpl_out,options)<0)
        goto error;
      }
      
     /*-------------------------------------------------------------------------
      * create the output dataset;
      * disable error checking in case the dataset cannot be created with the
      * modified dcpl; in that case use the original instead
      *-------------------------------------------------------------------------
      */
      H5E_BEGIN_TRY {
       dset_out=H5Dcreate(fidout,travt->objs[i].name,wtype_id,f_space_id,dcpl_out);
      } H5E_END_TRY;
      if (dset_out==FAIL)
      {
       if ((dset_out=H5Dcreate(fidout,travt->objs[i].name,wtype_id,f_space_id,dcpl_id))<0)
        goto error;
       apply_f=0;
      }
      
     /*-------------------------------------------------------------------------
      * read/write
      *-------------------------------------------------------------------------
      */
      if (nelmts)
      {
       size_t need = (size_t)(nelmts*msize);  /* bytes needed */
       if ( need < H5TOOLS_MALLOCSIZE )
        buf = HDmalloc(need);
       
       if (buf != NULL )
       {
        if (H5Dread(dset_in,wtype_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,buf)<0)
         goto error;
        if (H5Dwrite(dset_out,wtype_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,buf)<0)
         goto error;
       }      
       
       else /* possibly not enough memory, read/write by hyperslabs */
        
       {
        size_t        p_type_nbytes = msize; /*size of memory type */
        hsize_t       p_nelmts = nelmts;     /*total selected elmts */
        hsize_t       elmtno;                /*counter  */
        int           carry;                 /*counter carry value */
        unsigned int  vl_data = 0;           /*contains VL datatypes */
        
        /* stripmine info */
        hsize_t       sm_size[H5S_MAX_RANK]; /*stripmine size */
        hsize_t       sm_nbytes;             /*bytes per stripmine */
        hsize_t       sm_nelmts;             /*elements per stripmine*/
        hid_t         sm_space;              /*stripmine data space */
        
        /* hyperslab info */
        hsize_t       hs_offset[H5S_MAX_RANK];/*starting offset */
        hsize_t       hs_size[H5S_MAX_RANK];  /*size this pass */
        hsize_t       hs_nelmts;              /*elements in request */
        hsize_t       zero[8];                /*vector of zeros */
        int           k;          
        
        /* check if we have VL data in the dataset's datatype */
        if (H5Tdetect_class(wtype_id, H5T_VLEN) == TRUE)
         vl_data = TRUE;
        
        /*
         * determine the strip mine size and allocate a buffer. The strip mine is
         * a hyperslab whose size is manageable.
         */
        sm_nbytes = p_type_nbytes;
        
        for (k = rank; k > 0; --k) {
         sm_size[k - 1] = MIN(dims[k - 1], H5TOOLS_BUFSIZE / sm_nbytes);
         sm_nbytes *= sm_size[k - 1];
         assert(sm_nbytes > 0);
        }
        sm_buf = HDmalloc((size_t)sm_nbytes);
        
        sm_nelmts = sm_nbytes / p_type_nbytes;
        sm_space = H5Screate_simple(1, &sm_nelmts, NULL);
        
        /* the stripmine loop */
        memset(hs_offset, 0, sizeof hs_offset);
        memset(zero, 0, sizeof zero);
        
        for (elmtno = 0; elmtno < p_nelmts; elmtno += hs_nelmts) 
        {
         /* calculate the hyperslab size */
         if (rank > 0) 
         {
          for (k = 0, hs_nelmts = 1; k < rank; k++) 
          {
           hs_size[k] = MIN(dims[k] - hs_offset[k], sm_size[k]);
           hs_nelmts *= hs_size[k];
          }
          
          if (H5Sselect_hyperslab(f_space_id, H5S_SELECT_SET, hs_offset, NULL, hs_size, NULL)<0)
           goto error;
          if (H5Sselect_hyperslab(sm_space, H5S_SELECT_SET, zero, NULL, &hs_nelmts, NULL)<0)
           goto error;
         } 
         else 
         {
          H5Sselect_all(f_space_id);
          H5Sselect_all(sm_space);
          hs_nelmts = 1;
         } /* rank */
         
         /* read/write */
         if (H5Dread(dset_in, wtype_id, sm_space, f_space_id, H5P_DEFAULT, sm_buf) < 0) 
          goto error;
         if (H5Dwrite(dset_out, wtype_id, sm_space, f_space_id, H5P_DEFAULT, sm_buf) < 0) 
          goto error;
         
         /* reclaim any VL memory, if necessary */
         if(vl_data)
          H5Dvlen_reclaim(wtype_id, sm_space, H5P_DEFAULT, sm_buf);
         
         /* calculate the next hyperslab offset */
         for (k = rank, carry = 1; k > 0 && carry; --k) 
         {
          hs_offset[k - 1] += hs_size[k - 1];
          if (hs_offset[k - 1] == dims[k - 1])
           hs_offset[k - 1] = 0;
          else
           carry = 0;
         } /* k */
        } /* elmtno */
        
        H5Sclose(sm_space);
        /* free */
        if (sm_buf!=NULL)
        {
         HDfree(sm_buf);
         sm_buf=NULL;
        }
       } /* hyperslab read */
    }/*nelmts*/
    
    /*-------------------------------------------------------------------------
     * amount of compression used
     *-------------------------------------------------------------------------
     */
     if (options->verbose) 
     {
      if (apply_s && apply_f)
      {
       /* get the storage size of the input dataset */
       dsize_out=H5Dget_storage_size(dset_out);
       PER((hssize_t)dsize_in,(hssize_t)dsize_out);
       print_dataset_info(dcpl_out,travt->objs[i].name,per*100.0);
      }
      else
       print_dataset_info(dcpl_id,travt->objs[i].name,0.0);
     }
     
     if (apply_s==0 && options->verbose)
      printf(" <warning: filter not applied to %s. dataset smaller than %d bytes>\n",
      travt->objs[i].name,
      (int)options->threshold);
     
     if (apply_f==0 && options->verbose)
      printf(" <warning: could not apply the filter to %s>\n",
      travt->objs[i].name);
     
     /*-------------------------------------------------------------------------
      * copy attrs
      *-------------------------------------------------------------------------
      */
     if (copy_attr(dset_in,dset_out,options)<0)
      goto error;
     
     /*close */
     if (H5Dclose(dset_out)<0)
      goto error;
     
    }/*H5T_REFERENCE*/
   }/*h5tools_canreadf*/
   
   
   /*-------------------------------------------------------------------------
    * close
    *-------------------------------------------------------------------------
    */
    if (H5Tclose(ftype_id)<0)
     goto error;
    if (H5Tclose(wtype_id)<0)
     goto error;
    if (H5Pclose(dcpl_id)<0)
     goto error;
    if (H5Pclose(dcpl_out)<0)
     goto error;
    if (H5Sclose(f_space_id)<0)
     goto error;
    if (H5Dclose(dset_in)<0)
     goto error;


    }
  /*-------------------------------------------------------------------------
   * we do not have request for filter/chunking use H5Ocopy instead
   *-------------------------------------------------------------------------
   */
    else 
    {
     hid_t        pid;
     
     /* create property to pass copy options */
     if ( (pid = H5Pcreate(H5P_OBJECT_COPY)) < 0) 
      goto error;
     
     /*-------------------------------------------------------------------------
      * do the copy
      *-------------------------------------------------------------------------
      */
     
     if (H5Ocopy(fidin,          /* Source file or group identifier */
      travt->objs[i].name,       /* Name of the source object to be copied */
      fidout,                    /* Destination file or group identifier  */
      travt->objs[i].name,       /* Name of the destination object  */
      pid,                       /* Properties which apply to the copy   */
      H5P_DEFAULT)<0)            /* Properties which apply to the new hard link */              
      goto error;
     
     /* close property */
     if (H5Pclose(pid)<0)
      goto error;
     
    } /* end do we have request for filter/chunking */


   break;

/*-------------------------------------------------------------------------
 * H5G_TYPE
 *-------------------------------------------------------------------------
 */
  case H5G_TYPE:

   if ((type_in = H5Topen (fidin,travt->objs[i].name))<0)
    goto error;

   if ((type_out = H5Tcopy(type_in))<0)
    goto error;

   if ((H5Tcommit(fidout,travt->objs[i].name,type_out))<0)
    goto error;

/*-------------------------------------------------------------------------
 * copy attrs
 *-------------------------------------------------------------------------
 */
   if (copy_attr(type_in,type_out,options)<0)
    goto error;

   if (H5Tclose(type_in)<0)
    goto error;
   if (H5Tclose(type_out)<0)
    goto error;

   if (options->verbose)
    printf(FORMAT_OBJ,"type",travt->objs[i].name );

   break;


/*-------------------------------------------------------------------------
 * H5G_LINK
 * H5G_UDLINK
 *
 * Only handles external links; H5Lcopy will fail for other UD link types
 * since we don't have creation or copy callbacks for them.
 *-------------------------------------------------------------------------
 */

  case H5G_LINK:
  case H5G_UDLINK:
   {
    if(H5Lcopy(fidin, travt->objs[i].name,fidout, travt->objs[i].name, H5P_DEFAULT, H5P_DEFAULT) < 0)
        goto error;

    if (options->verbose)
     printf(FORMAT_OBJ,"link",travt->objs[i].name );

   }
   break;

  default:
   goto error;
  } /* switch */

  /* free */
  if (buf!=NULL)
  {
   HDfree(buf);
   buf=NULL;
  }

 } /* i */

/*-------------------------------------------------------------------------
 * the root is a special case, we get an ID for the root group
 * and copy its attributes using that ID
 *-------------------------------------------------------------------------
 */

 if ((grp_out = H5Gopen(fidout,"/"))<0)
  goto error;

 if ((grp_in  = H5Gopen(fidin,"/"))<0)
  goto error;

 if (copy_attr(grp_in,grp_out,options)<0)
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
  H5Sclose(f_space_id);
  H5Dclose(dset_in);
  H5Dclose(dset_out);
  H5Tclose(ftype_id);
  H5Tclose(wtype_id);
  H5Tclose(type_in);
  H5Tclose(type_out);
  /* free */
  if (buf!=NULL)
  {
   HDfree(buf);
   buf=NULL;
  }
  if (sm_buf!=NULL)
  {
   HDfree(sm_buf);
   sm_buf=NULL;
  }
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
              pack_opt_t *options
              )
{
 hid_t      attr_id=-1;      /* attr ID */
 hid_t      attr_out=-1;     /* attr ID */
 hid_t      space_id=-1;     /* space ID */
 hid_t      ftype_id=-1;     /* file type ID */
 hid_t      wtype_id=-1;     /* read/write type ID */
 size_t     msize;        /* size of type */
 void       *buf=NULL;    /* data buffer */
 hsize_t    nelmts;       /* number of elements in dataset */
 int        rank;         /* rank of dataset */
 hsize_t    dims[H5S_MAX_RANK];/* dimensions of dataset */
 char       name[255];
 int        n, j;
 unsigned   u;

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

  if (options->use_native==1)
   wtype_id = h5tools_get_native_type(ftype_id);
  else
   wtype_id = H5Tcopy(ftype_id); 

  if ((msize=H5Tget_size(wtype_id))==0)
   goto error;

/*-------------------------------------------------------------------------
 * object references are a special case
 * we cannot just copy the buffers, but instead we recreate the reference
 * this is done on a second sweep of the file that just copies
 * the referenced objects
 *-------------------------------------------------------------------------
 */
  if ( ! H5Tequal(wtype_id, H5T_STD_REF_OBJ))
  {
 /*-------------------------------------------------------------------------
  * read to memory
  *-------------------------------------------------------------------------
  */

   buf=(void *) HDmalloc((unsigned)(nelmts*msize));
   if ( buf==NULL){
    error_msg(progname, "cannot read into memory\n" );
    goto error;
   }
   if (H5Aread(attr_id,wtype_id,buf)<0)
    goto error;

   /*-------------------------------------------------------------------------
    * copy
    *-------------------------------------------------------------------------
    */

   if ((attr_out=H5Acreate(loc_out,name,ftype_id,space_id,H5P_DEFAULT))<0)
    goto error;
   if(H5Awrite(attr_out,wtype_id,buf)<0)
    goto error;

   /*close*/
   if (H5Aclose(attr_out)<0)
    goto error;


   if (buf)
    free(buf);


  } /*H5T_STD_REF_OBJ*/


  if (options->verbose)
   printf(FORMAT_OBJ_ATTR, "attr", name);

/*-------------------------------------------------------------------------
 * close
 *-------------------------------------------------------------------------
 */

  if (H5Tclose(ftype_id)<0) goto error;
  if (H5Tclose(wtype_id)<0) goto error;
  if (H5Sclose(space_id)<0) goto error;
  if (H5Aclose(attr_id)<0) goto error;

 } /* u */

  return 0;

error:
 H5E_BEGIN_TRY {
  H5Tclose(ftype_id);
  H5Tclose(wtype_id);
  H5Sclose(space_id);
  H5Aclose(attr_id);
  H5Aclose(attr_out);
  if (buf)
    free(buf);
 } H5E_END_TRY;
 return -1;
}

/*-------------------------------------------------------------------------
 * Function: do_hardlinks
 *
 * Purpose: duplicate hard links
 *
 * Return: 0, ok, -1 no
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: December, 10, 2003
 *
 *-------------------------------------------------------------------------
 */

static
int do_hardlinks(hid_t fidout,trav_table_t *travt)
{
 unsigned int i, j;

 for ( i = 0; i < travt->nobjs; i++)
 {
  switch ( travt->objs[i].type )
  {
  case H5G_GROUP:

   if (travt->objs[i].nlinks)
   {
    for ( j=0; j<travt->objs[i].nlinks; j++)
    {
      if (H5Glink(fidout,
      H5L_TYPE_HARD,
      travt->objs[i].name,
      travt->objs[i].links[j].new_name)<0)
      return -1;
    }
   }

   break;

  case H5G_DATASET:

   if (travt->objs[i].nlinks)
   {
    for ( j=0; j<travt->objs[i].nlinks; j++){
     if (H5Glink(fidout,
      H5L_TYPE_HARD,
      travt->objs[i].name,
      travt->objs[i].links[j].new_name)<0)
      return -1;
    }
   }

   break;

  case H5G_TYPE:
  case H5G_LINK:
  case H5G_UDLINK:

   /*nothing to do */
   break;

  default:

   break;
  }
 }

 return 0;

}



/*-------------------------------------------------------------------------
 * Function: do_copy_refobjs
 *
 * Purpose: duplicate all referenced HDF5 objects in the file in a second traversal
 *
 * Return: 0, ok, -1 no
 *
 * Programmer: Pedro Vicente, pvn@ncsa.uiuc.edu
 *
 * Date: December, 10, 2003
 *
 *-------------------------------------------------------------------------
 */

static
int do_copy_refobjs(hid_t fidin,
                    hid_t fidout,
                    trav_table_t *travt,
                    pack_opt_t *options) /* repack options */
{
 hid_t     grp_in=-1;            /* read group ID */
 hid_t     grp_out=-1;           /* write group ID */
 hid_t     dset_in=-1;           /* read dataset ID */
 hid_t     dset_out=-1;          /* write dataset ID */
 hid_t     type_in=-1;           /* named type ID */
 hid_t     dcpl_id=-1;           /* dataset creation property list ID */
 hid_t     space_id=-1;          /* space ID */
 hid_t     ftype_id=-1;          /* file type ID */
 hid_t     wtype_id=-1;          /* read/write type ID */
 size_t    msize;             /* size of type */
 hsize_t   nelmts;            /* number of elements in dataset */
 int       rank;              /* rank of dataset */
 hsize_t   dims[H5S_MAX_RANK];/* dimensions of dataset */
 int       next;              /* external files */
 unsigned  i;
 int j;

/*-------------------------------------------------------------------------
 * browse
 *-------------------------------------------------------------------------
 */

 for ( i = 0; i < travt->nobjs; i++)
 {
  switch ( travt->objs[i].type )
  {

  case H5G_GROUP:

   break;

  /*-------------------------------------------------------------------------
   * H5G_DATASET
   *-------------------------------------------------------------------------
   */
  case H5G_DATASET:

   if ((dset_in=H5Dopen(fidin,travt->objs[i].name))<0)
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
   
   if (options->use_native==1)
    wtype_id = h5tools_get_native_type(ftype_id);
   else
    wtype_id = H5Tcopy(ftype_id); 

   if ((msize=H5Tget_size(wtype_id))==0)
    goto error;

/*-------------------------------------------------------------------------
 * check for external files
 *-------------------------------------------------------------------------
 */
   if ((next=H5Pget_external_count (dcpl_id))<0)
    goto error;
/*-------------------------------------------------------------------------
 * check if the dataset creation property list has filters that
 * are not registered in the current configuration
 * 1) the external filters GZIP and SZIP might not be available
 * 2) the internal filters might be turned off
 *-------------------------------------------------------------------------
 */
   if (next==0 && h5tools_canreadf((NULL),dcpl_id)==1)
   {
/*-------------------------------------------------------------------------
 * test for a valid output dataset
 *-------------------------------------------------------------------------
 */
   dset_out = FAIL;

/*-------------------------------------------------------------------------
 * object references are a special case
 * we cannot just copy the buffers, but instead we recreate the reference
 *-------------------------------------------------------------------------
 */
   if (H5Tequal(wtype_id, H5T_STD_REF_OBJ))
   {
    H5G_obj_t        obj_type;
    hid_t            refobj_id;
    hobj_ref_t       *refbuf=NULL; /* buffer for object references */
    hobj_ref_t       *buf=NULL;
    const char*      refname;
    unsigned         u;

   /*-------------------------------------------------------------------------
    * read to memory
    *-------------------------------------------------------------------------
    */

    if (nelmts)
    {
     buf=(void *) HDmalloc((unsigned)(nelmts*msize));
     if ( buf==NULL){
      error_msg(progname, "cannot read into memory\n" );
      goto error;
     }
     if (H5Dread(dset_in,wtype_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,buf)<0)
      goto error;

     if ((obj_type = H5Rget_obj_type(dset_in,H5R_OBJECT,buf))<0)
      goto error;
     refbuf=HDmalloc((unsigned)nelmts*msize);
     if ( refbuf==NULL){
      error_msg(progname, "cannot allocate memory\n" );
      goto error;
     }
     for ( u=0; u<nelmts; u++)
     {
      H5E_BEGIN_TRY {
       if ((refobj_id = H5Rdereference(dset_in,H5R_OBJECT,&buf[u]))<0)
        continue;
      } H5E_END_TRY;
      /* get the name. a valid name could only occur in the
      second traversal of the file */
      if ((refname=MapIdToName(refobj_id,travt))!=NULL)
      {
       /* create the reference, -1 parameter for objects */
       if (H5Rcreate(&refbuf[u],fidout,refname,H5R_OBJECT,-1)<0)
        goto error;
       if (options->verbose)
        printf("object <%s> object reference created to <%s>\n",
        travt->objs[i].name,
        refname);
      }/*refname*/
      close_obj(obj_type,refobj_id);
     }/*  u */
    }/*nelmts*/

    /*-------------------------------------------------------------------------
     * create/write dataset/close
     *-------------------------------------------------------------------------
     */
    if ((dset_out=H5Dcreate(fidout,travt->objs[i].name,wtype_id,space_id,dcpl_id))<0)
     goto error;
    if (nelmts) {
     if (H5Dwrite(dset_out,wtype_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,refbuf)<0)
      goto error;
    }

    if (buf)
     free(buf);
    if (refbuf)
     free(refbuf);

   }/*H5T_STD_REF_OBJ*/

/*-------------------------------------------------------------------------
 * dataset region references
 *-------------------------------------------------------------------------
 */
   else if (H5Tequal(wtype_id, H5T_STD_REF_DSETREG))
   {
    H5G_obj_t        obj_type;
    hid_t            refobj_id;
    hdset_reg_ref_t  *refbuf=NULL; /* input buffer for region references */
    hdset_reg_ref_t  *buf=NULL;    /* output buffer */
    const char*      refname;
    unsigned         u;

   /*-------------------------------------------------------------------------
    * read input to memory
    *-------------------------------------------------------------------------
    */
    if (nelmts)
    {
     buf=(void *) HDmalloc((unsigned)(nelmts*msize));
     if ( buf==NULL){
      error_msg(progname, "cannot read into memory\n" );
      goto error;
     }
     if (H5Dread(dset_in,wtype_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,buf)<0)
      goto error;
     if ((obj_type = H5Rget_obj_type(dset_in,H5R_DATASET_REGION,buf))<0)
      goto error;

     /*-------------------------------------------------------------------------
      * create output
      *-------------------------------------------------------------------------
      */

     refbuf=HDcalloc(sizeof(hdset_reg_ref_t),(size_t)nelmts); /*init to zero */
     if ( refbuf==NULL){
      error_msg(progname, "cannot allocate memory\n" );
      goto error;
     }
     for ( u=0; u<nelmts; u++)
     {
      H5E_BEGIN_TRY {
       if ((refobj_id = H5Rdereference(dset_in,H5R_DATASET_REGION,&buf[u]))<0)
        continue;
      } H5E_END_TRY;

      /* get the name. a valid name could only occur in the
      second traversal of the file */
      if ((refname=MapIdToName(refobj_id,travt))!=NULL)
      {
       hid_t region_id;    /* region id of the referenced dataset */
       if ((region_id = H5Rget_region(dset_in,H5R_DATASET_REGION,&buf[u]))<0)
        goto error;
       /* create the reference, we need the space_id */
       if (H5Rcreate(&refbuf[u],fidout,refname,H5R_DATASET_REGION,region_id)<0)
        goto error;
       if (H5Sclose(region_id)<0)
        goto error;
       if (options->verbose)
        printf("object <%s> region reference created to <%s>\n",
        travt->objs[i].name,
        refname);
      }/*refname*/
      close_obj(obj_type,refobj_id);
     }/*  u */
    }/*nelmts*/

    /*-------------------------------------------------------------------------
     * create/write dataset/close
     *-------------------------------------------------------------------------
     */
    if ((dset_out=H5Dcreate(fidout,travt->objs[i].name,wtype_id,space_id,dcpl_id))<0)
     goto error;
    if (nelmts) {
     if (H5Dwrite(dset_out,wtype_id,H5S_ALL,H5S_ALL,H5P_DEFAULT,refbuf)<0)
      goto error;
    }

    if (buf)
     free(buf);
    if (refbuf)
     free(refbuf);
   } /* H5T_STD_REF_DSETREG */


/*-------------------------------------------------------------------------
 * not references, open previously created object in 1st traversal
 *-------------------------------------------------------------------------
 */
   else
   {
    if ((dset_out=H5Dopen(fidout,travt->objs[i].name))<0)
     goto error;
   }

   assert(dset_out!=FAIL);

/*-------------------------------------------------------------------------
 * copy referenced objects in attributes
 *-------------------------------------------------------------------------
 */
   if (copy_refs_attr(dset_in,dset_out,options,travt,fidout)<0)
    goto error;



   if (H5Dclose(dset_out)<0)
    goto error;

   }/*can_read*/

   /*-------------------------------------------------------------------------
    * close
    *-------------------------------------------------------------------------
    */

   if (H5Tclose(ftype_id)<0)
    goto error;
   if (H5Tclose(wtype_id)<0)
    goto error;
   if (H5Pclose(dcpl_id)<0)
    goto error;
   if (H5Sclose(space_id)<0)
    goto error;
   if (H5Dclose(dset_in)<0)
    goto error;


   break;


  case H5G_TYPE:
  case H5G_LINK:
  case H5G_UDLINK:

   /*nothing to do */
   break;

  default:

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

 if (copy_refs_attr(grp_in,grp_out,options,travt,fidout)<0)
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
  H5Tclose(wtype_id);
  H5Tclose(type_in);
 } H5E_END_TRY;
 return -1;

}



/*-------------------------------------------------------------------------
 * Function: copy_refs_attr
 *
 * Purpose: duplicate all referenced HDF5 objects located in attributes
 *  relative to LOC_IN, which is obtained either from
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

static int copy_refs_attr(hid_t loc_in,
                          hid_t loc_out,
                          pack_opt_t *options,
                          trav_table_t *travt,
                          hid_t fidout         /* for saving references */
                          )
{
 hid_t      attr_id=-1;      /* attr ID */
 hid_t      attr_out=-1;     /* attr ID */
 hid_t      space_id=-1;     /* space ID */
 hid_t      ftype_id=-1;     /* file type ID */
 hid_t      wtype_id=-1;     /* read/write type ID */
 size_t     msize;           /* size of type */
 hsize_t    nelmts;          /* number of elements in dataset */
 int        rank;            /* rank of dataset */
 hsize_t    dims[H5S_MAX_RANK];/* dimensions of dataset */
 char       name[255];
 int        n, j;
 unsigned   u;

 if ((n = H5Aget_num_attrs(loc_in))<0)
  goto error;

 for ( u = 0; u < (unsigned)n; u++)
 {

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


  /*-------------------------------------------------------------------------
   * elements
   *-------------------------------------------------------------------------
   */
  nelmts=1;
  for (j=0; j<rank; j++)
   nelmts*=dims[j];

  if (options->use_native==1)
   wtype_id = h5tools_get_native_type(ftype_id);
  else
   wtype_id = H5Tcopy(ftype_id); 

  if ((msize=H5Tget_size(wtype_id))==0)
   goto error;

/*-------------------------------------------------------------------------
 * object references are a special case
 * we cannot just copy the buffers, but instead we recreate the reference
 *-------------------------------------------------------------------------
 */
   if (H5Tequal(wtype_id, H5T_STD_REF_OBJ))
   {
    H5G_obj_t   obj_type;
    hid_t       refobj_id;
    hobj_ref_t  *refbuf=NULL;
    unsigned    k;
    const char* refname;
    hobj_ref_t  *buf=NULL;

   /*-------------------------------------------------------------------------
    * read input to memory
    *-------------------------------------------------------------------------
    */

    if (nelmts)
    {
     buf=(void *) HDmalloc((unsigned)(nelmts*msize));
     if ( buf==NULL){
      error_msg(progname, "cannot read into memory\n" );
      goto error;
     }
     if (H5Aread(attr_id,wtype_id,buf)<0)
      goto error;

     if ((obj_type = H5Rget_obj_type(attr_id,H5R_OBJECT,buf))<0)
      goto error;
     refbuf=HDmalloc((unsigned)nelmts*msize);
     if ( refbuf==NULL){
      error_msg(progname, "cannot allocate memory\n" );
      goto error;
     }
     for ( k=0; k<nelmts; k++)
     {
      H5E_BEGIN_TRY {
       if ((refobj_id = H5Rdereference(attr_id,H5R_OBJECT,&buf[k]))<0)
        goto error;
      } H5E_END_TRY;
      /* get the name. a valid name could only occur in the
      second traversal of the file */
      if ((refname=MapIdToName(refobj_id,travt))!=NULL)
      {
       /* create the reference */
       if (H5Rcreate(&refbuf[k],fidout,refname,H5R_OBJECT,-1)<0)
        goto error;
       if (options->verbose)
        printf("object <%s> reference created to <%s>\n",name,refname);
      }
      close_obj(obj_type,refobj_id);
     }/*  k */
    }/*nelmts*/

    /*-------------------------------------------------------------------------
     * copy
     *-------------------------------------------------------------------------
     */

    if ((attr_out=H5Acreate(loc_out,name,ftype_id,space_id,H5P_DEFAULT))<0)
     goto error;
    if (nelmts) {
     if(H5Awrite(attr_out,wtype_id,refbuf)<0)
      goto error;
    }

    if (H5Aclose(attr_out)<0)
     goto error;

    if (refbuf)
     free(refbuf);
    if (buf)
     free(buf);

   }/*H5T_STD_REF_OBJ*/

/*-------------------------------------------------------------------------
 * dataset region references
 *-------------------------------------------------------------------------
 */
   else if (H5Tequal(wtype_id, H5T_STD_REF_DSETREG))
   {
    H5G_obj_t        obj_type;
    hid_t            refobj_id;
    hdset_reg_ref_t  *refbuf=NULL; /* input buffer for region references */
    hdset_reg_ref_t  *buf=NULL;    /* output buffer */
    const char*      refname;
    unsigned         k;

   /*-------------------------------------------------------------------------
    * read input to memory
    *-------------------------------------------------------------------------
    */

    if (nelmts)
    {
     buf=(void *) HDmalloc((unsigned)(nelmts*msize));
     if ( buf==NULL){
      error_msg(progname, "cannot read into memory\n" );
      goto error;
     }
     if (H5Aread(attr_id,wtype_id,buf)<0)
      goto error;
     if ((obj_type = H5Rget_obj_type(attr_id,H5R_DATASET_REGION,buf))<0)
      goto error;

    /*-------------------------------------------------------------------------
     * create output
     *-------------------------------------------------------------------------
     */

     refbuf=HDcalloc(sizeof(hdset_reg_ref_t),(size_t)nelmts); /*init to zero */
     if ( refbuf==NULL){
      error_msg(progname, "cannot allocate memory\n" );
      goto error;
     }
     for ( k=0; k<nelmts; k++)
     {
      H5E_BEGIN_TRY {
       if ((refobj_id = H5Rdereference(attr_id,H5R_DATASET_REGION,&buf[k]))<0)
        continue;
      } H5E_END_TRY;
      /* get the name. a valid name could only occur in the
      second traversal of the file */
      if ((refname=MapIdToName(refobj_id,travt))!=NULL)
      {
       hid_t region_id;    /* region id of the referenced dataset */
       if ((region_id = H5Rget_region(attr_id,H5R_DATASET_REGION,&buf[k]))<0)
        goto error;
       /* create the reference, we need the space_id */
       if (H5Rcreate(&refbuf[k],fidout,refname,H5R_DATASET_REGION,region_id)<0)
        goto error;
       if (H5Sclose(region_id)<0)
        goto error;
       if (options->verbose)
        printf("object <%s> region reference created to <%s>\n",name,refname);
      }
      close_obj(obj_type,refobj_id);
     }/*  k */
    }/*nelmts */

    /*-------------------------------------------------------------------------
     * copy
     *-------------------------------------------------------------------------
     */

    if ((attr_out=H5Acreate(loc_out,name,ftype_id,space_id,H5P_DEFAULT))<0)
     goto error;
    if (nelmts) {
     if(H5Awrite(attr_out,wtype_id,refbuf)<0)
      goto error;
    }
    if (H5Aclose(attr_out)<0)
     goto error;
    if (refbuf)
     free(refbuf);
    if (buf)
     free(buf);
   } /* H5T_STD_REF_DSETREG */

/*-------------------------------------------------------------------------
 * close
 *-------------------------------------------------------------------------
 */

  if (H5Tclose(ftype_id)<0) goto error;
  if (H5Tclose(wtype_id)<0) goto error;
  if (H5Sclose(space_id)<0) goto error;
  if (H5Aclose(attr_id)<0) goto error;
 } /* u */

  return 0;

error:
 H5E_BEGIN_TRY {
  H5Tclose(ftype_id);
  H5Tclose(wtype_id);
  H5Sclose(space_id);
  H5Aclose(attr_id);
  H5Aclose(attr_out);
 } H5E_END_TRY;
 return -1;
}

/*-------------------------------------------------------------------------
 * Function: close_obj
 *
 * Purpose: Auxiliary function to close an object
 *
 *-------------------------------------------------------------------------
 */

static void close_obj(H5G_obj_t obj_type, hid_t obj_id)
{
 H5E_BEGIN_TRY
 {
  switch (obj_type)
  {
  case H5G_GROUP:
   H5Gclose(obj_id);
   break;
  case H5G_DATASET:
   H5Dclose(obj_id);
   break;
  case H5G_TYPE:
   H5Tclose(obj_id);
   break;
  default:
   break;
  }
 } H5E_END_TRY;
}

/*-------------------------------------------------------------------------
 * Function: MapIdToName
 *
 * Purpose: map an object ID to a name
 *
 *-------------------------------------------------------------------------
 */

static const char* MapIdToName(hid_t refobj_id,
                               trav_table_t *travt)
{
 hid_t id;
 hid_t fid;
 H5G_stat_t refstat;    /* Stat for the refobj id */
 H5G_stat_t objstat;    /* Stat for objects in the file */
 unsigned int   i;

 /* obtain information to identify the referenced object uniquely */
 if(H5Gget_objinfo(refobj_id, ".", 0, &refstat) <0)
  return NULL;

 /* obtains the file ID given an object ID.  This ID must be closed */
 if ((fid = H5Iget_file_id(refobj_id))<0)
 {
  return NULL;
 }

 /* linear search */
 for ( i=0; i<travt->nobjs; i++)
 {
  switch ( travt->objs[i].type )
  {
  default:
   break;

  /*-------------------------------------------------------------------------
   * H5G_DATASET
   *-------------------------------------------------------------------------
   */

  case H5G_DATASET:

   if ((id = H5Dopen(fid,travt->objs[i].name))<0)
    return NULL;
   if(H5Gget_objinfo(id, ".", 0, &objstat) <0)
    return NULL;
   if (H5Dclose(id)<0)
    return NULL;
   if (!HDmemcmp(&refstat.fileno, &objstat.fileno, sizeof(refstat.fileno)) && !HDmemcmp(&refstat.objno, &objstat.objno, sizeof(refstat.objno)))
   {
    H5Fclose(fid);
    return travt->objs[i].name;
   }
   break;
  }  /* switch */
 } /* i */

 if (H5Fclose(fid)<0)
  return NULL;

 return NULL;
}

