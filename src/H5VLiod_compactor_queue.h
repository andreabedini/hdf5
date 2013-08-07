#ifndef _H5VLiod_COMPACTOR_QUEUE_H
#define _H5VLiod_COMPACTOR_QUEUE_H


#include "H5VLiod_common.h"


#ifdef H5_HAVE_EFF

/* -----------------------------------------------------------------
 * Programmer:  Vishwanath Venkatesan <vish@hdfgroup.gov>
 *              June, 2013
 *
 * Purpose:	Compactor queue datastructure

 *------------------------------------------------------------------*/

#include "H5VLiod_server.h"
 
#define READ 100
#define WRITE 150
#define MERGED 297
#define NOT_MERGED 298
#define USED_IN_MERGING 299
/* Read states  */
#define NOT_SS 300
#define SS 301
#define SPLIT_FOR_SS 302
#define USED_IN_SS 303
/* ---------------- */
#define CP_SUCCESS 0
#define CP_FAIL -1



 
typedef struct {
  op_data_t *input_structure; 
  int type_request;
  int num_peers;
  int request_id;
}compactor_entry;

struct cqueue
{
  compactor_entry request;
  struct cqueue* next;
  struct cqueue* prev;
};
 
typedef struct cqueue node;
	 
struct cqlist
{
  node* head;
  node* tail;
};
 
typedef struct cqlist compactor; 


H5_DLL int H5VL_iod_add_requests_to_compactor(compactor*, compactor_entry request);
H5_DLL int H5VL_iod_remove_request_from_compactor(compactor*, compactor_entry *); 
H5_DLL int H5VL_iod_get_request_at_front (compactor *, compactor_entry *);
H5_DLL int H5VL_iod_get_request_at (compactor *, compactor_entry *, int j);
H5_DLL int H5VL_iod_display_compactor_requests(compactor*);
H5_DLL int H5VL_iod_remove_element_from_queue(compactor* s, node* d);
H5_DLL int H5VL_iod_get_number_of_requests (compactor *s); 
H5_DLL int H5VL_iod_init_compactor_queue(compactor **s);
H5_DLL int H5VL_iod_destroy_compactor_queue(compactor *s);

 
#endif /*H5_HAVE_EFF*/
#endif /*H5VLiod_compactor_queue_H*/
