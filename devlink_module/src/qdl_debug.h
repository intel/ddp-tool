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

#ifndef _QDL_DEBUG_H_
#define _QDL_DEBUG_H_

#include <stdio.h>

void qdl_enable_debuglog(FILE* fp);
void _qdl_print_debug(char* format, ...);
void qdl_disable_debuglog(void);

#define QDL_DEBUGLOG_ENTERING                         _qdl_print_debug("%s:%s:%d: Entering...\n", __FILE__, __FUNCTION__, __LINE__)
#define QDL_DEBUGLOG_ERROR_MSG(msg)                   _qdl_print_debug("%s:%s:%d: %s\n", msg)
#define QDL_DEBUGLOG_FUNCTION_FAIL(function, status)  _qdl_print_debug("%s:%s:%d: %s failed - error: %d\n", __FILE__, __FUNCTION__, __LINE__, function, status)

#endif /* _QDL_DEBUG_H_ */
