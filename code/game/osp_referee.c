// osp_referee.c
//
#include "g_local.h"

char *ConcatArgs(int start);

static void OSP_Ref_Help(gentity_t *ent, unsigned int index, int param);
static qboolean OSP_Ref_Usage(gentity_t *ent, char *cmd, unsigned int index);
static void OSP_Ref_Talk(gentity_t *ent, unsigned int index, int param);
static void OSP_Ref_Chat(gentity_t *ent, unsigned int index, int param);
//static void OSP_Ref_Ready(gentity_t *ent, int index, int param);
static void OSP_Ref_Reset(gentity_t *ent, unsigned int index, int param);
static void OSP_Ref_Cointoss(gentity_t *ent, unsigned int index, int param);
static void OSP_Ref_Name(gentity_t *ent, unsigned int index, int param);
static void OSP_Ref_Lock(gentity_t *ent, unsigned int index, int param);
static void OSP_Ref_Speclock(gentity_t *ent, unsigned int index, int param);
static void OSP_Ref_Specinvite(gentity_t *ent, unsigned int index, int param);
static void OSP_Ref_Delegate(gentity_t *ent, unsigned int index, int param);

typedef struct
{
    char *name;
    int rfu1;
    int param;
    void (*func)(gentity_t *, unsigned int, int);
    char *helptext;
} ospRefCmd_t;

static const ospRefCmd_t ospRefCmds[] = {
    {"?", 1, 1, OSP_Ref_Help, ":^7 Gives a list of admin commands"},
    {"commands", 1, 1, OSP_Ref_Help, ":^7 Gives a list of admin commands"},
    {"talk", 1, 0, OSP_Ref_Talk, " <msg>:^7 Chat with everyone on the server, anytime"},
    {"chat", 1, 0, OSP_Ref_Chat, " <msg>:^7 Chat with other referees on the server, anytime"},
    {"ready", 1, 0, OSP_Ref_Ready, ":^7 Readies everyone to start match"},
    {"reset", 1, 1, OSP_Ref_Reset, " <r|b>:^7 Reset specified team, both if not specified"},
    {"name", 1, 1, OSP_Ref_Name, " <r|b> <name>:^7 Rename the specified team to <name>"},
    {"lock", 1, 1, OSP_Ref_Lock, " <r|b>:^7 Lock specified team, both if not specified"},
    {"unlock", 1, 0, OSP_Ref_Lock, " <r|b>:^7 Unlock specified team, both if not specified"},
    {"speclock", 1, 1, OSP_Ref_Speclock, " <r|b>:^7 Spectator lock specified team, both if not specified"},
    {"specunlock", 1, 0, OSP_Ref_Speclock, " <r|b>:^7 Spectator unlock specified team, both if not specified"},
    {"specinvite", 1, 1, OSP_Ref_Specinvite, " <r|b> #:^7 Invite player with id# to spectate specified team"},
    {"specuninvite", 1, 0, OSP_Ref_Specinvite, " <r|b> #:^7 Uninvite player with id# to spectate specified team"},
    {"delegate", 1, 0, OSP_Ref_Delegate, " <r|b> #:^7 Make player with id# the team's captain"},
    {"cointoss", 1, 0, OSP_Ref_Cointoss, ":^7 Throw a coin and show the result to everybody"},
    {NULL, 0, 1, 0, NULL},
};

// osp: 0x00030793
void OSP_Cmd_Ref(gentity_t *e, int index, int admin)
{
    unsigned int i;
    const ospRefCmd_t *cmd;
    gentity_t *ent;
    char arg[128];
    unsigned int numCmds;

    numCmds = sizeof(ospRefCmds) / sizeof(ospRefCmds[0]);
    ent = e;

    memset(arg, 0, sizeof(arg));
    trap_Argv(1, arg, sizeof(arg) - 1);

    for (i = 0; i < numCmds; i++)
    {
        cmd = &ospRefCmds[i];

        if (trap_Argc() == 1)
        {
            OSP_Ref_Help(ent, i, cmd->param);
            return;
        }

        if (trap_Argc() == 2 && !admin)
        {
            if (strcmp(sv_refereePassword.string, arg) == 0 && strcmp(sv_refereePassword.string, "none") != 0 && strlen(sv_refereePassword.string) > 0)
            {
                osp.clients[ent - g_entities].referee = qtrue;
                trap_SendServerCommand(ent - g_entities, "print \"Type: ^3ref ?^7 for a list of referee commands.\n\"");
                trap_SendServerCommand(-1, va("cp \"%s^7 is now a referee\n\"", level.clients[ent - g_entities].pers.netname));
                return;
            }
        }

        if (cmd->func && Q_stricmp(arg, cmd->name) == 0)
        {
            if (!OSP_Ref_Usage(ent, arg, i))
            {
                if (!OSP_IsClientReferee(ent - g_entities))
                {
                    trap_SendServerCommand(ent - g_entities, "print \"^3Only referees may use that command.\n\"");
                }
                else
                {
                    cmd->func(ent, i, cmd->param);
                }
            }

            return;
        }
    }

    if (!OSP_IsClientReferee(ent - g_entities))
    {
        return;
    }

    if (admin)
    {
        OSP_Callvote(ent, qfalse, 2);
    }
    else
    {
        OSP_Callvote(ent, qfalse, 1);
    }
}

// osp: 0x000308c9
static void OSP_Ref_Help(gentity_t *e, unsigned int index, int param)
{
    int i;
    int numRows;
    int numCmds;
    gentity_t *ent;

    numCmds = (sizeof(ospRefCmds) / sizeof(ospRefCmds[0])) - 1;
    ent = e;
    numRows = numCmds / 4;
    if (numCmds % 4)
        numRows++;
    if (numRows < 0)
        return;

    trap_SendServerCommand(ent - g_entities, "print \"^5\nAvailable OSP Referee Commands:\n----------------------------\n\"");

    for (i = 0; i < numRows; i++)
    {
        if (i + (3 * numRows) + 1 <= numCmds)
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%-17s%-17s%-17s%-17s\n\"",
                                      ospRefCmds[i].name,
                                      ospRefCmds[i + numRows].name,
                                      ospRefCmds[i + (2 * numRows)].name,
                                      ospRefCmds[i + (3 * numRows)].name));
        }
        else if (i + (2 * numRows) + 1 <= numCmds)
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%-17s%-17s%-17s\n\"",
                                      ospRefCmds[i].name,
                                      ospRefCmds[i + numRows].name,
                                      ospRefCmds[i + (2 * numRows)].name));
        }
        else if (i + (numRows) + 1 <= numCmds)
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%-17s%-17s\n\"",
                                      ospRefCmds[i].name,
                                      ospRefCmds[i + numRows].name));
        }
        else
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%-17s\n\"",
                                      ospRefCmds[i].name));
        }
    }

    trap_SendServerCommand(ent - g_entities, "print \"\nType: ^3\\command_name ?^7 for more information\n\"");
    trap_SendServerCommand(ent - g_entities, va("print \"\n^6%s\n\"", "SoF2 OSP v(1.1)"));
    trap_SendServerCommand(ent - g_entities, "print \"^5http://www.OrangeSmoothie.org/^7\n\n\"");
}

// osp: 0x00030a39
static qboolean OSP_Ref_Usage(gentity_t *e, char *cmd, unsigned int idx)
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
        trap_SendServerCommand(ent - g_entities, va("print \"\n^3%s%s\n\n\"", cmd, ospRefCmds[idx].helptext));
        return qtrue;
    }

    return qfalse;
}

// osp: 0x00030a81
static void OSP_Ref_Talk(gentity_t *e, unsigned int index, int param)
{
    gentity_t *ent;
    char *p;

    ent = e;

    if (trap_Argc() <= 2)
    {
        trap_SendServerCommand(ent - g_entities, "print \"usage: /ref talk <message>\n\"");
        return;
    }

    p = ConcatArgs(2);
    trap_SendServerCommand(-1, va("chat %d \"^3*referee* ^7%s ^7: %s\"", ent->s.number, ent->client->pers.netname, p));
}

// osp: 0x00030ac5
static void OSP_Ref_Chat(gentity_t *e, unsigned int index, int param)
{
    int i;
    gentity_t *ent;
    char *p;

    ent = e;

    if (trap_Argc() <= 2)
    {
        trap_SendServerCommand(ent - g_entities, "print \"usage: /ref chat <message>\n\"");
        return;
    }

    p = ConcatArgs(2);

    for (i = 0; i < level.maxclients; i++)
    {
        if (!OSP_IsClientOnline(i))
        {
            continue;
        }

        if (!OSP_IsClientReferee(i))
        {
            continue;
        }

        trap_SendServerCommand(i, va("chat %d \"^6*referees only* ^5%s ^7: %s\"", ent->s.number, ent->client->pers.netname, p));
    }

    G_LogPrintf("referee chat: %s: %s\n", ent->client->pers.netname, p);
}

// osp: 0x00030b44
void OSP_Ref_Ready(gentity_t *e, unsigned int index, int param)
{
    int i;
    gclient_t *cl;
    gentity_t *ent;

    ent = e;

    if (osp.matchPhase != 1)
    {
        char *msg = "Match isn't in setup phase!\n";

        if (ent)
        {
            trap_SendServerCommand(ent - g_entities, va("print \"%s\"", msg));
        }
        else
        {
            Com_Printf("%s", msg);
        }

        return;
    }

    osp.forceReady = qtrue;
    trap_SendServerCommand(-1, "cp \"^3** Referee Action **\nForce everybody to ready\n\"");
    OSP_GlobalSound("sound/misc/events/tut_lift02.mp3");

    for (i = 0; i < level.numConnectedClients; i++)
    {
        cl = &level.clients[level.sortedClients[i]];

        if (cl->sess.team != TEAM_SPECTATOR)
        {
            cl->ospClient->ready = qtrue;
        }
    }
}

// osp: 0x00030bb5
static void OSP_Ref_Reset(gentity_t *e, unsigned int index, int param)
{
    team_t team;
    gentity_t *ent;
    char arg[32];

    ent = e;

    if (osp.matchPhase > 2)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You can not reset the team once the match has started\n\"");
        return;
    }

    if (trap_Argc() < 3)
    {
        team = 0;
    }
    else
    {
        trap_Argv(2, arg, sizeof(arg));

        if (arg[0] == 'r' || arg[0] == 'R')
        {
            team = TEAM_RED;
        }
        else if (arg[0] == 'b' || arg[0] == 'B')
        {
            team = TEAM_BLUE;
        }
        else
        {
            trap_SendServerCommand(ent - g_entities, "print \"Invalid team.\n\"");
            return;
        }
    }

    if (team)
    {
        OSP_ResetTeam(team, qtrue);
        OSP_ResetSpecinvites(team, qfalse);
        trap_SendServerCommand(-1, va("print \"%s resets %s to defaults\n\"", ent->client->pers.netname, OSP_GetTeamname(team)));
    }
    else
    {
        OSP_ResetTeam(TEAM_RED, qtrue);
        OSP_ResetSpecinvites(TEAM_RED, qfalse);
        OSP_ResetTeam(TEAM_BLUE, qtrue);
        OSP_ResetSpecinvites(TEAM_BLUE, qfalse);
        trap_SendServerCommand(-1, va("print \"%s resets both teams to defaults\n\"", ent->client->pers.netname));
    }
}

// osp: 0x00030c83
static void OSP_Ref_Cointoss(gentity_t *e, unsigned int index, int param)
{
    gentity_t *ent = e;
    trap_SendServerCommand(-1, va("cp \"%s^7 throws a coin,\nresult is: ^3%s^7!\n\"",
                                  ent->client->pers.netname,
                                  rand() & 0x1 ? "TAILS" : "HEADS"));
}

// osp: 0x00030cb5
static void OSP_Ref_Name(gentity_t *e, unsigned int index, int param)
{
    char arg[32];
    gentity_t *ent;
    team_t team;

    ent = e;

    if (trap_Argc() < 4)
    {
        trap_SendServerCommand(ent - g_entities, "print \"Not enough arguments\"");
        return;
    }

    trap_Argv(2, arg, sizeof(arg));

    if (arg[0] == 'r' || arg[0] == 'R')
    {
        team = TEAM_RED;
    }
    else if (arg[0] == 'b' || arg[0] == 'B')
    {
        team = TEAM_BLUE;
    }
    else
    {
        trap_SendServerCommand(ent - g_entities, "print \"Invalid team.\n\"");
        return;
    }

    memset(arg, 0, sizeof(arg));
    trap_Argv(3, arg, sizeof(arg) - 1);

    if (strlen(arg) < 2 || strlen(arg) > 31)
    {
        trap_SendServerCommand(ent - g_entities, "print \"Your team name is either too long or too short (min 2, max 32)\n\"");
        return;
    }

    trap_SendServerCommand(-1, va("print \"%s renames %s team to %s\n\"",
                                  ent->client->pers.netname,
                                  OSP_GetTeamname(team),
                                  arg));
    OSP_SetTeamname(team, arg);
}

// osp: 0x00030d6b
static void OSP_Ref_Lock(gentity_t *e, unsigned int index, int lock)
{
    gentity_t *ent;
    int team;

    ent = e;

    {
        char arg[32];

        if (trap_Argc() < 3)
        {
            team = 0;
        }
        else
        {
            trap_Argv(2, arg, sizeof(arg));

            if (arg[0] == 'r' || arg[0] == 'R')
            {
                team = TEAM_RED;
            }
            else if (arg[0] == 'b' || arg[0] == 'B')
            {
                team = TEAM_BLUE;
            }
            else
            {
                trap_SendServerCommand(ent - g_entities, "print \"Invalid team.\n\"");
                return;
            }
        }

        if (team)
        {
            if (ospTeams[team].locked == lock)
            {
                trap_SendServerCommand(ent - g_entities, va("print \"That team is already %sLOCKED\n\"", lock ? "^1" : "^3UN"));
                return;
            }
            else
            {
                ospTeams[team].locked = lock;
                trap_SendServerCommand(-1, va("print \"%s is now %sLOCKED ^7(^3%s^7)\n\"", OSP_GetTeamname(team), lock ? "^1" : "^3UN", ent->client->pers.netname));
            }
        }
        else
        {
            ospTeams[TEAM_RED].locked = lock;
            ospTeams[TEAM_BLUE].locked = lock;
            trap_SendServerCommand(-1, va("print \"Both teams are now %sLOCKED ^7(^3%s^7)\n\"", lock ? "^1" : "^3UN", ent->client->pers.netname));
        }

        OSP_UpdateTeaminfo();

        if (lock)
        {
            OSP_GlobalSound("sound/misc/menus/lock.wav");
        }
        else
        {
            OSP_GlobalSound("sound/misc/menus/unlock.wav");
        }
    }
}

// osp: 0x00030e66
static void OSP_Ref_Speclock(gentity_t *e, unsigned int index, int lock)
{
    team_t team;
    gentity_t *ent;
    char arg[32];

    ent = e;
    team = ent->client->sess.team;

    if (trap_Argc() < 3)
    {
        team = 0;
    }
    else
    {
        trap_Argv(2, arg, sizeof(arg));

        if (arg[0] == 'r' || arg[0] == 'R')
        {
            team = TEAM_RED;
        }
        else if (arg[0] == 'b' || arg[0] == 'B')
        {
            team = TEAM_BLUE;
        }
        else
        {
            trap_SendServerCommand(ent - g_entities, "print \"Invalid team.\n\"");
            return;
        }
    }

    if (team)
    {
        if (ospTeams[team].speclocked == lock)
        {
            trap_SendServerCommand(ent - g_entities, va("print \"That team is already %sLOCKED^7 from spectators\n\"", lock ? "^1" : "^3UN"));
            return;
        }

        OSP_LockSpectators(team, lock);
        trap_SendServerCommand(-1, va("print \"%s is now %sLOCKED^7 from spectating (^3%s^7)\n\"", OSP_GetTeamname(team), lock ? "^1" : "^3UN", ent->client->pers.netname));
    }
    else
    {
        OSP_LockSpectators(TEAM_RED, lock);
        OSP_LockSpectators(TEAM_BLUE, lock);
        trap_SendServerCommand(-1, va("print \"Both teams are now %sLOCKED^7 from spectating (^3%s^7)\n\"", lock ? "^1" : "^3UN", ent->client->pers.netname));
    }

    if (lock)
    {
        OSP_GlobalSound("sound/misc/menus/lock.wav");
    }
    else
    {
        OSP_GlobalSound("sound/misc/menus/unlock.wav");
    }
}

// osp: 0x00030f70
static void OSP_Ref_Specinvite(gentity_t *e, unsigned int index, int invite)
{
    char arg[16];
    int clientNum;
    gentity_t *ent;
    team_t team;

    ent = e;

    if (trap_Argc() < 4)
    {
        trap_SendServerCommand(ent - g_entities, "print \"Not enough arguments.\"");
        return;
    }

    trap_Argv(2, arg, sizeof(arg));

    if (arg[0] == 'r' || arg[0] == 'R')
    {
        team = TEAM_RED;
    }
    else if (arg[0] == 'b' || arg[0] == 'B')
    {
        team = TEAM_BLUE;
    }
    else
    {
        trap_SendServerCommand(ent - g_entities, "print \"Invalid team.\n\"");
        return;
    }

    trap_Argv(3, arg, sizeof(arg));
    clientNum = atoi(arg);

    if (((unsigned)arg[0] - '0' > 9) || !OSP_IsClientOnline(clientNum))
    {
        trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d (player isnt connected or is invalid)\n\"", clientNum));
        return;
    }

    if ((osp.clients[clientNum].specinvites & team) == invite)
    {
        trap_SendServerCommand(ent - g_entities,
                               va("print \"%s is %s invited to spectate that team\n\"",
                                  level.clients[clientNum].pers.netname,
                                  invite ? "already" : "not currently"));
        return;
    }

    OSP_InviteSpectator(team, invite, clientNum);
    trap_SendServerCommand(-1, va("print \"%s %sINVITES^7 %s to spectate %s\n\"",
                                  ent->client->pers.netname,
                                  invite ? "^1" : "^3UN",
                                  level.clients[clientNum].pers.netname,
                                  OSP_GetTeamname(team)));
}

// osp: 0x00031088
static void OSP_Ref_Delegate(gentity_t *e, unsigned int index, int param)
{
    int clientNum;
    {
        int team;
        {
            char arg[16];
            {
                gentity_t *ent;

                ent = e;

                if (trap_Argc() < 4)
                {
                    trap_SendServerCommand(ent - g_entities, "print \"Not enough arguments\n\"");
                    return;
                }

                trap_Argv(2, arg, sizeof(arg));

                if (arg[0] == 'r' || arg[0] == 'R')
                {
                    team = TEAM_RED;
                }
                else if (arg[0] == 'b' || arg[0] == 'B')
                {
                    team = TEAM_BLUE;
                }
                else
                {
                    trap_SendServerCommand(ent - g_entities, "print \"Invalid team.\n\"");
                    return;
                }

                trap_Argv(3, arg, sizeof(arg));
                clientNum = atoi(arg);

                if (((unsigned)arg[0] - '0' > 9) || !OSP_IsClientOnline(clientNum))
                {
                    trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d (player isnt connected or is invalid)\n\"", clientNum));
                    return;
                }

                if (team != level.clients[clientNum].sess.team)
                {
                    trap_SendServerCommand(ent - g_entities, "print \"You must delegate someone from the team!\"");
                    return;
                }

                if (team != level.clients[clientNum].sess.team)
                {
                    trap_SendServerCommand(ent - g_entities, "print \"That person is already captain of the team!\"");
                    return;
                }
                else
                {
                    int i;

                    for (i = 0; i < level.maxclients; i++)
                    {
                        gentity_t *other;
                        other = g_entities + i;

                        if (i == clientNum)
                        {
                            continue;
                        }

                        if (other->client->pers.connected != CON_CONNECTED)
                        {
                            continue;
                        }

                        if (other->client->sess.team == team && other->client->ospClient->captain)
                        {
                            other->client->ospClient->captain = qfalse;
                            ClientUserinfoChanged(i);
                        }
                    }
                }

                level.clients[clientNum].ospClient->captain = qtrue;
                ClientUserinfoChanged(clientNum);
                trap_SendServerCommand(-1, va("print \"%s ^5NAMES^7 %s captain of %s\n\"",
                                              ent->client->pers.netname,
                                              level.clients[clientNum].pers.netname,
                                              OSP_GetTeamname(team)));
            }
        }
    }
}
