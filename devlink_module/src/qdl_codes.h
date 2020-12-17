/*************************************************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2020 Intel Corporation.
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

#ifndef QDL_CODES_H_
#define QDL_CODES_H_

/* Errors */
#define QDL_SUCCESS                     0
#define QDL_INVALID_PARAMS              1
#define QDL_MEMORY_ERROR                2
#define QDL_OPEN_SOCKET_ERROR           3
#define QDL_DEVICE_NOT_FOUND            4
#define QDL_SEND_MSG_ERROR              5
#define QDL_RECEIVE_MSG_ERROR           6
#define QDL_BUFFER_TOO_SMALL_ERROR      7
#define QDL_PARSE_MSG_ERROR             8
#define QDL_NO_PCI_RESOURCES            9

/* Statuses */
#define QDL_MSG_END_OF_DUMP             100

#endif /* QDL_CODES_H_ */
