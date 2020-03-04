// osp_tcmd.c
//
#include "g_local.h"

static void OSP_Tcmd_Help(gentity_t *ent, int index, int param);
static qboolean OSP_Tcmd_Usage(gentity_t *ent, char *cmd, unsigned int index);
static void OSP_Tcmd_Info(gentity_t *ent, int index, int param);
static void OSP_Tcmd_Reset(gentity_t *ent, int index, int param);
static void OSP_Tcmd_Ready(gentity_t *ent, int index, int param);
static void OSP_Tcmd_Name(gentity_t *ent, int index, int param);
static void OSP_Tcmd_Lock(gentity_t *ent, int index, int param);
static void OSP_Tcmd_Specinvite(gentity_t *ent, int index, int param);
static void OSP_Tcmd_Speclock(gentity_t *ent, int index, int param);
static void OSP_Tcmd_Timeout(gentity_t *ent, int index, int param);
static void OSP_Tcmd_Delegate(gentity_t *ent, int index, int param);
static void OSP_Tcmd_Invite(gentity_t *ent, int index, int param);

typedef struct
{
    char *name;
    int matchphase;
    int param;
    void (*func)(gentity_t *, int, int);
    char *helptext;
} ospTeamCmd_t;

static ospTeamCmd_t ospTeamCmds[] = {
    {"?", 0, 1, OSP_Tcmd_Help, ":^7 Gives a list of admin commands"},
    {"commands", 0, 1, OSP_Tcmd_Help, ":^7 Gives a list of admin commands"},
    {"info", 1, 0, OSP_Tcmd_Info, ":^7 Output info about the team, and team settings"},
    {"reset", 1, 0, OSP_Tcmd_Reset, ":^7 Reset your team's settings to defaults"},
    {"ready", 1, 0, OSP_Tcmd_Ready, ":^7 Readies everyone to start match"},
    {"name", 1, 0, OSP_Tcmd_Name, ":^7 Set a name for your team"},
    {"lock", 2, 1, OSP_Tcmd_Lock, ":^7 Lock the team so no one can join"},
    {"unlock", 2, 0, OSP_Tcmd_Lock, ":^7 Unlock the team to allow people to join"},
    {"specinvite", 1, 1, OSP_Tcmd_Specinvite, " <player_ID> :^7 Allow specified spectator to spectate your team"},
    {"specuninvite", 1, 0, OSP_Tcmd_Specinvite, " <player_ID> :^7 Remove specified spectator's invitation to spectate"},
    {"speclock", 1, 1, OSP_Tcmd_Speclock, ":^7 Blocks any spectator from following your team"},
    {"specunlock", 1, 0, OSP_Tcmd_Speclock, ":^7 Remove the spectator lock on your team"},
    {"timein", 3, 0, OSP_Tcmd_Timeout, ":^7 Unpauses a match (if initiated by the issuing team)"},
    {"timeout", 3, 1, OSP_Tcmd_Timeout, ":^7 Allows a team to pause a match"},
    {"delegate", 0, 1, OSP_Tcmd_Delegate, " <player_ID> :^7 Delegate someone from your team to be captain"},
    {"invite", 0, 1, OSP_Tcmd_Invite, " <player_ID> :^7 Invite a spectator to your team"},
    {"remove", 0, 0, OSP_Tcmd_Invite, " <player_ID> :^7 Remove a player from your team"},
    {NULL, 0, 1, 0, NULL},
};

ospTeamData_t ospTeams[TEAM_NUM_TEAMS];

// osp: 0x00032079
void OSP_Cmd_Tcmd(gentity_t *e, int index, int param)
{
    unsigned int i;
    ospTeamCmd_t *cmd;
    gentity_t *ent;
    char arg[128];
    unsigned int numCmds;

    numCmds = sizeof(ospTeamCmds) / sizeof(ospTeamCmds[0]);
    ent = e;

    if (!level.gametypeData->teams)
    {
        trap_SendServerCommand(ent - g_entities, "print \"Team commands are NOT available in non-team gametypes.\n\"");
        return;
    }

    if (!team_controls.integer)
    {
        trap_SendServerCommand(ent - g_entities, "print \"Team commands are NOT enabled on this server.\n\"");
        return;
    }

    if (ent->client->sess.team != TEAM_RED && ent->client->sess.team != TEAM_BLUE)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You must be on a playing team to use team commands.\n\"");
        return;
    }

    if (!ent->client->ospClient->captain)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You are not a captain, you can\'t use team commands.\n\"");
        return;
    }
    else
    {
        memset(arg, 0, sizeof(arg));
        trap_Argv(1, arg, sizeof(arg) - 1);
    }

    for (i = 0; i < numCmds; i++)
    {
        cmd = &ospTeamCmds[i];

        if (trap_Argc() == 1)
        {
            OSP_Tcmd_Help(ent, i, cmd->param);
            return;
        }

        if (cmd->func && Q_stricmp(arg, cmd->name) == 0)
        {
            if (!OSP_Tcmd_Usage(ent, arg, i))
            {
                if (cmd->matchphase > osp.matchPhase)
                {
                    trap_SendServerCommand(ent - g_entities, "print \"You may not use that command yet.\n\"");
                }
                else
                {
                    cmd->func(ent, i, cmd->param);
                }
            }

            return;
        }
    }
}

// osp: 0x00032178
static void OSP_Tcmd_Help(gentity_t *e, int index, int param)
{
    int i;
    int numRows;
    int numCmds;
    gentity_t *ent;

    numCmds = (sizeof(ospTeamCmds) / sizeof(ospTeamCmds[0])) - 1;
    ent = e;
    numRows = numCmds / 4;
    if (numCmds % 4)
        numRows++;
    if (numRows < 0)
        return;

    trap_SendServerCommand(ent - g_entities, "print \"^5\nAvailable OSP Team Commands:\n----------------------------\n\"");

    for (i = 0; i < numRows; i++)
    {
        if (i + (3 * numRows) + 1 <= numCmds)
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%-17s%-17s%-17s%-17s\n\"",
                                      ospTeamCmds[i].name,
                                      ospTeamCmds[i + numRows].name,
                                      ospTeamCmds[i + (2 * numRows)].name,
                                      ospTeamCmds[i + (3 * numRows)].name));
        }
        else if (i + (2 * numRows) + 1 <= numCmds)
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%-17s%-17s%-17s\n\"",
                                      ospTeamCmds[i].name,
                                      ospTeamCmds[i + numRows].name,
                                      ospTeamCmds[i + (2 * numRows)].name));
        }
        else if (i + (numRows) + 1 <= numCmds)
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%-17s%-17s\n\"",
                                      ospTeamCmds[i].name,
                                      ospTeamCmds[i + numRows].name));
        }
        else
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%-17s\n\"",
                                      ospTeamCmds[i].name));
        }
    }

    trap_SendServerCommand(ent - g_entities, "print \"\nType: ^3\\command_name ?^7 for more information\n\"");
    trap_SendServerCommand(ent - g_entities, va("print \"\n^6%s\n\"", "SoF2 OSP v(1.1)"));
    trap_SendServerCommand(ent - g_entities, "print \"^5http://www.OrangeSmoothie.org/^7\n\n\"");
}

// osp: 0x000322e8
static qboolean OSP_Tcmd_Usage(gentity_t *e, char *cmd, unsigned int index)
{
    char arg[512];
    gentity_t *ent;

    ent = e;

    if (!ent)
    {
        return qfalse;
    }

    trap_Argv(2, arg, sizeof(arg));

    if (Q_stricmp(arg, "?") == 0)
    {
        trap_SendServerCommand(ent - g_entities, va("print \"\n^6%s%s\n\n\"", cmd, ospTeamCmds[index].helptext));
        return qtrue;
    }

    return qfalse;
}

// osp: 0x00032330
static void OSP_Tcmd_Info(gentity_t *e, int index, int param)
{
    gentity_t *ent = e;
    OSP_PrintTeaminfo(ent, ent->client->sess.team);
}

// osp: 0x0032346
static void OSP_Tcmd_Delegate(gentity_t *e, int index, int param)
{
    gentity_t *ent;
    int clientNum;
    char arg[16];

    ent = e;

    if (trap_Argc() < 3)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You must specify a player id#\n\"");
        return;
    }

    trap_Argv(2, arg, sizeof(arg));
    clientNum = atoi(arg);

    if ((unsigned)arg[0] - '0' > 9 || !OSP_IsClientOnline(clientNum))
    {
        trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d (player isn't connected or is invalid)\n\"", clientNum));
        return;
    }

    if (!ent->client->ospClient->captain)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You must be team captain to delegate!\"");
        return;
    }

    if (clientNum == ent - g_entities)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You cannot delegate yourself!\"");
        return;
    }

    if (ent->client->sess.team != level.clients[clientNum].sess.team)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You must delegate someone from your team!\"");
        return;
    }

    ent->client->ospClient->captain = qfalse;
    level.clients[clientNum].ospClient->captain = qtrue;
    ClientUserinfoChanged(clientNum);
    ClientUserinfoChanged(ent - g_entities);

    trap_SendServerCommand(-1, va("print \"%s ^5DELEGATES^7 %s to be team captain\n\"", ent->client->pers.netname, level.clients[clientNum].pers.netname));
}

// osp: 0x00032447
static void OSP_Tcmd_Ready(gentity_t *e, int index, int param)
{
    int i;
    int changes;
    team_t team;
    gentity_t *ent;

    ent = e;
    changes = 0;
    team = ent->client->sess.team;

    for (i = 0; i < level.maxclients; i++)
    {
        if (level.clients[i].pers.connected != CON_CONNECTED)
        {
            continue;
        }

        if (level.clients[i].sess.team != team)
        {
            continue;
        }

        if (osp.clients[i].ready)
        {
            continue;
        }

        osp.clients[i].ready = qtrue;
        ClientUserinfoChanged(i);
        changes++;
    }

    if (changes)
    {
        trap_SendServerCommand(-1, va("print \"%s readies everybody on %s\n\"", ent->client->pers.netname, OSP_GetTeamname(team)));
        trap_SetConfigstring(CS_READY, OSP_ReadyMask());
    }
    else
    {
        trap_SendServerCommand(ent - g_entities, "print \"Everyone is already ready on your team\n\"");
    }
}

// osp: 0x000324eb
static void OSP_Tcmd_Lock(gentity_t *e, int index, int lock)
{
    team_t team;
    gentity_t *ent;

    ent = e;
    team = ent->client->sess.team;

    if (ospTeams[team].locked == lock)
    {
        trap_SendServerCommand(ent - g_entities, va("print \"Your team is already %sLOCKED\n\"", lock ? "^1" : "^3UN"));
        return;
    }

    ospTeams[team].locked = lock;
    trap_SendServerCommand(-1, va("print \"%s is now %sLOCKED ^7(^3%s^7)\n\"",
                                  OSP_GetTeamname(team),
                                  lock ? "^1" : "^3UN",
                                  ent->client->pers.netname));

    if (lock)
    {
        OSP_GlobalSound("sound/misc/menus/lock.wav");
    }
    else
    {
        OSP_GlobalSound("sound/misc/menus/unlock.wav");
    }

    OSP_UpdateTeaminfo();
}

// osp: 0x00032575
static void OSP_Tcmd_Specinvite(gentity_t *e, int index, int invite)
{
    int clientNum;
    gentity_t *ent;
    team_t team;
    char arg[16];

    ent = e;
    team = ent->client->sess.team;

    if (trap_Argc() == 2 && !invite)
    {
        trap_SendServerCommand(-1, va("print \"%s ^3UNINVITES^7 everyone from spectating his/her team\n\"", ent->client->pers.netname));
        OSP_ResetSpecinvites(team, qtrue);
        return;
    }
    else if (trap_Argc() < 3)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You must specify a player id#\n\"");
        return;
    }
    else
    {
        trap_Argv(2, arg, sizeof(arg));
    }

    clientNum = atoi(arg);

    if ((unsigned)arg[0] - '0' > 9 || !OSP_IsClientOnline(clientNum))
    {
        trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d (player isn\'t connected or is invalid)\n\"", clientNum));
        return;
    }

    if ((osp.clients[clientNum].specinvites & team) == invite)
    {
        trap_SendServerCommand(ent - g_entities, va("print \"%s is %s invited\n\"", level.clients[clientNum].pers.netname, invite ? "already" : "not currently"));
        return;
    }

    OSP_InviteSpectator(team, invite, clientNum);
    trap_SendServerCommand(-1, va("print \"%s %sINVITES^7 %s to spectate his/her team\n\"", ent->client->pers.netname, invite ? "^1" : "^3UN", level.clients[clientNum].pers.netname));
}

// osp: 0x0003267e
static void OSP_MoveToSpec(team_t team, int clientNum, int ignoreClientNum)
{
    int i;
    gentity_t *ent;

    if (clientNum >= 0)
    {
        SetTeam(g_entities + clientNum, "s", NULL, qtrue);
        return;
    }

    for (i = 0; i < level.maxclients; i++)
    {
        ent = g_entities + i;

        if (ent->client->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        if (i == ignoreClientNum)
        {
            continue;
        }

        if (ent->client->sess.team == team)
        {
            SetTeam(ent, "s", NULL, qtrue);
        }
    }
}

// osp: 0x000326d9
static void OSP_MoveToTeam(team_t team, int clientNum)
{
    if (team == TEAM_RED)
    {
        SetTeam(g_entities + clientNum, "r", NULL, qtrue);
    }
    else if (team == TEAM_BLUE)
    {
        SetTeam(g_entities + clientNum, "b", NULL, qtrue);
    }
}

// osp: 0x00032706
static void OSP_Tcmd_Invite(gentity_t *e, int index, int invite)
{
    int clientNum;
    gentity_t *ent;
    team_t team;
    char arg[16];

    ent = e;
    team = ent->client->sess.team;

    if (!ospTeams[team].locked)
    {
        trap_SendServerCommand(ent - g_entities, "print \"Your team must be locked!\n\"");
        return;
    }

    if (trap_Argc() == 2 && !invite)
    {
        trap_SendServerCommand(-1, va("print \"%s ^3REMOVES^7 everyone from his team\n\"", ent->client->pers.netname));
        OSP_MoveToSpec(team, -1, ent - g_entities);
        return;
    }

    if (trap_Argc() < 3)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You must specify a player id#\n\"");
        return;
    }

    trap_Argv(2, arg, sizeof(arg));
    clientNum = atoi(arg);

    if ((unsigned)arg[0] - '0' > 9 || !OSP_IsClientOnline(clientNum))
    {
        trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d (player isn\'t connected or is invalid)\n\"", clientNum));
        return;
    }

    if ((level.clients[clientNum].sess.team == team && invite) ||
        (level.clients[clientNum].sess.team != team && !invite))
    {
        trap_SendServerCommand(ent - g_entities, va("print \"%s is %s on your team\n\"", level.clients[clientNum].pers.netname, invite ? "already" : "not currently"));
        return;
    }

    if (invite)
    {
        OSP_MoveToTeam(team, clientNum);
        trap_SendServerCommand(-1, va("print \"%s ^3INVITES^7 %s to join his/her team\n\"", ent->client->pers.netname, level.clients[clientNum].pers.netname));
    }
    else
    {
        OSP_MoveToSpec(team, clientNum, ent - g_entities);
        trap_SendServerCommand(-1, va("print \"%s ^1REMOVES^7 %s from his/her team\n\"", ent->client->pers.netname, level.clients[clientNum].pers.netname));
    }
}

// osp: 0x0003286d
static void OSP_Tcmd_Speclock(gentity_t *e, int index, int lock)
{
    team_t team;
    gentity_t *ent;

    ent = e;
    team = ent->client->sess.team;

    if (ospTeams[team].speclocked == lock)
    {
        trap_SendServerCommand(ent - g_entities, va("print \"Your team is already %sLOCKED^7 from spectators\n\"", lock ? "^1" : "^3UN"));
        return;
    }

    OSP_LockSpectators(team, lock);

    if (lock)
    {
        OSP_GlobalSound("sound/misc/menus/lock.wav");
    }
    else
    {
        OSP_GlobalSound("sound/misc/menus/unlock.wav");
    }

    trap_SendServerCommand(-1, va("print \"%s is now %sLOCKED^7 from spectating  (^3%s^7)\n\"", OSP_GetTeamname(team), lock ? "^1" : "^3UN", ent->client->pers.netname));
}

// osp: 0x000328f4
static void OSP_Tcmd_Name(gentity_t *e, int index, int param)
{
    char arg[32];
    gentity_t *ent;
    team_t team;

    ent = e;
    team = ent->client->sess.team;

    memset(arg, 0, sizeof(arg));
    trap_Argv(2, arg, sizeof(arg) - 1);

    if (osp.matchPhase > 2)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You CANNOT change your teamname once the match has started\n\"");
        return;
    }

    if (strlen(arg) < 2 || strlen(arg) > 31)
    {
        trap_SendServerCommand(ent - g_entities, "print \"Your team name is either too long or too short (min 2, max 32)\n\"");
        return;
    }

    OSP_SetTeamname(team, arg);
    trap_SendServerCommand(-1, va("print \"%s renames his/her team to %s\n\"", ent->client->pers.netname, arg));
}

// osp: 0x0003296
static void OSP_Tcmd_Timeout(gentity_t *e, int index, int timeout)
{
    gentity_t *ent;
    team_t team;
    ospPausePhase_t pause;

    ent = e;
    team = ent->client->sess.team;
    pause = team == TEAM_RED ? PP_TIMEOUT_RED : PP_TIMEOUT_BLUE;

    if (osp.paused > 1 && osp.paused != pause)
    {
        trap_SendServerCommand(ent - g_entities, va("print \"You CANNOT call a time%s at this time, you're not in control\n\"", timeout ? "out" : "in"));
        return;
    }

    if (timeout && !ospTeams[team].timeoutsLeft)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You don't have any timeouts remaining!\n\"");
        return;
    }

    if (timeout && osp.paused > 1)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You CANNOT call a timeout at this time, the match is already paused\n\"");
        return;
    }

    if (!timeout && !osp.paused)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You CANNOT call a timein at this time, the match is not currently paused\n\"");
        return;
    }

    if (osp.paused == PP_UNPAUSING)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You CANNOT call a timeout at this time, the match is currently resuming\n\"");
        return;
    }

    if (timeout)
    {
        ospTeams[team].timeoutsLeft--;
        OSP_Pause(ent, qtrue);
    }
    else
    {
        OSP_Unpause(ent, qtrue);
    }
}

// osp: 0x00032a42
static void OSP_Tcmd_Reset(gentity_t *e, int index, int param)
{
    gentity_t *ent;
    team_t team;

    ent = e;
    team = ent->client->sess.team;

    if (osp.matchPhase >= 3)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You CANNOT reset your team while the match is being played!\n\"");
        return;
    }

    OSP_ResetTeam(team, qtrue);
    OSP_ResetSpecinvites(team, qfalse);

    trap_SendServerCommand(-1, va("print \"%s resets his team's settings to defaults\n\"", ent->client->pers.netname));

    OSP_UpdateTeaminfo();
}

// osp: 0x00032a8e
void OSP_ResetTeam(team_t team, qboolean disableSpeclock)
{
    qboolean locked;

    if (disableSpeclock)
    {
        ospTeams[team].speclocked = qfalse;
    }

    if (!match_latejoin.integer && osp.matchPhase >= 3)
    {
        locked = qtrue;
    }
    else
    {
        locked = qfalse;
    }

    ospTeams[team].locked = locked;
    ospTeams[team].x0c = 0;
    ospTeams[team].timeoutsLeft = match_timeoutcount.integer;

    if (team == TEAM_RED)
    {
        trap_Cvar_Set("team_redName", "Red team");
    }
    else if (team == TEAM_BLUE)
    {
        trap_Cvar_Set("team_blueName", "Blue team");
    }

    OSP_UpdateTeaminfo();
}

// osp: 0x00032ae2
void OSP_ReadTeamSessionData(team_t team)
{
    char *p;
    char buf[256];
    int hasCustomName;
    int speclocked;
    int locked;

    trap_Cvar_VariableStringBuffer(va("teaminfo%i", team), buf, sizeof(buf));

    if (strlen(buf) < 5)
    {
        return;
    }

    p = buf;

    speclocked = atoi(p);
    if (!(p = strstr(p, " ")))
        return;

    locked = atoi(p);
    p++;
    if (!(p = strstr(p, " ")))
        return;

    hasCustomName = atoi(p);
    p++;
    if (!(p = strstr(p, " ")))
        return;

    p++;
    if (!p)
        return;

    ospTeams[team].speclocked = speclocked;
    ospTeams[team].locked = locked;
    ospTeams[team].hasCustomName = hasCustomName;

    if (!hasCustomName)
        return;

    trap_Cvar_Set(va("team_%sName", team == TEAM_RED ? "red" : "blue"), p);
}

// osp: 0x00032bb8
void OSP_WriteTeamSessionData(team_t team)
{
    trap_Cvar_Set(va("teaminfo%i", team), va("%i %i %i %s", ospTeams[team].speclocked, ospTeams[team].locked, ospTeams[team].hasCustomName, OSP_GetTeamname(team)));
}

// osp: 0x00032bf5
void OSP_InviteSpectator(team_t team, qboolean invite, int clientNum)
{
    if (invite)
        osp.clients[clientNum].specinvites |= team;
    else
    {
        osp.clients[clientNum].specinvites &= ~team;
        Cmd_FollowCycle_f(g_entities + clientNum, 1);
    }

    ClientUserinfoChanged(clientNum);
}

// osp: 0x00032c39
void OSP_LockSpectators(team_t team, qboolean lock)
{
    ospTeams[team].speclocked = lock;
    OSP_ResetSpecinvites(team, lock);
    OSP_UpdateTeaminfo();
}

// osp: 0x00032c51
void OSP_SetTeamname(team_t team, const char *name)
{
    ospTeams[team].hasCustomName = qtrue;

    if (team == TEAM_RED)
    {
        trap_SendConsoleCommand(EXEC_APPEND, va("set team_redName \"%s^7\"\n", name));
    }
    else
    {
        trap_SendConsoleCommand(EXEC_APPEND, va("set team_blueName \"%s^7\"\n", name));
    }
}

// osp: 0x00032c84
const char *OSP_GetTeamname(int team)
{
    switch (team)
    {
    case TEAM_FREE:
        return "FFA";
    case TEAM_RED:
        return team_redName.string;
    case TEAM_BLUE:
        return team_blueName.string;
    case TEAM_SPECTATOR:
        return "Spectators";
    default:
        return "unknown team";
    }
}

// osp: 0x00032cad
static int sub_00032cad(gentity_t *e)
{
    gentity_t *ent = e;

    if (ent->client->ospClient->x3c)
        return ent->client->ospClient->x3c;
    else
        return ent->client->sess.team;
}

// osp: 0x00032cd8
qboolean OSP_PlayersReady(void)
{
    gclient_t *cl;
    int i;
    int numUnready;
    int numReady;

    numReady = 0;
    numUnready = match_minplayers.integer;

    if (g_doWarmup.integer != 2)
    {
        return qtrue;
    }

    level.teamAliveCount[TEAM_RED] = 0;
    level.teamAliveCount[TEAM_BLUE] = 0;

    numUnready = 0;

    for (i = 0; i < level.numConnectedClients; i++)
    {
        cl = &level.clients[level.sortedClients[i]];

        if (cl->pers.connected != CON_CONNECTED || cl->sess.team == TEAM_SPECTATOR)
        {
            continue;
        }

        if (cl->ospClient->ready || (g_entities[i].r.svFlags & SVF_BOT))
        {
            numReady++;
            level.teamAliveCount[cl->sess.team]++;
        }
        else
        {
            numUnready++;
        }
    }

    numUnready = (numUnready > 0 || numReady > 0) ? numUnready : match_minplayers.integer;

    return (osp.forceReady || ((numReady + numUnready) > 0 && (numReady * 100) / (numReady + numUnready) >= match_readypercent.integer));
}

// osp: 0x00032da0
void OSP_CheckEmptyTeams(void)
{
    static qboolean restart = 0;
    static int checkTime = 0;

    if (checkTime > level.time)
    {
        return;
    }

    checkTime = level.time + 5000;

    if (!restart && (TeamCount(-1, TEAM_RED, NULL) == 0 || TeamCount(-1, TEAM_BLUE, NULL) == 0))
    {
        trap_SendServerCommand(-1, "print \"One team is empty, restarting in 5 seconds...\n\"");
        restart = qtrue;
        return;
    }

    if (restart && TeamCount(-1, TEAM_RED, NULL) != 0 && TeamCount(-1, TEAM_BLUE, NULL) != 0)
    {
        restart = qfalse;
        return;
    }

    if (restart)
    {
        trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
    }
}

// osp: 0x00032e0c
qboolean OSP_TeamchangeAllowed(team_t team, team_t oldTeam, gentity_t *e)
{
    gentity_t *ent;
    int count;

    ent = e;

    count = TeamCount(-1, team, NULL);

    if (team != TEAM_RED && team != TEAM_BLUE)
    {
        if (ent->client->ospClient->captain && (oldTeam == TEAM_RED || oldTeam == TEAM_BLUE))
        {
            gentity_t *other;
            int i;

            ent->client->ospClient->captain = qfalse;

            for (i = 0; i < level.maxclients; i++)
            {
                other = g_entities + i;

                if (other == ent)
                {
                    continue;
                }

                if (other->client->pers.connected != CON_CONNECTED)
                {
                    continue;
                }

                if (other->client->sess.team == oldTeam)
                {
                    other->client->ospClient->captain = qtrue;
                    ClientUserinfoChanged(i);
                    trap_SendServerCommand(-1, va("print \"%s is now captain of the %s team.\n\"",
                                                  other->client->pers.netname,
                                                  other->client->sess.team == TEAM_RED ? "^1RED^7" : "^4BLUE"));
                    break;
                }
            }
        }

        return qtrue;
    }

    if (count == 0)
    {
        OSP_ResetTeam(team, qtrue);
        trap_SendServerCommand(-1, va("print \"%s is now captain of the %s team.\n\"",
                                      ent->client->pers.netname,
                                      team == TEAM_RED ? "^1RED^7" : "^4BLUE"));
        ent->client->ospClient->captain = qtrue;
    }
    else
    {
        ent->client->ospClient->captain = qfalse;
    }

    if (level.gametypeData->teams && (team == TEAM_RED || team == TEAM_BLUE))
    {
        if (team_maxplayers.integer > 0 && team_maxplayers.integer <= count)
        {
            trap_SendServerCommand(ent - g_entities, va("cp \"%s is FULL!\n\"", OSP_GetTeamname(team)));
            return qfalse;
        }

        if (ospTeams[team].locked && !(ent->client->ospClient->x14 & team))
        {
            trap_SendServerCommand(ent - g_entities, va("cp \"%s is LOCKED!\n\"", OSP_GetTeamname(team)));
            return qfalse;
        }
    }

    return qtrue;
}

// osp: 0x00032f9a
void OSP_ResetSpecinvites(team_t team, qboolean locked)
{
    gentity_t *ent;
    int i;

    ospTeams[team].speclocked = locked;

    for (i = 0; i < level.numConnectedClients; i++)
    {
        ent = &g_entities[level.sortedClients[i]];

        if (OSP_IsClientAdmin(level.sortedClients[i], qtrue))
        {
            continue;
        }

        if (ent->client->ospClient->specinvites & team)
        {
            ent->client->ospClient->specinvites &= ~team;
            ClientUserinfoChanged(i);
        }
        else
        {
            ent->client->ospClient->specinvites &= ~team;
        }

        if (ent->client->sess.team != TEAM_SPECTATOR)
        {
            continue;
        }

        if (ent->client->sess.spectatorState == SPECTATOR_FOLLOW)
        {
            if (!locked)
            {
                continue;
            }

            if (OSP_CanFollow(ent, level.clients[ent->client->sess.spectatorClient].sess.team))
            {
                continue;
            }

            G_StopFollowing(ent);
        }
    }
}

// osp: 0x0003305b
void OSP_SwapTeamSettings(void)
{
    int hasName;
    int temp;
    char name[64];

    hasName = 0;

    temp = ospTeams[TEAM_RED].speclocked;
    ospTeams[TEAM_RED].speclocked = ospTeams[TEAM_BLUE].speclocked;
    ospTeams[TEAM_BLUE].speclocked = temp;

    temp = ospTeams[TEAM_RED].locked;
    ospTeams[TEAM_RED].locked = ospTeams[TEAM_BLUE].locked;
    ospTeams[TEAM_BLUE].locked = temp;

    hasName = ospTeams[TEAM_BLUE].hasCustomName;
    Q_strncpyz(name, team_blueName.string, 64);
    ospTeams[TEAM_BLUE].hasCustomName = ospTeams[TEAM_RED].hasCustomName;

    if (ospTeams[TEAM_RED].hasCustomName)
    {
        trap_SetConfigstring(CS_BLUE_TEAM_NAME, team_redName.string);
        trap_SendConsoleCommand(EXEC_APPEND, va("set team_blueName \"%s\"\n", team_redName.string));
    }
    else
    {
        trap_SetConfigstring(CS_BLUE_TEAM_NAME, "Blue team");
        trap_SendConsoleCommand(EXEC_APPEND, "set team_blueName \"Blue team\"\n");
    }

    ospTeams[TEAM_RED].hasCustomName = hasName;
    if (hasName)
    {
        trap_SetConfigstring(CS_RED_TEAM_NAME, team_blueName.string);
        trap_SendConsoleCommand(EXEC_APPEND, va("set team_redName \"%s\"\n", team_blueName.string));
    }
    else
    {
        trap_SetConfigstring(CS_RED_TEAM_NAME, "Red team");
        trap_SendConsoleCommand(EXEC_APPEND, "set team_redName \"Red team\"\n");
    }

    OSP_UpdateTeaminfo();
}

// osp: 0x000330e7
void OSP_PrintTeaminfo(gentity_t *e, team_t team)
{
    int i;
    char buf[1024];
    gentity_t *other;
    qboolean specinvited[MAX_CLIENTS];
    gentity_t *ent;

    ent = e;
    memset(specinvited, 0, sizeof(specinvited));

    trap_SendServerCommand(ent - g_entities, va("print \"\n^6Team information for ^7%s\n\"", OSP_GetTeamname(team)));

    for (i = 0; i < level.maxclients; i++)
    {
        other = g_entities + i;

        if (level.clients[i].pers.connected != CON_CONNECTED)
        {
            continue;
        }

        if (level.clients[i].sess.team == TEAM_SPECTATOR)
        {
            if ((osp.clients[i].specinvites & team) || OSP_IsClientAdmin(i, qtrue))
            {
                specinvited[i] = qtrue;
            }

            continue;
        }

        if (level.clients[i].sess.team != team)
        {
            continue;
        }

        if (osp.matchPhase < 3)
        {
            trap_SendServerCommand(
                ent - g_entities,
                va("print \"%s%-20s^7 - %-9s - %3dms\n\"",
                   team == TEAM_RED ? "^1" : "^4",
                   level.clients[i].pers.netname,
                   osp.clients[i].ready ? "^3READY^7" : "^1NOT READY^7",
                   level.clients[i].ps.ping));
        }
    }

    memset(buf, 0, sizeof(buf));

    if (ospTeams[team].speclocked)
    {
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            if (!specinvited[i])
            {
                continue;
            }

            if (OSP_IsClientAdmin(i, qtrue))
                Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "^5%s^7, ", level.clients[i].pers.netname);
            else
                Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "%s, ", level.clients[i].pers.netname);
        }

        if (!buf[0])
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "no one");
        }
    }
    else
        Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "everybody");

    trap_SendServerCommand(ent - g_entities,
                           va("print \"Remaining timeouts: %d\nTeam joining is %sLOCKED^7\nSpectating is %sALLOWED^7\nInvited spectators: %s\n\"",
                              ospTeams[team].timeoutsLeft,
                              ospTeams[team].locked ? "^1" : "^3UN",
                              ospTeams[team].speclocked ? "^1DIS" : "^3",
                              buf));
}
