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

#ifndef _QDL_DEBUG_H_
#define _QDL_DEBUG_H_

#include <stdio.h>

void qdl_enable_debuglog(FILE* fp);
void _qdl_print_debug(char* format, ...);
void qdl_disable_debuglog(void);

#define QDL_DEBUGLOG_ENTERING                         _qdl_print_debug("%s:%s:%d: Entering...\n", __FILE__, __FUNCTION__, __LINE__)
#define QDL_DEBUGLOG_ERROR_MSG(msg)                   _qdl_print_debug("%s:%s:%d: %s\n", __FILE__, __FUNCTION__, __LINE__, msg)
#define QDL_DEBUGLOG_FUNCTION_FAIL(function, status)  _qdl_print_debug("%s:%s:%d: %s failed - error: %d\n", __FILE__, __FUNCTION__, __LINE__, function, status)

#endif /* _QDL_DEBUG_H_ */
