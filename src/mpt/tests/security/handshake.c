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
#include "mpt/mpt.h"
#include "procs/handshake.h"

#define PROCESS1_ARGS MPT_ArgValues(DDS_DOMAIN_DEFAULT)
#define PROCESS2_ARGS MPT_ArgValues(DDS_DOMAIN_DEFAULT)
MPT_TestProcess(handshake_suite, handshake, process1, handshake_process1, PROCESS1_ARGS);
MPT_TestProcess(handshake_suite, handshake, process2, handshake_process2, PROCESS2_ARGS);
MPT_Test(handshake_suite, handshake, .init = handshake_init, .fini = handshake_fini);
#undef PROCESS1_ARGS
#undef PROCESS2_ARGS
