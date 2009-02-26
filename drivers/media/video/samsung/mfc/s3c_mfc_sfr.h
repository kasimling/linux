#ifndef _S3C_MFC_SFR_H
#define _S3C_MFC_SFR_H

#include "s3c_mfc_types.h"
#include "s3c_mfc_base.h"

int s3c_mfc_sleep(void);
int s3c_mfc_wakeup(void);
BOOL s3c_mfc_issue_command(int inst_no, s3c_mfc_codec_mode_t codec_mode, s3c_mfc_command_t mfc_cmd);
int  s3c_mfc_get_firmware_version(void);

void s3c_mfc_reset(void);
void s3c_mfc_clear_intr(void);
unsigned int s3c_mfc_intr_reason(void);
void s3c_mfc_set_eos(int buffer_mode);
void s3c_mfc_stream_end(void);
void s3c_mfc_firmware_into_code_down_reg(void);
void s3c_mfc_start_bit_processor(void);
void s3c_mfc_stop_bit_processor(void);
void s3c_mfc_config_sfr_bitproc_buffer(void);
void s3c_mfc_config_sfr_ctrl_opts(void);


#endif /* _S3C_MFC_SFR_H */
