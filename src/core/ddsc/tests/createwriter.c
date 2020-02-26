#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "dds/dds.h"

#include "CUnit/Test.h"

#include "dds/ddsrt/time.h"
#include "dds/ddsrt/strtol.h"
#include "dds/ddsrt/process.h"
#include "dds/ddsrt/environ.h"
#include "dds/ddsrt/cdtors.h"
#include "dds/ddsrt/sync.h"
#include "dds/ddsrt/hopscotch.h"
#include "dds/ddsrt/threads.h"

#include "createwriter.h"
#include "createwriterdata.h"

#define XASSERT(cond, ...) \
    do { \
        if (!(cond)) { \
            dumplog (logbuf[xs[i]], &logidx[xs[i]]); \
            TST_ASSERT (0, __VA_ARGS__); \
        } \
    } while (0)

#define XASSERT_FATAL(cond, ...) \
    do { \
        if (!(cond)) { \
            dumplog (logbuf[xs[i]], &logidx[xs[i]]); \
            TST_ASSERT_FATAL (0, __VA_ARGS__); \
        } \
    } while (0)

#define DDS_DOMAINID_PUB 0
#define DDS_DOMAINID_SUB 1
#define DDS_CONFIG_NO_PORT_GAIN "${CYCLONEDDS_URI}${CYCLONEDDS_URI:+,}<Discovery><ExternalDomainId>0</ExternalDomainId></Discovery>"
#define DDS_CONFIG_NO_PORT_GAIN_LOG "${CYCLONEDDS_URI}${CYCLONEDDS_URI:+,}<Tracing><OutputFile>cyclonedds_discstress_tests.${CYCLONEDDS_DOMAIN_ID}.${CYCLONEDDS_PID}.log</OutputFile><Verbosity>finest</Verbosity></Tracing><Discovery><ExternalDomainId>0</ExternalDomainId></Discovery>"

static dds_entity_t g_pub_domain = 0;
static dds_entity_t g_pub_participant = 0;

static dds_entity_t g_sub_domain = 0;
static dds_entity_t g_sub_participant = 0;

void createwriter_init_md (void)
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

  g_pub_participant = dds_create_participant(DDS_DOMAINID_PUB, NULL, NULL);
  TST_ASSERT_FATAL_GT (g_pub_participant, 0, "Could not create publisher participant");
  g_sub_participant = dds_create_participant(DDS_DOMAINID_SUB, NULL, NULL);
  TST_ASSERT_FATAL_GT (g_sub_participant, 0, "Could not create subcriber participant");
}

void createwriter_init_sd (void)
{
  g_pub_participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  TST_ASSERT_FATAL_GT (g_pub_participant, 0, "Could not create publisher participant");
  g_sub_participant = dds_create_participant(DDS_DOMAIN_DEFAULT, NULL, NULL);
  TST_ASSERT_FATAL_GT (g_sub_participant, 0, "Could not create subcriber participant");
}

void createwriter_fini (void)
{
}

#define N_WRITERS 3
#define N_READERS 4
#define N_ROUNDS 100
#define DEPTH 7

static dds_return_t get_matched_count_writers (uint32_t *count, dds_entity_t writers[N_WRITERS])
{
  *count = 0;
  for (int i = 0; i < N_WRITERS; i++)
  {
    dds_publication_matched_status_t st;
    dds_return_t rc = dds_get_publication_matched_status (writers[i], &st);
    if (rc != 0)
      return rc;
    *count += st.current_count;
  }
  return 0;
}

static int n_zz = 0;
static int zz[4] = {0};

static dds_return_t get_matched_count_readers (uint32_t *count, dds_entity_t readers[N_READERS])
{
  *count = 0;
  zz[0]++; n_zz++;
  for (int i = 0; i < N_READERS; i++)
  {
    zz[1]++; n_zz++;
    dds_subscription_matched_status_t st;
    dds_return_t rc = dds_get_subscription_matched_status (readers[i], &st);
    if (rc != 0) {
      zz[2]++; n_zz++;
      return rc;
    }
    *count += st.current_count;
  }
  zz[3]++; n_zz++;
  return 0;
}

static ddsrt_mutex_t locks[2];
#define PUBLISHER_LOCK &locks[0]
#define SUBSCRIBER_LOCK &locks[1]

const dds_domainid_t domainid = DDS_DOMAIN_DEFAULT;
const char *topic_name = "discstress_createwriter";

static uint32_t createwriter_publisher_thread(void *ptr)
{
  //dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t writers[N_WRITERS];
  dds_return_t rc;
  dds_qos_t *qos;
  (void)ptr;
  int id = (int) ddsrt_gettid ();
  int n_xx = 0;
  int xx[50] = {0};

  ddsrt_mutex_lock(PUBLISHER_LOCK);

  printf ("=== [Publisher(%d)] Start(%d) ...\n", id, g_pub_domain);
  xx[0]++; n_xx++;

  qos = dds_create_qos ();
  dds_qset_durability (qos, DDS_DURABILITY_VOLATILE);
  dds_qset_history (qos, DDS_HISTORY_KEEP_LAST, DEPTH);
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  xx[1]++; n_xx++;

  topic = dds_create_topic (g_pub_participant, &DiscStress_CreateWriter_Msg_desc, topic_name, qos, NULL);
  TST_ASSERT_FATAL_GT (topic, 0, "Could not create topic: %s\n", dds_strretcode(topic));
  xx[2]++; n_xx++;

  for (int i = 0; i < N_WRITERS; i++)
  {
    xx[3]++; n_xx++;
    writers[i] = dds_create_writer (g_pub_participant, topic, qos, NULL);
    TST_ASSERT_FATAL_GT (writers[i], 0, "Could not create writer: %s\n", dds_strretcode (writers[i]));
  }

  /* At some point in time, all remote readers will be known, and will consequently be matched
     immediately on creation of the writer.  That means it expects ACKs from all those readers, and
     that in turn means they should go into their "linger" state until all samples they wrote have
     been acknowledged (or the timeout occurs - but it shouldn't in a quiescent setup with a pretty
     reliable transport like a local loopback).  So other than waiting for some readers to show up
     at all, there's no need to look at matching events or to use anything other than volatile,
     provided the readers accept an initial short sequence in the first batch.  */
  printf ("=== [Publisher(%d)] Publishing while waiting for some reader ...\n", id);
  fflush (stdout);
  uint32_t seq = 0;
  int32_t round = -1;
  uint32_t wrseq = 0;
  bool matched = false;
  while (round < N_ROUNDS)
  {
    xx[4]++; n_xx++;
    if (matched) {
      xx[5]++; n_xx++;
      round++;
    } else {
      xx[6]++; n_xx++;
      uint32_t mc;
      rc = get_matched_count_writers (&mc, writers);
      TST_ASSERT_FATAL_EQ (rc, 0, "Could not get publication matched status: %s\n", dds_strretcode (rc));
      matched = (mc == N_READERS * N_WRITERS);
      if (matched)
      {
        xx[7]++; n_xx++;
        printf ("=== [Publisher(%d)] All readers found; continuing at [%"PRIu32",%"PRIu32"] for %d rounds\n",
                id, wrseq * N_WRITERS + 1, (wrseq + 1) * N_WRITERS, N_ROUNDS);
        fflush (stdout);
      }
    }

    for (uint32_t i = 0; i < N_WRITERS; i++)
    {
      xx[8]++; n_xx++;
      for (uint32_t j = 0; j < DEPTH; j++)
      {
        xx[9]++; n_xx++;
        /* Note: +1 makes wrseq equal to the writer entity id */
        DiscStress_CreateWriter_Msg m = {
          .round = round, .wrseq = wrseq * N_WRITERS + i + 1, .wridx = i, .histidx = j, .seq = seq
        };
        rc = dds_write (writers[i], &m);
        TST_ASSERT_FATAL_EQ (rc, 0, "Could not write data: %s\n", dds_strretcode (rc));
        seq++;
      }
    }

    /* Delete, then create writer: this should result in the other process processing alternating
       deletes and creates, and consequently, all readers should always have at least some matching
       writers.

       Round 0 is the first round where all readers have been matched with writer 0 */
    for (int i = 0; i < N_WRITERS; i++)
    {
      xx[10]++; n_xx++;
      rc = dds_delete (writers[i]);
      TST_ASSERT_FATAL_EQ (rc, 0, "Could not delete writer: %s\n", dds_strretcode (rc));
      writers[i] = dds_create_writer (g_pub_participant, topic, qos, NULL);
      TST_ASSERT_FATAL_GT (writers[i], 0, "Could not create writer: %s\n", dds_strretcode (writers[i]));
    }

    xx[11]++; n_xx++;
    wrseq++;
  }

  rc = dds_delete (g_pub_participant);
  TST_ASSERT_EQ (rc, DDS_RETCODE_OK, "Teardown failed\n");
  dds_delete_qos (qos);
  printf ("=== [Publisher(%d)] Done\n", id);
  printf("cnt_xx[]: %5d {", n_xx);
  for (int i = 0; i < 50; i++) {
    if (xx[i] > 0) printf(" %d:%d", i, xx[i]);
  }
  printf(" }\n");
  ddsrt_mutex_unlock(PUBLISHER_LOCK);

  return 0;
}

struct wrinfo {
  uint32_t rdid;
  uint32_t wrid;
  dds_instance_handle_t wr_iid;
  bool last_not_alive;
  uint32_t seen;
};

static uint32_t wrinfo_hash (const void *va)
{
  const struct wrinfo *a = va;
  return (uint32_t) (((a->rdid + UINT64_C (16292676669999574021)) *
                      (a->wrid + UINT64_C (10242350189706880077))) >> 32);
}

static int wrinfo_eq (const void *va, const void *vb)
{
  const struct wrinfo *a = va;
  const struct wrinfo *b = vb;
  return a->rdid == b->rdid && a->wrid == b->wrid;
}

#define LOGDEPTH 30
#define LOGLINE 200

static void dumplog (char logbuf[LOGDEPTH][LOGLINE], int *logidx)
{
  if (logbuf[*logidx][0])
    for (int i = 0; i < LOGDEPTH; i++)
      fputs (logbuf[i], stdout);
  for (int i = 0; i < *logidx; i++)
    fputs (logbuf[i], stdout);
  for (int i = 0; i < LOGDEPTH; i++)
    logbuf[i][0] = 0;
  *logidx = 0;
}

/*
 * The DiscStress_CreateWriter subscriber.
 * It waits for sample(s) and checks the content.
 */
static uint32_t createwriter_subscriber_thread(void *ptr)
{
  //dds_entity_t participant;
  dds_entity_t topic;
  dds_entity_t readers[N_READERS];
  dds_return_t rc;
  dds_qos_t *qos;
  (void)ptr;
  int id = (int) ddsrt_gettid ();
  int n_yy = 0;
  int yy[50] = {0};

  ddsrt_mutex_lock(SUBSCRIBER_LOCK);

  printf ("--- [Subscriber(%d)] Start(%d) ...\n", id, g_sub_domain);
  //yy[0]++; n_yy++;

  qos = dds_create_qos ();
  dds_qset_durability (qos, DDS_DURABILITY_VOLATILE);
  dds_qset_history (qos, DDS_HISTORY_KEEP_LAST, DEPTH);
  dds_qset_reliability (qos, DDS_RELIABILITY_RELIABLE, DDS_SECS (10));
  //yy[1]++; n_yy++;

  topic = dds_create_topic (g_sub_participant, &DiscStress_CreateWriter_Msg_desc, topic_name, qos, NULL);
  TST_ASSERT_FATAL_GT (topic, 0, "Could not create topic: %s\n", dds_strretcode(topic));
  //yy[2]++; n_yy++;

  /* Keep all history on the reader: then we should see DEPTH samples from each writer, except,
     perhaps, the very first time */
  dds_qset_history (qos, DDS_HISTORY_KEEP_ALL, 0);
  for (int i = 0; i < N_READERS; i++)
  {
    //yy[3]++; n_yy++;
    readers[i] = dds_create_reader (g_sub_participant, topic, qos, NULL);
    TST_ASSERT_FATAL_GT (readers[i], 0, "Could not create reader: %s\n", dds_strretcode (readers[i]));
  }

  printf ("--- [Subscriber(%d)] Waiting for some writer to match ...\n", id);
  fflush (stdout);

  /* Wait until we have matching writers */
  dds_entity_t ws = dds_create_waitset (g_sub_participant);
  TST_ASSERT_FATAL_GT (ws, 0, "Could not create waitset: %s\n", dds_strretcode (ws));
  for (int i = 0; i < N_READERS; i++)
  {
    //yy[4]++; n_yy++;
    rc = dds_set_status_mask (readers[i], DDS_SUBSCRIPTION_MATCHED_STATUS);
    TST_ASSERT_FATAL_EQ (rc, 0, "Could not set subscription matched mask: %s\n", dds_strretcode (rc));
    rc = dds_waitset_attach (ws, readers[i], i);
    TST_ASSERT_FATAL_EQ (rc, 0, "Could not attach reader to waitset: %s\n", dds_strretcode (rc));
  }

  {
    uint32_t mc;
    do {
      yy[5]++; n_yy++;
      rc = get_matched_count_readers (&mc, readers);
      TST_ASSERT_FATAL_EQ (rc, 0, "Could not get subscription matched status: %s\n", dds_strretcode (rc));
    } while (mc < N_READERS * N_WRITERS && (rc = dds_waitset_wait (ws, NULL, 0, DDS_INFINITY)) >= 0);
    TST_ASSERT_FATAL_GEQ (rc, 0, "Wait for writers failed: %s\n", dds_strretcode (rc));
  }

  /* Add DATA_AVAILABLE event; of course it would be easier to simply set it to desired value, but
     this is more fun */
  for (int i = 0; i < N_READERS; i++)
  {
    //yy[6]++; n_yy++;
    uint32_t mask;
    rc = dds_get_status_mask (readers[i], &mask);
    TST_ASSERT_FATAL_EQ (rc, 0, "Could not get status mask: %s\n", dds_strretcode (rc));
    TST_ASSERT_FATAL ((mask & DDS_SUBSCRIPTION_MATCHED_STATUS) != 0, "Retrieved status mask doesn't have MATCHED set\n");
    mask |= DDS_DATA_AVAILABLE_STATUS;
    rc = dds_set_status_mask (readers[i], mask);
    TST_ASSERT_FATAL_EQ (rc, 0, "Could not add data available status: %s\n", dds_strretcode (rc));
  }

  /* Loop while we have some matching writers */
  printf ("--- [Subscriber(%d)] Checking data ...\n", id);
  fflush (stdout);
  struct ddsrt_hh *wrinfo = ddsrt_hh_new (1, wrinfo_hash, wrinfo_eq);
  dds_entity_t xreader = 0;
  bool matched = true;
  char logbuf[N_READERS][LOGDEPTH][LOGLINE] = {{ "" }};
  int logidx[N_READERS] = { 0 };
  while (matched)
  {
    yy[7]++; n_yy++;
    dds_attach_t xs[N_READERS];

    {
      uint32_t mc;
      rc = get_matched_count_readers (&mc, readers);
      TST_ASSERT_FATAL_EQ (rc, 0, "Could not get subscription matched status: %s\n", dds_strretcode (rc));
      matched = (mc > 0);
    }

    /* Losing all matched writers will result in a transition to NO_WRITERS, and so in a DATA_AVAILABLE
       state, but the unregistering happens before the match count is updated.  I'm not sure it makes to
       make those atomic; I also don't think it is very elegant to do a final NO_WRITERS message with
       that was already signalled as no longer matching in a listener.


       Given that situation, waiting for data available, taking everything and immediately checking
       the subscription matched status doesn't guarantee in observing the absence of matched writers.
       Hence enabling the SUBSCRIPTION_MATCHED status on the readers, so that the actual removal will
       also trigger the waitset.

       The current_count == 0 case is so we do one final take after deciding to stop, just in case the
       unregisters & state change happened in between taking and checking the number of matched writers. */
    int32_t nxs = dds_waitset_wait (ws, xs, N_READERS, matched ? DDS_SECS (12) : 0);
    TST_ASSERT_FATAL_GEQ (nxs, 0, "Waiting for data failed: %s\n", dds_strretcode (nxs));
    if (nxs == 0 && matched)
    {
      yy[8]++; n_yy++;
      printf ("--- [Subscriber(%d)] Unexpected timeout\n", id);
      for (int i = 0; i < N_READERS; i++)
      {
        yy[9]++; n_yy++;
        dds_subscription_matched_status_t st;
        rc = dds_get_subscription_matched_status (readers[i], &st);
        TST_ASSERT_FATAL_EQ (rc, 0, "Could not get subscription matched status: %s\n", dds_strretcode (rc));
        printf ("--- [Subscriber(%d)] reader %d current_count %"PRIu32"\n", id, i, st.current_count);
      }
      fflush (stdout);
      TST_ASSERT_FATAL (0, "Timed out\n");
    }

#define READ_LEN 3
    for (int32_t i = 0; i < nxs; i++)
    {
      yy[10]++; n_yy++;
      void *raw[READ_LEN] = { NULL };
      dds_sample_info_t si[READ_LEN];
      int32_t n;
      while ((n = dds_take (readers[xs[i]], raw, si, READ_LEN, READ_LEN)) > 0)
      {
        yy[11]++; n_yy++;
        for (int32_t j = 0; j < n; j++)
        {
          DiscStress_CreateWriter_Msg const * const s = raw[j];
          if (si[j].valid_data) {
            if (s->round >= 0) {
              /* Cyclone always sets the key value, other fields are 0 for invalid data */
              struct wrinfo wri_key = { .wrid = s->wrseq, .rdid = (uint32_t) xs[i] };
              struct wrinfo *wri;

              TST_ASSERT_FATAL_LT (s->wridx, N_WRITERS, "Writer id out of range (%"PRIu32" %"PRIu32"\n)", s->wrseq, s->wridx);

              if ((wri = ddsrt_hh_lookup (wrinfo, &wri_key)) == NULL)
              {
                //yy[12]++; n_yy++;
                wri = malloc (sizeof (*wri));
                *wri = wri_key;
                rc = ddsrt_hh_add (wrinfo, wri);
                TST_ASSERT_FATAL_NEQ (rc, 0, "Both wrinfo lookup and add failed\n");
              }

              snprintf (logbuf[xs[i]][logidx[xs[i]]], sizeof (logbuf[xs[i]][logidx[xs[i]]]),
                        "%"PRId32": %"PRId32".%"PRIu32" %"PRIu32".%"PRIu32" iid %"PRIx64" new %"PRIx64" st %c%c seq %"PRIu32" seen %"PRIu32"\n",
                        (uint32_t) xs[i], s->round, s->wrseq, s->wridx, s->histidx, wri->wr_iid, si[j].publication_handle,
                        (si[j].instance_state == DDS_IST_ALIVE) ? 'A' : (si[j].instance_state == DDS_IST_NOT_ALIVE_DISPOSED) ? 'D' : 'U',
                        si[j].valid_data ? 'v' : 'i', s->seq, wri->seen);
              if (++logidx[xs[i]] == LOGDEPTH)
                logidx[xs[i]] = 0;

              XASSERT (wri->wr_iid == 0 || wri->wr_iid == si[j].publication_handle, "Mismatch between wrid and publication handle");

              XASSERT_FATAL (s->histidx < DEPTH, "depth_idx out of range");
              XASSERT ((wri->seen & (1u << s->histidx)) == 0, "Duplicate sample\n");
              if (s->histidx > 0)
                XASSERT ((wri->seen & (1u << (s->histidx - 1))) != 0, "Out of order sample (1)\n");
              XASSERT (wri->seen < (1u << s->histidx), "Out of order sample (2)\n");
              wri->wr_iid = si[j].publication_handle;
              wri->seen |= 1u << s->histidx;
              yy[13]++; n_yy++;
            } else {
              yy[14]++; n_yy++;
            }
            yy[15]++; n_yy++; // [13] + [14] = [15]
          } else {
            yy[16]++; n_yy++;
          }
          yy[17]++; n_yy++; // [15] + [16] = [17]
        }
        rc = dds_return_loan (readers[xs[i]], raw, n);
        TST_ASSERT_FATAL_EQ (rc, 0, "Could not return loan: %s\n", dds_strretcode (rc));

        /* Flip-flop between create & deleting a reader to ensure matching activity on the proxy
           writers, as that, too should occasionally push the delivery out of the fast path */
        if (xreader)
        {
          yy[18]++; n_yy++;
          rc = dds_delete (xreader);
          TST_ASSERT_FATAL_EQ (rc, 0, "Error on deleting extra reader: %s\n", dds_strretcode (rc));
          xreader = 0;
        }
        else
        {
          yy[19]++; n_yy++;
          xreader = dds_create_reader (g_sub_participant, topic, qos, NULL);
          TST_ASSERT_FATAL_GT (xreader, 0, "Could not create extra reader: %s\n", dds_strretcode (xreader));
        }
        yy[20]++; n_yy++; // [18] + [19] = [20]
      }
      TST_ASSERT_FATAL_EQ (rc, 0, "Error on reading: %s\n", dds_strretcode (rc));
    }
  }

  rc = dds_delete (g_sub_participant);
  TST_ASSERT_EQ (rc, DDS_RETCODE_OK, "Teardown failed\n");
  dds_delete_qos (qos);

  struct ddsrt_hh_iter it;
  uint32_t nwri = 0;
  for (struct wrinfo *wri = ddsrt_hh_iter_first (wrinfo, &it); wri; wri = ddsrt_hh_iter_next (&it))
  {
    //yy[21]++; n_yy++;
    nwri++;
    if (wri->seen != (1u << DEPTH) - 1)
    {
      TST_ASSERT (0, "Some data missing at end (rd %d wr %d seen %"PRIx32")\n", wri->rdid, wri->wrid, wri->seen);
    }
    /* simple iteration won't touch an object pointer twice */
    free (wri);
  }
  ddsrt_hh_free (wrinfo);
  TST_ASSERT (nwri >= (N_ROUNDS / 3) * N_READERS * N_WRITERS, "Less data received than expected\n");
  printf ("--- [Subscriber(%d)] Done after %"PRIu32" sets\n", id, nwri / (N_READERS * N_WRITERS));
  printf("cnt_yy[]: %5d {", n_yy);
  for (int i = 0; i < 50; i++) {
    if (yy[i] > 0) printf(" %d:%d", i, yy[i]);
  }
  printf(" }\n");
  //printf("cnt_zz[]: %5d {", n_zz);
  //for (int i = 0; i < 4; i++) {
  //  if (zz[i] > 0) printf(" %d:%d", i, zz[i]);
  //}
  //printf(" }\n");


  ddsrt_mutex_unlock(SUBSCRIBER_LOCK);
  return 0;
}

static void start_system(void)
{
  dds_return_t ret;
  ddsrt_thread_t thrs[2];
#define PUBLISHER_THREAD thrs[0]
#define SUBSCRIBER_THREAD thrs[1]
  ddsrt_threadattr_t attr = {0};
  uint32_t res = 0;

  ddsrt_mutex_init(PUBLISHER_LOCK);
  ddsrt_mutex_init(SUBSCRIBER_LOCK);
  ddsrt_mutex_lock(PUBLISHER_LOCK);
  ddsrt_mutex_lock(SUBSCRIBER_LOCK);
  ret = ddsrt_thread_create(&PUBLISHER_THREAD, "", &attr, &createwriter_publisher_thread, NULL);
  TST_ASSERT_FATAL_EQ (ret, DDS_RETCODE_OK, "Could not create publisher thread");
  ret = ddsrt_thread_create(&SUBSCRIBER_THREAD, "", &attr, &createwriter_subscriber_thread, NULL);
  TST_ASSERT_FATAL_EQ (ret, DDS_RETCODE_OK, "Could not create subscriber thread");

  ddsrt_mutex_unlock(SUBSCRIBER_LOCK);
  dds_sleepfor(DDS_MSECS(100));
  ddsrt_mutex_unlock(PUBLISHER_LOCK);

  ddsrt_thread_join(PUBLISHER_THREAD, &res);
  ddsrt_thread_join(SUBSCRIBER_THREAD, &res);

  ddsrt_mutex_destroy(SUBSCRIBER_LOCK);
  ddsrt_mutex_destroy(PUBLISHER_LOCK);
}

CU_Test(discstress, createwriter_md, .init = createwriter_init_md, .fini = createwriter_fini)
{
  start_system();
}

CU_Test(discstress, createwriter_sd, .init = createwriter_init_sd, .fini = createwriter_fini)
{
  start_system();
}
