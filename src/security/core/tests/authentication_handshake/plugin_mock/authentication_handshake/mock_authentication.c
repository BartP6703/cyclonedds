/*
 * Copyright(c) 2006 to 2020 ADLINK Technology Limited and others
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v. 2.0 which is available at
 * http://www.eclipse.org/legal/epl-2.0, or the Eclipse Distribution License
 * v. 1.0 which is available at
 * http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * SPDX-License-Identifier: EPL-2.0 OR BSD-3-Clause
 */

#include "dds/security/dds_security_api.h"
#include "dds/security/core/dds_security_utils.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/sync.h"
#include "mock_authentication.h"
#include <string.h>
#include <stdio.h>

/**
 * Implementation structure for storing encapsulated members of the instance
 * while giving only the interface definition to user
 */

typedef struct dds_security_authentication_impl {
  dds_security_authentication base;
  dds_security_authentication *instance; //int id; //sample internal member
} dds_security_authentication_impl;

static struct MessageQueue testMessagaQueue;

static struct Message *
createMessage(
    MessageKind_t kind,
    DDS_Security_IdentityHandle lidHandle,
    DDS_Security_IdentityHandle ridHandle,
    DDS_Security_IdentityHandle hsHandle,
    const DDS_Security_GUID_t *lguid,
    const DDS_Security_GUID_t *rguid,
    DDS_Security_ValidationResult_t result,
    const DDS_Security_DataHolder *token)
{
    struct Message *msg;

    msg = ddsrt_malloc(sizeof(*msg));
    memset(msg, 0, sizeof(*msg));
    msg->kind = kind;
    msg->lidHandle = lidHandle;
    msg->ridHandle = ridHandle;
    msg->hsHandle = hsHandle;
    msg->result = result;
    if (lguid) {
        memcpy(&msg->lguid, lguid, sizeof(msg->lguid));
    }
    if (rguid) {
        memcpy(&msg->rguid, rguid, sizeof(msg->rguid));
    }
    if (token) {
        DDS_Security_DataHolder_copy(&msg->token, token);
    }

    return msg;
}

static void
deleteMessage(
    struct Message *msg)
{
    if (msg) {
        DDS_Security_DataHolder_deinit(&msg->token);
        ddsrt_free(msg);
    }
}


static void
initMessageQueue(
    struct MessageQueue *queue)
{
    queue->head = NULL;
    queue->tail = NULL;
    ddsrt_mutex_init(&queue->lock);
    ddsrt_cond_init(&queue->cond);
}

static void
deinitMessageQueue(
    struct MessageQueue *queue)
{
    struct Message *msg;

    msg = queue->head;
    while (msg) {
        queue->head = msg->next;
        deleteMessage(msg);
        msg = queue->head;
    }
    ddsrt_cond_destroy(&queue->cond);
    ddsrt_mutex_destroy(&queue->lock);
}

static void
insertMessage(
    struct MessageQueue *queue,
    struct Message *msg)
{
    ddsrt_mutex_lock(&queue->lock);

    if (!queue->head) {
        queue->head = msg;
    } else {
        queue->tail->next = msg;
    }
    queue->tail = msg;

    ddsrt_cond_signal(&queue->cond);
    ddsrt_mutex_unlock(&queue->lock);
}

static int
messageMatched(
    struct Message *msg,
    MessageKind_t kind,
    DDS_Security_IdentityHandle lidHandle,
    DDS_Security_IdentityHandle ridHandle,
    DDS_Security_IdentityHandle hsHandle)
{
    int r = 1;

    if (msg->kind != kind) {
        r = 0;
    } else if (lidHandle && msg->lidHandle != lidHandle) {
        r = 0;
    } else if (ridHandle && msg->ridHandle != ridHandle) {
        r = 0;
    } else if (hsHandle && msg->hsHandle != hsHandle) {
        r = 0;
    }

    return r;
}

static struct Message *
readMessage(
    struct MessageQueue *queue,
    MessageKind_t kind,
    DDS_Security_IdentityHandle lidHandle,
    DDS_Security_IdentityHandle ridHandle,
    DDS_Security_IdentityHandle hsHandle,
    dds_duration_t timeout)
{
    struct Message *msg = NULL, *cur, *prev;
    int r = 1;

    ddsrt_mutex_lock(&queue->lock);

    do {
        cur = queue->head;
        prev = NULL;
        while (cur && !msg) {
            if (messageMatched(cur, kind, lidHandle, ridHandle, hsHandle)) {
                msg = cur;
                if (prev) {
                    prev->next = msg->next;
                } else {
                    queue->head = msg->next;
                }
                if (queue->tail == msg) {
                    queue->tail = prev;
                }
            } else {
                prev = cur;
                cur = cur->next;
            }
        }
        if (!msg) {
            if (!ddsrt_cond_waitfor(&queue->cond, &queue->lock, timeout)) {
                r = 0;
            }
        }
    } while (r && !msg);

    ddsrt_mutex_unlock(&queue->lock);

    return msg;
}



struct Message *
test_authentication_plugin_read(
    MessageKind_t kind,
    DDS_Security_IdentityHandle lidHandle,
    DDS_Security_IdentityHandle ridHandle,
    DDS_Security_IdentityHandle hsHandle,
    dds_duration_t timeout)
{
    printf("%s: started\n", __func__);
    return readMessage(&testMessagaQueue, kind, lidHandle, ridHandle, hsHandle, timeout);
}

void
test_authentication_plugin_delete_message(
    struct Message *msg)
{
    printf("%s:", __func__);
    if (msg) {
        printf(" kind:%d", (int)msg->kind);
    }
    printf("\n");
    deleteMessage(msg);
}

static DDS_Security_ValidationResult_t
test_validate_local_identity(
        dds_security_authentication *instance,
        DDS_Security_IdentityHandle *local_identity_handle,
        DDS_Security_GUID_t *adjusted_participant_guid,
        const DDS_Security_DomainId domain_id,
        const DDS_Security_Qos *participant_qos,
        const DDS_Security_GUID_t *candidate_participant_guid,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;
    DDS_Security_ValidationResult_t result;
    struct Message *msg;

    printf("%s: started\n", __func__);
    result = impl->instance->validate_local_identity(
                impl->instance, local_identity_handle, adjusted_participant_guid, domain_id, participant_qos, candidate_participant_guid, ex);

    msg = createMessage(MESSAGE_KIND_VALIDATE_LOCAL_IDENTITY, *local_identity_handle, 0, 0, adjusted_participant_guid, NULL, result, NULL);
    insertMessage(&testMessagaQueue, msg);

    return result;
}

static DDS_Security_boolean
test_get_identity_token(dds_security_authentication *instance,
        DDS_Security_IdentityToken *identity_token,
        const DDS_Security_IdentityHandle handle,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;
    DDS_Security_boolean ret;
    printf("%s: started\n", __func__);

    ret = impl->instance->get_identity_token(impl->instance, identity_token, handle, ex);

    printf("%s: returns %d\n", __func__, (int)ret);

    return ret;
}


static DDS_Security_boolean test_get_identity_status_token(
        dds_security_authentication *instance,
        DDS_Security_IdentityStatusToken *identity_status_token,
        const DDS_Security_IdentityHandle handle,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->get_identity_status_token(impl->instance, identity_status_token, handle, ex);
}

static DDS_Security_boolean test_set_permissions_credential_and_token(
        dds_security_authentication *instance,
        const DDS_Security_IdentityHandle handle,
        const DDS_Security_PermissionsCredentialToken *permissions_credential,
        const DDS_Security_PermissionsToken *permissions_token,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->set_permissions_credential_and_token(impl->instance, handle, permissions_credential, permissions_token, ex);
}

static DDS_Security_ValidationResult_t
test_validate_remote_identity(
        dds_security_authentication *instance,
        DDS_Security_IdentityHandle *remote_identity_handle,
        DDS_Security_AuthRequestMessageToken *local_auth_request_token,
        const DDS_Security_AuthRequestMessageToken *remote_auth_request_token,
        const DDS_Security_IdentityHandle local_identity_handle,
        const DDS_Security_IdentityToken *remote_identity_token,
        const DDS_Security_GUID_t *remote_participant_guid,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;
    DDS_Security_ValidationResult_t result;
    struct Message *msg;

    printf("%s: started\n", __func__);
    result = impl->instance->validate_remote_identity(
              impl->instance, remote_identity_handle, local_auth_request_token, remote_auth_request_token,
              local_identity_handle, remote_identity_token, remote_participant_guid, ex);

    msg = createMessage(MESSAGE_KIND_VALIDATE_REMOTE_IDENTITY, local_identity_handle, *remote_identity_handle, 0, NULL, remote_participant_guid, result, local_auth_request_token);
    insertMessage(&testMessagaQueue, msg);

    return result;
}

static DDS_Security_ValidationResult_t
test_begin_handshake_request(
        dds_security_authentication *instance,
        DDS_Security_HandshakeHandle *handshake_handle,
        DDS_Security_HandshakeMessageToken *handshake_message,
        const DDS_Security_IdentityHandle initiator_identity_handle,
        const DDS_Security_IdentityHandle replier_identity_handle,
        const DDS_Security_OctetSeq *serialized_local_participant_data,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;
    DDS_Security_ValidationResult_t result;
    struct Message *msg;

    printf("%s: started\n", __func__);
    result = impl->instance->begin_handshake_request(
              impl->instance, handshake_handle, handshake_message, initiator_identity_handle,
              replier_identity_handle, serialized_local_participant_data, ex);

    msg = createMessage(MESSAGE_KIND_BEGIN_HANDSHAKE_REQUEST, initiator_identity_handle, replier_identity_handle, *handshake_handle, NULL, NULL, result, handshake_message);
    insertMessage(&testMessagaQueue, msg);

    return result;
}


static DDS_Security_ValidationResult_t
test_begin_handshake_reply(
        dds_security_authentication *instance,
        DDS_Security_HandshakeHandle *handshake_handle,
        DDS_Security_HandshakeMessageToken *handshake_message_out,
        const DDS_Security_HandshakeMessageToken *handshake_message_in,
        const DDS_Security_IdentityHandle initiator_identity_handle,
        const DDS_Security_IdentityHandle replier_identity_handle,
        const DDS_Security_OctetSeq *serialized_local_participant_data,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;
    DDS_Security_ValidationResult_t result;
    struct Message *msg;

    printf("%s: started\n", __func__);
    result = impl->instance->begin_handshake_reply(
              impl->instance, handshake_handle, handshake_message_out, handshake_message_in,
              initiator_identity_handle, replier_identity_handle, serialized_local_participant_data, ex);

    msg = createMessage(MESSAGE_KIND_BEGIN_HANDSHAKE_REPLY, replier_identity_handle, initiator_identity_handle, *handshake_handle, NULL, NULL, result, handshake_message_out);
    insertMessage(&testMessagaQueue, msg);

    return result;
}

static DDS_Security_ValidationResult_t test_process_handshake(
        dds_security_authentication *instance,
        DDS_Security_HandshakeMessageToken *handshake_message_out,
        const DDS_Security_HandshakeMessageToken *handshake_message_in,
        const DDS_Security_HandshakeHandle handshake_handle,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;
    DDS_Security_ValidationResult_t result;
    struct Message *msg;

    printf("%s: started\n", __func__);
    result = impl->instance->process_handshake(impl->instance, handshake_message_out, handshake_message_in, handshake_handle, ex);

    msg = createMessage(MESSAGE_KIND_PROCESS_HANDSHAKE, 0, 0, handshake_handle, NULL, NULL, result, handshake_message_out);
    insertMessage(&testMessagaQueue, msg);

    return result;
}

static DDS_Security_SharedSecretHandle test_get_shared_secret(
        dds_security_authentication *instance,
        const DDS_Security_HandshakeHandle handshake_handle,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->get_shared_secret(impl->instance, handshake_handle, ex);
}

static DDS_Security_boolean test_get_authenticated_peer_credential_token(
        dds_security_authentication *instance,
        DDS_Security_AuthenticatedPeerCredentialToken *peer_credential_token,
        const DDS_Security_HandshakeHandle handshake_handle,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->get_authenticated_peer_credential_token(impl->instance, peer_credential_token, handshake_handle, ex);
}

static DDS_Security_boolean test_set_listener(dds_security_authentication *instance,
        const dds_security_authentication_listener *listener,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->set_listener(impl->instance, listener, ex);
}

static DDS_Security_boolean test_return_identity_token(dds_security_authentication *instance,
        const DDS_Security_IdentityToken *token,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->return_identity_token(impl->instance, token, ex);
}

static DDS_Security_boolean test_return_identity_status_token(
        dds_security_authentication *instance,
        const DDS_Security_IdentityStatusToken *token,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->return_identity_status_token(impl->instance, token, ex);
}

static DDS_Security_boolean test_return_authenticated_peer_credential_token(
        dds_security_authentication *instance,
        const DDS_Security_AuthenticatedPeerCredentialToken *peer_credential_token,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->return_authenticated_peer_credential_token(impl->instance, peer_credential_token, ex);
}

static DDS_Security_boolean
test_return_handshake_handle(dds_security_authentication *instance,
        const DDS_Security_HandshakeHandle handshake_handle,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->return_handshake_handle(impl->instance, handshake_handle, ex);
}

static DDS_Security_boolean
test_return_identity_handle(
        dds_security_authentication *instance,
        const DDS_Security_IdentityHandle identity_handle,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->return_identity_handle(impl->instance, identity_handle, ex);
}

static DDS_Security_boolean test_return_sharedsecret_handle(
        dds_security_authentication *instance,
        const DDS_Security_SharedSecretHandle sharedsecret_handle,
        DDS_Security_SecurityException *ex)
{
    struct dds_security_authentication_impl *impl = (struct dds_security_authentication_impl *)instance;

    printf("%s: started\n", __func__);
    return impl->instance->return_sharedsecret_handle(impl->instance, sharedsecret_handle, ex);
}

int32_t init_test_authentication( const char *argument, void **context)
{
    struct dds_security_authentication_impl *authentication;

    printf("%s: started\n", __func__);
    authentication = ddsrt_malloc(sizeof(*authentication));

    //allocate implementation wrapper
    authentication->base.validate_local_identity = &test_validate_local_identity;
    authentication->base.get_identity_token = &test_get_identity_token;
    authentication->base.get_identity_status_token = &test_get_identity_status_token;
    authentication->base.set_permissions_credential_and_token = &test_set_permissions_credential_and_token;
    authentication->base.validate_remote_identity = &test_validate_remote_identity;
    authentication->base.begin_handshake_request = &test_begin_handshake_request;
    authentication->base.begin_handshake_reply = &test_begin_handshake_reply;
    authentication->base.process_handshake = &test_process_handshake;
    authentication->base.get_shared_secret = &test_get_shared_secret;
    authentication->base.get_authenticated_peer_credential_token = &test_get_authenticated_peer_credential_token;
    authentication->base.set_listener = &test_set_listener;
    authentication->base.return_identity_token = &test_return_identity_token;
    authentication->base.return_identity_status_token = &test_return_identity_status_token;
    authentication->base.return_authenticated_peer_credential_token = &test_return_authenticated_peer_credential_token;
    authentication->base.return_handshake_handle = &test_return_handshake_handle;
    authentication->base.return_identity_handle = &test_return_identity_handle;
    authentication->base.return_sharedsecret_handle = &test_return_sharedsecret_handle;

    *context = authentication;
    return init_authentication(argument, (void**)&authentication->instance);
}

int32_t finalize_test_authentication(void *instance)
{
    struct dds_security_authentication_impl *authentication = instance;

    printf("%s: started\n", __func__);
    return finalize_authentication(authentication->instance);
}

int test_authentication_plugin_init(void)
{
    printf("%s: started\n", __func__);
    initMessageQueue(&testMessagaQueue);

    return 1;
}

int test_authentication_plugin_deinit(void)
{
    printf("%s: started\n", __func__);
    deinitMessageQueue(&testMessagaQueue);

    return 1;
}
