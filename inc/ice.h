/*************************************************************************************************************
* Copyright (C) 2020 Intel Corporation                                                                       *
*                                                                                                            *
* Redistribution and use in source and binary forms, with or without modification, are permitted provided    *
* that the following conditions are met:                                                                     *
*                                                                                                            *
* 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the  *
*    following disclaimer.                                                                                   *
* 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and   *
*      the following disclaimer in the documentation and/or other materials provided with the distribution.  *
* 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or    *
*    promote products derived from this software without specific prior written permission.                  *
*                                                                                                            *
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED     *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A     *
* PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR   *
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)  *
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING   *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE        *
* POSSIBILITY OF SUCH DAMAGE.                                                                                *
*                                                                                                            *
* SPDX-License-Identifier: BSD-3-Clause                                                                      *
*************************************************************************************************************/

#ifndef _DEF_ICE_H_
#define _DEF_ICE_H_

#include "ddp_types.h"
#include "ddp.h"

#define ICE_MIN_FW_VERSION_MAJOR         5
#define ICE_MIN_FW_VERSION_MINOR         2
#define ICE_MIN_FW_VERSION_PATCH         1

#define ICE_ADMINQ_FLAG_BUF              0x1000 /* no additional buffer */

/* CVL CSR FW Admin Queue access defines */
#define ICE_GL_HIBA_REGISTER             0x00081000 /* Host Interface Buffer Area */
#define ICE_GL_HIDA_REGISTER             0x00082000 /* Host Interface Descriptor Area */
#define ICE_GL_HIDA2_REGISTER            0x00082020 /* Host Interface Descriptor Area */
#define ICE_GL_HICR_REGISTER             0x00082040 /* Host Interface Control Register */
#define ICE_GL_HICR_EN_REGISTER          0x00082044 /* Host Interface Enable Register */

#define GL_HIBA(_i)                      (ICE_GL_HIBA_REGISTER + ((_i) * 4))
#define GL_HIDA(_i)                      (ICE_GL_HIDA_REGISTER + ((_i) * 4))
#define GL_HIDA_2(_i)                    (ICE_GL_HIDA2_REGISTER + ((_i) * 4))

#define ICE_GL_HICR_EN_ENABLE_BIT        (1 << 0) /* 1 */
#define ICE_GL_HICR_COMMAND_BIT          (1 << 1) /* 2 */
#define ICE_GL_HICR_STATUS_VALID_BIT     (1 << 2) /* 4 */
#define ICE_GL_HICR_EVENT_VALID_BIT      (1 << 3) /* 8 */

#define ICE_DESC_COOKIE_L_DWORD_OFFSET   3

#define ICE_AQ_FLAG_BUF                  (1 << 12)  /* 0x1000 */
#define ICE_AQ_FLAG_SI                   (1 << 13)  /* 0x2000 */
#define ICE_LOCK_SEMAPHORE_VALUE         0xBABABABA /* Lock semaphore value */

#define ICE_PROFILE_NAME_LENGTH          28
#define ICE_PROFILES_NUMBER              3

#define ICE_DEV_ID_ADAPTIVE_VF           0x1889

/* AdminQ commands */
#define ICE_ADMINQ_COMMAND_GET_VERSION          0x1
#define ICE_ADMINQ_COMMAND_GET_DDP_PROFILE_LIST 0x0C43

/* [DCR-3443] Timestamp spacing for Tools AQ: queue is active if spacing is within the range [LO..HI] */
#define ICE_TOOLSQ_ACTIVE_STAMP_SPACING_LO      0
#define ICE_TOOLSQ_ACTIVE_STAMP_SPACING_HI      200

/* [DCR-3443] Timestamp spacing for Tools AQ: queue is expired if spacing is outside the range [LO..HI] */
#define ICE_TOOLSQ_EXPIRED_STAMP_SPACING_LO     -5
#define ICE_TOOLSQ_EXPIRED_STAMP_SPACING_HI     205

extern driver_os_context_t Global_driver_os_ctx[family_last];

typedef struct _ice_ddp_profile_t{
    ddp_profile_version_t version;
    char                  name[ICE_PROFILE_NAME_LENGTH];
    uint32_t              track_id;
    uint8_t               is_in_nvm;
    uint8_t               is_active;
    uint8_t               is_active_at_boot;
    uint8_t               is_modified;
} ice_ddp_profile_t;

typedef struct _ice_aqc_get_ver_t{
    uint32_t rom_ver;
    uint32_t fw_build;
    uint8_t  fw_branch;
    uint8_t  fw_major;
    uint8_t  fw_minor;
    uint8_t  fw_patch;
    uint8_t  api_branch;
    uint8_t  api_major;
    uint8_t  api_minor;
    uint8_t  api_patch;
} ice_aqc_get_ver_t;

typedef struct _ice_profiles_info_t{
    uint32_t              count;
    ice_ddp_profile_t     profile[ICE_PROFILES_NUMBER];
} ice_profiles_info_t;

ddp_status_t
ice_verify_driver(void);

void
ice_initialize_device(adapter_t* adapter);

#endif /* _DEF_ICE_H_ */
