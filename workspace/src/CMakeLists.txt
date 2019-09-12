#
# Copyright(c) 2006 to 2019 ADLINK Technology Limited and others
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License v. 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
# v. 1.0 which is available at
# http://www.eclipse.org/org/documents/edl-v10.php.
#
# SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
#
cmake_minimum_required(VERSION 3.7)

add_executable(FreeRTOSmain main.c)
target_link_libraries(FreeRTOSmain CycloneDDS::ddsc)

#if(WIN32)
#  target_compile_definitions(pubsub PRIVATE _CRT_SECURE_NO_WARNINGS)
#endif()

install(
  TARGETS FreeRTOSmain
  DESTINATION "${CMAKE_INSTALL_BINDIR}"
  COMPONENT dev
)
