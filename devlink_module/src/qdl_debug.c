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

#include <stdarg.h>
#include <stdio.h>

static FILE* global_debug_fp = NULL;

/**
 * qdl_enable_debuglog
 * @fp: file pointer for logs, if NULL logs will be printed to stdout
 *
 * Enables debug logs for devlink module.
 */
void qdl_enable_debuglog(FILE* fp)
{
	if(fp == NULL) {
		global_debug_fp = stdout;
	} else {
		global_debug_fp = fp;
	}
}

/**
 * qdl_disable_debuglog
 *
 * Disables debug logs for devlink module.
 */
void qdl_disable_debuglog(void)
{
	global_debug_fp = NULL;
}

/**
 * _qdl_print_debug
 * @format: format string
 *
 * Prints debug message.
 */
void
_qdl_print_debug(char* format, ...)
{
	va_list arg_ptr;

	if(global_debug_fp != NULL) {
		va_start(arg_ptr, format);
		vfprintf(global_debug_fp, format, arg_ptr);
		va_end(arg_ptr);
	}
}

