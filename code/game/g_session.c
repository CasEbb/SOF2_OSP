// Copyright (C) 2001-2002 Raven Software
//
#include "g_local.h"

/*
=======================================================================
  SESSION DATA
Session data is the only data that stays persistant across level loads
and map restarts.
=======================================================================
*/

/*
================
G_WriteClientSessionData

Called on game shutdown
================
*/
void G_WriteClientSessionData(gclient_t *client)
{
    const char *s;
    const char *var;

    // OSP
    //s = va("%i", client->sess.team);
    s = va("%i %i %i %i %i %i %i %i %i",
           client->sess.team,
           client->ospClient->referee,
           client->ospClient->admin,
           client->ospClient->adminPowerSuspended,
           client->ospClient->x38,
           client->ospClient->specinvites,
           client->sess.spectatorState,
           client->ospClient->captain,
           client->ospClient->subbing);
    // END

    var = va("session%i", client - level.clients);

    trap_Cvar_Set(var, s);
}

/*
================
G_ReadSessionData

Called on a reconnect
================
*/
void G_ReadSessionData(gclient_t *client)
{
    char s[MAX_STRING_CHARS];
    const char *var;
    int sessionTeam;

    var = va("session%i", client - level.clients);
    trap_Cvar_VariableStringBuffer(var, s, sizeof(s));

    // OSP
    // sscanf(s, "%i", &sessionTeam);
    sessionTeam = atoi(s);

    while (1)
    {
        char *buf;

        buf = strstr(s, " ");
        if (!buf)
            break;
        client->ospClient->referee = atoi(buf);
        buf++;

        buf = strstr(buf, " ");
        if (!buf)
            break;
        client->ospClient->admin = atoi(buf);
        buf++;

        buf = strstr(buf, " ");
        if (!buf)
            break;
        client->ospClient->adminPowerSuspended = atoi(buf);
        buf++;

        buf = strstr(buf, " ");
        if (!buf)
            break;
        client->ospClient->x38 = atoi(buf);
        buf++;

        buf = strstr(buf, " ");
        if (!buf)
            break;
        client->ospClient->specinvites = atoi(buf);
        buf++;

        buf = strstr(buf, " ");
        if (!buf)
            break;
        client->sess.spectatorState = atoi(buf);
        buf++;

        buf = strstr(buf, " ");
        if (!buf)
            break;
        client->ospClient->captain = atoi(buf);
        buf++;

        buf = strstr(buf, " ");
        if (!buf)
            break;
        client->ospClient->subbing = atoi(buf);

        break;
    }
    // END

    // bk001205 - format issues
    client->sess.team = (team_t)sessionTeam;

    // OSP
    if (level.newSession)
    {
        char userinfo[MAX_INFO_STRING];
        trap_GetUserinfo(client - level.clients, userinfo, sizeof(userinfo));
        G_InitSessionData(client, userinfo);
    }

    if (g_randomTeams.integer && level.gametypeData->teams && client->sess.team != TEAM_SPECTATOR)
    {
        int counts[TEAM_NUM_TEAMS];
        team_t team;

        counts[TEAM_BLUE] = TeamCount(client - level.clients, TEAM_BLUE, qfalse);
        counts[TEAM_RED] = TeamCount(client - level.clients, TEAM_RED, qfalse);

        if (counts[TEAM_BLUE] > counts[TEAM_RED])
        {
            team = TEAM_RED;
        }
        else if (counts[TEAM_RED] > counts[TEAM_BLUE])
        {
            team = TEAM_BLUE;
        }
        else
        {
            team = (rand() % 2) + 1;
        }

        client->sess.team = team;
    }
    // END
}

/*
================
G_InitSessionData

Called on a first-time connect
================
*/
void G_InitSessionData(gclient_t *client, char *userinfo)
{
    clientSession_t *sess;
    const char *value;

    sess = &client->sess;

    // initial team determination
    if (level.gametypeData->teams)
    {
        if (g_teamAutoJoin.integer)
        {
            sess->team = PickTeam(-1);
        }
        else
        {
            // always spawn as spectator in team games
            sess->team = TEAM_SPECTATOR;
        }
    }
    else
    {
        value = Info_ValueForKey(userinfo, "team");
        if (value[0] == 's')
        {
            // a willing spectator, not a waiting-in-line
            sess->team = TEAM_SPECTATOR;
        }
        else
        {
            if (g_maxGameClients.integer > 0 && level.numNonSpectatorClients >= g_maxGameClients.integer)
            {
                sess->team = TEAM_SPECTATOR;
            }
            else
            {
                sess->team = TEAM_FREE;
            }
        }
    }

    sess->spectatorState = SPECTATOR_FREE;
    sess->spectatorTime = level.time;

    G_WriteClientSessionData(client);
}

/*
==================
G_InitWorldSession

==================
*/
void G_InitWorldSession(void)
{
    char s[MAX_STRING_CHARS];
    int gt;

    trap_Cvar_VariableStringBuffer("session", s, sizeof(s));

    gt = BG_FindGametype(s);

    // if the gametype changed since the last session, don't use any
    // client sessions
    // OSP
    //if (level.gametype != gt)
    if (bg_gametypeData[level.gametype].teams != bg_gametypeData[gt].teams)
    // END
    {
        level.newSession = qtrue;
        Com_Printf("Gametype changed, clearing session data.\n");
    }
}

/*
==================
G_WriteSessionData
==================
*/
void G_WriteSessionData(void)
{
    int i;

    trap_Cvar_Set("session", level.gametypeData->name);

    for (i = 0; i < level.maxclients; i++)
    {
        if (level.clients[i].pers.connected == CON_CONNECTED)
        {
            G_WriteClientSessionData(&level.clients[i]);
        }
    }

    // OSP
    if (team_controls.integer && level.gametypeData->teams)
    {
        OSP_WriteTeamSessionData(TEAM_RED);
        OSP_WriteTeamSessionData(TEAM_BLUE);
    }
    // END
}
