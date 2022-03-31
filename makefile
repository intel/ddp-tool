##############################################################################################################
# Copyright (C) 2019 Intel Corporation                                                                       #
#                                                                                                            #
# Redistribution and use in source and binary forms, with or without modification, are permitted provided    #
# that the following conditions are met:                                                                     #
#                                                                                                            #
# 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the  #
#    following disclaimer.                                                                                   #
# 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and   #
#      the following disclaimer in the documentation and/or other materials provided with the distribution.  #
# 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or    #
#    promote products derived from this software without specific prior written permission.                  #
#                                                                                                            #
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED     #
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A     #
# PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR   #
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED #
# TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)  #
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING   #
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE        #
# POSSIBILITY OF SUCH DAMAGE.                                                                                #
#                                                                                                            #
# SPDX-License-Identifier: BSD-3-Clause                                                                      #
##############################################################################################################

CC=gcc

# Add general SDLe flags for secure compilation
CFLAGS= -fstack-protector -fPIE -fPIC -Wformat -Wformat-security -Wall

# Add flags preventing compiler from optimizing security checks
CFLAGS  += -fno-delete-null-pointer-checks -fno-strict-overflow -fwrapv

LDFLAGS= -z noexecstack -z relro -z now -pie

OBJ_DEVLINK = devlink_module/src/qdl.o devlink_module/src/qdl_msg.o devlink_module/src/qdl_pci.o devlink_module/src/qdl_debug.o
OBJ= src/ddp.o src/ddp_list.o src/os.o src/cmdparams.o src/output.o src/i40e.o src/ice.o $(OBJ_DEVLINK)

ifeq ($(type), debug)
# No code optimization, produce debugging information
	CFLAGS+= -O0 -g
else
# High code optimization, perform some implicit buffer overflow checks, make all warnings errors
	CFLAGS+= -O2 -D_FORTIFY_SOURCE=2 -Werror -Wl,-strip-debug
endif

# Add include directories of depentent libraries
CFLAGS  += -I./inc -I./src -I./devlink_module/src

ddptool: $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^ $(CFLAGS)
	rm src/*.o devlink_module/src/*.o

clean:
	rm src/*.o devlink_module/src/*.o
