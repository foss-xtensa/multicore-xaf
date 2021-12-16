/*
 * Copyright (c) 2009 Tensilica Inc.  ALL RIGHTS RESERVED.
 * These coded instructions, statements, and computer programs are the
 * copyrighted works and confidential proprietary information of
 * Tensilica Inc.  They may be adapted and modified by bona fide
 * purchasers for internal use, but neither the original nor any adapted
 * or modified version may be disclosed or distributed to third parties
 * in any manner, medium, or form, in whole or in part, without the prior
 * written consent of Tensilica Inc.
 */

#ifndef __XF_SHARED__
#define __XF_SHARED__

#include "xaf-api.h"

#ifndef XF_SHMEM_SIZE
#define XF_SHMEM_SIZE  ((1024<<8)*XF_CFG_CORES_NUM)
#endif

extern char shared_mem[];

#endif //__XF_SHARED__