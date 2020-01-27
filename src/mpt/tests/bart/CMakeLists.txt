#
# Copyright(c) 2019 ADLINK Technology Limited and others
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v. 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
# v. 1.0 which is available at
# http://www.eclipse.org/org/documents/edl-v10.php.
#
# SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
#
include(${MPT_CMAKE})

idlc_generate(bart_BartData_lib "procs/BartData.idl")

set(sources_bart
    "procs/bart.c"
    "bart.c")
add_mpt_executable(mpt_bart_exe ${sources_bart})

target_include_directories(
mpt_bart_exe PRIVATE
"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/ddsi/include>")

target_link_libraries(mpt_bart_exe PRIVATE bart_BartData_lib)
