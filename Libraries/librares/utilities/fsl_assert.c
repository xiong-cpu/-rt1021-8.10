/*
* Copyright (c) 2015-2016, Freescale Semiconductor, Inc.
* Copyright 2016-2017 NXP
* All rights reserved.
*
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include "fsl_common.h"
#include "RT102X_config.h"
#include "fsl_debug_console.h"

#ifndef NDEBUG
#if (defined(__CC_ARM)) || (defined(__ARMCC_VERSION)) || (defined(__ICCARM__))
void __aeabi_assert(const char *failedExpr, const char *file, int line)
{
#if (1 == DEBUG_PRINTF)
    PRINTF("ASSERT ERROR \" %s \": file \"%s\" Line \"%d\" \n", failedExpr, file, line);
#endif
    for (;;)
    {
        __BKPT(0);
    }
}
#elif(defined(__GNUC__))
void __assert_func(const char *file, int line, const char *func, const char *failedExpr)
{
#if (1 == DEBUG_PRINTF)
    PRINTF("ASSERT ERROR \" %s \": file \"%s\" Line \"%d\" function name \"%s\" \n", failedExpr, file, line, func);
#endif
    for (;;)
    {
        __BKPT(0);
    }
}
#endif /* (defined(__CC_ARM) || (defined(__ICCARM__)) || (defined(__ARMCC_VERSION)) */
#endif /* NDEBUG */
