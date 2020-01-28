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
#include "dds/ddsrt/string.h"
#include "dds/security/dds_security_api.h"
#include "handshake.h"
#include "../mock/handshake_export.h"

void handshake_init (void)
{
}

void handshake_fini (void)
{
}

struct Identity localIdentityList[MAX_LOCAL_IDENTITIES];
uint32_t numLocal = 0;
struct Identity remoteIdentityList[MAX_REMOTE_IDENTITIES];
int numRemote = 0;
struct Handshake handshakeList[MAX_HANDSHAKES];
int numHandshake = 0;
FILE *logfile = NULL;

const char *controls[] = {"failed", "success", "started", "stop", "continue"};
const int numControls = sizeof(controls)/sizeof(const char*);

const int CT_FAILED   = 0;
const int CT_SUCCESS  = 1;
const int CT_STARTED  = 2;
const int CT_STOP     = 3;

const int CT_CONTINUE = 4;



void
syncControlInit (
    struct SyncControl *sync,
    const char *recvName,
    const char *sendName)
{
    printf("%s: \n", __func__);
    sync->recvName = ddsrt_strdup(recvName);
    sync->sendCtrl = fopen(sendName, "w");
    sync->pos = 0;
}

void
syncControlDeinit(
    struct SyncControl *sync)
{
    printf("%s: \n", __func__);
    dds_free(sync->recvName);
    fclose(sync->sendCtrl);
}

int
syncControlRead(
   struct SyncControl *sync,
   char *token,
   int size,
   int timeout)
{
    int found = 0;
    FILE *f;
    dds_duration_t delay = DDS_SECS(1);

    printf("%s: \n", __func__);
    while (!found && timeout > 0) {
        printf("%s: timeout:%d\n", __func__, timeout);
        f = fopen(sync->recvName, "r");
        if (f) {
            if (fseek(f, sync->pos, SEEK_SET) != -1) {
                if (fgets(token, size, f)) {
                    printf("%s: token:%s", __func__, token);
                    found = 1;
                }
                sync->pos = ftell(f);
            }
            fclose(f);
        } else {
            printf("%s: %s could not be opened\n", __func__, sync->recvName);
	}
        if (!found) {
            dds_sleepfor(delay);
        }
        timeout--;
    }

    if (found) {
        size_t l = strlen(token);
        while (l > 0 && ((token[l-1] == '\r') || (token[l-1] == '\n'))) {
            token[--l] = '\0';
        }
    }

    printf("%s: found:%d\n", __func__, found);
    return found;
}

int
syncControlWaitFor(
    struct SyncControl *sync,
    int timeout)
{
    int idx = -1;
    char token[256];
    int i;

    printf("%s: \n", __func__);
    if (syncControlRead(sync, token, sizeof(token), timeout)) {
        for (i = 0; (i < numControls) && (idx < 0); i++) {
            if (strcmp(controls[i], token) == 0) {
                printf("Recv: %s\n", token);
                idx = (int)i;
            }
        }
    }
    printf("%s: idx:%d\n", __func__, idx);
    return idx;
}

void
syncControlSend(
    struct SyncControl *sync,
    int idx)
{
    printf("%s: \n", __func__);
    assert(idx < numControls);
    printf("%s: token:%s\n", __func__, controls[idx]);
    fprintf(sync->sendCtrl, "%s\n", controls[idx]);
    fflush(sync->sendCtrl);
    printf("Send: %s\n", controls[idx]);
}

void
addLocalIdentity(
    DDS_Security_IdentityHandle handle,
    DDS_Security_GUID_t *guid)
{
    printf("%s: \n", __func__);
    localIdentityList[numLocal].handle = handle;
    memcpy(&localIdentityList[numLocal].guid, guid, sizeof(DDS_Security_GUID_t));
    numLocal++;
}

int
findLocalIdentity(
    DDS_Security_IdentityHandle handle)
{
    int i;

    printf("%s: \n", __func__);
    for (i = 0; i < (int)numLocal; i++) {
        if (localIdentityList[i].handle == handle) {
            printf("%s: %d found @%d\n", __func__, (int)handle, i);
            return i;
        }
    }

    printf("%s: %d not found\n", __func__, (int)handle);
    return -1;
}

int
findRemoteIdentity(
    DDS_Security_IdentityHandle handle)
{
    int i;

    printf("%s: \n", __func__);
    for (i = 0; i < numRemote; i++) {
        if (remoteIdentityList[i].handle == handle) {
            printf("%s: %d found @%d\n", __func__, (int)handle, i);
            return i;
        }
    }

    printf("%s: %d not found\n", __func__, (int)handle);
    return -1;
}

void
addRemoteIdentity(
    DDS_Security_IdentityHandle handle,
    DDS_Security_GUID_t *guid)
{
    int idx;

    printf("%s: \n", __func__);
    idx = findRemoteIdentity(handle);
    if (idx < 0) {
        remoteIdentityList[numRemote].handle = handle;
        memcpy(&remoteIdentityList[numRemote].guid, guid, sizeof(DDS_Security_GUID_t));
        numRemote++;
    }
}

void
addHandshake(
    DDS_Security_HandshakeHandle handle,
    int isRequest,
    DDS_Security_IdentityHandle lHandle,
    DDS_Security_IdentityHandle rHandle,
    DDS_Security_ValidationResult_t result)
{
    printf("%s: \n", __func__);
    handshakeList[numHandshake].handle = handle;
    handshakeList[numHandshake].isRequest = isRequest;
    handshakeList[numHandshake].handshakeResult = result;
    handshakeList[numHandshake].lidx = findLocalIdentity(lHandle);
    handshakeList[numHandshake].ridx = findRemoteIdentity(rHandle);
    handshakeList[numHandshake].finalResult = DDS_SECURITY_VALIDATION_FAILED;
    numHandshake++;
}

int
findHandshake(
    DDS_Security_HandshakeHandle handle)
{
    int i;

    printf("%s: \n", __func__);
    for (i = 0; i < numHandshake; i++) {
        if (handshakeList[i].handle == handle) {
            printf("%s: %d found @%d\n", __func__, (int)handle, i);
            return i;
        }
    }

    printf("%s: %d not found\n", __func__, (int)handle);
    return -1;
}

int
handleProcessMessage(
    DDS_Security_IdentityHandle handshake)
{
    struct Message *msg;
    int idx;
    dds_duration_t timeout = DDS_SECS(10);

    printf("%s: \n", __func__);
    msg = test_authentication_plugin_read(MESSAGE_KIND_PROCESS_HANDSHAKE, 0, 0, handshake, timeout);
    if (msg) {
        idx = findHandshake(msg->hsHandle);
        if (idx >= 0) {
            handshakeList[idx].finalResult = msg->result;
            printf("%s: return 1\n", __func__);
            return 1;
        }
    }

    printf("%s: return 0\n", __func__);
    return 0;
}

int
handleBeginHandshakeRequest(
    DDS_Security_IdentityHandle lid,
    DDS_Security_IdentityHandle rid)
{
    struct Message *msg;
    dds_duration_t timeout = DDS_SECS(10);

    printf("%s: \n", __func__);
    msg = test_authentication_plugin_read(MESSAGE_KIND_BEGIN_HANDSHAKE_REQUEST, lid, rid, 0, timeout);
    if (msg) {
        addHandshake(msg->hsHandle, 1, msg->lidHandle, msg->ridHandle, msg->result);
        return handleProcessMessage(msg->hsHandle);
    }

    printf("%s: return 0\n", __func__);
    return 0;
}

int
handleBeginHandshakeReply(
    DDS_Security_IdentityHandle lid,
    DDS_Security_IdentityHandle rid)
{
    struct Message *msg;
    dds_duration_t timeout = DDS_SECS(10);

    printf("%s: \n", __func__);
    msg = test_authentication_plugin_read(MESSAGE_KIND_BEGIN_HANDSHAKE_REPLY, lid, rid, 0, timeout);
    if (msg) {
        addHandshake(msg->hsHandle, 0, msg->lidHandle, msg->ridHandle, msg->result);
        return handleProcessMessage(msg->hsHandle);
    }

    printf("%s: return 0\n", __func__);
    return 0;
}

int
handleValidateRemoteIdentity(
    DDS_Security_IdentityHandle lid,
    uint32_t count)
{
    int r = 1;
    struct Message *msg;
    DDS_Security_IdentityHandle rid;
    dds_duration_t timeout = DDS_SECS(10);

    printf("%s: \n", __func__);
    msg = test_authentication_plugin_read(MESSAGE_KIND_VALIDATE_REMOTE_IDENTITY, lid, 0, 0, timeout);
    while (msg && (count > 0) && r) {
        rid = msg->ridHandle;
        addRemoteIdentity(msg->ridHandle, &msg->rguid);
        if (msg->result == DDS_SECURITY_VALIDATION_PENDING_HANDSHAKE_REQUEST) {
            r = handleBeginHandshakeRequest(lid, rid);
        } else if (msg->result == DDS_SECURITY_VALIDATION_PENDING_HANDSHAKE_MESSAGE) {
            r = handleBeginHandshakeReply(lid, rid);
        } else {
            r = 0;
        }
        count--;
        if (count) {
            msg = test_authentication_plugin_read(MESSAGE_KIND_VALIDATE_REMOTE_IDENTITY, lid, 0, 0, timeout);
        }
    }
    return r && !count;
}

int
validate_handshake(
    void)
{
    struct Message *msg;
    dds_duration_t timeout = DDS_SECS(10);
    int r = 1;
    int i;

    printf("%s: \n", __func__);
    msg = test_authentication_plugin_read(MESSAGE_KIND_VALIDATE_LOCAL_IDENTITY, 0, 0, 0, timeout);
    while (msg) {
        addLocalIdentity(msg->lidHandle, &msg->lguid);
        msg = test_authentication_plugin_read(MESSAGE_KIND_VALIDATE_LOCAL_IDENTITY, 0, 0, 0, timeout);
    }

    for (i = 0; r && (i < (int)numLocal); i++) {
        r = handleValidateRemoteIdentity(localIdentityList[i].handle, numLocal);
    }

    return r;
}

MPT_ProcessEntry (handshake_process1,
                  MPT_Args (dds_domainid_t domainid))
{
  dds_entity_t participant;
  const char *recvCtrl = "pctrl_1b.log";
  const char *sendCtrl = "pctrl_1a.log";
  //char logname[64];
  struct SyncControl sync;
  int r = 1;
  int remote;

  (void)mpt__args__;
  (void)mpt__retval__;

  syncControlInit(&sync, recvCtrl, sendCtrl);
  test_authentication_plugin_init();

  /* Create a Participant. */
  participant = dds_create_participant (domainid, NULL, NULL);
  if (participant < 0)
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));

  if (syncControlWaitFor(&sync, 10) == CT_STARTED) {
    syncControlSend(&sync, CT_CONTINUE);
    r = validate_handshake();
    syncControlSend(&sync, CT_STOP);
    remote = syncControlWaitFor(&sync, 10);
    if (r && (remote == CT_SUCCESS)) {
      printf("%s: r:%d, %s received\n", __func__, r, controls[remote]);
    } else if (!r && (remote != CT_SUCCESS)) {
      printf("%s: r:%d, %s received\n", __func__, r, controls[remote]);
      printf("Test failed at A and B");
    } else if (!r) {
      printf("%s: r:%d", __func__, r);
      if (remote > 0) {
        printf(", %s received", controls[remote]);
      }
      printf("\n");
      printf("Test failed at A");
    } else {
      printf("%s: r:%d", __func__, r);
      if (remote > 0) {
        printf(", %s received", controls[remote]);
      }
      printf("\n");
      printf("Test failed at B");
    }
  } else {
    printf("Failed to receive result for process B");
  }

  dds_delete( participant );
}

MPT_ProcessEntry (handshake_process2,
                  MPT_Args (dds_domainid_t domainid))
{
  dds_entity_t participant;
  const char *recvCtrl = "pctrl_1b.log";
  const char *sendCtrl = "pctrl_1a.log";
  struct SyncControl sync;
  dds_return_t rc;
  int r = 1;

  (void)mpt__args__;
  (void)mpt__retval__;

  syncControlInit(&sync, recvCtrl, sendCtrl);
  test_authentication_plugin_init();

  /* Create a Participant. */
  participant = dds_create_participant (domainid, NULL, NULL);
  if (participant < 0)
    DDS_FATAL("dds_create_participant: %s\n", dds_strretcode(-participant));

  syncControlSend(&sync, CT_STARTED);
  if (syncControlWaitFor(&sync, 10) != CT_CONTINUE) {
    goto failed_to_start;
  }

  r = validate_handshake();

  (void)syncControlWaitFor(&sync, 10);

  /* Deleting the participant will delete all its children recursively as well. */
  rc = dds_delete (participant);
  if (rc != DDS_RETCODE_OK)
    DDS_FATAL("dds_delete: %s\n", dds_strretcode(-rc));

  test_authentication_plugin_deinit();

  if (r) {
    syncControlSend(&sync, CT_SUCCESS);
  } else {
    syncControlSend(&sync, CT_FAILED);
  }

  syncControlDeinit(&sync);

  printf("%s[SF]: finish\n", __func__);
  //return r;

failed_to_start:
  syncControlSend(&sync, CT_FAILED);
  syncControlDeinit(&sync);
  printf("%s[SF]: finish(error)\n", __func__);
  //return -1;
}
