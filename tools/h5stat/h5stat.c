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
#include "H5private.h"    /* Generic Functions      */
#include "h5tools.h"
#include "h5tools_utils.h"
#include "h5tools_ref.h"
#include "h5trav.h"
#include "hdf5.h"

/* Name of tool */
#define PROGRAMNAME "h5stat"

/* Parameters to control statistics gathered */
#define SIZE_SMALL_GROUPS       10
#define SIZE_SMALL_ATTRS  10
#define SIZE_SMALL_DSETS        10
#define SIZE_SMALL_SECTS        10

#define  H5_NFILTERS_IMPL        8     /* Number of currently implemented filters + one to
                                          accommodate for user-define filters + one
                                          to accomodate datasets whithout any filters */

/* File space management strategies: see H5Fpublic.h for declarations */
const char *FS_STRATEGY_NAME[] = {
    "unknown",
    "H5F_FILE_SPACE_ALL_PERSIST",
    "H5F_FILE_SPACE_ALL",
    "H5F_FILE_SPACE_AGGR_VFD",
    "H5F_FILE_SPACE_VFD",
    NULL
};

/* Datatype statistics for datasets */
typedef struct dtype_info_t {
    hid_t tid;                          /* ID of datatype */
    unsigned long count;                /* Number of types found */
    unsigned long named;                /* Number of types that are named */
} dtype_info_t;

typedef struct ohdr_info_t {
    hsize_t total_size;                 /* Total size of object headers */
    hsize_t free_size;                  /* Total free space in object headers */
} ohdr_info_t;

/* Info to pass to the iteration functions */
typedef struct iter_t {
    hid_t fid;                          /* File ID */
    hsize_t filesize;      /* Size of the file */
    unsigned long uniq_groups;          /* Number of unique groups */
    unsigned long uniq_dsets;           /* Number of unique datasets */
    unsigned long uniq_dtypes;          /* Number of unique named datatypes */
    unsigned long uniq_links;           /* Number of unique links */
    unsigned long uniq_others;          /* Number of other unique objects */

    unsigned long max_links;            /* Maximum # of links to an object */
    hsize_t max_fanout;                 /* Maximum fanout from a group */
    unsigned long num_small_groups[SIZE_SMALL_GROUPS];     /* Size of small groups tracked */
    unsigned group_nbins;               /* Number of bins for group counts */
    unsigned long *group_bins;          /* Pointer to array of bins for group counts */
    ohdr_info_t group_ohdr_info;        /* Object header information for groups */

    hsize_t  max_attrs;                     /* Maximum attributes from a group */
    unsigned long num_small_attrs[SIZE_SMALL_ATTRS];    /* Size of small attributes tracked */
    unsigned attr_nbins;                /* Number of bins for attribute counts */
    unsigned long *attr_bins;           /* Pointer to array of bins for attribute counts */

    unsigned max_dset_rank;             /* Maximum rank of dataset */
    unsigned long dset_rank_count[H5S_MAX_RANK];   /* Number of datasets of each rank */
    hsize_t max_dset_dims;              /* Maximum dimension size of dataset */
    unsigned long small_dset_dims[SIZE_SMALL_DSETS];    /* Size of dimensions of small datasets tracked */
    unsigned long dset_layouts[H5D_NLAYOUTS];           /* Type of storage for each dataset */
    unsigned long dset_comptype[H5_NFILTERS_IMPL]; /* Number of currently implemented filters */
    unsigned long dset_ntypes;          /* Number of diff. dataset datatypes found */
    dtype_info_t *dset_type_info;       /* Pointer to dataset datatype information found */
    unsigned dset_dim_nbins;            /* Number of bins for dataset dimensions */
    unsigned long *dset_dim_bins;       /* Pointer to array of bins for dataset dimensions */
    ohdr_info_t dset_ohdr_info;         /* Object header information for datasets */
    hsize_t dset_storage_size;          /* Size of raw data for datasets */
    hsize_t dset_external_storage_size; /* Size of raw data for datasets with external storage */
    ohdr_info_t dtype_ohdr_info;        /* Object header information for datatypes */
    hsize_t groups_btree_storage_size;  /* btree size for group */
    hsize_t groups_heap_storage_size;   /* heap size for group */
    hsize_t attrs_btree_storage_size;   /* btree size for attributes (1.8) */
    hsize_t attrs_heap_storage_size;    /* fractal heap size for attributes (1.8) */
    hsize_t SM_hdr_storage_size;        /* header size for SOHM table (1.8) */
    hsize_t SM_index_storage_size;      /* index (btree & list) size for SOHM table (1.8) */
    hsize_t SM_heap_storage_size;       /* fractal heap size for SOHM table (1.8) */
    hsize_t super_size;                 /* superblock size */
    hsize_t super_ext_size;             /* superblock extension size */
    hsize_t ublk_size;                  /* user block size (if exists) */
    H5F_fs_strategy_t fs_strategy;  	/* File space management strategy */
    hsize_t fs_threshold;               /* Free-space section threshold */
    hsize_t fsp_size;               	/* File space page size */
    hsize_t free_space;                 /* amount of freespace in the file */
    hsize_t free_hdr;                   /* size of free space manager metadata in the file */
    unsigned long num_small_sects[SIZE_SMALL_SECTS];   /* Size of small free-space sections */
    unsigned sect_nbins;                /* Number of bins for free-space section sizes */
    unsigned long *sect_bins;           /* Pointer to array of bins for free-space section sizes */
    hsize_t datasets_index_storage_size;/* meta size for chunked dataset's indexing type */
    hsize_t datasets_heap_storage_size; /* heap size for dataset with external storage */
    unsigned long nexternal;            /* Number of external files for a dataset */
    int           local;                /* Flag to indicate iteration over the object*/
} iter_t;


static int        display_all = TRUE;

/* Enable the printing of selected statistics */
static int        display_file = FALSE;     /* display file information */
static int        display_group = FALSE;    /* display groups information */
static int        display_dset = FALSE;     /* display datasets information */
static int        display_dset_dtype_meta = FALSE;  /* display datasets' datatype information */
static int        display_attr = FALSE;     /* display attributes information */
static int        display_free_sections = FALSE;    /* display free space information */
static int        display_summary = FALSE;  /* display summary of file space information */

static int        display_file_metadata = FALSE;    /* display file space info for file's metadata */
static int        display_group_metadata = FALSE;   /* display file space info for groups' metadata */
static int        display_dset_metadata = FALSE;    /* display file space info for datasets' metadata */

static int        display_object = FALSE;  /* not implemented yet */

/* a structure for handling the order command-line parameters come in */
struct handler_t {
    char *obj;
};


static const char *s_opts ="ADdFfhGgsSTO:V";
/* e.g. "filemetadata" has to precedue "file"; "groupmetadata" has to precede "group" etc. */
static struct long_options l_opts[] = {
    {"help", no_arg, 'h'},
    {"hel", no_arg, 'h'},
    {"he", no_arg, 'h'},
    {"filemetadata", no_arg, 'F'},
    {"filemetadat", no_arg, 'F'},
    {"filemetada", no_arg, 'F'},
    {"filemetad", no_arg, 'F'},
    {"filemeta", no_arg, 'F'},
    {"filemet", no_arg, 'F'},
    {"fileme", no_arg, 'F'},
    {"filem", no_arg, 'F'},
    {"file", no_arg, 'f'},
    {"fil", no_arg, 'f'},
    {"fi", no_arg, 'f'},
    {"groupmetadata", no_arg, 'G'},
    {"groupmetadat", no_arg, 'G'},
    {"groupmetada", no_arg, 'G'},
    {"groupmetad", no_arg, 'G'},
    {"groupmeta", no_arg, 'G'},
    {"groupmet", no_arg, 'G'},
    {"groupme", no_arg, 'G'},
    {"groupm", no_arg, 'G'},
    {"group", no_arg, 'g'},
    {"grou", no_arg, 'g'},
    {"gro", no_arg, 'g'},
    {"gr", no_arg, 'g'},
    {"dsetmetadata", no_arg, 'D'},
    {"dsetmetadat", no_arg, 'D'},
    {"dsetmetada", no_arg, 'D'},
    {"dsetmetad", no_arg, 'D'},
    {"dsetmeta", no_arg, 'D'},
    {"dsetmet", no_arg, 'D'},
    {"dsetme", no_arg, 'D'},
    {"dsetm", no_arg, 'D'},
    {"dset", no_arg, 'd'},
    {"dse", no_arg, 'd'},
    {"ds", no_arg, 'd'},
    {"dtypemetadata", no_arg, 'T'},
    {"dtypemetadat", no_arg, 'T'},
    {"dtypemetada", no_arg, 'T'},
    {"dtypemetad", no_arg, 'T'},
    {"dtypemeta", no_arg, 'T'},
    {"dtypemet", no_arg, 'T'},
    {"dtypeme", no_arg, 'T'},
    {"dtypem", no_arg, 'T'},
    {"dtype", no_arg, 'T'},
    {"dtyp", no_arg, 'T'},
    {"dty", no_arg, 'T'},
    {"dt", no_arg, 'T'},
    { "object", require_arg, 'O' },
    { "objec", require_arg, 'O' },
    { "obje", require_arg, 'O' },
    { "obj", require_arg, 'O' },
    { "ob", require_arg, 'O' },
    { "version", no_arg, 'V' },
    { "versio", no_arg, 'V' },
    { "versi", no_arg, 'V' },
    { "vers", no_arg, 'V' },
    { "ver", no_arg, 'V' },
    { "ve", no_arg, 'V' },
    { "attribute", no_arg, 'A' },
    { "attribut", no_arg, 'A' },
    { "attribu", no_arg, 'A' },
    { "attrib", no_arg, 'A' },
    { "attri", no_arg, 'A' },
    { "attr", no_arg, 'A' },
    { "att", no_arg, 'A' },
    { "at", no_arg, 'A' },
    { "freespace", no_arg, 's' },
    { "freespac", no_arg, 's' },
    { "freespa", no_arg, 's' },
    { "freesp", no_arg, 's' },
    { "frees", no_arg, 's' },
    { "free", no_arg, 's' },
    { "fre", no_arg, 's' },
    { "fr", no_arg, 's' },
    { "summary", no_arg, 'S' },
    { "summar", no_arg, 'S' },
    { "summa", no_arg, 'S' },
    { "summ", no_arg, 'S' },
    { "sum", no_arg, 'S' },
    { "su", no_arg, 'S' },
    { NULL, 0, '\0' }
};

static void
leave(int ret)
{
   h5tools_close();
   HDexit(ret);
}


static void usage(const char *prog)
{
     HDfflush(stdout);
     HDfprintf(stdout, "Usage: %s [OPTIONS] file\n", prog);
     HDfprintf(stdout, "\n");
     HDfprintf(stdout, "      OPTIONS\n");
     HDfprintf(stdout, "     -h, --help            Print a usage message and exit\n");
     HDfprintf(stdout, "     -V, --version         Print version number and exit\n");
     HDfprintf(stdout, "     -f, --file            Print file information\n");
     HDfprintf(stdout, "     -F, --filemetadata    Print file space information for file's metadata\n");
     HDfprintf(stdout, "     -g, --group           Print group information\n");
     HDfprintf(stdout, "     -G, --groupmetadata   Print file space information for groups' metadata\n");
     HDfprintf(stdout, "     -d, --dset            Print dataset information\n");
     HDfprintf(stdout, "     -D, --dsetmetadata    Print file space information for datasets' metadata\n");
     HDfprintf(stdout, "     -T, --dtypemetadata   Print datasets' datatype information\n");
     HDfprintf(stdout, "     -A, --attribute       Print attribute information\n");
     HDfprintf(stdout, "     -s, --freespace       Print free space information\n");
     HDfprintf(stdout, "     -S, --summary         Print summary of file space information\n");
}


/*-------------------------------------------------------------------------
 * Function: ceil_log10
 *
 * Purpose: Compute the ceiling of log_10(x)
 *
 * Return: >0 on success, 0 on failure
 *
 * Programmer: Quincey Koziol
 *              Monday, August 22, 2005
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static unsigned
ceil_log10(unsigned long x)
{
    unsigned long pow10 = 1;
    unsigned ret = 0;

    while(x >= pow10) {
        pow10 *= 10;
        ret++;
    } /* end while */

    return(ret);
} /* ceil_log10() */


/*-------------------------------------------------------------------------
 * Function: attribute_stats
 *
 * Purpose: Gather statistics about attributes on an object
 *
 * Return:  Success: 0
 *
 *          Failure: -1
 *
 * Programmer:    Quincey Koziol
 *                Tuesday, July 17, 2007
 *
 *-------------------------------------------------------------------------
 */
static herr_t
attribute_stats(iter_t *iter, const H5O_info_t *oi)
{
    unsigned     bin;               /* "bin" the number of objects falls in */

    /* Update dataset & attribute metadata info */
    iter->attrs_btree_storage_size += oi->meta_size.attr.index_size;
    iter->attrs_heap_storage_size += oi->meta_size.attr.heap_size;

    /* Update small # of attribute count & limits */
    if(oi->num_attrs < SIZE_SMALL_ATTRS)
        (iter->num_small_attrs[(size_t)oi->num_attrs])++;
    if(oi->num_attrs > iter->max_attrs)
        iter->max_attrs = oi->num_attrs;

    /* Add attribute count to proper bin */
    bin = ceil_log10((unsigned long)oi->num_attrs);
    if((bin + 1) > iter->attr_nbins) {
  iter->attr_bins = (unsigned long *)HDrealloc(iter->attr_bins, (bin + 1) * sizeof(unsigned long));
        HDassert(iter->attr_bins);

  /* Initialize counts for intermediate bins */
        while(iter->attr_nbins < bin)
      iter->attr_bins[iter->attr_nbins++] = 0;
        iter->attr_nbins++;

        /* Initialize count for new bin */
        iter->attr_bins[bin] = 1;
     } /* end if */
     else
         (iter->attr_bins[bin])++;

     return 0;
} /* end attribute_stats() */


/*-------------------------------------------------------------------------
 * Function: group_stats
 *
 * Purpose: Gather statistics about the group
 *
 * Return: Success: 0
 *
 *  Failure: -1
 *
 * Programmer: Quincey Koziol
 *             Tuesday, August 16, 2005
 *
 * Modifications: Refactored code from the walk_function
 *                EIP, Wednesday, August 16, 2006
 *
 *      Vailin Choi 12 July 2007
 *      1. Gathered storage info for btree and heap
 *         (groups and attributes)
 *      2. Gathered info for attributes
 *
 *      Vailin Choi 14 July 2007
 *      Cast "num_objs" and "num_attrs" to size_t
 *      Due to the -Mbounds problem for the pgi-32 bit compiler on indexing
 *
 *-------------------------------------------------------------------------
 */
static herr_t
group_stats(iter_t *iter, const char *name, const H5O_info_t *oi)
{
    H5G_info_t     ginfo;                  /* Group information */
    unsigned     bin;                     /* "bin" the number of objects falls in */
    herr_t     ret;

    /* Gather statistics about this type of object */
    iter->uniq_groups++;

    /* Get object header information */
    iter->group_ohdr_info.total_size += oi->hdr.space.total;
    iter->group_ohdr_info.free_size += oi->hdr.space.free;

    /* Get group information */
    ret = H5Gget_info_by_name(iter->fid, name, &ginfo, H5P_DEFAULT);
    HDassert(ret >= 0);

    /* Update link stats */
    if(ginfo.nlinks < SIZE_SMALL_GROUPS)
        (iter->num_small_groups[(size_t)ginfo.nlinks])++;
    if(ginfo.nlinks > iter->max_fanout)
        iter->max_fanout = ginfo.nlinks;

    /* Add group count to proper bin */
    bin = ceil_log10((unsigned long)ginfo.nlinks);
    if((bin + 1) > iter->group_nbins) {
        /* Allocate more storage for info about dataset's datatype */
        iter->group_bins = (unsigned long *)HDrealloc(iter->group_bins, (bin + 1) * sizeof(unsigned long));
        HDassert(iter->group_bins);

  /* Initialize counts for intermediate bins */
        while(iter->group_nbins < bin)
            iter->group_bins[iter->group_nbins++] = 0;
        iter->group_nbins++;

        /* Initialize count for new bin */
        iter->group_bins[bin] = 1;
    } /* end if */
    else
        (iter->group_bins[bin])++;

    /* Update group metadata info */
    iter->groups_btree_storage_size += oi->meta_size.obj.index_size;
    iter->groups_heap_storage_size += oi->meta_size.obj.heap_size;

    /* Update attribute metadata info */
    ret = attribute_stats(iter, oi);
    HDassert(ret >= 0);

    return 0;
} /* end group_stats() */


/*-------------------------------------------------------------------------
 * Function: dataset_stats
 *
 * Purpose: Gather statistics about the dataset
 *
 * Return:  Success: 0
 *
 *          Failure: -1
 *
 * Programmer:    Quincey Koziol
 *                Tuesday, August 16, 2005
 *
 *-------------------------------------------------------------------------
 */
static herr_t
dataset_stats(iter_t *iter, const char *name, const H5O_info_t *oi)
{
    unsigned     bin;               /* "bin" the number of objects falls in */
    hid_t     did;               /* Dataset ID */
    hid_t     sid;               /* Dataspace ID */
    hid_t     tid;               /* Datatype ID */
    hid_t     dcpl;              /* Dataset creation property list ID */
    hsize_t     dims[H5S_MAX_RANK];/* Dimensions of dataset */
    H5D_layout_t   lout;              /* Layout of dataset */
    unsigned     type_found;        /* Whether the dataset's datatype was */
                                         /* already found */
    int     ndims;             /* Number of dimensions of dataset */
    hsize_t     storage;           /* Size of dataset storage */
    unsigned     u;                 /* Local index variable */
    int     num_ext;           /* Number of external files for a dataset */
    int     nfltr;             /* Number of filters for a dataset */
    H5Z_filter_t  fltr;              /* Filter identifier */
    herr_t     ret;

    /* Gather statistics about this type of object */
    iter->uniq_dsets++;

    /* Get object header information */
    iter->dset_ohdr_info.total_size += oi->hdr.space.total;
    iter->dset_ohdr_info.free_size += oi->hdr.space.free;

    did = H5Dopen2(iter->fid, name, H5P_DEFAULT);
    HDassert(did > 0);

    /* Update dataset metadata info */
    iter->datasets_index_storage_size += oi->meta_size.obj.index_size;
    iter->datasets_heap_storage_size += oi->meta_size.obj.heap_size;

    /* Update attribute metadata info */
    ret = attribute_stats(iter, oi);
    HDassert(ret >= 0);

    /* Get storage info */
    storage = H5Dget_storage_size(did);

    /* Gather layout statistics */
    dcpl = H5Dget_create_plist(did);
    HDassert(dcpl > 0);

    lout = H5Pget_layout(dcpl);
    HDassert(lout >= 0);

    /* Object header's total size for H5D_COMPACT layout includes raw data size */
    /* "storage" also includes H5D_COMPACT raw data size */
    if(lout == H5D_COMPACT)
        iter->dset_ohdr_info.total_size -= storage;

    /* Track the layout type for dataset */
    (iter->dset_layouts[lout])++;

    /* Get the number of external files for the dataset */
    num_ext = H5Pget_external_count(dcpl);
    assert (num_ext >= 0);

    /* Accumulate raw data size accordingly */
    if(num_ext) {
        iter->nexternal += (unsigned long)num_ext;
        iter->dset_external_storage_size += (unsigned long)storage;
    } else
        iter->dset_storage_size += storage;

    /* Gather dataspace statistics */
    sid = H5Dget_space(did);
    HDassert(sid > 0);

    ndims = H5Sget_simple_extent_dims(sid, dims, NULL);
    HDassert(ndims >= 0);

    /* Check for larger rank of dataset */
    if((unsigned)ndims > iter->max_dset_rank)
        iter->max_dset_rank = (unsigned)ndims;

    /* Track the number of datasets with each rank */
    (iter->dset_rank_count[ndims])++;

    /* Only gather dim size statistics on 1-D datasets */
    if(ndims == 1) {
       iter->max_dset_dims = dims[0];
       if(dims[0] < SIZE_SMALL_DSETS)
           (iter->small_dset_dims[(size_t)dims[0]])++;

       /* Add dim count to proper bin */
       bin = ceil_log10((unsigned long)dims[0]);
       if((bin + 1) > iter->dset_dim_nbins) {
          /* Allocate more storage for info about dataset's datatype */
          iter->dset_dim_bins = (unsigned long *)HDrealloc(iter->dset_dim_bins, (bin + 1) * sizeof(unsigned long));
          HDassert(iter->dset_dim_bins);

          /* Initialize counts for intermediate bins */
          while(iter->dset_dim_nbins < bin)
              iter->dset_dim_bins[iter->dset_dim_nbins++] = 0;
          iter->dset_dim_nbins++;

          /* Initialize count for this bin */
          iter->dset_dim_bins[bin] = 1;
        } /* end if */
        else
            (iter->dset_dim_bins[bin])++;
    } /* end if */

    ret = H5Sclose(sid);
    HDassert(ret >= 0);

    /* Gather datatype statistics */
    tid = H5Dget_type(did);
    HDassert(tid > 0);

    type_found = FALSE;
    for(u = 0; u < iter->dset_ntypes; u++)
        if(H5Tequal(iter->dset_type_info[u].tid, tid) > 0) {
            type_found = TRUE;
            break;
        } /* end for */
    if(type_found)
         (iter->dset_type_info[u].count)++;
    else {
        unsigned curr_ntype = iter->dset_ntypes;

        /* Increment # of datatypes seen for datasets */
        iter->dset_ntypes++;

        /* Allocate more storage for info about dataset's datatype */
        iter->dset_type_info = (dtype_info_t *)HDrealloc(iter->dset_type_info, iter->dset_ntypes * sizeof(dtype_info_t));
        HDassert(iter->dset_type_info);

        /* Initialize information about datatype */
        iter->dset_type_info[curr_ntype].tid = H5Tcopy(tid);
        HDassert(iter->dset_type_info[curr_ntype].tid > 0);
        iter->dset_type_info[curr_ntype].count = 1;
        iter->dset_type_info[curr_ntype].named = 0;

        /* Set index for later */
        u = curr_ntype;
    } /* end else */

    /* Check if the datatype is a named datatype */
    if(H5Tcommitted(tid) > 0)
        (iter->dset_type_info[u].named)++;

    ret = H5Tclose(tid);
    HDassert(ret >= 0);

    /* Track different filters */
    if((nfltr = H5Pget_nfilters(dcpl)) >= 0) {
       if(nfltr == 0)
           iter->dset_comptype[0]++;
        for(u = 0; u < (unsigned)nfltr; u++) {
            fltr = H5Pget_filter2(dcpl, u, 0, 0, 0, 0, 0, NULL);
            if(fltr >= 0) {
                if(fltr < (H5_NFILTERS_IMPL - 1))
                    iter->dset_comptype[fltr]++;
                else
                    iter->dset_comptype[H5_NFILTERS_IMPL - 1]++; /*other filters*/
            } /* end if */
        } /* end for */
    } /* endif nfltr */

     ret = H5Pclose(dcpl);
     HDassert(ret >= 0);

     ret = H5Dclose(did);
     HDassert(ret >= 0);

     return 0;
}  /* end dataset_stats() */


/*-------------------------------------------------------------------------
 * Function: datatype_stats
 *
 * Purpose: Gather statistics about the datatype
 *
 * Return:  Success: 0
 *          Failure: -1
 *
 * Programmer:    Vailin Choi; July 7th, 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
datatype_stats(iter_t *iter, const H5O_info_t *oi)
{
    herr_t ret;

    /* Gather statistics about this type of object */
    iter->uniq_dtypes++;

    /* Get object header information */
    iter->dtype_ohdr_info.total_size += oi->hdr.space.total;
    iter->dtype_ohdr_info.free_size += oi->hdr.space.free;

    /* Update attribute metadata info */
    ret = attribute_stats(iter, oi);
    HDassert(ret >= 0);

     return 0;
}  /* end datatype_stats() */


/*-------------------------------------------------------------------------
 * Function: obj_stats
 *
 * Purpose: Gather statistics about an object
 *
 * Return: Success: 0
 *       Failure: -1
 *
 * Programmer: Quincey Koziol
 *             Tuesday, November 6, 2007
 *
 *-------------------------------------------------------------------------
 */
static herr_t
obj_stats(const char *path, const H5O_info_t *oi, const char *already_visited,
    void *_iter)
{
    iter_t *iter = (iter_t *)_iter;

    /* If the object has already been seen then just return */
    if(NULL == already_visited) {
        /* Gather some general statistics about the object */
        if(oi->rc > iter->max_links)
            iter->max_links = oi->rc;

        switch(oi->type) {
            case H5O_TYPE_GROUP:
                group_stats(iter, path, oi);
                break;

            case H5O_TYPE_DATASET:
                dataset_stats(iter, path, oi);
                break;

            case H5O_TYPE_NAMED_DATATYPE:
    datatype_stats(iter, oi);
                break;

            default:
                /* Gather statistics about this type of object */
                iter->uniq_others++;
                break;
        } /* end switch */
    } /* end if */

    return 0;
} /* end obj_stats() */


/*-------------------------------------------------------------------------
 * Function: lnk_stats
 *
 * Purpose: Gather statistics about a link
 *
 * Return: Success: 0
 *
 *  Failure: -1
 *
 * Programmer: Quincey Koziol
 *             Tuesday, November 6, 2007
 *
 *-------------------------------------------------------------------------
 */
static herr_t
lnk_stats(const char UNUSED *path, const H5L_info_t *li, void *_iter)
{
    iter_t *iter = (iter_t *)_iter;

    switch(li->type) {
        case H5L_TYPE_SOFT:
        case H5L_TYPE_EXTERNAL:
            /* Gather statistics about links and UD links */
            iter->uniq_links++;
            break;

        default:
            /* Gather statistics about this type of object */
            iter->uniq_others++;
            break;
    } /* end switch() */

    return 0;
} /* end lnk_stats() */

/*-------------------------------------------------------------------------
 * Function: freespace_stats
 *
 * Purpose: Gather statistics for free space sections in the file
 *
 * Return: Success: 0
 *       Failure: -1
 *
 * Programmer:  Vailin Choi; July 7th, 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
freespace_stats(hid_t fid, iter_t *iter)
{
    H5F_sect_info_t *sect_info = NULL;    /* Free space sections */
    ssize_t   nsects;                   /* Number of free space sections */
    size_t   u;                        /* Local index variable */

    /* Query section information */
    if((nsects = H5Fget_free_sections(fid, H5FD_MEM_DEFAULT, 0, NULL)) < 0)
  return(FAIL);
    else if(nsects) {
  if(NULL == (sect_info = (H5F_sect_info_t *)HDcalloc((size_t)nsects, sizeof(H5F_sect_info_t))))
      return(FAIL);
  nsects = H5Fget_free_sections(fid, H5FD_MEM_DEFAULT, (size_t)nsects, sect_info);
  HDassert(nsects);
    } /* end else-if */

    for(u = 0; u < (size_t)nsects; u++) {
        unsigned   bin;       /* "bin" the number of objects falls in */

  if(sect_info[u].size < SIZE_SMALL_SECTS)
      (iter->num_small_sects[(size_t)sect_info[u].size])++;

        /* Add section size to proper bin */
        bin = ceil_log10((unsigned long)sect_info[u].size);
        if(bin >= iter->sect_nbins) {
            /* Allocate more storage for section info */
            iter->sect_bins = (unsigned long *)HDrealloc(iter->sect_bins, (bin + 1) * sizeof(unsigned long));
            HDassert(iter->sect_bins);

            /* Initialize counts for intermediate bins */
            while(iter->sect_nbins < bin)
                iter->sect_bins[iter->sect_nbins++] = 0;
            iter->sect_nbins++;

            /* Initialize count for this bin */
            iter->sect_bins[bin] = 1;
        } /* end if */
        else
            (iter->sect_bins[bin])++;
    } /* end for */

    if(sect_info)
        free(sect_info);

    return 0;
} /* end freespace_stats() */


/*-------------------------------------------------------------------------
 * Function: parse_command_line
 *
 * Purpose: Parses command line and sets up global variable to control output
 *
 * Return: Success: 0
 *
 * Failure: -1
 *
 * Programmer: Elena Pourmal
 *             Saturday, August 12, 2006
 *
 *-------------------------------------------------------------------------
 */
static struct handler_t *
parse_command_line(int argc, const char *argv[])
{
    int                opt, i;
    struct handler_t   *hand = NULL;

    /* Allocate space to hold the command line info */
    if((hand = (struct handler_t *)HDcalloc((size_t)argc, sizeof(struct handler_t)))==NULL) {
        error_msg("unable to parse command line arguments \n");
        goto error;
    }

    /* parse command line options */
    while ((opt = get_option(argc, argv, s_opts, l_opts)) != EOF) {
        switch ((char)opt) {
            case 'h':
                usage(h5tools_getprogname());
                h5tools_setstatus(EXIT_SUCCESS);
                if (hand) {
                    for (i = 0; i < argc; i++)
                        if(hand[i].obj) {
                            free(hand[i].obj);
                            hand[i].obj=NULL;
                        }

                    free(hand);
                    hand = NULL;
                }
                goto done;
                break;

            case 'V':
                print_version(h5tools_getprogname());
                h5tools_setstatus(EXIT_SUCCESS);
                if (hand) {
                    for (i = 0; i < argc; i++)
                        if(hand[i].obj) {
                            free(hand[i].obj);
                            hand[i].obj=NULL;
                        }

                    free(hand);
                    hand = NULL;
                }
                goto done;
                break;

            case 'F':
                display_all = FALSE;
                display_file_metadata = TRUE;
                break;

            case 'f':
                display_all = FALSE;
                display_file = TRUE;
                break;

            case 'G':
                display_all = FALSE;
                display_group_metadata = TRUE;
                break;

            case 'g':
                display_all = FALSE;
                display_group = TRUE;
                break;

            case 'D':
                display_all = FALSE;
                display_dset_metadata = TRUE;
                break;

            case 'd':
                display_all = FALSE;
                display_dset = TRUE;
                break;

            case 'T':
                display_all = FALSE;
                display_dset_dtype_meta = TRUE;
                break;

            case 'A':
                display_all = FALSE;
                display_attr = TRUE;
                break;

            case 's':
                display_all = FALSE;
                display_free_sections = TRUE;
                break;

            case 'S':
                display_all = FALSE;
                display_summary = TRUE;
                break;

            case 'O':
                display_all = FALSE;
                display_object = TRUE;
                for(i = 0; i < argc; i++)
                    if(!hand[i].obj) {
                        hand[i].obj = HDstrdup(opt_arg);
                        break;
                    } /* end if */
                break;

            default:
                usage(h5tools_getprogname());
                h5tools_setstatus(EXIT_FAILURE);
                goto error;
        } /* end switch */
    } /* end while */

    /* check for file name to be processed */
    if (argc <= opt_ind) {
        error_msg("missing file name\n");
        usage(h5tools_getprogname());
        h5tools_setstatus(EXIT_FAILURE);
        goto error;
    } /* end if */

done:
    return hand;

error:
    if (hand) {
        for (i = 0; i < argc; i++)
            if(hand[i].obj) {
                free(hand[i].obj);
                hand[i].obj=NULL;
            }

        free(hand);
        hand = NULL;
    }
    h5tools_setstatus(EXIT_FAILURE);

    return hand;
}


/*-------------------------------------------------------------------------
 * Function: iter_free
 *
 * Purpose: Free iter structure
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 *-------------------------------------------------------------------------
 */
static herr_t
iter_free(iter_t *iter)
{
    /* Clear array of bins for group counts */
    if(iter->group_bins)
        free(iter->group_bins);
    iter->group_bins = NULL;
    /* Clear array of bins for attribute counts */
    if(iter->attr_bins)
        free(iter->attr_bins);
    iter->attr_bins = NULL;
    /* Clear dataset datatype information found */
    if(iter->dset_type_info)
        free(iter->dset_type_info);
    iter->dset_type_info = NULL;
    /* Clear array of bins for dataset dimensions */
    if(iter->dset_dim_bins)
        free(iter->dset_dim_bins);
    iter->dset_dim_bins = NULL;
    /* Clear array of bins for free-space section sizes */
    if(iter->sect_bins)
        free(iter->sect_bins);
    iter->sect_bins = NULL;

    return 0;
}


/*-------------------------------------------------------------------------
 * Function: print_file_info
 *
 * Purpose: Prints information about file
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Elena Pourmal
 *             Saturday, August 12, 2006
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
print_file_info(const iter_t *iter)
{
    printf("File information\n");
    printf("\t# of unique groups: %lu\n", iter->uniq_groups);
    printf("\t# of unique datasets: %lu\n", iter->uniq_dsets);
    printf("\t# of unique named datatypes: %lu\n", iter->uniq_dtypes);
    printf("\t# of unique links: %lu\n", iter->uniq_links);
    printf("\t# of unique other: %lu\n", iter->uniq_others);
    printf("\tMax. # of links to object: %lu\n", iter->max_links);
    HDfprintf(stdout, "\tMax. # of objects in group: %Hu\n", iter->max_fanout);

    return 0;
} /* print_file_info() */


/*-------------------------------------------------------------------------
 * Function: print_file_metadata
 *
 * Purpose: Prints file space information for file's metadata
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Elena Pourmal
 *             Saturday, August 12, 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
print_file_metadata(const iter_t *iter)
{
    HDfprintf(stdout, "File space information for file metadata (in bytes):\n");
    HDfprintf(stdout, "\tSuperblock: %Hu\n", iter->super_size);
    HDfprintf(stdout, "\tSuperblock extension: %Hu\n", iter->super_ext_size);
    HDfprintf(stdout, "\tUser block: %Hu\n", iter->ublk_size);

    HDfprintf(stdout, "\tObject headers: (total/unused)\n");
    HDfprintf(stdout, "\t\tGroups: %Hu/%Hu\n",
                iter->group_ohdr_info.total_size,
    iter->group_ohdr_info.free_size);
    HDfprintf(stdout, "\t\tDatasets(exclude compact data): %Hu/%Hu\n",
    iter->dset_ohdr_info.total_size,
    iter->dset_ohdr_info.free_size);
    HDfprintf(stdout, "\t\tDatatypes: %Hu/%Hu\n",
                iter->dtype_ohdr_info.total_size,
    iter->dtype_ohdr_info.free_size);

    HDfprintf(stdout, "\tGroups:\n");
    HDfprintf(stdout, "\t\tB-tree/List: %Hu\n", iter->groups_btree_storage_size);
    HDfprintf(stdout, "\t\tHeap: %Hu\n", iter->groups_heap_storage_size);

    HDfprintf(stdout, "\tAttributes:\n");
    HDfprintf(stdout, "\t\tB-tree/List: %Hu\n", iter->attrs_btree_storage_size);
    HDfprintf(stdout, "\t\tHeap: %Hu\n", iter->attrs_heap_storage_size);

    HDfprintf(stdout, "\tChunked datasets:\n");
    HDfprintf(stdout, "\t\tIndex: %Hu\n", iter->datasets_index_storage_size);

    HDfprintf(stdout, "\tDatasets:\n");
    HDfprintf(stdout, "\t\tHeap: %Hu\n", iter->datasets_heap_storage_size);

    HDfprintf(stdout, "\tShared Messages:\n");
    HDfprintf(stdout, "\t\tHeader: %Hu\n", iter->SM_hdr_storage_size);
    HDfprintf(stdout, "\t\tB-tree/List: %Hu\n", iter->SM_index_storage_size);
    HDfprintf(stdout, "\t\tHeap: %Hu\n", iter->SM_heap_storage_size);

    HDfprintf(stdout, "\tFree-space managers:\n");
    HDfprintf(stdout, "\t\tHeader: %Hu\n", iter->free_hdr);
    HDfprintf(stdout, "\t\tAmount of free space: %Hu\n", iter->free_space);

    return 0;
} /* print_file_metadata() */


/*-------------------------------------------------------------------------
 * Function: print_group_info
 *
 * Purpose: Prints information about groups in the file
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Elena Pourmal
 *             Saturday, August 12, 2006
 *
 * Modifications:
 *  bug #1253; Oct 6th 2008; Vailin Choi
 *  Fixed segmentation fault: print iter->group_bins[0] when
 *  there is iter->group_nbins
 *
 *-------------------------------------------------------------------------
 */
static herr_t
print_group_info(const iter_t *iter)
{
    unsigned long power;        /* Temporary "power" for bins */
    unsigned long total;        /* Total count for various statistics */
    unsigned u;                 /* Local index variable */

    printf("Small groups:\n");
    total = 0;
    for(u = 0; u < SIZE_SMALL_GROUPS; u++) {
        if(iter->num_small_groups[u] > 0) {
            printf("\t# of groups of size %u: %lu\n", u, iter->num_small_groups[u]);
            total += iter->num_small_groups[u];
        } /* end if */
    } /* end for */
    printf("\tTotal # of small groups: %lu\n", total);

    printf("Group bins:\n");
    total = 0;
    if((iter->group_nbins > 0) && (iter->group_bins[0] > 0)) {
       printf("\t# of groups of size 0: %lu\n", iter->group_bins[0]);
       total = iter->group_bins[0];
    } /* end if */
    power = 1;
    for(u = 1; u < iter->group_nbins; u++) {
        if(iter->group_bins[u] > 0) {
           printf("\t# of groups of size %lu - %lu: %lu\n", power, (power * 10) - 1,
                    iter->group_bins[u]);
           total += iter->group_bins[u];
        } /* end if */
        power *= 10;
    } /* end for */
    printf("\tTotal # of groups: %lu\n", total);

    return 0;
} /* print_group_info() */


/*-------------------------------------------------------------------------
 * Function: print_group_metadata
 *
 * Purpose: Prints file space information for groups' metadata
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Vailin Choi; October 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
print_group_metadata(const iter_t *iter)
{
    printf("File space information for groups' metadata (in bytes):\n");

    HDfprintf(stdout, "\tObject headers (total/unused): %Hu/%Hu\n",
            iter->group_ohdr_info.total_size, iter->group_ohdr_info.free_size);

    HDfprintf(stdout, "\tB-tree/List: %Hu\n", iter->groups_btree_storage_size);
    HDfprintf(stdout, "\tHeap: %Hu\n", iter->groups_heap_storage_size);

    return 0;
} /* print_group_metadata() */


/*-------------------------------------------------------------------------
 * Function: print_dataset_info
 *
 * Purpose: Prints information about datasets in the file
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Elena Pourmal
 *             Saturday, August 12, 2006
 *
 *-------------------------------------------------------------------------
 */
static herr_t
print_dataset_info(const iter_t *iter)
{
    unsigned long power;        /* Temporary "power" for bins */
    unsigned long total;        /* Total count for various statistics */
    unsigned u;                 /* Local index variable */

    if(iter->uniq_dsets > 0) {
        printf("Dataset dimension information:\n");
        printf("\tMax. rank of datasets: %u\n", iter->max_dset_rank);
        printf("\tDataset ranks:\n");
        for(u = 0; u < H5S_MAX_RANK; u++)
            if(iter->dset_rank_count[u] > 0)
                printf("\t\t# of dataset with rank %u: %lu\n", u, iter->dset_rank_count[u]);

        printf("1-D Dataset information:\n");
        HDfprintf(stdout, "\tMax. dimension size of 1-D datasets: %Hu\n", iter->max_dset_dims);
        printf("\tSmall 1-D datasets:\n");
        total = 0;
        for(u = 0; u < SIZE_SMALL_DSETS; u++) {
            if(iter->small_dset_dims[u] > 0) {
                printf("\t\t# of dataset dimensions of size %u: %lu\n", u,
                         iter->small_dset_dims[u]);
                total += iter->small_dset_dims[u];
            } /* end if */
        } /* end for */
        printf("\t\tTotal small datasets: %lu\n", total);

        /* Protect against no datasets in file */
        if(iter->dset_dim_nbins > 0) {
            printf("\t1-D Dataset dimension bins:\n");
            total = 0;
            if(iter->dset_dim_bins[0] > 0) {
                printf("\t\t# of datasets of size 0: %lu\n", iter->dset_dim_bins[0]);
                total = iter->dset_dim_bins[0];
            } /* end if */
            power = 1;
            for(u = 1; u < iter->dset_dim_nbins; u++) {
                if(iter->dset_dim_bins[u] > 0) {
                    printf("\t\t# of datasets of size %lu - %lu: %lu\n", power, (power * 10) - 1,
                             iter->dset_dim_bins[u]);
                    total += iter->dset_dim_bins[u];
                } /* end if */
                power *= 10;
            } /* end for */
            printf("\t\tTotal # of datasets: %lu\n", total);
        } /* end if */

        printf("Dataset storage information:\n");
        HDfprintf(stdout, "\tTotal raw data size: %Hu\n", iter->dset_storage_size);
        HDfprintf(stdout, "\tTotal external raw data size: %Hu\n", iter->dset_external_storage_size);

        printf("Dataset layout information:\n");
        for(u = 0; u < H5D_NLAYOUTS; u++)
        printf("\tDataset layout counts[%s]: %lu\n", (u == 0 ? "COMPACT" :
                (u == 1 ? "CONTIG" : "CHUNKED")), iter->dset_layouts[u]);
        printf("\tNumber of external files : %lu\n", iter->nexternal);

        printf("Dataset filters information:\n");
        printf("\tNumber of datasets with:\n");
        printf("\t\tNO filter: %lu\n", iter->dset_comptype[H5Z_FILTER_ERROR+1]);
        printf("\t\tGZIP filter: %lu\n", iter->dset_comptype[H5Z_FILTER_DEFLATE]);
        printf("\t\tSHUFFLE filter: %lu\n", iter->dset_comptype[H5Z_FILTER_SHUFFLE]);
        printf("\t\tFLETCHER32 filter: %lu\n", iter->dset_comptype[H5Z_FILTER_FLETCHER32]);
        printf("\t\tSZIP filter: %lu\n", iter->dset_comptype[H5Z_FILTER_SZIP]);
        printf("\t\tNBIT filter: %lu\n", iter->dset_comptype[H5Z_FILTER_NBIT]);
        printf("\t\tSCALEOFFSET filter: %lu\n", iter->dset_comptype[H5Z_FILTER_SCALEOFFSET]);
        printf("\t\tUSER-DEFINED filter: %lu\n", iter->dset_comptype[H5_NFILTERS_IMPL-1]);
    } /* end if */

    return 0;
} /* print_dataset_info() */


/*-------------------------------------------------------------------------
 * Function: print_dataset_metadata
 *
 * Purpose: Prints file space information for datasets' metadata
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer:  Vailin Choi; October 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
print_dset_metadata(const iter_t *iter)
{
    printf("File space information for datasets' metadata (in bytes):\n");

    HDfprintf(stdout, "\tObject headers (total/unused): %Hu/%Hu\n",
            iter->dset_ohdr_info.total_size, iter->dset_ohdr_info.free_size);

    HDfprintf(stdout, "\tIndex for Chunked datasets: %Hu\n",
            iter->datasets_index_storage_size);
    HDfprintf(stdout, "\tHeap: %Hu\n", iter->datasets_heap_storage_size);

    return 0;
} /* print_dset_metadata() */


/*-------------------------------------------------------------------------
 * Function: print_dset_dtype_meta
 *
 * Purpose: Prints datasets' datatype information
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Vailin Choi; October 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
print_dset_dtype_meta(const iter_t *iter)
{
    unsigned long total;        /* Total count for various statistics */
    size_t   dtype_size;        /* Size of encoded datatype */
    unsigned u;                 /* Local index variable */

    if(iter->dset_ntypes) {
        printf("Dataset datatype information:\n");
        printf("\t# of unique datatypes used by datasets: %lu\n", iter->dset_ntypes);
        total = 0;
        for(u = 0; u < iter->dset_ntypes; u++) {
            H5Tencode(iter->dset_type_info[u].tid, NULL, &dtype_size);
            printf("\tDataset datatype #%u:\n", u);
            printf("\t\tCount (total/named) = (%lu/%lu)\n",
                    iter->dset_type_info[u].count, iter->dset_type_info[u].named);
            printf("\t\tSize (desc./elmt) = (%lu/%lu)\n", (unsigned long)dtype_size,
                    (unsigned long)H5Tget_size(iter->dset_type_info[u].tid));
            H5Tclose(iter->dset_type_info[u].tid);
            total += iter->dset_type_info[u].count;
        } /* end for */
        printf("\tTotal dataset datatype count: %lu\n", total);
    } /* end if */

    return 0;
} /* print_dset_dtype_meta() */


/*-------------------------------------------------------------------------
 * Function: print_attr_info
 *
 * Purpose: Prints information about attributes in the file
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Vailin Choi
 *             July 12, 2007
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static herr_t
print_attr_info(const iter_t *iter)
{
    unsigned long power;        /* Temporary "power" for bins */
    unsigned long total;        /* Total count for various statistics */
    unsigned u;                 /* Local index variable */

    printf("Small # of attributes:\n");
    total = 0;
    for(u = 1; u < SIZE_SMALL_ATTRS; u++) {
        if(iter->num_small_attrs[u] > 0) {
            printf("\t# of objects with %u attributes: %lu\n", u, iter->num_small_attrs[u]);
            total += iter->num_small_attrs[u];
        } /* end if */
    } /* end for */
    printf("\tTotal # of objects with small # of attributes: %lu\n", total);

    printf("Attribute bins:\n");
    total = 0;
    power = 1;
    for(u = 1; u < iter->attr_nbins; u++) {
        if(iter->attr_bins[u] > 0) {
           printf("\t# of objects with %lu - %lu attributes: %lu\n", power, (power * 10) - 1,
                    iter->attr_bins[u]);
           total += iter->attr_bins[u];
        } /* end if */
        power *= 10;
    } /* end for */
    printf("\tTotal # of objects with attributes: %lu\n", total);
    printf("\tMax. # of attributes to objects: %lu\n", (unsigned long)iter->max_attrs);

    return 0;
} /* print_attr_info() */


/*-------------------------------------------------------------------------
 * Function: print_freespace_info
 *
 * Purpose: Prints information about free space in the file
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Vailin Choi; July 7th, 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
print_freespace_info(const iter_t *iter)
{
    unsigned long power;        /* Temporary "power" for bins */
    unsigned long total;        /* Total count for various statistics */
    unsigned u;                 /* Local index variable */

    HDfprintf(stdout, "Free-space section threshold: %Hu bytes\n", iter->fs_threshold);
    printf("Small size free-space sections (< %u bytes):\n", (unsigned)SIZE_SMALL_SECTS);
    total = 0;
    for(u = 0; u < SIZE_SMALL_SECTS; u++) {
        if(iter->num_small_sects[u] > 0) {
            printf("\t# of sections of size %u: %lu\n", u, iter->num_small_sects[u]);
            total += iter->num_small_sects[u];
        } /* end if */
    } /* end for */
    printf("\tTotal # of small size sections: %lu\n", total);

    printf("Free-space section bins:\n");

    total = 0;
    power = 1;
    for(u = 1; u < iter->sect_nbins; u++) {
        if(iter->sect_bins[u] > 0) {
           printf("\t# of sections of size %lu - %lu: %lu\n", power, (power * 10) - 1,
                    iter->sect_bins[u]);
           total += iter->sect_bins[u];
        } /* end if */
        power *= 10;
    } /* end for */
    printf("\tTotal # of sections: %lu\n", total);

    return 0;
} /* print_freespace_info() */


/*-------------------------------------------------------------------------
 * Function: print_storage_summary
 *
 * Purpose: Prints file space information for the file
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Vailin Choi; August 2009
 *
 *-------------------------------------------------------------------------
 */
static herr_t
print_storage_summary(const iter_t *iter)
{
    hsize_t total_meta = 0;
    hsize_t unaccount = 0;
    float   percent = 0.0;

    HDfprintf(stdout, "File space management strategy: %s\n", FS_STRATEGY_NAME[iter->fs_strategy]);
    HDfprintf(stdout, "File space page size: %Hu bytes\n", iter->fsp_size);
    printf("Summary of file space information:\n");
    total_meta =
            iter->super_size + iter->super_ext_size + iter->ublk_size +
            iter->group_ohdr_info.total_size +
            iter->dset_ohdr_info.total_size +
            iter->dtype_ohdr_info.total_size +
            iter->groups_btree_storage_size +
            iter->groups_heap_storage_size +
            iter->attrs_btree_storage_size +
            iter->attrs_heap_storage_size +
            iter->datasets_index_storage_size +
            iter->datasets_heap_storage_size +
            iter->SM_hdr_storage_size +
            iter->SM_index_storage_size +
            iter->SM_heap_storage_size +
            iter->free_hdr;

    HDfprintf(stdout, "  File metadata: %Hu bytes\n", total_meta);
    HDfprintf(stdout, "  Raw data: %Hu bytes\n", iter->dset_storage_size);

    percent = ((float)iter->free_space / (float)iter->filesize) * 100;
    HDfprintf(stdout, "  Amount/Percent of tracked free space: %Hu bytes/%3.1f%\n",
                iter->free_space, percent);

    if(iter->filesize < (total_meta + iter->dset_storage_size + iter->free_space)) {
        unaccount = (total_meta + iter->dset_storage_size + iter->free_space) - iter->filesize;
        HDfprintf(stdout, "  ??? File has %Hu more bytes accounted for than its size! ???\n", unaccount);
    }
    else {
        unaccount = iter->filesize - (total_meta + iter->dset_storage_size + iter->free_space);
        HDfprintf(stdout, "  Unaccounted space: %Hu bytes\n", unaccount);
    }

    HDfprintf(stdout, "Total space: %Hu bytes\n", total_meta + iter->dset_storage_size + iter->free_space + unaccount);

    if(iter->nexternal)
        HDfprintf(stdout, "External raw data: %Hu bytes\n", iter->dset_external_storage_size);

    return 0;
} /* print_storage_summary() */


/*-------------------------------------------------------------------------
 * Function: print_file_statistics
 *
 * Purpose: Prints file statistics
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Elena Pourmal
 *             Saturday, August 12, 2006
 *
 *-------------------------------------------------------------------------
 */
static void
print_file_statistics(const iter_t *iter)
{
    if(display_all) {
        display_file = TRUE;
        display_group = TRUE;
        display_dset = TRUE;
        display_dset_dtype_meta = TRUE;
        display_attr = TRUE;
        display_free_sections = TRUE;
        display_summary = TRUE;

        display_file_metadata = TRUE;
        display_group_metadata = TRUE;
        display_dset_metadata = TRUE;
    }

    if(display_file)            print_file_info(iter);
    if(display_file_metadata)   print_file_metadata(iter);

    if(display_group)           print_group_info(iter);
    if(!display_all && display_group_metadata)   print_group_metadata(iter);

    if(display_dset)            print_dataset_info(iter);
    if(display_dset_dtype_meta) print_dset_dtype_meta(iter);
    if(!display_all && display_dset_metadata)   print_dset_metadata(iter);

    if(display_attr)            print_attr_info(iter);
    if(display_free_sections)   print_freespace_info(iter);
    if(display_summary)         print_storage_summary(iter);
} /* print_file_statistics() */


/*-------------------------------------------------------------------------
 * Function: print_object_statistics
 *
 * Purpose: Prints object statistics
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Elena Pourmal
 *             Thursday, August 17, 2006
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
print_object_statistics(const char *name)
{
    printf("Object name %s\n", name);
} /* print_object_statistics() */


/*-------------------------------------------------------------------------
 * Function: print_statistics
 *
 * Purpose: Prints statistics
 *
 * Return: Success: 0
 *
 * Failure: Never fails
 *
 * Programmer: Elena Pourmal
 *             Thursday, August 17, 2006
 *
 * Modifications:
 *
 *-------------------------------------------------------------------------
 */
static void
print_statistics(const char *name, const iter_t *iter)
{
    if(display_object)
        print_object_statistics(name);
    else
        print_file_statistics(iter);
} /* print_statistics() */


/*-------------------------------------------------------------------------
 * Function: main
 *
 * Modifications:
 *      2/2010; Vailin Choi
 *      Get the size of user block
 *
 *-------------------------------------------------------------------------
 */
int
main(int argc, const char *argv[])
{
    iter_t              iter;
    const char         *fname = NULL;
    hid_t               fid = -1;
    hid_t               fcpl;
    struct handler_t   *hand = NULL;
    H5F_info2_t         finfo;
    int                 i;

    h5tools_setprogname(PROGRAMNAME);
    h5tools_setstatus(EXIT_SUCCESS);

    /* Disable error reporting */
    H5Eset_auto2(H5E_DEFAULT, NULL, NULL);

    /* Initialize h5tools lib */
    h5tools_init();
    
    if((hand = parse_command_line(argc, argv))==NULL) {
        goto done;
    }

    fname = argv[opt_ind];

    printf("Filename: %s\n", fname);

    HDmemset(&iter, 0, sizeof(iter));

    fid = H5Fopen(fname, H5F_ACC_RDONLY, H5P_DEFAULT);
    if(fid < 0) {
        error_msg("unable to open file \"%s\"\n", fname);
        h5tools_setstatus(EXIT_FAILURE);
        goto done;
    } /* end if */

    /* Initialize iter structure */
    iter.fid = fid;

    if(H5Fget_filesize(fid, &iter.filesize) < 0)
        warn_msg("Unable to retrieve file size\n");
    HDassert(iter.filesize != 0);

    /* Get storge info for file-level structures */
    if(H5Fget_info2(fid, &finfo) < 0)
        warn_msg("Unable to retrieve file info\n");
    else {
        iter.super_size = finfo.super.super_size;
        iter.super_ext_size = finfo.super.super_ext_size;
        iter.SM_hdr_storage_size = finfo.sohm.hdr_size;
        iter.SM_index_storage_size = finfo.sohm.msgs_info.index_size;
        iter.SM_heap_storage_size = finfo.sohm.msgs_info.heap_size;
        iter.free_space = finfo.free.tot_space;
        iter.free_hdr = finfo.free.meta_size;
    } /* end else */

    if((fcpl = H5Fget_create_plist(fid)) < 0)
        warn_msg("Unable to retrieve file creation property\n");

    if(H5Pget_userblock(fcpl, &iter.ublk_size) < 0)
        warn_msg("Unable to retrieve userblock size\n");

    if(H5Pget_file_space_strategy(fcpl, &iter.fs_strategy, &iter.fs_threshold) < 0)
        warn_msg("Unable to retrieve file space information\n");
    HDassert(iter.fs_strategy != 0 && iter.fs_strategy < H5F_FILE_SPACE_NTYPES);

    if(H5Pget_file_space_page_size(fcpl, &iter.fsp_size) < 0)
        warn_msg("Unable to retrieve file space page size\n");

    /* get information for free-space sections */
    if(freespace_stats(fid, &iter) < 0)
        warn_msg("Unable to retrieve freespace info\n");

    /* Walk the objects or all file */
    if(display_object) {
        unsigned u;

        u = 0;
        while(hand[u].obj) {
            if (h5trav_visit(fid, hand[u].obj, TRUE, TRUE, obj_stats, lnk_stats, &iter) < 0)
                warn_msg("Unable to traverse object \"%s\"\n", hand[u].obj);
            else
                print_statistics(hand[u].obj, &iter);
            u++;
        } /* end while */
    } /* end if */
    else {
        if (h5trav_visit(fid, "/", TRUE, TRUE, obj_stats, lnk_stats, &iter) < 0)
            warn_msg("Unable to traverse objects/links in file \"%s\"\n", fname);
  else
      print_statistics("/", &iter);
    } /* end else */

done:
    if(hand) {
        for (i = 0; i < argc; i++)
            if(hand[i].obj) {
                free(hand[i].obj);
                hand[i].obj=NULL;
            }

        free(hand);
        hand = NULL;

        /* Free iter structure */
        iter_free(&iter);
    
        if(fid >= 0 && H5Fclose(fid) < 0) {
            error_msg("unable to close file \"%s\"\n", fname);
            h5tools_setstatus(EXIT_FAILURE);
        }
    }

    leave(h5tools_getstatus());
}

