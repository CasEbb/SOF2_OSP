// Copyright (C) 2001-2002 Raven Software.
//

#include "gt_local.h"

#define ITEM_RED_DOCUMENTS 300
#define ITEM_BLUE_DOCUMENTS 301

void GT_Init(void);
void GT_RunFrame(int time);
int GT_Event(int cmd, int time, int arg0, int arg1, int arg2, int arg3, int arg4);

gametypeLocals_t gametype;

typedef struct
{
    vmCvar_t *vmCvar;
    char *cvarName;
    char *defaultString;
    int cvarFlags;
    float mMinValue, mMaxValue;
    int modificationCount; // for tracking changes
    qboolean trackChange;  // track this variable, and announce if changed
    qboolean teamShader;   // track and if changed, update shader state
} cvarTable_t;

vmCvar_t gt_vipRoundTimelimit;

static cvarTable_t gametypeCvarTable[] = {
    {&gt_vipRoundTimelimit, "gt_vipRoundTimelimit", "3", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {NULL, NULL, NULL, 0, 0.0f, 0.0f, 0, qfalse},
};

static int gametypeCvarTableSize = sizeof(gametypeCvarTable) / sizeof(gametypeCvarTable[0]);

/*
================
vmMain

This is the only way control passes into the module.
This must be the very first function compiled into the .q3vm file
================
*/
int vmMain(int command, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11)
{
    switch (command)
    {
    case GAMETYPE_INIT:
        GT_Init();
        return 0;

    case GAMETYPE_START:
        gametype.firstFrame = qtrue;
        return 0;

    case GAMETYPE_RUN_FRAME:
        GT_RunFrame(arg0);
        return 0;

    case GAMETYPE_EVENT:
        return GT_Event(arg0, arg1, arg2, arg3, arg4, arg5, arg6);
    }

    return -1;
}

/*
=================
GT_RegisterCvars
=================
*/
void GT_RegisterCvars(void)
{
    cvarTable_t *cv;

    for (cv = gametypeCvarTable; cv->cvarName != NULL; cv++)
    {
        trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags, cv->mMinValue, cv->mMaxValue);

        if (cv->vmCvar)
        {
            cv->modificationCount = cv->vmCvar->modificationCount;
        }
    }
}

/*
=================
GT_UpdateCvars
=================
*/
void GT_UpdateCvars(void)
{
    cvarTable_t *cv;

    for (cv = gametypeCvarTable; cv->cvarName != NULL; cv++)
    {
        if (cv->vmCvar)
        {
            trap_Cvar_Update(cv->vmCvar);

            if (cv->modificationCount != cv->vmCvar->modificationCount)
            {
                cv->modificationCount = cv->vmCvar->modificationCount;
            }
        }
    }
}

/*
================
GT_Init

initializes the gametype by spawning the gametype items and 
preparing them
================
*/
void GT_Init(void)
{
    gtItemDef_t itemRedDocuments;
    gtItemDef_t itemBlueDocuments;

    memset(&gametype, 0, sizeof(gametype));

    GT_RegisterCvars();

    memset(&itemRedDocuments, 0, sizeof(itemRedDocuments));
    trap_Cmd_RegisterItem(ITEM_RED_DOCUMENTS, "red_documents", &itemRedDocuments);

    memset(&itemBlueDocuments, 0, sizeof(itemBlueDocuments));
    trap_Cmd_RegisterItem(ITEM_BLUE_DOCUMENTS, "blue_documents", &itemBlueDocuments);

    gametype.firstFrame = qtrue;
}

char *OSP_GetTeamName(int team)
{
    switch (team)
    {
    case TEAM_FREE:
        return "FFA";
    case TEAM_RED:
    {
        char name[64];
        memset(name, 0, sizeof(name));
        trap_Cvar_VariableStringBuffer("team_redName", name, sizeof(name) - 1);
        return va("%s", name);
    }
    case TEAM_BLUE:
    {
        char name[64];
        memset(name, 0, sizeof(name));
        trap_Cvar_VariableStringBuffer("team_blueName", name, sizeof(name) - 1);
        return va("%s", name);
    }
    case TEAM_SPECTATOR:
        return "Spectators";
    }

    return "unknown team";
}

/*
================
GT_RunFrame

Runs all thinking code for gametype
================
*/
void GT_RunFrame(int time)
{
    int x[2];
    gametype.time = time;

    if (gametype.firstFrame)
    {
        int clients[2][MAX_CLIENTS];
        int count[3];

        gametype.roundOver = qfalse;

        count[0] = trap_Cmd_GetClientList(TEAM_RED, clients[0], 64);
        count[1] = trap_Cmd_GetClientList(TEAM_BLUE, clients[1], 64);

        if (count[0] && count[1])
        {
            do
            {
                gametype.vipClient[TEAM_RED] = clients[0][rand() % count[0]];
            } while (gametype.vipClient[TEAM_RED] == gametype.previousVipClient[TEAM_RED] && count[0] > 1);

            gametype.previousVipClient[TEAM_RED] = gametype.vipClient[TEAM_RED];
            trap_Cmd_GiveClientItem(gametype.vipClient[TEAM_RED], ITEM_RED_DOCUMENTS);

            do
            {
                gametype.vipClient[TEAM_BLUE] = clients[1][rand() % count[1]];
            } while (gametype.vipClient[TEAM_BLUE] == gametype.previousVipClient[TEAM_BLUE] && count[1] > 1);

            gametype.previousVipClient[TEAM_BLUE] = gametype.vipClient[TEAM_BLUE];
            trap_Cmd_GiveClientItem(gametype.vipClient[TEAM_BLUE], ITEM_BLUE_DOCUMENTS);

            gametype.firstFrame = qfalse;
        }
    }

    GT_UpdateCvars();
}

/*
================
GT_Event

Handles all events sent to the gametype
================
*/
int GT_Event(int cmd, int time, int arg0, int arg1, int arg2, int arg3, int arg4)
{
    switch (cmd)
    {
    case GTEV_CLIENT_DEATH:
    {
        char name[64];
        // arg0 = clientID
        // arg1 = clientTeam
        // arg2 = killerID
        // arg3 = killerTeam
        if (!gametype.roundOver)
        {
            if (arg0 == gametype.vipClient[TEAM_RED])
            {
                trap_Cmd_GetClientName(arg0, name, sizeof(name));
                gametype.roundOver = qtrue;
                trap_Cmd_AddTeamScore(TEAM_BLUE, 1);

                if (arg2 >= 0 && arg1 != arg3)
                {
                    trap_Cmd_AddClientScore(arg2, 4);
                }

                trap_Cmd_TextMessage(-1, va("The ^1RED^7 V.I.P. (%s^7) has been killed!", name));
                trap_Cmd_Restart(5);
                break;
            }

            if (arg0 == gametype.vipClient[TEAM_BLUE])
            {
                trap_Cmd_GetClientName(arg0, name, sizeof(name));
                gametype.roundOver = qtrue;
                trap_Cmd_AddTeamScore(TEAM_RED, 1);

                if (arg2 >= 0 && arg1 != arg3)
                {
                    trap_Cmd_AddClientScore(arg2, 4);
                }

                trap_Cmd_TextMessage(-1, va("The ^4BLUE^7 V.I.P. (%s^7) has been killed!", name));
                trap_Cmd_Restart(5);
                break;
            }

            if (arg2 >= 0 && arg1 != arg3 && arg2 != gametype.vipClient[arg3] && arg4 > gametype.time)
            {
                trap_Cmd_AddClientScore(arg2, 2);
                break;
            }

            if (arg2 >= 0 && arg1 != arg3 && (arg2 == gametype.vipClient[TEAM_RED] || arg2 == gametype.vipClient[TEAM_BLUE]))
            {
                trap_Cmd_AddClientScore(arg2, 2);
                break;
            }
        }
        break;
    }
    default:
        break;
    }

    return 0;
}

#ifndef GAMETYPE_HARD_LINKED
// this is only here so the functions in q_shared.c and bg_*.c can link (FIXME)

void QDECL Com_Error(int level, const char *msg, ...)
{
    va_list argptr;
    char text[1024];

    va_start(argptr, msg);
    vsprintf(text, msg, argptr);
    va_end(argptr);

    trap_Error(text);
}

void QDECL Com_Printf(const char *msg, ...)
{
    va_list argptr;
    char text[1024];

    va_start(argptr, msg);
    vsprintf(text, msg, argptr);
    va_end(argptr);

    trap_Print(text);
}

#endif
