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
#include <limits.h>

#include "dds/dds.h"
#include "CUnit/Theory.h"
#include "config_env.h"
#include "BartData.h"

#include "dds/version.h"
#include "dds__entity.h"
#include "dds/ddsi/q_entity.h"
#include "dds/ddsi/ddsi_entity_index.h"
#include "dds/ddsrt/cdtors.h"
#include "dds/ddsrt/misc.h"
#include "dds/ddsrt/process.h"
#include "dds/ddsrt/threads.h"
#include "dds/ddsrt/environ.h"
#include "dds/ddsrt/atomics.h"
#include "dds/ddsrt/time.h"

#define DDS_DOMAINID_PUB 0
#define DDS_DOMAINID_SUB 1
#define DDS_CONFIG_NO_PORT_GAIN "${CYCLONEDDS_URI}${CYCLONEDDS_URI:+,}<Discovery><ExternalDomainId>0</ExternalDomainId></Discovery>"
#define DDS_CONFIG_NO_PORT_GAIN_LOG "${CYCLONEDDS_URI}${CYCLONEDDS_URI:+,}<Tracing><OutputFile>cyclonedds_liveliness_tests.${CYCLONEDDS_DOMAIN_ID}.${CYCLONEDDS_PID}.log</OutputFile><Verbosity>finest</Verbosity></Tracing><Discovery><ExternalDomainId>0</ExternalDomainId></Discovery>"

static dds_entity_t g_pub_domain = 0;
static dds_entity_t g_pub_participant = 0;

static dds_entity_t g_sub_domain = 0;
static dds_entity_t g_sub_participant = 0;

#define MAX_SAMPLES 1

static void bart_init(void)
{
  /* Domains for pub and sub use a different domain id, but the portgain setting
   * in configuration is 0, so that both domains will map to the same port number.
   * This allows to create two domains in a single test process. */
  char *conf_pub = ddsrt_expand_envvars(DDS_CONFIG_NO_PORT_GAIN, DDS_DOMAINID_PUB);
  char *conf_sub = ddsrt_expand_envvars(DDS_CONFIG_NO_PORT_GAIN, DDS_DOMAINID_SUB);
  g_pub_domain = dds_create_domain(DDS_DOMAINID_PUB, conf_pub);
  g_sub_domain = dds_create_domain(DDS_DOMAINID_SUB, conf_sub);
  dds_free(conf_pub);
  dds_free(conf_sub);

  CU_ASSERT_FATAL((g_pub_participant = dds_create_participant(DDS_DOMAINID_PUB, NULL, NULL)) > 0);
  CU_ASSERT_FATAL((g_sub_participant = dds_create_participant(DDS_DOMAINID_SUB, NULL, NULL)) > 0);
}

static void bart_fini(void)
{
  CU_ASSERT_EQUAL_FATAL(dds_delete(g_pub_participant), DDS_RETCODE_OK);
  CU_ASSERT_EQUAL_FATAL(dds_delete(g_sub_participant), DDS_RETCODE_OK);
  CU_ASSERT_EQUAL_FATAL(dds_delete(g_pub_domain), DDS_RETCODE_OK);
  CU_ASSERT_EQUAL_FATAL(dds_delete(g_sub_domain), DDS_RETCODE_OK);
}

CU_Test(ddsc_bart, test1, .init = bart_init, .fini = bart_fini)
{
  dds_entity_t pub_topic = 0;
  dds_entity_t sub_topic = 0;
  dds_entity_t pub_writer = 0;
  dds_entity_t sub_reader = 0;
  dds_qos_t *qos;
  BartData_Msg msg;
  BartData_Msg *msg2;
  void *samples[MAX_SAMPLES];
  dds_sample_info_t infos[MAX_SAMPLES];
  uint32_t status = 0;
  dds_return_t rc;

  CU_ASSERT_FATAL((pub_topic = dds_create_topic(g_pub_participant, &BartData_Msg_desc, "BartData_Msg", NULL, NULL)) > 0);
  CU_ASSERT_FATAL((pub_writer = dds_create_writer(g_pub_participant, pub_topic, NULL, NULL)) > 0);

  CU_ASSERT_FATAL((sub_topic = dds_create_topic(g_sub_participant, &BartData_Msg_desc, "BartData_Msg", NULL, NULL)) > 0);

  CU_ASSERT_FATAL((qos = dds_create_qos()) != NULL);
  dds_qset_reliability(qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  CU_ASSERT_FATAL((sub_reader = dds_create_reader(g_sub_participant, sub_topic, qos, NULL)) > 0);
  dds_delete_qos(qos);

  CU_ASSERT_EQUAL_FATAL(dds_set_status_mask(pub_writer, DDS_PUBLICATION_MATCHED_STATUS), DDS_RETCODE_OK);

  while (!(status & DDS_PUBLICATION_MATCHED_STATUS)) {
    CU_ASSERT_EQUAL_FATAL(dds_get_status_changes(pub_writer, &status), DDS_RETCODE_OK);
    dds_sleepfor(DDS_MSECS(20));
  }

  msg.userID = 1;
  msg.message = "Hello World";
  printf ("=== [Publisher]  Writing : ");
  printf ("Message (%"PRId32", %s)\n", msg.userID, msg.message);

  CU_ASSERT_EQUAL_FATAL(dds_write(pub_writer, &msg), DDS_RETCODE_OK);

  printf ("\n=== [Subscriber] Waiting for a sample ...\n");

  samples[0] = BartData_Msg__alloc();

  for (;;) {
	rc = dds_read(sub_reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
    //CU_ASSERT_FATAL(rc < 0);

    if ((rc > 0) && (infos[0].valid_data)) {
        msg2 = (BartData_Msg *)samples[0];
        printf ("=== [Subscriber] Received : ");
        printf ("Message (%"PRId32", %s)\n", msg2->userID, msg2->message);
        break;
    } else {
        dds_sleepfor(DDS_MSECS(20));
    }
  }

  BartData_Msg_free(samples[0], DDS_FREE_ALL);
}
