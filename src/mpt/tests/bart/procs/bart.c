/*
 * Copyright(c) 2019 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "mpt/mpt.h"
#include "dds/dds.h"
#include "BartData.h"
#include "bart.h"

#define MAX_SAMPLES 1

void bart_init (void)
{
}

void bart_fini (void)
{
}

static void create_participant_and_topic( dds_domainid_t domainid, dds_entity_t *participant, dds_entity_t *topic )
{
  /* Create a Participant. */
  *participant = dds_create_participant (domainid, NULL, NULL);
  if (*participant < 0)
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-(*participant)));

  /* Create a Topic. */
  *topic = dds_create_topic (
    *participant, &BartData_Msg_desc, "BartData_Msg", NULL, NULL);
  if (*topic < 0)
    DDS_FATAL("dds_create_topic: %s\n", dds_strretcode(-(*topic)));
}

static void create_writer( dds_entity_t *writer, dds_entity_t participant, dds_entity_t topic )
{
  /* Create a Writer. */
  *writer = dds_create_writer (participant, topic, NULL, NULL);
  if (*writer < 0)
    DDS_FATAL("dds_create_write: %s\n", dds_strretcode(-(*writer)));
}

static void create_reliable_reader( dds_entity_t *reader, dds_entity_t participant, dds_entity_t topic )
{
  dds_qos_t *qos;

  /* Create a reliable Reader. */
  qos = dds_create_qos ();
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  *reader = dds_create_reader (participant, topic, qos, NULL);
  if (*reader < 0)
    DDS_FATAL("dds_create_reader: %s\n", dds_strretcode(-(*reader)));
  dds_delete_qos(qos);

}

static void delete_participant( dds_entity_t participant )
{
  dds_return_t rc;

  /* Deleting the participant will delete all its children recursively as well. */
  rc = dds_delete (participant);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));
}

MPT_ProcessEntry (bart_publisher,
                  MPT_Args (dds_domainid_t domainid))
{
  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t writer;
  dds_return_t rc;
  BartData_Msg msg;
  uint32_t status = 0;

  (void)mpt__retval__;
  (void)mpt__args__;

  create_participant_and_topic( domainid, &participant, &topic );

  create_writer( &writer, participant, topic );
  
  printf("=== [Publisher]  Waiting for a reader to be discovered ...\n");
  fflush (stdout);

  rc = dds_set_status_mask(writer, DDS_PUBLICATION_MATCHED_STATUS);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_set_status_mask: %s\n", dds_strretcode(-rc));

  while(!(status & DDS_PUBLICATION_MATCHED_STATUS))
  {
    rc = dds_get_status_changes (writer, &status);
    if (rc != DDS_RETCODE_OK)
      DDS_FATAL("dds_get_status_changes: %s\n", dds_strretcode(-rc));

    /* Polling sleep. */
    dds_sleepfor (DDS_MSECS (20));
  }

  /* Create a message to write. */
  msg.userID = 1;
  msg.message = "Hello World";

  printf ("=== [Publisher]  Writing : ");
  printf ("Message (%"PRId32", %s)\n", msg.userID, msg.message);
  fflush (stdout);

  rc = dds_write (writer, &msg);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_write: %s\n", dds_strretcode(-rc));

  delete_participant( participant );
}

MPT_ProcessEntry (bart_subscriber,
                  MPT_Args (dds_domainid_t domainid))
{
  dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t reader;
  BartData_Msg *msg;
  void *samples[MAX_SAMPLES];
  dds_sample_info_t infos[MAX_SAMPLES];
  dds_return_t rc;

  (void)mpt__retval__;
  (void)mpt__args__;

  create_participant_and_topic( domainid, &participant, &topic );

  create_reliable_reader( &reader, participant, topic );
  
  printf ("\n=== [Subscriber] Waiting for a sample ...\n");
  fflush (stdout);

  /* Initialize sample buffer, by pointing the void pointer within
   * the buffer array to a valid sample memory location. */
  samples[0] = BartData_Msg__alloc ();

  /* Poll until data has been read. */
  while (true)
  {
    /* Do the actual read.
     * The return value contains the number of read samples. */
    rc = dds_read (reader, samples, infos, MAX_SAMPLES, MAX_SAMPLES);
    if (rc < 0)
      DDS_FATAL("dds_read: %s\n", dds_strretcode(-rc));

    /* Check if we read some data and it is valid. */
    if ((rc > 0) && (infos[0].valid_data))
    {
      /* Print Message. */
      msg = (BartData_Msg*) samples[0];
      printf ("=== [Subscriber] Received : ");
      printf ("Message (%"PRId32", %s)\n", msg->userID, msg->message);
      fflush (stdout);
      break;
    }
    else
    {
      /* Polling sleep. */
      dds_sleepfor (DDS_MSECS (20));
    }
  }

  /* Free the data location. */
  BartData_Msg_free (samples[0], DDS_FREE_ALL);

  delete_participant( participant );
}
