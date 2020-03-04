// osp_servercmds.c
//
#include "cg_local.h"

// osp: 0x00028a61
static void OSP_ParseScores(void)
{
    int i;

    cg.scoreBoardSpectators[0] = '\0';

    cg.numScores = atoi(CG_Argv(1));
    if (cg.numScores > MAX_CLIENTS)
    {
        cg.numScores = MAX_CLIENTS;
    }

    cg.teamScores[0] = atoi(CG_Argv(2));
    cg.teamScores[1] = atoi(CG_Argv(3));

    memset(cg.scores, 0, sizeof(cg.scores));
    for (i = 0; i < cg.numScores; i++)
    {
        cg.scores[i].client = atoi(CG_Argv(i * 12 + 4));
        cg.scores[i].score = atoi(CG_Argv(i * 12 + 5));
        cg.scores[i].kills = atoi(CG_Argv(i * 12 + 6));
        cg.scores[i].deaths = atoi(CG_Argv(i * 12 + 7));
        cg.scores[i].ping = atoi(CG_Argv(i * 12 + 8));
        cg.scores[i].time = atoi(CG_Argv(i * 12 + 9));
        cgs.clientinfo[cg.scores[i].client].ghost = atoi(CG_Argv(i * 12 + 10));
        cgs.clientinfo[cg.scores[i].client].gametypeitems = atoi(CG_Argv(i * 12 + 11));
        cg.scores[i].teamkillDamage = atoi(CG_Argv(i * 12 + 12));
        cg.scores[i].damageDone = atoi(CG_Argv(i * 12 + 13));
        cg.scores[i].accuracy = atoi(CG_Argv(i * 12 + 14)) / 10.0f;
        cg.scores[i].rank = atoi(CG_Argv(i * 12 + 15));

        if (cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS)
        {
            cg.scores[i].client = 0;
        }

        if (cg.scores[i].ping < 0)
        {
            cg.scores[i].time = 0;
        }

        cgs.clientinfo[cg.scores[i].client].score = cg.scores[i].score;
        cg.scores[i].team = cgs.clientinfo[cg.scores[i].client].team;

        if (cg.scores[i].team == TEAM_SPECTATOR)
        {
            if (cg.scoreBoardSpectators[0])
            {
                strcat(cg.scoreBoardSpectators, ", ");
            }

            strcat(cg.scoreBoardSpectators, va("%s (%d)", cgs.clientinfo[cg.scores[i].client].name, cg.scores[i].ping));
        }
    }
}

// osp: 0x00028ca0
static void OSP_ParseTeamInformation(void)
{
    clientInfo_t *cl;
    int offset;
    int i;

    for (i = 0; i < cgs.maxclients; i++)
    {
        cl = &cgs.clientinfo[i];
        cl->health = 0;
        cl->armor = 0;
        cl->location = 0;
        cl->weapon = 0;
    }

    for (i = 0; i < cgs.maxclients; i++)
    {
        int idnum;

        offset = 6 * i;

        if (!CG_Argv(offset + 1)[0])
            break;

        idnum = atoi(CG_Argv(offset + 1));
        cl = &cgs.clientinfo[idnum];

        cl->health = atoi(CG_Argv(offset + 2));
        cl->armor = atoi(CG_Argv(offset + 3));
        cl->location = atoi(CG_Argv(offset + 4));
        cl->weapon = atoi(CG_Argv(offset + 5));
        cl->gametypeitems = atoi(CG_Argv(offset + 6));
    }
}

// osp: 0x00028d89
static void OSP_CvarCheck(void)
{
    char val[16];
    int clientNum;

    clientNum = atoi(CG_Argv(1));
    trap_Cvar_VariableStringBuffer(CG_Argv(2), val, sizeof(val));
    memset(ospCg.x457924, 0, sizeof(ospCg.x457924));
    strncpy(ospCg.x457924, va("cvarreport %d %s %s", clientNum, CG_Argv(2), strlen(val) > 0 ? val : "unset"), sizeof(ospCg.x457924));
    ospCg.x457920 = cg.time + 1500;
}

// osp: 0x00028ded
void OSP_ParseReadyup(void)
{
    int i;
    clientInfo_t *cl;
    int x18;
    int x1c;
    const char *info;
    char *s;

    info = CG_ConfigString(CS_READY);

    if (strlen(info) < 1)
    {
        if (cgs.clientinfo[cg.clientNum].ready)
        {
            trap_Cvar_Set("ui_info_ready", "1");
        }
        else
        {
            trap_Cvar_Set("ui_info_ready", "0");
        }
    }
    else
    {
        s = strstr(info, " ");
        s++;

        x18 = atoi(info);
        x1c = atoi(s);

        for (i = 0; i < cgs.maxclients; i++)
        {
            cl = &cgs.clientinfo[i];

            if (i < 32)
            {
                cl->ready = x18 & (1 << i) ? qtrue : qfalse;
            }
            else
            {
                cl->ready = x1c & (1 << (i - 32)) ? qtrue : qfalse;
            }
        }

        if (cgs.clientinfo[cg.clientNum].ready)
        {
            trap_Cvar_Set("ui_info_ready", "1");
        }
        else
        {
            trap_Cvar_Set("ui_info_ready", "0");
        }
    }
}

// osp: 0x00028eb2
void OSP_ParseMatchphase(void)
{
    ospCg.matchphase = atoi(CG_ConfigString(CS_MATCH_PHASE));
}

// osp: 0x00028ec6
void OSP_ParseTeamname(team_t team)
{
    if (team == TEAM_RED)
    {
        Q_strncpyz(ospCg.teamNames[team], CG_ConfigString(CS_RED_TEAM_NAME), 64);
        trap_Cvar_Set("ui_redteamname", ospCg.teamNames[team]);
        trap_Cvar_Set("ui_redteamnameDisplay", ospCg.teamNames[team]);
    }
    else if (team == TEAM_BLUE)
    {
        Q_strncpyz(ospCg.teamNames[team], CG_ConfigString(CS_BLUE_TEAM_NAME), 64);
        trap_Cvar_Set("ui_blueteamname", ospCg.teamNames[team]);
        trap_Cvar_Set("ui_blueteamnameDisplay", ospCg.teamNames[team]);
    }
}

// osp: 0x00028f2d
void OSP_ParseTeaminfo(void)
{
    char *s;
    const char *info;

    info = CG_ConfigString(CS_OSP_TEAMINFO);

    s = Info_ValueForKey(info, "paused");
    if (s[0] > '0')
        trap_Cvar_Set("ui_paused", "1");
    else
        trap_Cvar_Set("ui_paused", "0");

    s = Info_ValueForKey(info, "teamc");
    trap_Cvar_Set("ui_teamcontrols", s);

    s = Info_ValueForKey(info, "rtj");
    trap_Cvar_Set("ui_redteamlock", s);

    s = Info_ValueForKey(info, "btj");
    trap_Cvar_Set("ui_blueteamlock", s);

    s = Info_ValueForKey(info, "rsl");
    trap_Cvar_Set("ui_redteamspeclock", s);

    s = Info_ValueForKey(info, "bsl");
    trap_Cvar_Set("ui_blueteamspeclock", s);
}

// osp: 0x00028fc6
static void OSP_ParseAwards(qboolean awards)
{
    char *s;
    const char *info;

    if (awards)
    {
        info = CG_ConfigString(CS_AWARDS);
        memset(ospCg.awards, 0, sizeof(ospCg.awards[0]) * 7);

        if (strlen(info) > 0)
        {
            s = Info_ValueForKey(info, "kN");
            Q_strncpyz(ospCg.awards[AWARD_LETHAL].winnerName, s, 32);
            s = Info_ValueForKey(info, "kC");
            ospCg.awards[AWARD_LETHAL].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "dN");
            Q_strncpyz(ospCg.awards[AWARD_DAMAGE].winnerName, s, 32);
            s = Info_ValueForKey(info, "dC");
            ospCg.awards[AWARD_DAMAGE].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "hN");
            Q_strncpyz(ospCg.awards[AWARD_HEADSHOTS].winnerName, s, 32);
            s = Info_ValueForKey(info, "hC");
            ospCg.awards[AWARD_HEADSHOTS].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "gN");
            Q_strncpyz(ospCg.awards[AWARD_EXPLOSIVES].winnerName, s, 32);
            s = Info_ValueForKey(info, "gC");
            ospCg.awards[AWARD_EXPLOSIVES].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "knN");
            Q_strncpyz(ospCg.awards[AWARD_CRAZIEST].winnerName, s, 32);
            s = Info_ValueForKey(info, "knC");
            ospCg.awards[AWARD_CRAZIEST].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "sN");
            Q_strncpyz(ospCg.awards[AWARD_SCORE].winnerName, s, 32);
            s = Info_ValueForKey(info, "sC");
            ospCg.awards[AWARD_SCORE].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "rN");
            Q_strncpyz(ospCg.awards[AWARD_SURVIVOR].winnerName, s, 32);
            s = Info_ValueForKey(info, "rC");
            ospCg.awards[AWARD_SURVIVOR].winnerScore = atoi(s);
        }
    }
    else
    {
        info = CG_ConfigString(CS_BADAWARDS);
        memset(&ospCg.awards[7], 0, sizeof(ospCg.awards[0]) * 7);

        if (strlen(info) > 0)
        {
            s = Info_ValueForKey(info, "suN");
            Q_strncpyz(ospCg.awards[BADAWARD_QUITTER].winnerName, s, 32);
            s = Info_ValueForKey(info, "suC");
            ospCg.awards[BADAWARD_QUITTER].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "lN");
            Q_strncpyz(ospCg.awards[BADAWARD_LEMON].winnerName, s, 32);
            s = Info_ValueForKey(info, "lC");
            ospCg.awards[BADAWARD_LEMON].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "nN");
            Q_strncpyz(ospCg.awards[BADAWARD_NEWBIE].winnerName, s, 32);
            s = Info_ValueForKey(info, "nC");
            ospCg.awards[BADAWARD_NEWBIE].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "etN");
            Q_strncpyz(ospCg.awards[BADAWARD_FREEKILL].winnerName, s, 32);
            s = Info_ValueForKey(info, "etC");
            ospCg.awards[BADAWARD_FREEKILL].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "ceN");
            Q_strncpyz(ospCg.awards[BADAWARD_CROSSEYED].winnerName, s, 32);
            s = Info_ValueForKey(info, "ceC");
            ospCg.awards[BADAWARD_CROSSEYED].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "spN");
            Q_strncpyz(ospCg.awards[BADAWARD_SPAMMER].winnerName, s, 32);
            s = Info_ValueForKey(info, "spC");
            ospCg.awards[BADAWARD_SPAMMER].winnerScore = atoi(s);

            s = Info_ValueForKey(info, "cbN");
            Q_strncpyz(ospCg.awards[BADAWARD_CHATTYBITCH].winnerName, s, 32);
            s = Info_ValueForKey(info, "cbC");
            ospCg.awards[BADAWARD_CHATTYBITCH].winnerScore = atoi(s);
        }
    }
}

// osp: 0x0002929d
static char *OSP_DemoFilename(void)
{
    char *p;
    qtime_t time;
    char x58[64];
    char x98[64];
    char *xd8;
    {
        char teamnames[2][64];

        strncpy(x98, cgs.clientinfo[cg.clientNum].name, 64);
        x98[63] = 0;
        p = x98;
        Q_CleanStr(p);

        while (*p)
        {
            if (Q_isalpha(*p) || OSP_IsDigit(*p))
            {
                p++;
            }
            else
            {
                *p = '_';
                p++;
            }
        }

        strncpy(x58, cgs.clientinfo[cg.snap->ps.clientNum].name, 64);
        x58[63] = 0;
        p = x58;
        Q_CleanStr(p);

        while (*p)
        {
            if (Q_isalpha(*p) || OSP_IsDigit(*p))
            {
                p++;
            }
            else
            {
                *p = '_';
                p++;
            }
        }

        if (cgs.gametypeData->teams)
        {
            strncpy(teamnames[0], ospCg.teamNames[TEAM_RED], 64);
            p = teamnames[0];
            Q_CleanStr(p);

            while (*p)
            {
                if (Q_isalpha(*p) || OSP_IsDigit(*p))
                {
                    p++;
                }
                else
                {
                    *p = '_';
                    p++;
                }
            }

            strncpy(teamnames[1], ospCg.teamNames[TEAM_BLUE], 64);
            p = teamnames[1];
            Q_CleanStr(p);

            while (*p)
            {
                if (Q_isalpha(*p) || OSP_IsDigit(*p))
                {
                    p++;
                }
                else
                {
                    *p = '_';
                    p++;
                }
            }
        }

        trap_RealTime(&time);

        if (cgs.gametypeData->teams)
        {
            xd8 = va("%s_vs_%s-POV(%s)%i.%i.%i-%i.%i.%i",
                     teamnames[0],
                     teamnames[1],
                     x58,
                     time.tm_year + 1900,
                     time.tm_mon + 1,
                     time.tm_mday,
                     time.tm_hour,
                     time.tm_min,
                     time.tm_sec);
        }
        else
        {
            xd8 = va("%s-POV(%s)%i.%i.%i-%i.%i.%i",
                     x98,
                     x58,
                     time.tm_year + 1900,
                     time.tm_mon + 1,
                     time.tm_mday,
                     time.tm_hour,
                     time.tm_min,
                     time.tm_sec);
        }

        return xd8;
    }
}

// osp: 0x0002942d
void OSP_Record(qboolean start)
{
    if (start)
    {
        trap_SendConsoleCommand(va("stoprecord; g_synchronousclients 1; record %s; g_synchronousclients 0;\n", OSP_DemoFilename()));
    }
    else
    {
        trap_SendConsoleCommand("stoprecord;\n");
    }
}

// osp: 0x0002944e
void OSP_Screenshot(void)
{
    trap_SendConsoleCommand(va("screenshot %s;\n", OSP_DemoFilename()));
}

// osp: 0x00029464
static void OSP_ParseStats(void)
{
    int i;

    ospCg.x4581bc = 0;
    ospCg.x4581c4 = 0;
    ospCg.x4581c0 = 0;

    for (i = 1; i < 6; i++)
    {
        ospCg.x457d54[i] = atoi(CG_Argv(i));
        ospCg.x4581bc += ospCg.x457d54[i];
    }

    for (i = 1; i < 6; i++)
    {
        ospCg.x457d6c[i] = atoi(CG_Argv(i + 5));
        ospCg.x4581c4 += ospCg.x457d6c[i];
    }

    ospCg.x4581c0 = atoi(CG_Argv(11));
}

// osp: 0x000294ed
qboolean OSP_ServerCommand(const char *cmd)
{
    if (!strcmp(cmd, "ti"))
    {
        OSP_ParseTeamInformation();
        return qtrue;
    }

    if (!strcmp(cmd, "ar_start"))
    {
        if (osp_autoRecord.integer)
        {
            OSP_Record(qtrue);
        }

        return qtrue;
    }

    if (!strcmp(cmd, "ar_end"))
    {
        if (osp_autoRecord.integer)
        {
            OSP_Record(qfalse);
        }

        return qtrue;
    }

    if (!strcmp(cmd, "autoss"))
    {
        if (osp_autoScreenshot.integer)
        {
            OSP_Screenshot();
        }

        return qtrue;
    }

    if (!strcmp(cmd, "osp_scores"))
    {
        OSP_ParseScores();
        CG_UpdateTeamCountCvars();
        return qtrue;
    }

    if (!strcmp(cmd, "cvarcheck"))
    {
        OSP_CvarCheck();
        return qtrue;
    }

    if (!strcmp(cmd, "stats"))
    {
        OSP_ParseStats();
        return qtrue;
    }

    return qfalse;
}

// osp: 0x00029591
qboolean OSP_ConfigStringModified(int num)
{
    switch (num)
    {
    case CS_READY:
        OSP_ParseReadyup();
        return qtrue;
    case CS_MATCH_PHASE:
        OSP_ParseMatchphase();
        return qtrue;
    case CS_RED_TEAM_NAME:
        OSP_ParseTeamname(TEAM_RED);
        return qtrue;
    case CS_BLUE_TEAM_NAME:
        OSP_ParseTeamname(TEAM_BLUE);
        return qtrue;
    case CS_OSP_TEAMINFO:
        OSP_ParseTeaminfo();
        return qtrue;
    case CS_AWARDS:
        OSP_ParseAwards(qtrue);
        return qtrue;
    case CS_BADAWARDS:
        OSP_ParseAwards(qfalse);
        return qtrue;
    }

    return qfalse;
}
