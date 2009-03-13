#ifndef _S3C_MFC_BUFFER_MANAGER_H_
#define _S3C_MFC_BUFFER_MANAGER_H_

#include "s3c_mfc_interface.h"
#include "s3c_mfc_common.h"


typedef struct tag_alloc_mem_t
{
	struct tag_alloc_mem_t *prev;
	struct tag_alloc_mem_t *next;
	union {
		unsigned int cached_p_addr;	/* physical address of cacheable area */
		unsigned int uncached_p_addr;	/* physical address of non-cacheable area */
	};
	unsigned char *v_addr;	/*  virtual address in cached area */
	unsigned char *u_addr;	/*  copyed virtual address for user mode process */
	int size;		/*  memory size */	
	int inst_no;
	char cache_flag;
} s3c_mfc_alloc_mem_t;


typedef struct tag_free_mem_t
{
	struct tag_free_mem_t *prev;
	struct tag_free_mem_t *next;
	unsigned int start_addr;
	unsigned int size;
	char cache_flag;
} s3c_mfc_free_mem_t;

int s3c_mfc_init_buffer_manager(void);
void s3c_mfc_merge_frag(int inst_no);
MFC_ERROR_CODE s3c_mfc_release_alloc_mem(s3c_mfc_inst_ctx  *MfcCtx,  s3c_mfc_args *args);
MFC_ERROR_CODE s3c_mfc_get_phys_addr(s3c_mfc_inst_ctx *MfcCtx, s3c_mfc_args *args);
MFC_ERROR_CODE s3c_mfc_get_virt_addr(s3c_mfc_inst_ctx  *MfcCtx,  s3c_mfc_args *args);

#endif /* _S3C_MFC_BUFFER_MANAGER_H_ */

