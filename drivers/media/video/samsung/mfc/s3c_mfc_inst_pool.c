/*
 * Project Name MFC DRIVER
 * Copyright (c) Samsung Electronics 
 * All right reserved.
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics  ("Confidential Information").   
 * you shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung Electronics 
 *
 * This file stores information about different instancesof MFC.
 *
 * @name MFC DRIVER MODULE Module (MFC_Inst_Pool.c)
 * @author Simon Chun (simon.chun@samsung.com)
 */

#include "s3c_mfc_config.h"
#include "s3c_mfc_inst_pool.h"

#if !defined(S3C_MFC_NUM_INSTANCES_MAX)
#error "S3C_MFC_NUM_INSTANCES_MAX should be defined."
#endif
#if ((S3C_MFC_NUM_INSTANCES_MAX <= 0) || (S3C_MFC_NUM_INSTANCES_MAX > 8))
#error "S3C_MFC_NUM_INSTANCES_MAX should be in the range of 1 ~ 8."
#endif

static int s3c_mfc_inst_no = 0;
static int s3c_mfc_inst_status[S3C_MFC_NUM_INSTANCES_MAX] = {0, };
static int s3c_mfc_num_inst_avail = S3C_MFC_NUM_INSTANCES_MAX;

int s3c_mfc_inst_pool_num_avail(void)
{
	return s3c_mfc_num_inst_avail;
}

int s3c_mfc_inst_pool_occupy(void)
{
	int  i;

	if (s3c_mfc_num_inst_avail == 0)
		return -1;

	for (i = 0; i < S3C_MFC_NUM_INSTANCES_MAX; i++) {
		if (s3c_mfc_inst_status[s3c_mfc_inst_no] == 0) {
			s3c_mfc_num_inst_avail--;
			s3c_mfc_inst_status[s3c_mfc_inst_no] = 1;
			return s3c_mfc_inst_no;
		}

		s3c_mfc_inst_no = (s3c_mfc_inst_no + 1) % S3C_MFC_NUM_INSTANCES_MAX;
	}

	return -1;
}


int s3c_mfc_inst_pool_release(int instance_no)
{
	if (instance_no >= S3C_MFC_NUM_INSTANCES_MAX || instance_no < 0) {
		return -1;
	}

	if (s3c_mfc_inst_status[instance_no] == 0)
		return -1;

	s3c_mfc_num_inst_avail++;
	s3c_mfc_inst_status[instance_no] = 0;

	return instance_no;
}


void s3c_mfc_inst_pool_occypy_all(void)
{
	int  i;

	if (s3c_mfc_num_inst_avail == 0)
		return;

	for (i = 0; i < S3C_MFC_NUM_INSTANCES_MAX; i++) {
		if (s3c_mfc_inst_status[i] == 0) {
			s3c_mfc_num_inst_avail--;
			s3c_mfc_inst_status[i] = 1;
		}
	}
}

void s3c_mfc_inst_pool_release_all(void)
{
	int  i;

	if (s3c_mfc_num_inst_avail == S3C_MFC_NUM_INSTANCES_MAX)
		return;

	for (i = 0; i < S3C_MFC_NUM_INSTANCES_MAX; i++) {
		if (s3c_mfc_inst_status[i] == 1) {
			s3c_mfc_num_inst_avail++;
			s3c_mfc_inst_status[i] = 0;
		}
	}
}
