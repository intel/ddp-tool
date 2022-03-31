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

#ifndef _DEF_I40E_H_
#define _DEF_I40E_H_

#include "ddp_types.h"
#include "ddp.h"

#define I40E_ADMINQ_COMMAND_GET_FW_VERSION       0x0001
#define I40E_ADMINQ_COMMAND_GET_DDP_PROFILE_LIST 0x0271

#define I40E_ADMINQ_FLAG_BUF                     0x1000 /* no additional buffer */
#define I40E_ADMINQ_FLAG_SI                      0x2000 /* do not interupt when this commands completes */

#define I40E_MIN_FW_VERSION_MAJOR                6
#define I40E_MIN_FW_VERSION_MINOR                1

#define LINUX_40G_VIRTUAL_DEVID           0x154C
#define BARLEYVILLE_40G_VIRTUAL_DEVID     0xFBFB

extern driver_os_context_t Global_driver_os_ctx[family_last];

ddp_status_t
i40e_verify_driver(void);

void
i40e_initialize_device(adapter_t* adapter);

#endif /* _DEF_I40E_H_ */
