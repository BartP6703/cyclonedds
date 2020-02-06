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
#include <stdlib.h>
#include <assert.h>

#include "dds/dds.h"
#include "CUnit/Test.h"
#include "config_env.h"

#include "dds/version.h"
#include "dds/ddsrt/cdtors.h"
#include "dds/ddsrt/environ.h"
#include "dds/ddsrt/heap.h"
#include "dds/ddsrt/string.h"
#include "dds/ddsi/q_misc.h"
#include "dds/ddsi/q_xqos.h"

#include "dds/security/dds_security_api_defs.h"

#include "handshake.h"

#define CYCLONEDDS_URI DDS_PROJECT_NAME_NOSPACE_CAPS"_URI"

const char *config =
      "<"DDS_PROJECT_NAME">"
        "<Domain id=\"any\">"
          "<Tracing><Verbosity>finest</></>"
            "<DDSSecurity>"
            "<Authentication>"
                "<Library finalizeFunction=\"finalize_test_authentication\" initFunction=\"init_test_authentication\" path=\"/home/bart/eclipse_cyclone/fork/cyclonedds/build/src/security/core/tests/authentication_handshake/libdds_security_authentication_handshake.so\"/>"
                "<IdentityCertificate>"
                    "-----BEGIN CERTIFICATE-----"
                    "MIIDYDCCAkigAwIBAgIBBDANBgkqhkiG9w0BAQsFADByMQswCQYDVQQGEwJOTDEL"
                    "MAkGA1UECBMCT1YxEzARBgNVBAoTCkFETGluayBJU1QxGTAXBgNVBAMTEElkZW50"
                    "aXR5IENBIFRlc3QxJjAkBgkqhkiG9w0BCQEWF2luZm9AaXN0LmFkbGlua3RlY2gu"
                    "Y29tMB4XDTE4MDMxMjAwMDAwMFoXDTI3MDMxMTIzNTk1OVowdTELMAkGA1UEBhMC"
                    "TkwxCzAJBgNVBAgTAk9WMRAwDgYDVQQKEwdBRExpbmsgMREwDwYDVQQLEwhJU1Qg"
                    "VGVzdDETMBEGA1UEAxMKQWxpY2UgVGVzdDEfMB0GCSqGSIb3DQEJARYQYWxpY2VA"
                    "YWRsaW5rLmlzdDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANBW+tEZ"
                    "Baw7EQCEXyzH9n7IkZ8PQIKe8hG1LAOGYOF/oUYQZJO/HxbWoC4rFqOC20+A6is6"
                    "kFwr1Zzp/Wurk9CrFXo5Nomi6ActH6LUM57nYqN68w6U38z/XkQxVY/ESZ5dySfD"
                    "9Q1C8R+zdE8gwbimdYmwX7ioz336nghM2CoAHPDRthQeJupl8x4V7isOltr9CGx8"
                    "+imJXbGr39OK6u87cNLeu23sUkOIC0lSRMIqIQK3oJtHS70J2qecXdqp9MhE7Xky"
                    "/GPlI8ptQ1gJ8A3cAOvtI9mtMJMszs2EKWTLfeTcmfJHKKhKjvCgDdh3Jan4x5YP"
                    "Yg7HG6H+ceOUkMMCAwEAATANBgkqhkiG9w0BAQsFAAOCAQEAkvuqZzyJ3Nu4/Eo5"
                    "kD0nVgYGBUl7cspu+636q39zPSrxLEDMUWz+u8oXLpyGcgiZ8lZulPTV8dmOn+3C"
                    "Vg55c5C+gbnbX3MDyb3wB17296RmxYf6YNul4sFOmj6+g2i+Dw9WH0PBCVKbA84F"
                    "jR3Gx2Pfoifor3DvT0YFSsjNIRt090u4dQglbIb6cWEafC7O24t5jFhGPvJ7L9SE"
                    "gB0Drh/HmKTVuaqaRkoOKkKaKuWoXsszK1ZFda1DHommnR5LpYPsDRQ2fVM4EuBF"
                    "By03727uneuG8HLuNcLEV9H0i7LxtyfFkyCPUQvWG5jehb7xPOz/Ml26NAwwjlTJ"
                    "xEEFrw=="
                    "-----END CERTIFICATE-----"
                "</IdentityCertificate>"
                "<IdentityCA>"
                    "-----BEGIN CERTIFICATE-----"
                    "MIIEKTCCAxGgAwIBAgIBATANBgkqhkiG9w0BAQsFADByMQswCQYDVQQGEwJOTDEL"
                    "MAkGA1UECBMCT1YxEzARBgNVBAoTCkFETGluayBJU1QxGTAXBgNVBAMTEElkZW50"
                    "aXR5IENBIFRlc3QxJjAkBgkqhkiG9w0BCQEWF2luZm9AaXN0LmFkbGlua3RlY2gu"
                    "Y29tMB4XDTE4MDMxMjAwMDAwMFoXDTI3MDMxMTIzNTk1OVowcjELMAkGA1UEBhMC"
                    "TkwxCzAJBgNVBAgTAk9WMRMwEQYDVQQKEwpBRExpbmsgSVNUMRkwFwYDVQQDExBJ"
                    "ZGVudGl0eSBDQSBUZXN0MSYwJAYJKoZIhvcNAQkBFhdpbmZvQGlzdC5hZGxpbmt0"
                    "ZWNoLmNvbTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBANa/ENFfGVXg"
                    "bPLTzBdDfiZQcp5dWZ//Pb8ErFOJu8uosVHFv8t69dgjHgNHB4OsjmjnR7GfKUZT"
                    "0cMvWJnjsC7DDlBwFET9rj4k40n96bbVCH9I7+tNhsoqzc6Eu+5h4sk7VfNGTM2Z"
                    "SyCd4GiSZRuA44rRbhXI7/LDpr4hY5J9ZDo5AM9ZyoLAoh774H3CZWD67S35XvUs"
                    "72dzE6uKG/vxBbvZ7eW2GLO6ewa9UxlnLVMPfJdpkp/xYXwwcPW2+2YXCge1ujxs"
                    "tjrOQJ5HUySh6DkE/kZpx8zwYWm9AaCrsvCIX1thsqgvKy+U5v1FS1L58eGc6s//"
                    "9yMgNhU29R0CAwEAAaOByTCBxjAMBgNVHRMEBTADAQH/MB0GA1UdDgQWBBRNVUJN"
                    "FzhJPReYT4QSx6dK53CXCTAfBgNVHSMEGDAWgBRNVUJNFzhJPReYT4QSx6dK53CX"
                    "CTAPBgNVHQ8BAf8EBQMDB/+AMGUGA1UdJQEB/wRbMFkGCCsGAQUFBwMBBggrBgEF"
                    "BQcDAgYIKwYBBQUHAwMGCCsGAQUFBwMEBggrBgEFBQcDCAYIKwYBBQUHAwkGCCsG"
                    "AQUFBwMNBggrBgEFBQcDDgYHKwYBBQIDBTANBgkqhkiG9w0BAQsFAAOCAQEAcOLF"
                    "ZYdJguj0uxeXB8v3xnUr1AWz9+gwg0URdfNLU2KvF2lsb/uznv6168b3/FcPgezN"
                    "Ihl9GqB+RvGwgXS/1UelCGbQiIUdsNxk246P4uOGPIyW32RoJcYPWZcpY+cw11tQ"
                    "NOnk994Y5/8ad1DmcxVLLqq5kwpXGWQufV1zOONq8B+mCvcVAmM4vkyF/de56Lwa"
                    "sAMpk1p77uhaDnuq2lIR4q3QHX2wGctFid5Q375DRscFQteY01r/dtwBBrMn0wuL"
                    "AMNx9ZGD+zAoOUaslpIlEQ+keAxk3jgGMWFMxF81YfhEnXzevSQXWpyek86XUyFL"
                    "O9IAQi5pa15gXjSbUg=="
                    "-----END CERTIFICATE-----"
                "</IdentityCA>"
                "<PrivateKey>"
                    "-----BEGIN RSA PRIVATE KEY-----"
                    "MIIEowIBAAKCAQEA0Fb60RkFrDsRAIRfLMf2fsiRnw9Agp7yEbUsA4Zg4X+hRhBk"
                    "k78fFtagLisWo4LbT4DqKzqQXCvVnOn9a6uT0KsVejk2iaLoBy0fotQznudio3rz"
                    "DpTfzP9eRDFVj8RJnl3JJ8P1DULxH7N0TyDBuKZ1ibBfuKjPffqeCEzYKgAc8NG2"
                    "FB4m6mXzHhXuKw6W2v0IbHz6KYldsavf04rq7ztw0t67bexSQ4gLSVJEwiohAreg"
                    "m0dLvQnap5xd2qn0yETteTL8Y+Ujym1DWAnwDdwA6+0j2a0wkyzOzYQpZMt95NyZ"
                    "8kcoqEqO8KAN2HclqfjHlg9iDscbof5x45SQwwIDAQABAoIBAG0dYPeqd0IhHWJ7"
                    "8azufbchLMN1pX/D51xG2uptssfnpHuhkkufSZUYi4QipRS2ME6PYhWJ8pmTi6lH"
                    "E6cUkbI0KGd/F4U2gPdhNrR9Fxwea5bbifkVF7Gx/ZkRjZJiZ3w9+mCNTQbJDKhh"
                    "wITAzzT6WYznhvqbzzBX1fTa6kv0GAQtX7aHKM+XIwkhX2gzU5TU80bvH8aMrT05"
                    "tAMGQqkUeRnpo0yucBl4VmTZzd/+X/d2UyXR0my15jE5iH5o+p+E6qTRE9D+MGUd"
                    "MQ6Ftj0Untqy1lcog1ZLL6zPlnwcD4jgY5VCYDgvabnrSwymOJapPLsAEdWdq+U5"
                    "ec44BMECgYEA/+3qPUrd4XxA517qO3fCGBvf2Gkr7w5ZDeATOTHGuD8QZeK0nxPl"
                    "CWhRjdgkqo0fyf1cjczL5XgYayo+YxkO1Z4RUU+8lJAHlVx9izOQo+MTQfkwH4BK"
                    "LYlHxMoHJwAOXXoE+dmBaDh5xT0mDUGU750r763L6EFovE4qRBn9hxkCgYEA0GWz"
                    "rpOPNxb419WxG9npoQYdCZ5IbmEOGDH3ReggVzWHmW8sqtkqTZm5srcyDpqAc1Gu"
                    "paUveMblEBbU+NFJjLWOfwB5PCp8jsrqRgCQSxolShiVkc3Vu3oyzMus9PDge1eo"
                    "9mwVGO7ojQKWRu/WVAakENPaAjeyyhv4dqSNnjsCgYEAlwe8yszqoY1k8+U0T0G+"
                    "HeIdOCXgkmOiNCj+zyrLvaEhuS6PLq1b5TBVqGJcSPWdQ+MrglbQIKu9pUg5ptt7"
                    "wJ5WU+i9PeK9Ruxc/g/BFKYFkFJQjtZzb+nqm3wpul8zGwDN/O/ZiTqCyd3rHbmM"
                    "/dZ/viKPCZHIEBAEq0m3LskCgYBndzcAo+5k8ZjWwBfQth5SfhCIp/daJgGzbYtR"
                    "P/BenAsY2KOap3tjT8Fsw5usuHSxzIojX6H0Gvu7Qzq11mLn43Q+BeQrRQTWeFRc"
                    "MQdy4iZFZXNNEp7dF8yE9VKHwdgSJPGUdxD6chMvf2tRCN6mlS171VLV6wVvZvez"
                    "H/vX5QKBgD2Dq/NHpjCpAsECP9awmNF5Akn5WJbRGmegwXIih2mOtgtYYDeuQyxY"
                    "ZCrdJFfIUjUVPagshEmUklKhkYMYpzy2PQDVtaVcm6UNFroxT5h+J+KDs1LN1H8G"
                    "LsASrzyAg8EpRulwXEfLrWKiu9DKv8bMEgO4Ovgz8zTKJZIFhcac"
                    "-----END RSA PRIVATE KEY-----"
                "</PrivateKey>"
            "</Authentication>"
            "<AccessControl>"
                "<Library finalizeFunction=\"finalize_access_control\" initFunction=\"init_access_control\" path=\"/home/bart/eclipse_cyclone/fork/cyclonedds/build/lib/libdds_security_ac.so\"/>"
                "<Governance>file:Governance.p7s</Governance>"
                "<PermissionsCA>file:Permissions_CA.pem</PermissionsCA>"
                "<Permissions>file:Permissions.p7s</Permissions>"
            "</AccessControl>"
            "<Cryptographic>"
                "<Library finalizeFunction=\"finalize_crypto\" initFunction=\"init_crypto\" path=\"/home/bart/eclipse_cyclone/fork/cyclonedds/build/lib/libdds_security_crypto.so\"/>"
            "</Cryptographic>"
        "</DDSSecurity>"
        "</Domain>"
      "</"DDS_PROJECT_NAME">";

#define DDS_DOMAINID_PART1 0
#define DDS_DOMAINID_PART2 1
#define DDS_CONFIG_NO_PORT_GAIN "${CYCLONEDDS_URI}${CYCLONEDDS_URI:+,}<Discovery><ExternalDomainId>0</ExternalDomainId></Discovery>"
#define DDS_CONFIG_NO_PORT_GAIN_LOG "${CYCLONEDDS_URI}${CYCLONEDDS_URI:+,}<Tracing><OutputFile>cyclonedds_liveliness_tests.${CYCLONEDDS_DOMAIN_ID}.${CYCLONEDDS_PID}.log</OutputFile><Verbosity>finest</Verbosity></Tracing><Discovery><ExternalDomainId>0</ExternalDomainId></Discovery>"

static dds_entity_t g_part1_domain = 0;
static dds_entity_t g_part1_participant = 0;

static dds_entity_t g_part2_domain = 0;
static dds_entity_t g_part2_participant = 0;

void handshake_init (void)
{
  /* Domains for pub and sub use a different domain id, but the portgain setting
   * in configuration is 0, so that both domains will map to the same port number.
   * This allows to create two domains in a single test process. */
  char *conf_part1;
  char *conf_part2;

  test_authentication_plugin_init();

  ddsrt_setenv(CYCLONEDDS_URI, config);
  conf_part1 = ddsrt_expand_envvars(DDS_CONFIG_NO_PORT_GAIN, DDS_DOMAINID_PART1);
  conf_part2 = ddsrt_expand_envvars(DDS_CONFIG_NO_PORT_GAIN, DDS_DOMAINID_PART2);
  g_part1_domain = dds_create_domain(DDS_DOMAINID_PART1, conf_part1);
  g_part2_domain = dds_create_domain(DDS_DOMAINID_PART2, conf_part2);
  dds_free(conf_part1);
  dds_free(conf_part2);

  CU_ASSERT_FATAL((g_part1_participant = dds_create_participant(DDS_DOMAINID_PART1, NULL, NULL)) > 0);
  CU_ASSERT_FATAL((g_part2_participant = dds_create_participant(DDS_DOMAINID_PART2, NULL, NULL)) > 0);

  ddsrt_setenv(CYCLONEDDS_URI, "");
}

void handshake_fini (void)
{
  CU_ASSERT_EQUAL_FATAL(dds_delete(g_part1_participant), DDS_RETCODE_OK);
  CU_ASSERT_EQUAL_FATAL(dds_delete(g_part2_participant), DDS_RETCODE_OK);
  CU_ASSERT_EQUAL_FATAL(dds_delete(g_part1_domain), DDS_RETCODE_OK);
  CU_ASSERT_EQUAL_FATAL(dds_delete(g_part2_domain), DDS_RETCODE_OK);

  test_authentication_plugin_deinit();
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



void syncControlInit ( const char *id, struct SyncControl *sync, const char *fileName)
{
    (void)id;
    sync->fileName = ddsrt_strdup(fileName);
    sync->sendCtrl = fopen(fileName, "w");
    sync->pos = 0;
}

void syncControlDeinit( const char *id, struct SyncControl *sync)
{
    (void)id;
    dds_free(sync->recvName);
    fclose(sync->sendCtrl);
}

int syncControlRead( const char *id, struct SyncControl *sync, char *token, int size, int timeout)
{
    int found = 0;
    FILE *f;
    dds_duration_t delay = DDS_SECS(1);

    (void)id;
    while (!found && timeout > 0) {
        f = fopen(sync->fileName, "r");
        if (f) {
            if (fseek(f, sync->pos, SEEK_SET) != -1) {
                if (fgets(token, size, f)) {
                    found = 1;
                }
                sync->pos = ftell(f);
            }
            fclose(f);
        } else {
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

    return found;
}

int syncControlWaitFor( const char *id, struct SyncControl *sync, int timeout)
{
    int idx = -1;
    char token[256];
    int i;

    (void)id;
    if (syncControlRead(id, sync, token, sizeof(token), timeout)) {
        for (i = 0; (i < numControls) && (idx < 0); i++) {
            if (strcmp(controls[i], token) == 0) {
                printf("Recv: %s\n", token);
                idx = (int)i;
            }
        }
    }
    return idx;
}

void syncControlSend( const char *id, struct SyncControl *sync, int idx)
{
    (void)id;
    assert(idx < numControls);
    fprintf(sync->sendCtrl, "%s\n", controls[idx]);
    fflush(sync->sendCtrl);
    printf("Send: %s\n", controls[idx]);
}

void
addLocalIdentity(
    DDS_Security_IdentityHandle handle,
    DDS_Security_GUID_t *guid)
{
    localIdentityList[numLocal].handle = handle;
    memcpy(&localIdentityList[numLocal].guid, guid, sizeof(DDS_Security_GUID_t));
    numLocal++;
}

int
findLocalIdentity(
    DDS_Security_IdentityHandle handle)
{
    int i;

    for (i = 0; i < (int)numLocal; i++) {
        if (localIdentityList[i].handle == handle) {
            return i;
        }
    }

    return -1;
}

int
findRemoteIdentity(
    DDS_Security_IdentityHandle handle)
{
    int i;

    for (i = 0; i < numRemote; i++) {
        if (remoteIdentityList[i].handle == handle) {
            return i;
        }
    }

    return -1;
}

void
addRemoteIdentity(
    DDS_Security_IdentityHandle handle,
    DDS_Security_GUID_t *guid)
{
    int idx;

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

    for (i = 0; i < numHandshake; i++) {
        if (handshakeList[i].handle == handle) {
            return i;
        }
    }

    return -1;
}

int
handleProcessMessage(
    DDS_Security_IdentityHandle handshake)
{
    struct Message *msg;
    int idx;
    dds_duration_t timeout = DDS_SECS(10);

    msg = test_authentication_plugin_read(MESSAGE_KIND_PROCESS_HANDSHAKE, 0, 0, handshake, timeout);
    if (msg) {
        idx = findHandshake(msg->hsHandle);
        if (idx >= 0) {
            handshakeList[idx].finalResult = msg->result;
            return 1;
        }
    }

    return 0;
}

int
handleBeginHandshakeRequest(
    DDS_Security_IdentityHandle lid,
    DDS_Security_IdentityHandle rid)
{
    struct Message *msg;
    dds_duration_t timeout = DDS_SECS(10);

    msg = test_authentication_plugin_read(MESSAGE_KIND_BEGIN_HANDSHAKE_REQUEST, lid, rid, 0, timeout);
    if (msg) {
        addHandshake(msg->hsHandle, 1, msg->lidHandle, msg->ridHandle, msg->result);
        return handleProcessMessage(msg->hsHandle);
    }

    return 0;
}

int
handleBeginHandshakeReply(
    DDS_Security_IdentityHandle lid,
    DDS_Security_IdentityHandle rid)
{
    struct Message *msg;
    dds_duration_t timeout = DDS_SECS(10);

    msg = test_authentication_plugin_read(MESSAGE_KIND_BEGIN_HANDSHAKE_REPLY, lid, rid, 0, timeout);
    if (msg) {
        addHandshake(msg->hsHandle, 0, msg->lidHandle, msg->ridHandle, msg->result);
        return handleProcessMessage(msg->hsHandle);
    }

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

CU_Test(bart_handshake, plugin_read, .init = handshake_init, .fini = handshake_fini)
{
    struct Message *msg;
    dds_duration_t timeout = DDS_SECS(10);

    msg = test_authentication_plugin_read(MESSAGE_KIND_VALIDATE_LOCAL_IDENTITY, 0, 0, 0, timeout);
    while (msg) {
    	printf("%s(): msg->instance:%p\n", __func__, msg->instance);
        addLocalIdentity(msg->lidHandle, &msg->lguid);
        msg = test_authentication_plugin_read(MESSAGE_KIND_VALIDATE_LOCAL_IDENTITY, 0, 0, 0, timeout);
    }
}

