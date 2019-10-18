/*
 * Copyright(c) 2006 to 2018 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <lwip/inet.h>
#include <lwip/netif.h> /* netif_list, netif_mutex */
#include <lwip/sockets.h>

#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/io.h"
#include "dds/ddsrt/ifaddrs.h"
#include "dds/ddsrt/retcode.h"
#include "dds/ddsrt/string.h"

extern const int *const os_supp_afs;

static uint32_t
getflags(
  const struct netif *netif,
  const ip_addr_t *addr)
{
  uint32_t flags = 0;

  printf("get_flags: netif->flags:%02x\n", netif->flags);

  if (netif->flags & NETIF_FLAG_UP) {
    flags |= IFF_UP;
    printf("get_flags: netif_flag_up -> %02x (iff_up)\n", flags);
  }
  if (netif->flags & NETIF_FLAG_BROADCAST) {
    flags |= IFF_BROADCAST;
    printf("get_flags: netif_flag_broadcast -> %02x (iff_broadcast)\n", flags);
  }
  if (netif->flags & NETIF_FLAG_IGMP) {
    flags |= IFF_MULTICAST;
    printf("get_flags: netif_flag_igmp -> %02x (iff_multicast)\n", flags);
  }
  if (ip_addr_isloopback(addr)) {
    flags |= IFF_LOOPBACK;
    printf("get_flags: ip_addr_is_loopback -> %02x (iff_loopback)\n", flags);
  }

  printf("get_flags: flags:%02x\n", flags);
  return flags;
}

static void
sockaddr_from_ip_addr(
  struct sockaddr *sockaddr,
  const ip_addr_t *addr)
{
  if (IP_IS_V4(addr)) {
    memset(sockaddr, 0, sizeof(struct sockaddr_in));
    ((struct sockaddr_in *)sockaddr)->sin_len = sizeof(struct sockaddr_in);
    ((struct sockaddr_in *)sockaddr)->sin_family = AF_INET;
    inet_addr_from_ip4addr(&((struct sockaddr_in *)sockaddr)->sin_addr, addr);
#if DDSRT_HAVE_IPV6
  } else {
    assert(IP_IS_V6(addr));
    memset(sockaddr, 0, sizeof(struct sockaddr_in6));
    ((struct sockaddr_in6 *)sockaddr)->sin6_len = sizeof(struct sockaddr_in6);
    ((struct sockaddr_in6 *)sockaddr)->sin6_family = AF_INET6;
    inet6_addr_from_ip6addr(&((struct sockaddr_in6 *)sockaddr)->sin6_addr, addr);
#endif
  }
}

/*static*/ dds_return_t
copyaddr(
  ddsrt_ifaddrs_t **ifap,
  /*const*/ struct netif *netif,
  /*const*/ ip_addr_t *addr)
{
  dds_return_t rc = DDS_RETCODE_OK;
  ddsrt_ifaddrs_t *ifa;
  struct sockaddr_storage sa;

  assert(ifap != NULL);
  assert(netif != NULL);
  assert(addr != NULL);

  sockaddr_from_ip_addr((struct sockaddr *)&sa, addr);

  /* Network interface name is of the form "et0", where the first two letters
     are the "name" field and the digit is the num field of the netif
     structure as described in lwip/netif.h */

  if ((ifa = ddsrt_calloc_s(1, sizeof(*ifa))) == NULL ||
      (ifa->addr = ddsrt_memdup(&sa, sa.s2_len)) == NULL ||
      (ddsrt_asprintf(&ifa->name, "%s%d", netif->name, netif->num) == -1))
  {
    rc = DDS_RETCODE_OUT_OF_RESOURCES;
  } else {
    ifa->flags = getflags(netif, addr);
    ifa->index = netif->num;
    ifa->type = DDSRT_IFTYPE_UNKNOWN;

    if (IP_IS_V4(addr)) {
      static const size_t sz = sizeof(struct sockaddr_in);
      if ((ifa->netmask = ddsrt_calloc_s(1, sz)) == NULL ||
          (ifa->broadaddr = ddsrt_calloc_s(1, sz)) == NULL)
      {
        rc = DDS_RETCODE_OUT_OF_RESOURCES;
      } else {
        ip_addr_t broadaddr = IPADDR4_INIT(
          ip_2_ip4(&netif->ip_addr)->addr |
          ip_2_ip4(&netif->netmask)->addr);

        sockaddr_from_ip_addr((struct sockaddr*)ifa->netmask, &netif->netmask);
        sockaddr_from_ip_addr((struct sockaddr*)ifa->broadaddr, &broadaddr);
      }
    }
  }

  if (rc == DDS_RETCODE_OK) {
    *ifap = ifa;
  } else {
    ddsrt_freeifaddrs(ifa);
  }

  return rc;
}

dds_return_t
ddsrt_getifaddrs(
  ddsrt_ifaddrs_t **ifap,
  const int *afs)
{
  dds_return_t rc = DDS_RETCODE_OK;
  int use_ip4, use_ip6;
  struct netif netif;
  ddsrt_ifaddrs_t *ifa, *next_ifa, *root_ifa;

  assert(ifap != NULL);

  if (afs == NULL) {
    afs = os_supp_afs;
  }

  use_ip4 = use_ip6 = 0;
  for (int i = 0; afs[i] != DDSRT_AF_TERM; i++) {
    if (afs[i] == AF_INET) {
      use_ip4 = 1;
    } else if (afs[i] == AF_INET6) {
      use_ip6 = 1;
    }
  }

  ifa = next_ifa = root_ifa = NULL;

  //for (netif = netif_list; netif != NULL; netif = netif->next)
  //{
  //netif = netif_list;

  pthread_mutex_lock(&netif_mutex);


  if (netif_list != NULL) {
	unsigned char *src = (unsigned char *)netif_list;
	unsigned char *dst = (unsigned char *)&netif;
	printf("offset(flags):%d\n", (int)((void *)&netif_list->flags - (void *)netif_list));
	for (int i = 0; i < (int)sizeof(struct netif); i++) {
	  *dst = *src;
	  printf("%d/%d: %02x -> %02x\n", i, (int)sizeof(struct netif), *src, *dst);
	  dst++;
	  src++;
	}
	//memcpy(&netif, netif_list, sizeof(struct netif));
	netif.flags = 37; //hackhack
	netif.flags |= NETIF_FLAG_IGMP; //hackhack
	netif.name[0] = 'l'; //hackhack
	netif.name[1] = 'o'; //hackhack
	char *name = netif.name;
	u8_t flags = netif.flags;
    printf("ddsrt_getifaddr: netif:%p \"%s\" %02x\n", (void *)&netif_list, name, flags);
    if (use_ip4 && IP_IS_V4(&netif.ip_addr)) {

      printf("ddsrt_getifaddrs5:\n");
      (void)check_list(&netif);
      rc = copyaddr(&next_ifa, &netif, &netif.ip_addr);
      printf("ddsrt_getifaddrs6:\n");
      (void)check_list(&netif);

      if (rc == DDS_RETCODE_OK) {
        if (ifa == NULL) {
          ifa = root_ifa = next_ifa;
        } else {
          ifa->next = next_ifa;
          ifa = next_ifa;
        }
      } else {
    	  //break;
      }
    }

#if DDSRT_HAVE_IPV6
    if (use_ip6) {
      int pref = 1;
again:
      /* List preferred IPv6 address first. */
      for (int i = 0;
               i < LWIP_IPV6_NUM_ADDRESSES && rc == DDS_RETCODE_OK;
               i++)
      {
        if ((ip6_addr_ispreferred(netif->ip_addr_state[i]) &&  pref) ||
            (ip6_addr_isvalid(netif->ip_addr_state[i])     && !pref))
        {
          rc = copyaddr(&next_ifa, netif, &netif->ip_addr[i]);
          if (rc == DDS_RETCODE_OK) {
            if (ifa == NULL) {
              ifa = root_ifa = next_ifa;
            } else {
              ifa->next = next_ifa;
              ifa = next_ifa;
            }
          }
        }
      }

      if (rc == DDS_RETCODE_OK && pref) {
        pref = 0;
        goto again;
      }
    }
#endif
  }

  pthread_mutex_unlock(&netif_mutex);

  if (rc == DDS_RETCODE_OK) {
    *ifap = ifa;
  } else {
    ddsrt_freeifaddrs(root_ifa);
  }

  return rc;
}
