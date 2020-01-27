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
#include "procs/bart.h"

#define TEST_PUB_ARGS MPT_ArgValues(DDS_DOMAIN_DEFAULT)
#define TEST_SUB_ARGS MPT_ArgValues(DDS_DOMAIN_DEFAULT)
MPT_TestProcess(bart_suite, bart, pub, bart_publisher,  TEST_PUB_ARGS);
MPT_TestProcess(bart_suite, bart, sub, bart_subscriber, TEST_SUB_ARGS);
MPT_Test(bart_suite, bart, .init = bart_init, .fini = bart_fini);
#undef TEST_SUB_ARGS
#undef TEST_PUB_ARGS
