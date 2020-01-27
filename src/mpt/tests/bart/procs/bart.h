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

#if defined (__cplusplus)
extern "C" {
#endif

void bart_init (void);
void bart_fini (void);

MPT_ProcessEntry (bart_publisher,
                  MPT_Args (dds_domainid_t domainid));

MPT_ProcessEntry (bart_subscriber,
                  MPT_Args (dds_domainid_t domainid));

#if defined (__cplusplus)
}
#endif

#endif
