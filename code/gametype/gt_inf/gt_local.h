// Copyright (C) 2001-2002 Raven Software.
//
// gt_local.h -- local definitions for gametype module

#include "../../game/q_shared.h"
#include "../gt_public.h"
#include "../gt_syscalls.h"

typedef struct gametypeLocals_s
{
    int time;

    int caseTakenSound;
    int caseCaptureSound;
    int caseReturnSound;

    // OSP
    int x10;
    int x14;
    int x18;
    // END
} gametypeLocals_t;

extern gametypeLocals_t gametype;
