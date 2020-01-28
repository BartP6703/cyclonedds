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

#ifndef MPT_BART_RW_H
#define MPT_BART_RW_H

#include <stdio.h>
#include <string.h>

#include "dds/dds.h"
#include "mpt/mpt.h"

#include "../mock/mock_authentication.h"

#if defined (__cplusplus)
extern "C" {
#endif


#define MAX_LOCAL_IDENTITIES   8
#define MAX_REMOTE_IDENTITIES  8
#define MAX_HANDSHAKES        32

union guid {
    DDS_Security_GUID_t g;
    unsigned u[4];
};


struct Identity {
    DDS_Security_IdentityHandle handle;
    union guid guid;
};

struct Handshake {
    DDS_Security_HandshakeHandle handle;
    int isRequest;
    int lidx;
    int ridx;
    DDS_Security_ValidationResult_t handshakeResult;
    DDS_Security_ValidationResult_t finalResult;
};

struct SyncControl {
    char *recvName;
    FILE *sendCtrl;
    long pos;
};


void
syncControlInit (
    struct SyncControl *sync,
    const char *recvName,
    const char *sendName);

void
syncControlDeinit(
    struct SyncControl *sync);

int
syncControlRead(
   struct SyncControl *sync,
   char *token,
   int size,
   int timeout);

int
syncControlWaitFor(
    struct SyncControl *sync,
    int timeout);

void
syncControlSend(
    struct SyncControl *sync,
    int idx);

void
addLocalIdentity(
    DDS_Security_IdentityHandle handle,
    DDS_Security_GUID_t *guid);

int
findLocalIdentity(
    DDS_Security_IdentityHandle handle);

int
findRemoteIdentity(
    DDS_Security_IdentityHandle handle);

void
addRemoteIdentity(
    DDS_Security_IdentityHandle handle,
    DDS_Security_GUID_t *guid);

void
addHandshake(
    DDS_Security_HandshakeHandle handle,
    int isRequest,
    DDS_Security_IdentityHandle lHandle,
    DDS_Security_IdentityHandle rHandle,
    DDS_Security_ValidationResult_t result);

int
findHandshake(
    DDS_Security_HandshakeHandle handle);

int
handleProcessMessage(
    DDS_Security_IdentityHandle handshake);

int
handleBeginHandshakeRequest(
    DDS_Security_IdentityHandle lid,
    DDS_Security_IdentityHandle rid);

int
handleBeginHandshakeReply(
    DDS_Security_IdentityHandle lid,
    DDS_Security_IdentityHandle rid);

int
handleValidateRemoteIdentity(
    DDS_Security_IdentityHandle lid,
    uint32_t count);

int
validate_handshake(
    void);

void handshake_init (void);
void handshake_fini (void);

MPT_ProcessEntry (handshake_process1,
                  MPT_Args (dds_domainid_t domainid));

MPT_ProcessEntry (handshake_process2,
                  MPT_Args (dds_domainid_t domainid));

#if defined (__cplusplus)
}
#endif

#endif
