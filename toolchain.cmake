set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_HOST_PROCESSOR}")

set(root "${CMAKE_CURRENT_LIST_DIR}")
set(workspace "${root}/workspace")
set(lwip "${workspace}/bsp/libsrc/lwip210")

set(include_paths "-I${CMAKE_CURRENT_LIST_DIR}/ports/freertos-posix/FreeRTOS-Sim/Source/include -I${CMAKE_CURRENT_LIST_DIR}/ports/freertos-posix/include -I${CMAKE_CURRENT_LIST_DIR}/ports/freertos-posix/build/install/FreeRTOS-Sim/include -I${workspace}/bsp/include -I${lwip}/contrib/ports/unix/lib")

set(library_paths "-L${CMAKE_CURRENT_LIST_DIR}/ports/freertos-posix/build/install/FreeRTOS-Sim/lib -L${lwip}/contrib/ports/unix/lib/build")

set(c_libraries "-lfreertos-sim -lfreertos-sim-loader -lpthread -llwip")
set(cxx_libraries "${c_libraries}")

set(objects "${root}/loader/init.o ${root}/loader/tls.o")

set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

set(CMAKE_C_FLAGS "${include_paths} ${c_flags} ${defines}")
set(CMAKE_CXX_FLAGS "${include_paths} ${cxx_flags} ${defines}")
set(CMAKE_EXE_LINKER_FLAGS "${linker_flags} ${library_paths}")
set(CMAKE_C_STANDARD_LIBRARIES "${c_objects} ${library_paths} ${c_libraries}")
set(CMAKE_CXX_STANDARD_LIBRARIES "${cxx_objects} ${library_paths} ${cxx_libraries}")

set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
