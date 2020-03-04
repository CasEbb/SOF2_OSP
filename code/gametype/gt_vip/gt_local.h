// Copyright (C) 2001-2002 Raven Software.
//
// gt_local.h -- local definitions for gametype module

#include "../../game/q_shared.h"
#include "../gt_public.h"
#include "../gt_syscalls.h"

typedef struct gametypeLocals_s
{
    qboolean firstFrame;
    int time;
    int vipClient[TEAM_NUM_TEAMS];
    int previousVipClient[TEAM_NUM_TEAMS];
    int roundOver;
} gametypeLocals_t;

extern gametypeLocals_t gametype;
