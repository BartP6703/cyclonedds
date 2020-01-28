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

set(sources_handshake
    "procs/handshake.c"
    "handshake.c")
add_mpt_executable(mpt_handshake_exe ${sources_handshake})

target_include_directories(
mpt_handshake_exe PRIVATE
"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../../core/ddsi/include>")

include(GenerateExportHeader)
include(CUnit)

set(authentication_handshake_test_sources
  "handshake.c")

target_link_libraries(mpt_handshake_exe PRIVATE security_api)
target_link_libraries(mpt_handshake_exe PRIVATE ddsc)
target_link_libraries(mpt_handshake_exe PRIVATE dds_security_auth)

set(CUnit_plugin_mock_dir ".")

# Let the cunit application know the location and name of the library.
configure_file("config_env.h.in" "config_env.h")

function(add_mock libname casename)
  PREPEND(srcs_mock "${CMAKE_CURRENT_LIST_DIR}/mock"
    "mock_${libname}.c")
  add_library("dds_security_${casename}" SHARED "")
  
  generate_export_header(
    "dds_security_${casename}"
    BASE_NAME SECURITY
    EXPORT_FILE_NAME "${CMAKE_CURRENT_LIST_DIR}/mock/${casename}_export.h")
  
  set_target_properties(
    "dds_security_${casename}"
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY                ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY                ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY_DEBUG          ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY_RELEASE        ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL     ${CMAKE_CURRENT_BINARY_DIR})

  #find_package(OpenSSL )
  #target_link_libraries("dds_security_${casename}" PUBLIC OpenSSL::SSL)
  #target_link_libraries("dds_security_${casename}" PUBLIC ddsc)
  #target_link_libraries("dds_security_${casename}" PUBLIC dds_security_auth)
  target_link_libraries(mpt_handshake_exe PRIVATE dds_security_${casename})
  target_link_libraries(mpt_handshake_exe PRIVATE dds_security_auth)
  
  target_sources("dds_security_${casename}"
    PRIVATE
      ${srcs_mock})
  target_include_directories("dds_security_${casename}"
    PUBLIC
      "$<BUILD_INTERFACE:$<TARGET_PROPERTY:security_api,INTERFACE_INCLUDE_DIRECTORIES>>"
      "$<BUILD_INTERFACE:$<TARGET_PROPERTY:security_core,INTERFACE_INCLUDE_DIRECTORIES>>"
      "$<BUILD_INTERFACE:$<TARGET_PROPERTY:ddsrt,INTERFACE_INCLUDE_DIRECTORIES>>"
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>")
endfunction()

#PLUGIN MOCKS
add_mock(authentication handshake)
