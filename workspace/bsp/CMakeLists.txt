#
# Copyright(c) 2006 to 2018 ADLINK Technology Limited and others
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
project(LwIP VERSION 2.1.0)

include(GNUInstallDirs)

# Some distributions place libraries in lib64 when the architecture is x86_64,
# but since the simulator is compiled with -m32, lib is a better name.
if(UNIX AND CMAKE_INSTALL_LIBDIR STREQUAL "lib64")
  set(CMAKE_INSTALL_LIBDIR "lib")
endif()

# Conflicts may be introduced when placing the libraries or headers in the
# default system locations, i.e. /usr/lib and /usr/include on *NIX platforms.
# The install prefix must therefore be postfixed with the project name.
if(UNIX)
  set(CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/${CMAKE_PROJECT_NAME}")
endif()

set(LWIP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libsrc/lwip210/"
      CACHE STRING "Location of LwIP sources")

set(source_path "${LWIP_SOURCE_DIR}/src/lwip-2.1.0/src")

message("source_path:${source_path}")

list(APPEND sources
  "${source_path}/netif/bridgeif.c"
  "${source_path}/netif/bridgeif_fdb.c"
  "${source_path}/netif/ethernet.c"
  "${source_path}/netif/lowpan6_ble.c"
  "${source_path}/netif/lowpan6.c"
  "${source_path}/netif/lowpan6_common.c"
  "${source_path}/netif/slipif.c"
  "${source_path}/netif/zepif.c"
  "${source_path}/netif/ppp/auth.c"
  "${source_path}/netif/ppp/ccp.c"
  "${source_path}/netif/ppp/chap-md5.c"
  "${source_path}/netif/ppp/chap_ms.c"
  "${source_path}/netif/ppp/chap-new.c"
  "${source_path}/netif/ppp/demand.c"
  "${source_path}/netif/ppp/eap.c"
  "${source_path}/netif/ppp/ecp.c"
  "${source_path}/netif/ppp/eui64.c"
  "${source_path}/netif/ppp/fsm.c"
  "${source_path}/netif/ppp/ipcp.c"
  "${source_path}/netif/ppp/ipv6cp.c"
  "${source_path}/netif/ppp/lcp.c"
  "${source_path}/netif/ppp/magic.c"
  "${source_path}/netif/ppp/mppe.c"
  "${source_path}/netif/ppp/multilink.c"
  "${source_path}/netif/ppp/pppapi.c"
  "${source_path}/netif/ppp/ppp.c"
  "${source_path}/netif/ppp/pppcrypt.c"
  "${source_path}/netif/ppp/pppoe.c"
  "${source_path}/netif/ppp/pppol2tp.c"
  "${source_path}/netif/ppp/pppos.c"
  "${source_path}/netif/ppp/upap.c"
  "${source_path}/netif/ppp/utils.c"
  "${source_path}/netif/ppp/vj.c"
  "${source_path}/netif/ppp/polarssl/arc4.c"
  "${source_path}/netif/ppp/polarssl/des.c"
  "${source_path}/netif/ppp/polarssl/md4.c"
  "${source_path}/netif/ppp/polarssl/md5.c"
  "${source_path}/netif/ppp/polarssl/sha1.c"
  "${source_path}/core/altcp_alloc.c"
  "${source_path}/core/altcp.c"
  "${source_path}/core/altcp_tcp.c"
  "${source_path}/core/def.c"
  "${source_path}/core/dns.c"
  "${source_path}/core/inet_chksum.c"
  "${source_path}/core/init.c"
  "${source_path}/core/ip.c"
  "${source_path}/core/mem.c"
  "${source_path}/core/memp.c"
  "${source_path}/core/netif.c"
  "${source_path}/core/pbuf.c"
  "${source_path}/core/raw.c"
  "${source_path}/core/stats.c"
  "${source_path}/core/sys.c"
  "${source_path}/core/tcp.c"
  "${source_path}/core/tcp_in.c"
  "${source_path}/core/tcp_out.c"
  "${source_path}/core/timeouts.c"
  "${source_path}/core/udp.c"
  "${source_path}/core/ipv6/dhcp6.c"
  "${source_path}/core/ipv6/ethip6.c"
  "${source_path}/core/ipv6/icmp6.c"
  "${source_path}/core/ipv6/inet6.c"
  "${source_path}/core/ipv6/ip6_addr.c"
  "${source_path}/core/ipv6/ip6.c"
  "${source_path}/core/ipv6/ip6_frag.c"
  "${source_path}/core/ipv6/mld6.c"
  "${source_path}/core/ipv6/nd6.c"
  "${source_path}/core/ipv4/autoip.c"
  "${source_path}/core/ipv4/dhcp.c"
  "${source_path}/core/ipv4/etharp.c"
  "${source_path}/core/ipv4/icmp.c"
  "${source_path}/core/ipv4/igmp.c"
  "${source_path}/core/ipv4/ip4_addr.c"
  "${source_path}/core/ipv4/ip4.c"
  "${source_path}/core/ipv4/ip4_frag.c"
  "${source_path}/api/api_lib.c"
  "${source_path}/api/api_msg.c"
  "${source_path}/api/err.c"
  "${source_path}/api/if_api.c"
  "${source_path}/api/netbuf.c"
  "${source_path}/api/netdb.c"
  "${source_path}/api/netifapi.c"
  "${source_path}/api/sockets.c"
  "${source_path}/api/tcpip.c")

list(APPEND headers
  "${source_path}/include/compat/posix/netdb.h"
  "${source_path}/include/compat/posix/arpa/inet.h"
  "${source_path}/include/compat/posix/net/if.h"
  "${source_path}/include/compat/posix/sys/socket.h"
  "${source_path}/include/compat/stdc/errno.h"
  "${source_path}/include/lwip/altcp.h"
  "${source_path}/include/lwip/altcp_tcp.h"
  "${source_path}/include/lwip/altcp_tls.h"
  "${source_path}/include/lwip/api.h"
  "${source_path}/include/lwip/arch.h"
  "${source_path}/include/lwip/autoip.h"
  "${source_path}/include/lwip/debug.h"
  "${source_path}/include/lwip/def.h"
  "${source_path}/include/lwip/dhcp6.h"
  "${source_path}/include/lwip/dhcp.h"
  "${source_path}/include/lwip/dns.h"
  "${source_path}/include/lwip/err.h"
  "${source_path}/include/lwip/errno.h"
  "${source_path}/include/lwip/etharp.h"
  "${source_path}/include/lwip/ethip6.h"
  "${source_path}/include/lwip/icmp6.h"
  "${source_path}/include/lwip/icmp.h"
  "${source_path}/include/lwip/if_api.h"
  "${source_path}/include/lwip/igmp.h"
  "${source_path}/include/lwip/inet_chksum.h"
  "${source_path}/include/lwip/inet.h"
  "${source_path}/include/lwip/init.h"
  "${source_path}/include/lwip/ip4_addr.h"
  "${source_path}/include/lwip/ip4_frag.h"
  "${source_path}/include/lwip/ip4.h"
  "${source_path}/include/lwip/ip6_addr.h"
  "${source_path}/include/lwip/ip6_frag.h"
  "${source_path}/include/lwip/ip6.h"
  "${source_path}/include/lwip/ip6_zone.h"
  "${source_path}/include/lwip/ip_addr.h"
  "${source_path}/include/lwip/ip.h"
  "${source_path}/include/lwip/mem.h"
  "${source_path}/include/lwip/memp.h"
  "${source_path}/include/lwip/mld6.h"
  "${source_path}/include/lwip/nd6.h"
  "${source_path}/include/lwip/netbuf.h"
  "${source_path}/include/lwip/netdb.h"
  "${source_path}/include/lwip/netifapi.h"
  "${source_path}/include/lwip/netif.h"
  "${source_path}/include/lwip/opt.h"
  "${source_path}/include/lwip/pbuf.h"
  "${source_path}/include/lwip/raw.h"
  "${source_path}/include/lwip/sio.h"
  "${source_path}/include/lwip/snmp.h"
  "${source_path}/include/lwip/sockets.h"
  "${source_path}/include/lwip/stats.h"
  "${source_path}/include/lwip/sys.h"
  "${source_path}/include/lwip/tcpbase.h"
  "${source_path}/include/lwip/tcp.h"
  "${source_path}/include/lwip/tcpip.h"
  "${source_path}/include/lwip/timeouts.h"
  "${source_path}/include/lwip/udp.h"
  "${source_path}/include/lwip/priv/altcp_priv.h"
  "${source_path}/include/lwip/priv/api_msg.h"
  "${source_path}/include/lwip/priv/memp_priv.h"
  "${source_path}/include/lwip/priv/mem_priv.h"
  "${source_path}/include/lwip/priv/memp_std.h"
  "${source_path}/include/lwip/priv/nd6_priv.h"
  "${source_path}/include/lwip/priv/raw_priv.h"
  "${source_path}/include/lwip/priv/sockets_priv.h"
  "${source_path}/include/lwip/priv/tcpip_priv.h"
  "${source_path}/include/lwip/priv/tcp_priv.h"
  "${source_path}/include/lwip/prot/autoip.h"
  "${source_path}/include/lwip/prot/dhcp6.h"
  "${source_path}/include/lwip/prot/dhcp.h"
  "${source_path}/include/lwip/prot/dns.h"
  "${source_path}/include/lwip/prot/etharp.h"
  "${source_path}/include/lwip/prot/ethernet.h"
  "${source_path}/include/lwip/prot/iana.h"
  "${source_path}/include/lwip/prot/icmp6.h"
  "${source_path}/include/lwip/prot/icmp.h"
  "${source_path}/include/lwip/prot/ieee.h"
  "${source_path}/include/lwip/prot/igmp.h"
  "${source_path}/include/lwip/prot/ip4.h"
  "${source_path}/include/lwip/prot/ip6.h"
  "${source_path}/include/lwip/prot/ip.h"
  "${source_path}/include/lwip/prot/mld6.h"
  "${source_path}/include/lwip/prot/nd6.h"
  "${source_path}/include/lwip/prot/tcp.h"
  "${source_path}/include/lwip/prot/udp.h"
  "${source_path}/include/netif/bridgeif.h"
  "${source_path}/include/netif/bridgeif_opts.h"
  "${source_path}/include/netif/etharp.h"
  "${source_path}/include/netif/ethernet.h"
  "${source_path}/include/netif/ieee802154.h"
  "${source_path}/include/netif/lowpan6_ble.h"
  "${source_path}/include/netif/lowpan6_common.h"
  "${source_path}/include/netif/lowpan6.h"
  "${source_path}/include/netif/lowpan6_opts.h"
  "${source_path}/include/netif/slipif.h"
  "${source_path}/include/netif/zepif.h"
  "${source_path}/include/netif/ppp/ccp.h"
  "${source_path}/include/netif/ppp/chap-md5.h"
  "${source_path}/include/netif/ppp/chap_ms.h"
  "${source_path}/include/netif/ppp/chap-new.h"
  "${source_path}/include/netif/ppp/eap.h"
  "${source_path}/include/netif/ppp/ecp.h"
  "${source_path}/include/netif/ppp/eui64.h"
  "${source_path}/include/netif/ppp/fsm.h"
  "${source_path}/include/netif/ppp/ipcp.h"
  "${source_path}/include/netif/ppp/ipv6cp.h"
  "${source_path}/include/netif/ppp/lcp.h"
  "${source_path}/include/netif/ppp/magic.h"
  "${source_path}/include/netif/ppp/mppe.h"
  "${source_path}/include/netif/ppp/pppapi.h"
  "${source_path}/include/netif/ppp/pppcrypt.h"
  "${source_path}/include/netif/ppp/pppdebug.h"
  "${source_path}/include/netif/ppp/ppp.h"
  "${source_path}/include/netif/ppp/ppp_impl.h"
  "${source_path}/include/netif/ppp/pppoe.h"
  "${source_path}/include/netif/ppp/pppol2tp.h"
  "${source_path}/include/netif/ppp/ppp_opts.h"
  "${source_path}/include/netif/ppp/pppos.h"
  "${source_path}/include/netif/ppp/upap.h"
  "${source_path}/include/netif/ppp/vj.h"
  "${source_path}/include/netif/ppp/polarssl/arc4.h"
  "${source_path}/include/netif/ppp/polarssl/des.h"
  "${source_path}/include/netif/ppp/polarssl/md4.h"
  "${source_path}/include/netif/ppp/polarssl/md5.h"
  "${source_path}/include/netif/ppp/polarssl/sha1.h"
  "${source_path}/include/posix/errno.h"
  "${source_path}/include/posix/netdb.h"
  "${source_path}/include/posix/sys/socket.h")

add_library(lwip ${sources})

target_compile_definitions(
  lwip PUBLIC __GCC_POSIX=1 MAX_NUMBER_OF_TASKS=300)

target_compile_options(
  lwip
    PUBLIC
      -m64
    PRIVATE
      -W -Wall -Werror -Wmissing-braces -Wno-cast-align -Wparentheses -Wshadow
      -Wno-sign-compare -Wswitch -Wuninitialized -Wunknown-pragmas
      -Wunused-function -Wunused-label -Wunused-parameter -Wunused-value
      -Wunused-variable -Wmissing-prototypes -mcmodel=large)

target_include_directories(
  lwip
    PUBLIC
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
      "$<BUILD_INTERFACE:${source_path}/include>"
      "$<BUILD_INTERFACE:${source_path}/portable/GCC/POSIX>"
      "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

target_link_libraries(
  lwip PUBLIC -m64 -pthread)

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG" OR
   CMAKE_BUILD_TYPE STREQUAL "RELWITHDEBINFO")
  target_compile_options(lwip PUBLIC -ggdb)
  target_link_libraries(lwip PUBLIC -ggdb)
endif()

target_link_libraries(lwip)

install(
  FILES ${headers}
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

configure_file("lwip.cmake.in" "lwip.cmake" @ONLY)
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/lwip.cmake"
  DESTINATION "${CMAKE_INSTALL_DATADIR}")

install(
  TARGETS lwip
  EXPORT "${CMAKE_PROJECT_NAME}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}")
