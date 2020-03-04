// osp_cmds.c
//
#include "g_local.h"

char *ConcatArgs(int start);
void G_Say(gentity_t *ent, gentity_t *target, int mode, const char *chatText);
void Cmd_Follow_f(gentity_t *ent);

static qboolean OSP_Cmd_Usage(gentity_t *ent, const char *cmd, unsigned int idx);
static void OSP_Cmd_Players(gentity_t *ent, int index, int param);
static void OSP_Cmd_Statsall(gentity_t *ent, int index, int param);
static void OSP_Cmd_Stats(gentity_t *e, int index, int param);
static void OSP_Cmd_CvarReport(gentity_t *ent);
static void OSP_Cmd_SayTeamnl(gentity_t *ent, int index, int param);
static void OSP_Cmd_Ready(gentity_t *ent, int index, int param);
static void OSP_Cmd_Follow(gentity_t *ent, int index, int param);
static void OSP_Cmd_Callvote(gentity_t *ent, int index, int param);
static void OSP_Cmd_Help(gentity_t *ent, int index, int param);
static void OSP_Cmd_Motd(gentity_t *ent, int index, int param);
static void OSP_Cmd_Matchinfo(gentity_t *ent, int index, int param);
static void OSP_Cmd_Settings(gentity_t *ent, int index, int param);
static void OSP_Cmd_OspScore(gentity_t *ent, int index, int param);
static void OSP_Cmd_OspStats(gentity_t *ent, int index, int param);
static void OSP_Cmd_Showex(gentity_t *ent, int index, int param);
static void OSP_Cmd_Sub(gentity_t *ent, int index, int param);

typedef struct
{
    char *name;
    int level;
    qboolean noIntermission;
    int arg;
    void (*func)(gentity_t *ent, int arg1, int arg2);
    char *helptext;
} ospCmd_t;

static ospCmd_t ospCmds[] = {
    {"?", 0, qtrue, 1, OSP_Cmd_Help, ":^7 Gives a list of OSP-specific commands"},
    {"callvote", 0, qtrue, 0, OSP_Cmd_Callvote, " <params>:^7 Calls a vote"},
    {"commands", 0, qtrue, 1, OSP_Cmd_Help, ":^7 Gives a list of OSP-specific commands"},
    {"follow", 0, qfalse, 1, OSP_Cmd_Follow, " <player_ID|blue|red>:^7 Spectates a particular player or team"},
    {"motd", 0, qfalse, 0, OSP_Cmd_Motd, ":^7 Show server's message of the day"},
    {"notready", 0, qtrue, 0, OSP_Cmd_Ready, ":^7 Sets your status to ^5not ready^7 to start a match"},
    {"players", 0, qtrue, 1, OSP_Cmd_Players, ":^7 Lists all active players and their IDs/information"},
    {"ready", 0, qtrue, 1, OSP_Cmd_Ready, ":^7 Sets your status to ^5ready^7 to start a match"},
    {"say_teamnl", 0, qtrue, 1, OSP_Cmd_SayTeamnl, " <msg>:^7 Sends a team chat without location info"},
    {"stats", 0, qtrue, 0, OSP_Cmd_Stats, " <player_ID>:^7 Shows weapon accuracy stats for a player"},
    {"statsall", 0, qtrue, 0, OSP_Cmd_Statsall, ":^7 Shows weapon accuracy stats for all players"},
    {"tokens", 0, qfalse, 0, OSP_Cmd_Tokens, ":^7 Description of all chat tokens"},
    {"matchinfo", 0, qfalse, 0, OSP_Cmd_Matchinfo, " <r|b>:^7 Give match info about specified team or both if unspecified"},
    {"settings", 0, qfalse, 0, OSP_Cmd_Settings, ":^7 List gameplay settings to players"},
    {"ref", 1, qtrue, 0, OSP_Cmd_Ref, " <player_ID> <reason>:^7 Kick player with corresponding id giving specified reason"},
    {"adm", 2, qtrue, 0, OSP_Cmd_Adm, ":^7 Access the admin commands"},
    {"tcmd", 0, qtrue, 0, OSP_Cmd_Tcmd, ":^7 Access the team commands"},
    {"osp_score", 0, qtrue, 0, OSP_Cmd_OspScore, ":^7 Get osp scores"},
    {"osp_stats", 0, qtrue, 0, OSP_Cmd_OspStats, ":^7 Get osp stats"},
    {"showex", 0, qtrue, 0, OSP_Cmd_Showex, ":^7 Show the extraction zone on alt inf maps"},
    {"sub", 0, qtrue, 0, OSP_Cmd_Sub, ":^7 Will prevent you from spawning (subs)"},
    {NULL, 0, qfalse, 1, 0, NULL},
};

// osp: 0x0002b80f
qboolean OSP_ClientCommand(gentity_t *e, const char *cmd, qboolean noIntermission)
{
    ospCmd_t *command;
    unsigned int i;
    gentity_t *ent;
    size_t numCmds;

    numCmds = sizeof(ospCmds) / sizeof(ospCmds[0]);
    ent = e;

    if (ent->client->ospClient->motdTime)
    {
        OSP_StopMotd(ent);
    }

    ent->client->ospClient->afkTime = level.time;

    if (Q_stricmp("cvarreport", cmd) == 0)
    {
        OSP_Cmd_CvarReport(ent);
        return qtrue;
    }

    for (i = 0; i < numCmds; i++)
    {
        command = &ospCmds[i];

        if (command->func && command->noIntermission == noIntermission && Q_stricmp(cmd, command->name) == 0)
        {
            if (!OSP_Cmd_Usage(ent, cmd, i))
            {
                command->func(ent, i, command->arg);
            }

            return qtrue;
        }
    }

    return qfalse;
}

// osp: 0x0002b8b0
static qboolean OSP_Cmd_Usage(gentity_t *e, const char *cmd, unsigned int idx)
{
    char arg[512];
    gentity_t *ent;

    ent = e;

    if (!ent)
    {
        return qfalse;
    }

    trap_Argv(1, arg, sizeof(arg));

    if (Q_stricmp(arg, "?") == 0 && Q_stricmp(cmd, "adm") == 0 && Q_stricmp(cmd, "ref") == 0)
    {
        if (ospCmds[idx].level == 2)
            trap_SendServerCommand(ent - g_entities, va("print \"\n^5%s%s\n\n\"", cmd, ospCmds[idx].helptext));
        else if (ospCmds[idx].level == 1)
            trap_SendServerCommand(ent - g_entities, va("print \"\n^3%s%s\n\n\"", cmd, ospCmds[idx].helptext));
        else
            trap_SendServerCommand(ent - g_entities, va("print \"\n^2%s%s\n\n\"", cmd, ospCmds[idx].helptext));

        return qtrue;
    }

    return qfalse;
}

// osp: 0x0002b964
static qboolean OSP_Cmd_Throttle(gentity_t *e, const char *cmd)
{
    gentity_t *ent = e;

    if (ent->client->ospClient->cmdThrottle - level.time > 0)
    {
        trap_SendServerCommand(ent - g_entities, va("print \"Wait another %.1fs to issue ^3%s\n\"", ((float)(ent->client->ospClient->cmdThrottle - level.time) * 1.0) / 1000.0, cmd));
        return qfalse;
    }

    ent->client->ospClient->cmdThrottle = level.time + 3000;
    return qtrue;
}

// osp: 0x002b9ba
static void OSP_Cmd_Players(gentity_t *e, int idx, int arg)
{
    gentity_t *other;
    int i;
    gentity_t *ent;
    int maxrate;
    char buf[4096];

    ent = e;

    maxrate = trap_Cvar_VariableIntegerValue("sv_maxrate");
    memset(buf, 0, sizeof(buf));
    trap_SendServerCommand(ent - g_entities, va("print \"^2[  # - Name                 - Rate  - Snaps ]^7\n\""));

    for (i = 0; i < level.maxclients; i++)
    {
        other = g_entities + i;

        if (other->client && other->client->pers.connected)
        {
            char *c;
            char name[64];
            char cleanname[20];

            strncpy(name, other->client->pers.netname, sizeof(name));
            name[sizeof(name) - 1] = '\0';
            Q_CleanStr(name);
            while (c = strstr(name, "^"))
            {
                *c = ' ';
            }
            strncpy(cleanname, name, sizeof(cleanname));
            cleanname[sizeof(cleanname) - 1] = '\0';

            if (other == ent)
                trap_SendServerCommand(ent - g_entities, va("print \"^2[ ^3%2d ^2- %s%-20s ^2- ^3%5d ^2- ^3%5d ^2]^7<-^2You%s%s^7\n\"", i, other->client->sess.team == TEAM_RED ? "^1" : (other->client->sess.team == TEAM_BLUE ? "^4" : "^7"), cleanname, other->client->ospClient->rate > maxrate ? maxrate : other->client->ospClient->rate, other->client->ospClient->snaps, OSP_IsClientAdmin(i, qfalse) ? "^7/^5Adm" : (OSP_IsClientReferee(i) ? "^7/^3Ref" : ""), OSP_IsClientAdmin(i, qfalse) ? (OSP_IsClientAdmin(i, qtrue) ? "(POWERS)" : "^3(NO POWERS)") : ""));
            else if (OSP_IsClientAdmin(i, qfalse))
                trap_SendServerCommand(ent - g_entities, va("print \"^2[ ^5%2d ^2- %s%-20s ^2- ^5%5d ^2- ^5%5d ^2]^7<-^5Admin%s^7\n\"", i, other->client->sess.team == TEAM_RED ? "^1" : (other->client->sess.team == TEAM_BLUE ? "^4" : "^7"), cleanname, other->client->ospClient->rate > maxrate ? maxrate : other->client->ospClient->rate, other->client->ospClient->snaps, OSP_IsClientAdmin(i, qtrue) ? "(POWERS)" : "^3(NO POWERS)"));
            else if (OSP_IsClientReferee(i))
                trap_SendServerCommand(ent - g_entities, va("print \"^2[ ^5%2d ^2- %s%-20s ^2- ^5%5d ^2- ^5%5d ^2]^7<-^3Referee^7\n\"", i, other->client->sess.team == TEAM_RED ? "^1" : (other->client->sess.team == TEAM_BLUE ? "^4" : "^7"), cleanname, other->client->ospClient->rate > maxrate ? maxrate : other->client->ospClient->rate, other->client->ospClient->snaps));
            else
                trap_SendServerCommand(ent - g_entities, va("print \"^2[ ^7%2d ^2- %s%-20s ^2- ^7%5d ^2- ^7%5d ^2]^7\n\"", i, other->client->sess.team == TEAM_RED ? "^1" : (other->client->sess.team == TEAM_BLUE ? "^4" : "^7"), cleanname, other->client->ospClient->rate > maxrate ? maxrate : other->client->ospClient->rate, other->client->ospClient->snaps));
        }
    }
}

// osp: 0x0002bc9f
static void OSP_PrintStats(gentity_t *ent, gentity_t *other)
{
    weapon_t weapon;
    ospStats_t *stats;
    char buf[1024];

    stats = other->client->ospStats;
    trap_SendServerCommand(ent - g_entities, va("print \"Stats for %s\n\"", other->client->pers.netname));

    for (weapon = WP_KNIFE; weapon < WP_NUM_WEAPONS; weapon++)
    {
        if (!stats->weapon[ATTACK_NORMAL][weapon].shots && !stats->weapon[ATTACK_ALTERNATE][weapon].shots)
        {
            continue;
        }

        memset(buf, 0, sizeof(buf));

        Com_sprintf(buf + strlen(buf),
                    sizeof(buf) - strlen(buf),
                    "print \"^2[^3%s^2]",
                    bg_weaponNames[weapon]);

        if (stats->weapon[ATTACK_NORMAL][weapon].shots || stats->weapon[ATTACK_ALTERNATE][weapon].shots)
        {
            Com_sprintf(buf + strlen(buf),
                        sizeof(buf) - strlen(buf),
                        " ^7%.2f^2(^7%d/%d^2)",
                        stats->weapon[ATTACK_NORMAL][weapon].shots ? ((float)stats->weapon[ATTACK_NORMAL][weapon].hits / stats->weapon[ATTACK_NORMAL][weapon].shots) * 100 : 0,
                        stats->weapon[ATTACK_NORMAL][weapon].hits,
                        stats->weapon[ATTACK_NORMAL][weapon].shots);
        }

        if (stats->weapon[ATTACK_ALTERNATE][weapon].shots)
        {
            Com_sprintf(buf + strlen(buf),
                        sizeof(buf) - strlen(buf),
                        " ^3%s: ^7%.2f^2(^7%d/%d^2)",
                        osp_alternateAttackNames[weapon],
                        ((float)stats->weapon[ATTACK_ALTERNATE][weapon].hits / stats->weapon[ATTACK_ALTERNATE][weapon].shots) * 100,
                        stats->weapon[ATTACK_ALTERNATE][weapon].hits,
                        stats->weapon[ATTACK_ALTERNATE][weapon].shots);
        }

        if ((stats->weapon[ATTACK_NORMAL][weapon].hits || stats->weapon[ATTACK_ALTERNATE][weapon].hits) &&
            (stats->weapon[ATTACK_NORMAL][weapon].headhits || stats->weapon[ATTACK_ALTERNATE][weapon].headhits))
        {
            Com_sprintf(buf + strlen(buf),
                        sizeof(buf) - strlen(buf),
                        " ^3Headshots: ^7%.2f^2(^7%d/%d^2)",
                        (
                            (float)(stats->weapon[ATTACK_NORMAL][weapon].headhits + stats->weapon[ATTACK_ALTERNATE][weapon].headhits) /
                            (stats->weapon[ATTACK_NORMAL][weapon].hits + stats->weapon[ATTACK_ALTERNATE][weapon].hits)) *
                            100,
                        (stats->weapon[ATTACK_NORMAL][weapon].headhits + stats->weapon[ATTACK_ALTERNATE][weapon].headhits),
                        (stats->weapon[ATTACK_NORMAL][weapon].hits + stats->weapon[ATTACK_ALTERNATE][weapon].hits));
        }

        Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "^7\n\"");
        trap_SendServerCommand(ent - g_entities, buf);
    }

    trap_SendServerCommand(
        ent - g_entities,
        va("print \"^2[^1Damage given^2] ^7%d\n^2[^1Damage taken^2] ^7%d\n^2[^1Team damage^2] ^7%d\n^2[^1Kills^2] ^7%d\n^2[^1Team kills^2] ^7%d\n^2[^1Deaths^2] ^7%d\n^2[^1Suicides^2] ^7%d\n^2[^1Time Played^2] ^7%d minutes, %d seconds\n\"",
           stats->damageGiven,
           stats->damageTaken,
           stats->teamDamage,
           stats->kills,
           stats->teamKills,
           stats->deaths,
           stats->suicides,
           (level.time - other->client->pers.enterTime) / 60000,
           ((level.time - other->client->pers.enterTime) / 1000) % 60));
}

// osp: 0x0002bf9a
void OSP_PrintAllStats(gentity_t *ent, qboolean toAll)
{
    ospStats_t *stats;
    gentity_t *other;
    int i;
    int clientNum;

    if (toAll)
    {
        clientNum = -1;
    }
    else
    {
        clientNum = ent - g_entities;
    }

    trap_SendServerCommand(clientNum, "print \"^2General Stats\n^2[Name           ][Pistol    ][Primary   ][Secondary ][Grenades  ][Damage]\n\"");

    for (i = 0; i < level.maxclients; i++)
    {
        other = g_entities + i;

        if (other && other->client && other->client->ospStats)
        {
            int grenade;
            int pistol;
            int primary;
            int secondary;
            char name[16];

            if (g_doWarmup.integer == 2 && (ent->client->sess.team == TEAM_RED || ent->client->sess.team == TEAM_BLUE) && other->client->sess.team != ent->client->sess.team)
            {
                continue;
            }

            pistol = OSP_GetPistol(other);
            primary = OSP_GetPrimaryWeapon(other);
            secondary = OSP_GetSecondaryWeapon(other);
            grenade = OSP_GetGrenade(other);

            stats = other->client->ospStats;

            strncpy(name, other->client->pers.netname, sizeof(name) - 1);
            name[sizeof(name) - 1] = '\0';

            trap_SendServerCommand(clientNum,
                                   va("print \"^2[^7%-15s^2][^3%-4s ^7%3.1f^2][^3%-4s ^7%3.1f^2][^3%-4s ^7%3.1f^2][^3%-4s ^7%3.1f^2][^7%6i^2]\n\"",
                                      name,
                                      osp_shortWeaponNames[pistol],
                                      stats->weapon[ATTACK_NORMAL][pistol].shots ? ((float)stats->weapon[ATTACK_NORMAL][pistol].hits / stats->weapon[ATTACK_NORMAL][pistol].shots) * 100 : 0,
                                      osp_shortWeaponNames[primary],
                                      stats->weapon[ATTACK_NORMAL][primary].shots ? ((float)stats->weapon[ATTACK_NORMAL][primary].hits / stats->weapon[ATTACK_NORMAL][primary].shots) * 100 : 0,
                                      osp_shortWeaponNames[secondary],
                                      stats->weapon[ATTACK_NORMAL][secondary].shots ? ((float)stats->weapon[ATTACK_NORMAL][secondary].hits / stats->weapon[ATTACK_NORMAL][secondary].shots) * 100 : 0,
                                      osp_shortWeaponNames[grenade],
                                      (stats->weapon[ATTACK_NORMAL][grenade].shots || stats->weapon[ATTACK_ALTERNATE][grenade].shots) ? (((float)stats->weapon[ATTACK_NORMAL][grenade].hits + stats->weapon[ATTACK_ALTERNATE][grenade].hits) / (stats->weapon[ATTACK_NORMAL][grenade].shots + stats->weapon[ATTACK_ALTERNATE][grenade].shots)) * 100 : 0,
                                      stats->damageGiven));
        }
    }
}

// osp: 0x0002c244
void OSP_PrintStatsToAllClients(void)
{
    gentity_t *ent;
    int i;

    for (i = 0; i < level.maxclients; i++)
    {
        ent = g_entities + i;

        if (ent && ent->client && ent->client->ospStats)
        {
            OSP_PrintStats(ent, ent);
        }
    }
}

// osp: 0x0002c284
static void OSP_Cmd_Statsall(gentity_t *e, int idx, int param)
{
    gentity_t *ent = e;
    OSP_PrintAllStats(ent, qfalse);
}

// osp: 0x0002c293
static void OSP_Cmd_Stats(gentity_t *e, int idx, int param)
{
    gentity_t *ent;
    gentity_t *other;
    char arg[16];

    ent = e;
    other = ent;

    if (trap_Argc() >= 2)
    {
        int clientNum;

        trap_Argv(1, arg, sizeof(arg));
        clientNum = atoi(arg);

        if (OSP_IsClientOnline(clientNum))
        {
            other = g_entities + clientNum;
        }
        else
        {
            trap_SendServerCommand(ent - g_entities, va("print \"^7bad client num %d\n\"", clientNum));
            return;
        }
    }

    if (g_doWarmup.integer == 2 && (ent->client->sess.team == TEAM_RED || ent->client->sess.team == TEAM_BLUE) && other->client->sess.team != other->client->sess.team)
    {
        trap_SendServerCommand(ent - g_entities, "print \"This player is not on your team.\n\"");
        return;
    }

    OSP_PrintStats(ent, other);
}

// osp: 0x0002c327
static void OSP_Cmd_CvarReport(gentity_t *e)
{
    gentity_t *ent;
    char arg1[2];
    char arg2[128];
    char arg3[16];

    ent = e;

    if (trap_Argc() < 4)
    {
        return;
    }

    trap_Argv(1, arg1, sizeof(arg1));
    trap_Argv(2, arg2, sizeof(arg2));
    trap_Argv(3, arg3, sizeof(arg3));

    trap_SendServerCommand(atoi(arg1), va("print \"^3(%d)[^7%s^3] ^5(%s)=^7%s\n\"", ent - g_entities, ent->client->pers.netname, arg2, arg3));
}

// osp: 0x0002c37b
static void OSP_Cmd_SayTeamnl(gentity_t *e, int index, int param)
{
    gentity_t *ent;
    char *p;

    ent = e;

    if (trap_Argc() < 2)
    {
        return;
    }

    p = ConcatArgs(1);
    G_Say(ent, NULL, SAY_TEAM_NL, p);
}

// osp: 0x0002c3a3
static void sub_0002c3a3(void)
{
    gclient_t *cl;
    int i;
    int readyMask;

    readyMask = 0;

    for (i = 0; i < level.maxclients; i++)
    {
        cl = &level.clients[i];

        if (cl->pers.connected != CON_CONNECTED || !cl->ospClient)
        {
            continue;
        }

        if (g_entities[cl->ps.clientNum].r.svFlags & SVF_BOT)
        {
            continue;
        }

        if (cl->ospClient->ready && i < 16)
        {
            readyMask |= (1 << i);
        }
    }

    for (i = 0; i < g_maxclients.integer; i++)
    {
        cl = &level.clients[i];

        if (cl->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        cl->ps.stats[STAT_CLIENTS_READY] = readyMask;
    }
}

// osp: 0x0002c422
static void OSP_Cmd_Ready(gentity_t *e, int index, int ready)
{
    gentity_t *ent;
    const char *prefix[] = {" NOT", ""};

    ent = e;

    if (osp.matchPhase > 2)
    {
        trap_SendServerCommand(ent - g_entities, "print \"Match is already in progress!\n\"");
        return;
    }

    if (!ready && osp.matchPhase == 2)
    {
        trap_SendServerCommand(ent - g_entities, "print \"Countdown started.... ^3/notready^7 ignored!\n\"");
        return;
    }

    if (ent->client->sess.team == TEAM_SPECTATOR)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You must be in the game to be ^3ready^7!\n\"");
        return;
    }

    if (osp.forceReady)
    {
        return;
    }

    if (level.numPlayingClients < match_minplayers.integer)
    {
        trap_SendServerCommand(ent - g_entities, "print \"Not enough players to start match!\n\"");
        return;
    }

    if (ent->client->ospClient->ready == ready)
        trap_SendServerCommand(ent - g_entities, va("print \"You are already%s ready!\n\"", prefix[ready]));
    else
    {
        ent->client->ospClient->ready = ready;
        ent->client->ospClient->warmupTime = 1;

        trap_SendServerCommand(-1, va("print \"%s is %s READY^7!\n\"", ent->client->pers.netname, ready ? "now^3" : "^1NOT"));
    }

    trap_SetConfigstring(CS_READY, OSP_ReadyMask());
}

// osp: 0x0002c4fa
static void OSP_Cmd_Follow(gentity_t *ent, int index, int param)
{
    Cmd_Follow_f(ent);
}

// osp: 0x0002c503
static void OSP_Cmd_Callvote(gentity_t *e, int index, int param)
{
    gentity_t *ent;
    int adminAction;

    ent = e;
    adminAction = 0;

    if (OSP_IsClientReferee(ent - g_entities))
    {
        adminAction = 1;
    }

    if (OSP_IsClientAdmin(ent - g_entities, qfalse))
    {
        adminAction = 2;
    }

    OSP_Callvote(ent, qfalse, adminAction);
}

// osp: 0x0002c53e
static void OSP_Cmd_Help(gentity_t *e, int index, int param)
{
    int i;
    int numRows;
    int numCmds;
    gentity_t *ent;

    numCmds = (sizeof(ospCmds) / sizeof(ospCmds[0])) - 1;
    ent = e;
    numRows = numCmds / 4;
    if (numCmds % 4)
        numRows++;
    if (numRows < 0)
        return;

    trap_SendServerCommand(ent - g_entities, "print \"^5\nAvailable OSP Game-Commands:\n----------------------------\n\"");

    for (i = 0; i < numRows; i++)
    {
        if (i + (3 * numRows) + 1 <= numCmds)
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%s%-17s%s%-17s%s%-17s%s%-17s\n\"",
                                      ospCmds[i].level ? "^5" : "^3",
                                      ospCmds[i].name,
                                      ospCmds[i + numRows].level ? "^5" : "^3",
                                      ospCmds[i + numRows].name,
                                      ospCmds[i + (2 * numRows)].level ? "^5" : "^3",
                                      ospCmds[i + (2 * numRows)].name,
                                      ospCmds[i + (3 * numRows)].level ? "^5" : "^3",
                                      ospCmds[i + (3 * numRows)].name));
        }
        else if (i + (2 * numRows) + 1 <= numCmds)
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%s%-17s%s%-17s%s%-17s\n\"",
                                      ospCmds[i].level ? "^5" : "^3",
                                      ospCmds[i].name,
                                      ospCmds[i + numRows].level ? "^5" : "^3",
                                      ospCmds[i + numRows].name,
                                      ospCmds[i + (2 * numRows)].level ? "^5" : "^3",
                                      ospCmds[i + (2 * numRows)].name));
        }
        else if (i + (numRows) + 1 <= numCmds)
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%s%-17s%s%-17s\n\"",
                                      ospCmds[i].level ? "^5" : "^3",
                                      ospCmds[i].name,
                                      ospCmds[i + numRows].level ? "^5" : "^3",
                                      ospCmds[i + numRows].name));
        }
        else
        {
            trap_SendServerCommand(ent - g_entities,
                                   va("print \"%s%-17s\n\"",
                                      ospCmds[i].level ? "^5" : "^3",
                                      ospCmds[i].name));
        }
    }

    trap_SendServerCommand(ent - g_entities, "print \"\nType: ^3\\command_name ?^7 for more information\n\"");
    trap_SendServerCommand(ent - g_entities, va("print \"\n^6%s\n\"", "SoF2 OSP v(1.1)"));
    trap_SendServerCommand(ent - g_entities, "print \"^5http://www.OrangeSmoothie.org/^7\n\n\"");
}

// osp: 0x0002c788
static void OSP_Cmd_Motd(gentity_t *e, int index, int param)
{
    gentity_t *ent = e;
    ent->client->ospClient->motdTime = level.time - 1;
}

// osp: 0x0002c79e
static void OSP_Cmd_Matchinfo(gentity_t *e, int index, int param)
{
    gentity_t *ent;
    char arg[16];

    ent = e;

    if (!level.gametypeData->teams)
    {
        trap_SendServerCommand(ent - g_entities, "print \"This command is only available in team based gametypes\"");
        return;
    }

    if (!team_controls.integer)
    {
        trap_SendServerCommand(ent - g_entities, "print \"This command is only available if team controls are enabled\"");
        return;
    }

    if (trap_Argc() == 1)
    {
        OSP_PrintTeaminfo(ent, TEAM_RED);
        OSP_PrintTeaminfo(ent, TEAM_BLUE);
    }
    else
    {
        trap_Argv(1, arg, sizeof(arg));

        if (arg[0] == 'r' || arg[0] == 'R')
        {
            OSP_PrintTeaminfo(ent, TEAM_RED);
        }
        else if (arg[0] == 'b' || arg[0] == 'B')
        {
            OSP_PrintTeaminfo(ent, TEAM_BLUE);
        }
        else
        {
            trap_SendServerCommand(ent - g_entities, "print \"Invalid team.\n\"");
        }
    }
}

// osp: 0x0002c829
static void OSP_Cmd_Settings(gentity_t *e, int index, int param)
{
    char buf[1024];
    gentity_t *ent;

    ent = e;

    trap_SendServerCommand(ent - g_entities, "print \"\n^3Settings for this server\n");

    memset(buf, 0, sizeof(buf));

    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Modversion.....................[%s]\n",
                osp_modversion.integer == 2 ? "^3SOF2++^7" : (osp_modversion.integer == 1 ? "^3OSP++^7" : "^1REGULAR^7"));
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Speed..........................[^3%d^7] %s\n",
                g_speed.integer,
                g_speed.integer == 280 ? "(default)" : "");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Gravity........................[^3%d^7] %s\n",
                g_gravity.integer,
                g_gravity.integer == 800 ? "(default)" : "");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Smoothclients..................[^3%s^7]\n",
                g_smoothClients.integer ? "^3ON^7" : "^1OFF^7");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Scorelimit.....................[^3%s^7]\n",
                g_scorelimit.integer ? g_scorelimit.string : "NONE");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Timelimit......................[^3%s^7]\n",
                g_timelimit.integer ? g_timelimit.string : "NONE");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Warmup.........................[%s]\n",
                g_doWarmup.integer == 2 ? "^3READY UP^7" : (g_doWarmup.integer == 1 ? "^3COUNTDOWN^7" : "^1NONE^7"));
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "RespawnInterval................[^3%s^7]\n",
                g_respawnInterval.integer ? g_respawnInterval.string : "NONE");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "RespawnInvulnerability.........[^3%s^7]\n",
                g_respawnInvulnerability.integer ? g_respawnInvulnerability.string : "NONE");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Fixed Physics..................[^3%s^7]\n",
                pmove_fixed.integer ? "^3ON^7" : "^1OFF^7");

    trap_SendServerCommand(ent - g_entities, va("print \"%s\"", buf));
    memset(buf, 0, sizeof(buf));

    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Friendly fire..................[%s]\n",
                g_friendlyFire.integer ? "^3ON^7" : "^1OFF^7");

    if (g_friendlyFire.integer)
    {
        Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                    "  Max TK.....................[^3%s^7]\n",
                    g_teamkillDamageMax.integer ? g_teamkillDamageMax.string : "NONE");
        Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                    "  Ban time...................[^3%s^7]\n",
                    g_teamkillBanTime.integer ? g_teamkillBanTime.string : "NONE");
    }

    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Falling damage.................[%s]\n",
                (g_dmflags.integer & DF_NO_FALLING) ? "^1OFF^7" : "^3ON^7");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Footsteps......................[%s]\n",
                (g_dmflags.integer & DF_NO_FOOTSTEPS) ? "^1OFF^7" : "^3ON^7");

    trap_SendServerCommand(ent - g_entities, va("print \"%s\"", buf));
    memset(buf, 0, sizeof(buf));

    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Team controls..................[%s]\n",
                team_controls.integer == 0 ? "^1OFF^7" : "^3ON^7");

    if (team_controls.integer)
    {
        Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                    "  Timeouts.................[^3%s^7]\n",
                    match_timeoutcount.integer ? match_timeoutcount.string : "NONE");

        if (match_timeoutcount.integer)
        {
            Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                        "  Timeout length...........[^3%s^7]\n",
                        match_timeoutlength.integer ? match_timeoutlength.string : "UNLIMITTED");
        }
    }

    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Fixed radar....................[%s]\n",
                g_radarFix.integer ? "^3ON^7" : "^1OFF^7");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Admin powers...................[%s]\n",
                sv_enableAdminPowers.integer ? "^3ON^7" : "^1OFF^7");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "OSP Realistic Mode.............[%s]\n",
                osp_realisticMode.integer ? "^3ON^7" : "^1OFF^7");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Enemy kill report..............[%s]\n",
                osp_enemyKillReport.integer ? "^3ON^7" : "^1OFF^7");
    Com_sprintf(buf + strlen(buf), sizeof(buf) - strlen(buf),
                "Weapon Mods (RD)...............[%s]\n",
                sv_modifiedfiles.integer ? "^3ON^7" : "^1OFF^7");

    trap_SendServerCommand(ent - g_entities, va("print \"%s\"", buf));
}

// osp: 0x0002cc8c
static void OSP_Cmd_OspScore(gentity_t *e, int index, int param)
{
    gentity_t *ent = e;

    if (!ent->client->ospClient->clientVersion)
    {
        DeathmatchScoreboardMessage(ent);
    }
    else
    {
        OSP_ScoreboardMessage(e);
    }
}

// osp: 0x0002ccae
static void OSP_Cmd_OspStats(gentity_t *e, int index, int param)
{
    gentity_t *ent = e;

    if (ent && ent->client && ent->client->ospStats)
    {
        trap_SendServerCommand(ent - g_entities,
                               va("stats %i %i %i %i %i %i %i %i %i %i %i",
                                  ent->client->ospStats->x44[1],
                                  ent->client->ospStats->x44[2],
                                  ent->client->ospStats->x44[3],
                                  ent->client->ospStats->x44[4],
                                  ent->client->ospStats->x44[5],
                                  ent->client->ospStats->x5c[1],
                                  ent->client->ospStats->x5c[2],
                                  ent->client->ospStats->x5c[3],
                                  ent->client->ospStats->x5c[4],
                                  ent->client->ospStats->x5c[5],
                                  OSP_NumShots(ent->client->ospStats)));
    }
}

// osp: 0x0002cd3a
static int OSP_ReplaceBadword(char *text, char *token)
{
    int i;
    char *p;
    qboolean result;

    result = qfalse;

    // found token in text?
    while (NULL != (p = sub_0002f577(text, token)))
    {
        // replace with *
        for (i = 0; i < strlen(token); i++, p++)
        {
            *p = '*';
        }

        // let caller know
        result = qtrue;
    }

    return result;
}

// osp: 0x0002cd80
qboolean OSP_FilterChattext(char *text)
{
    qboolean hasBadWords;
    const char *buf;
    char *token;

    hasBadWords = qfalse;
    buf = osp.badwords;

    while (1)
    {
        token = COM_Parse(&buf);

        if (!token || !token[0])
        {
            break;
        }

        // search for token in text
        if (OSP_ReplaceBadword(text, token))
        {
            // let caller know
            hasBadWords = qtrue;
        }
    }

    return hasBadWords;
}

// osp: 0x0002cdb7
void OSP_ScoreboardMessage(gentity_t *e)
{
    gclient_t *cl;
    int i;
    char entry[1024];
    char string[2000];
    int stringlength;
    int j;
    int numSorted;
    gentity_t *ent;

    ent = e;
    string[0] = 0;
    stringlength = 0;
    numSorted = level.numConnectedClients;

    for (i = 0; i < numSorted; i++)
    {
        int ping;

        cl = &level.clients[level.sortedClients[i]];

        if (cl->pers.connected == CON_CONNECTING)
        {
            ping = -1;
        }
        else
        {
            ping = cl->ps.ping < 999 ? cl->ps.ping : 999;
        }

        {
            Com_sprintf(entry, sizeof(entry),
                        " %i %i %i %i %i %i %i %i %i %i %i %i",
                        level.sortedClients[i],
                        (g_alternateSort.integer == 0)
                            ? (cl->sess.score)
                            : ((g_alternateSort.integer == 1)
                                   ? (int)(OSP_GetKDR(cl) * 100)
                                   : (int)(OSP_GetKPM(cl) * 3600)),
                        cl->sess.kills,
                        cl->sess.deaths,
                        ping,
                        (level.time - cl->pers.enterTime) / 60000,
                        (cl->sess.ghost || cl->ps.pm_type == PM_DEAD) ? qtrue : qfalse,
                        g_entities[level.sortedClients[i]].s.gametypeitems,
                        g_teamkillDamageMax.integer ? 100 * cl->sess.teamkillDamage / g_teamkillDamageMax.integer : 0,
                        osp.stats[level.sortedClients[i]].damageGiven / 100,
                        OSP_AccuracyInt(cl->ospStats),
                        OSP_ClientFlags(level.sortedClients[i]));
        }

        j = strlen(entry);
        if (stringlength + j > 1022)
        {
            break;
        }

        strcpy(string + stringlength, entry);
        stringlength += j;
    }

    trap_SendServerCommand(ent - g_entities, va("osp_scores %i %i %i%s",
                                                i,
                                                level.teamScores[TEAM_RED],
                                                level.teamScores[TEAM_BLUE],
                                                string));
}

// osp: 0x0002cf16
static void OSP_Cmd_Showex(gentity_t *e, int index, int param)
{
    gentity_t *ent;
    gentity_t *target;

    ent = e;

    if (strcmp(level.gametypeData->name, "inf"))
    {
        trap_SendServerCommand(ent - g_entities, "print \"This command is only available in the Infiltration gametype\n\"");
        return;
    }

    ent->client->ospClient->x98 = 0;

    target = NULL;
    while (NULL != (target = G_Find(target, FOFS(classname), "gametype_trigger")))
    {
        gentity_t *tent;
        vec3_t corners[8];
        int i;

        VectorCopy(target->r.maxs, corners[0]);

        VectorCopy(target->r.maxs, corners[1]);
        corners[1][2] = target->r.mins[2];

        VectorCopy(target->r.mins, corners[2]);

        VectorCopy(target->r.mins, corners[3]);
        corners[3][2] = target->r.maxs[2];

        VectorCopy(corners[0], corners[4]);
        corners[4][0] = corners[2][0];

        VectorCopy(corners[1], corners[5]);
        corners[5][0] = corners[3][0];

        VectorCopy(corners[2], corners[6]);
        corners[6][0] = corners[0][0];

        VectorCopy(corners[3], corners[7]);
        corners[7][0] = corners[1][0];

        for (i = 0; i < 8; i += 2)
        {
            tent = G_TempEntity(corners[i], EV_BOTWAYPOINT);
            tent->r.svFlags = SVF_SINGLECLIENT;
            tent->r.singleClient = ent - g_entities;
            if (ent - g_entities < 32)
                tent->r.broadcastClients[0] = (1 << (ent - g_entities));
            else
                tent->r.broadcastClients[1] = (1 << ((ent - g_entities) - 32));
            VectorCopy(corners[i + 1], tent->s.angles);
            tent->s.time = 15000;
            tent->s.time2 = 0xff3333;
        }

        tent = G_TempEntity(corners[0], EV_BOTWAYPOINT);
        tent->r.svFlags = SVF_SINGLECLIENT;
        tent->r.singleClient = ent - g_entities;
        if (ent - g_entities < 32)
            tent->r.broadcastClients[0] = (1 << (ent - g_entities));
        else
            tent->r.broadcastClients[1] = (1 << ((ent - g_entities) - 32));
        VectorCopy(corners[2], tent->s.angles);
        tent->s.time = 15000;
        tent->s.time2 = 0xff3333;

        tent = G_TempEntity(corners[5], EV_BOTWAYPOINT);
        tent->r.svFlags = SVF_SINGLECLIENT;
        tent->r.singleClient = ent - g_entities;
        if (ent - g_entities < 32)
            tent->r.broadcastClients[0] = (1 << (ent - g_entities));
        else
            tent->r.broadcastClients[1] = (1 << ((ent - g_entities) - 32));
        VectorCopy(corners[7], tent->s.angles);
        tent->s.time = 15000;
        tent->s.time2 = 0xff3333;

        tent = G_TempEntity(corners[1], EV_BOTWAYPOINT);
        tent->r.svFlags = SVF_SINGLECLIENT;
        tent->r.singleClient = ent - g_entities;
        if (ent - g_entities < 32)
            tent->r.broadcastClients[0] = (1 << (ent - g_entities));
        else
            tent->r.broadcastClients[1] = (1 << ((ent - g_entities) - 32));
        VectorCopy(corners[3], tent->s.angles);
        tent->s.time = 15000;
        tent->s.time2 = 0xff3333;

        tent = G_TempEntity(corners[4], EV_BOTWAYPOINT);
        tent->r.svFlags = SVF_SINGLECLIENT;
        tent->r.singleClient = ent - g_entities;
        if (ent - g_entities < 32)
            tent->r.broadcastClients[0] = (1 << (ent - g_entities));
        else
            tent->r.broadcastClients[1] = (1 << ((ent - g_entities) - 32));
        VectorCopy(corners[6], tent->s.angles);
        tent->s.time = 15000;
        tent->s.time2 = 0xff3333;

        break;
    }
}

// osp: 0x0002d161
static void OSP_Cmd_Sub(gentity_t *e, int index, int param)
{
    gentity_t *ent;

    ent = e;

    if (!level.gametypeData->teams)
    {
        trap_SendServerCommand(ent - g_entities, "print \"This command is only available in team based gametypes\n\"");
        return;
    }

    if (g_doWarmup.integer != 2)
    {
        trap_SendServerCommand(ent - g_entities, "print \"This command is only available in competitive mode (g_dowarmup 2)\n\"");
        return;
    }

    ent->client->ospClient->subbing = !ent->client->ospClient->subbing;

    if (ent->client->ospClient->subbing)
        trap_SendServerCommand(-1, va("cp \"%s is now subbing\n\"", ent->client->pers.netname));
    else
        trap_SendServerCommand(-1, va("cp \"%s is NOT subbing any longer\n\"", ent->client->pers.netname));
}

// osp: 0x0002d1e8
static void OSP_TransferM203(gentity_t *ent, gentity_t *other)
{
    int ammoIndex;

    ammoIndex = AMMO_40;

    if (!ent->client->hasM203)
    {
        trap_SendServerCommand(ent - g_entities, "print \"You don't have m203\n\"");
        return;
    }

    if (ent->client->sess.team != other->client->sess.team)
    {
        trap_SendServerCommand(ent - g_entities, "print \"This player isn't on your team\n\"");
        return;
    }

    if (!(other->client->ps.stats[STAT_WEAPONS] & (1 << WP_M4_ASSAULT_RIFLE)))
    {
        trap_SendServerCommand(ent - g_entities, "print \"This player doesn't have m4\n\"");
        return;
    }

    other->client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE] = ent->client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE];
    other->client->ps.ammo[ammoIndex] = ent->client->ps.ammo[ammoIndex];
    other->client->hasM203 = qtrue;

    ent->client->ps.clip[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE] = 0;
    ent->client->ps.ammo[ammoIndex] = 0;
    ent->client->hasM203 = qfalse;

    trap_SendServerCommand(ent - g_entities, va("print \"%s now has m203\n\"", other->client->pers.netname));
}

// osp: 0x0002d2cd
static void OSP_Cmd_GiveM203(gentity_t *e, int index, int param)
{
    gentity_t *ent;
    gentity_t *other;
    char arg[16];

    ent = e;
    other = ent;

    if (trap_Argc() >= 2)
    {
        int clientNum;

        trap_Argv(1, arg, sizeof(arg));
        clientNum = atoi(arg);

        if (OSP_IsClientOnline(clientNum))
        {
            other = g_entities + clientNum;
        }
        else
        {
            trap_SendServerCommand(ent - g_entities, va("print \"^7bad client number %d\n\"", clientNum));
            return;
        }
    }

    if ((ent->client->sess.team == TEAM_RED || ent->client->sess.team == TEAM_BLUE) &&
        ent->client->sess.team != other->client->sess.team)
    {
        trap_SendServerCommand(ent - g_entities, "print \"This player is NOT on your team.\n\"");
        return;
    }

    OSP_TransferM203(ent, other);
}
