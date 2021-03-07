/*************************************************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 - 2021 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials, and your use of them is governed
 * by the express license under which they were provided to you ("License"). Unless the License provides
 * otherwise, you may not use, modify, copy, publish, distribute, disclose or transmit this software or the
 * related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express or implied warranties, other
 * than those that are expressly stated in the License.
 *
 ************************************************************************************************************/

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

