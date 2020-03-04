// osp_client.c
//
#include "g_local.h"

// osp: 0x0002ac25
qboolean OSP_IsClientOnline(int clientNum)
{
    if (clientNum < 0)
    {
        return qfalse;
    }

    if (clientNum >= level.maxclients)
    {
        return qfalse;
    }

    if (!level.clients[clientNum].pers.connected)
    {
        return qfalse;
    }

    return qtrue;
}

// osp: 0x0002ac4b
char *OSP_GetClientName(int clientNum, int len)
{
    char buf[128];
    char *result;

    if (len >= 127)
    {
        len = 127;
    }

    if (!OSP_IsClientOnline(clientNum))
    {
        return "";
    }

    memset(buf, 0, sizeof(buf));
    strncpy(buf, g_entities[clientNum].client->pers.netname, len);
    buf[len] = '\0';

    result = va("%s", buf);
    return result;
}

// osp: 0x0002ac94
char *OSP_GetGuid(int clientNum)
{
    char userinfo[MAX_INFO_STRING];
    char *value;

    trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));
    value = Info_ValueForKey(userinfo, "cl_guid");
    return value;
}

// osp: 0x0002acb0
void OSP_SetCachedGuid(int clientNum)
{
    trap_Cvar_Set(va("guid%d", clientNum), OSP_GetGuid(clientNum));
}

// osp: 0002accc
void OSP_ClearCachedGuid(int clientNum)
{
    trap_Cvar_Set(va("guid%d", clientNum), "none");
}

// osp: 0x0002ace0
char *OSP_GetCachedGuid(int clientNum)
{
    char guid[64];
    char *result;

    memset(guid, 0, sizeof(guid));
    trap_Cvar_VariableStringBuffer(va("guid%d", clientNum), guid, sizeof(guid));
    result = va("%s", guid);

    return result;
}

// osp 0x0002ad0e
qboolean OSP_LoadAdmin(int clientNum)
{
    char *guid;
    int i;

    guid = OSP_GetGuid(clientNum);

    for (i = 0; i < MAX_ADMINS && i < osp.numAdmins; i++)
    {
        if (Q_strncmp(guid, osp.adminList[i], 32) == 0)
        {
            return qtrue;
        }
    }

    return qfalse;
}

// osp: 0x0002ad4a
qboolean OSP_IsClientAdmin(int clientNum, qboolean withPowers)
{
    if (withPowers && osp.clients[clientNum].adminPowerSuspended)
    {
        return qfalse;
    }

    if (withPowers && !sv_enableAdminPowers.integer)
    {
        return qfalse;
    }

    if (osp.clients[clientNum].admin)
    {
        return qtrue;
    }

    return qfalse;
}

// osp: 0x0002ad89
qboolean OSP_IsClientReferee(int clientNum)
{
    if (osp.clients[clientNum].referee)
    {
        return qtrue;
    }

    if (osp.clients[clientNum].adminPowerSuspended)
    {
        return qtrue;
    }

    if (osp.clients[clientNum].admin)
    {
        return qtrue;
    }

    return qfalse;
}

// osp: 0x0002adbe
qboolean OSP_IsClientCaptain(int clientNum)
{
    if (osp.clients[clientNum].captain)
    {
        return qtrue;
    }

    return qfalse;
}

// osp: 0x0002add3
qboolean OSP_ClientConnect(int clientNum)
{
    gentity_t *ent;
    ospStats_t *ospStats;
    ospClient_t *ospClient;
    qboolean firstTime;

    ent = &g_entities[clientNum];

    ospStats = &osp.stats[clientNum];
    memset(ospStats, 0, sizeof(ospStats_t));
    ent->client->ospStats = ospStats;

    ospClient = &osp.clients[clientNum];
    memset(ospClient, 0, sizeof(ospClient_t));
    ent->client->ospClient = ospClient;

    if (strcmp(OSP_GetGuid(clientNum), OSP_GetCachedGuid(clientNum)) == 0)
    {
        firstTime = qfalse;
    }
    else
    {
        firstTime = qtrue;
    }

    if (firstTime)
    {
        ent->client->ospClient->motdTime = level.time - 1;
    }

    ent->client->ospClient->admin = OSP_LoadAdmin(clientNum);

    return firstTime;
}

// osp: 0x0002ae66
char *OSP_ClientUserinfoChanged(int clientNum)
{
    int grade = 0;
    int specinviteBlue = -1;
    int specinviteRed = -1;
    gentity_t *ent;
    char userinfo[MAX_INFO_STRING];
    char *s;

    {
        char result[256];

        ent = g_entities + clientNum;

        trap_GetUserinfo(clientNum, userinfo, sizeof(userinfo));

        s = Info_ValueForKey(userinfo, "rate");
        ent->client->ospClient->rate = atoi(s);

        s = Info_ValueForKey(userinfo, "snaps");
        ent->client->ospClient->snaps = atoi(s);

        s = Info_ValueForKey(userinfo, "osp_drawTeamOverlay");
        if (atoi(s))
        {
            ent->client->ospClient->teamOverlay = qtrue;
        }
        else
        {
            ent->client->ospClient->teamOverlay = qfalse;
        }

        s = Info_ValueForKey(userinfo, "cg_ospClient");
        if (atof(s) > 0.0f)
            ent->client->ospClient->clientVersion = atof(s);
        else
            ent->client->ospClient->clientVersion = 0.0;

        s = Info_ValueForKey(userinfo, "osp_useKeyForLadders");
        if (atoi(s))
        {
            ent->client->ospClient->useKeyForLadders = qtrue;
        }
        else
        {
            ent->client->ospClient->useKeyForLadders = qfalse;
        }

        s = Info_ValueForKey(userinfo, "osp_oldScoreboard");
        if (s[0] && s[0] == '0')
        {
            ent->client->ospClient->ospScoreboard = qtrue;
        }
        else
        {
            ent->client->ospClient->ospScoreboard = qfalse;
        }

        if (ent->client->sess.team == TEAM_SPECTATOR)
        {
            if (ent->client->ospClient->specinvites & 0x1)
                specinviteRed = qtrue;
            else
                specinviteRed = qfalse;

            if (ent->client->ospClient->specinvites & 0x2)
                specinviteBlue = qtrue;
            else
                specinviteBlue = qfalse;
        }

        if (OSP_IsClientAdmin(clientNum, qfalse))
        {
            grade |= 1;
        }

        if (OSP_IsClientReferee(clientNum))
        {
            grade |= 2;
        }

        if (OSP_IsClientCaptain(clientNum))
        {
            grade |= 4;
        }

        Com_sprintf(result, sizeof(result),
                    "\\sir\\%d\\sib\\%d\\grade\\%d",
                    specinviteRed,
                    specinviteBlue,
                    grade);

        // [Cas]
        // The compiler gives us a warning for doing this, and for good reason.
        // This is bad and should never, ever be done. But yeah, apparently this
        // is how OSP does it works OK for the Q3VM.
        return result;
    }
}

// osp: 0x0002b014
void OSP_ClientSpawn(int clientNum)
{
    gclient_t *cl;

    cl = &level.clients[clientNum];

    cl->ospStats = &osp.stats[clientNum];
    cl->ospClient = &osp.clients[clientNum];

    if (cl->sess.team == TEAM_SPECTATOR)
    {
        cl->inactivityTime = level.time + (1000 * g_spectatorInactivity.integer);
    }

    if (osp.paused)
    {
        VectorCopy(level.clients[clientNum].ps.viewangles, osp.x646e70[clientNum]);
    }

    level.clients[clientNum].ospClient->bbqTime = 0;
}

// osp 0x0002b071
void OSP_ClientDisconnect(int clientNum)
{
    trap_GT_SendEvent(GTEV_CLIENT_DEATH, level.time, g_entities[clientNum].s.number, g_entities[clientNum].client->sess.team, -1, -1, 0);

    OSP_ClearCachedGuid(clientNum);

    if (osp.paused)
        VectorClear(osp.x646e70[clientNum]);

    if (osp.clients[clientNum].captain && (level.clients[clientNum].sess.team == TEAM_RED || level.clients[clientNum].sess.team == TEAM_BLUE))
    {
        int i;
        gentity_t *ent;

        for (i = 0; i < level.maxclients; i++)
        {
            ent = g_entities + i;

            if (i == clientNum)
            {
                continue;
            }

            if (ent->client->pers.connected != CON_CONNECTED)
            {
                continue;
            }

            if (ent->client->sess.team == level.clients[clientNum].sess.team)
            {
                ent->client->ospClient->captain = qtrue;
                ClientUserinfoChanged(i);
                trap_SendServerCommand(-1, va("print \"%s is now the captain of the %s team.\n\"", ent->client->pers.netname, (ent->client->sess.team == TEAM_RED) ? "^1RED^7" : "^4BLUE"));
                break;
            }
        }
    }
}

// osp: 0x0002b16b
qboolean OSP_ClientCleanName(const char *in, char *out, int outSize, qboolean colors, qboolean x5c)
{
    int len;
    int colorlessLen;
    char ch;
    char *p;
    int spaces;
    qboolean x28;

    x28 = qfalse;

    //save room for trailing null byte
    outSize--;

    len = 0;
    colorlessLen = 0;
    p = out;
    *p = 0;
    spaces = 0;

    while (1)
    {
        ch = *in++;
        if (!ch)
        {
            break;
        }

        // don't allow leading spaces
        if (!*p && ch == ' ')
        {
            continue;
        }

        // check colors
        if (ch == Q_COLOR_ESCAPE)
        {
            // solo trailing carat is not a color prefix
            if (!*in)
            {
                break;
            }

            if (*in != Q_COLOR_ESCAPE)
            {
                char x30;
                x30 = *in;

                // don't allow black in a name, period
                if ((!colors && !g_coloredTeamplayNames.integer) || ColorIndex(x30) == 0)
                {
                    in++;
                    continue;
                }

                if (g_coloredTeamplayNames.integer == 1 && x5c == TEAM_RED)
                {
                    if (x30 == COLOR_BLUE)
                    {
                        x30 = COLOR_RED;
                        x28 = qtrue;
                    }
                    else if (x30 == COLOR_CYAN)
                    {
                        x30 = COLOR_YELLOW;
                        x28 = qtrue;
                    }
                    else if (x30 != COLOR_WHITE || x30 > COLOR_WHITE || x30 < COLOR_RED)
                    {
                        x30 = COLOR_RED;
                        x28 = qtrue;
                    }
                }

                if (g_coloredTeamplayNames.integer == 1 && x5c == TEAM_BLUE)
                {
                    if (x30 == COLOR_RED)
                    {
                        x30 = COLOR_BLUE;
                        x28 = qtrue;
                    }
                    else if (x30 == COLOR_YELLOW)
                    {
                        x30 = COLOR_CYAN;
                        x28 = qtrue;
                    }
                    else if (x30 != COLOR_WHITE || x30 > COLOR_WHITE || x30 < COLOR_RED)
                    {
                        x30 = COLOR_BLUE;
                        x28 = qtrue;
                    }
                }

                // make sure room in dest for both chars
                if (len > outSize - 2)
                {
                    break;
                }

                *out++ = ch;
                *out++ = x30;
                in++;
                len += 2;
                continue;
            }
            else
            {
                *out++ = ch;
                *out++ = ch;
                in++;
                continue;
            }
        }

        // don't allow too many consecutive spaces
        if (ch == ' ')
        {
            spaces++;
            if (spaces > 3)
            {
                continue;
            }
        }
        else
        {
            spaces = 0;
        }

        if (len > outSize - 1)
        {
            break;
        }

        *out++ = ch;
        colorlessLen++;
        len++;
    }

    *out = 0;

    // Trim whitespace off the end of the name
    for (out--; out >= p && (*out == ' ' || *out == '\t'); out--)
    {
        *out = 0;
    }

    if (strlen(out) < outSize - 2)
    {
        strcat(out, S_COLOR_WHITE);
    }
    else
    {
        out[outSize] = '\0';
        out[outSize - 1] = COLOR_WHITE;
        out[outSize - 2] = Q_COLOR_ESCAPE;
    }

    // don't allow empty names
    if (*p == 0 || colorlessLen == 0)
    {
        Q_strncpyz(p, "UnnamedPlayer", outSize);
    }

    return x28;
}

// osp: 0x0002b374
static void OSP_CleanString(char *in, char *out)
{
    while (*in)
    {
        if (*in == 0x1b)
        {
            in += 2;
        }
        else if (*in < ' ')
        {
            in++;
        }
        else
        {
            *out++ = tolower(*in++);
        }
    }

    *out = 0;
}

// osp: 0x0002b3c6
int OSP_ClientForString(gentity_t *e, char *s)
{
    gclient_t *cl;
    int clientNum;
    char name[1024];
    char needle[1024];
    gentity_t *ent;

    ent = e;

    if ((s[0] >= '0' && s[0] <= '9') || (s[1] && s[0] == ' ' && s[1] >= '0' && s[1] <= '9'))
    {
        clientNum = atoi(s);
        if (clientNum < 0 || clientNum >= level.maxclients)
        {
            trap_SendServerCommand(ent - g_entities, va("print \"Bad client slot: %i\n\"", clientNum));
            return -1;
        }

        cl = &level.clients[clientNum];
        if (cl->pers.connected != CON_CONNECTED)
        {
            trap_SendServerCommand(ent - g_entities, va("print \"Client %i is not active\n\"", clientNum));
            return -1;
        }

        return clientNum;
    }

    OSP_CleanString(s, needle);

    // check for a name match
    for (clientNum = 0, cl = level.clients; clientNum < level.maxclients; clientNum++, cl++)
    {
        if (cl->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        OSP_CleanString(cl->pers.netname, name);

        if (strcmp(name, needle) == 0)
        {
            return clientNum;
        }
    }

    trap_SendServerCommand(ent - g_entities, va("print \"User %s is not on the server\n\"", s));
    return -1;
}

// osp: 0x0002b4b4
void sub_0002b4b4(team_t team)
{
    static int x3a74 = 0;
    int i;

    if (x3a74 > level.time)
    {
        return;
    }

    x3a74 = level.time + 1000;

    osp.x646e3c[team][0] = 0;
    osp.x646e3c[team][1] = 0;

    for (i = 0; i < level.maxclients; i++)
    {
        gentity_t *ent;

        ent = &g_entities[i];

        if (level.clients[i].pers.connected != CON_CONNECTED)
        {
            continue;
        }

        if (level.clients[i].sess.team != team)
        {
            continue;
        }

        if (i < 32)
            osp.x646e3c[team][0] |= (1 << i);
        else
            osp.x646e3c[team][1] |= (1 << (i - 32));
    }
}

// osp: 0x0002b537
int OSP_ClientFlags(int clientNum)
{
    int result = 0;

    if (osp.clients[clientNum].captain)
    {
        result |= 0x1;
    }

    if (osp.clients[clientNum].admin)
    {
        result |= 0x2;
    }

    if (osp.clients[clientNum].adminPowerSuspended || sv_enableAdminPowers.integer == 0)
    {
        result |= 0x4;
    }

    if (osp.clients[clientNum].referee)
    {
        result |= 0x8;
    }

    if (osp.clients[clientNum].subbing)
    {
        result |= 0x10;
    }

    return result;
}

int osp_3a78[] = {
    0,  // WP_NONE
    30, // WP_KNIFE
    25, // WP_M1911A1_PISTOL
    25, // WP_USSOCOM_PISTOL
    25, // WP_SILVER_TALON
    30, // WP_M590_SHOTGUN
    35, // WP_MICRO_UZI_SUBMACHINEGUN
    75, // WP_M3A1_SUBMACHINEGUN
    75, // WP_MP5
    30, // WP_USAS_12_SHOTGUN
    75, // WP_M4_ASSAULT_RIFLE
    75, // WP_AK74_ASSAULT_RIFLE
    75, // WP_SIG551
    80, // WP_MSG90A1
    40, // WP_M60_MACHINEGUN
    10, // WP_MM1_GRENADE_LAUNCHER
    10, // WP_RPG7_LAUNCHER
    0,  // WP_M84_GRENADE
    25, // WP_SMOHG92_GRENADE
    25, // WP_ANM14_GRENADE
    0,  // WP_M15_GRENADE
    25, // WP_USSOCOM_SILENCED
    75, // WP_OICW
    20, // WP_MDN11_GRENADE
    20, // WP_F1_GRENADE
    20, // WP_L2A2_GRENADE
    20, // WP_M67_GRENADE
};

// osp: 0x0002b5a3
float sub_0002b5a3(int clientNum)
{
    weapon_t weapon;
    ospStats_t *stats;
    int x10;
    int x14;
    float x18;
    float x1c;
    float x20;
    float x24;
    float x28;

    x10 = 0;
    x14 = 0;
    x20 = 0;
    x18 = 0;
    x1c = 0;
    x24 = 0;
    x28 = 0;

    if (level.clients[clientNum].pers.connected != CON_CONNECTED)
    {
        return 0;
    }

    stats = &osp.stats[clientNum];

    if (!stats)
    {
        return 0;
    }

    if (stats->x2c < 600000)
    {
        return 0;
    }

    if (stats->kills < 5)
    {
        return 0;
    }

    if (stats->x24 > level.time && !level.intermissiontime)
    {
        return stats->x28;
    }

    for (weapon = WP_KNIFE; weapon < WP_NUM_WEAPONS; weapon++)
    {
        if (stats->weapon[ATTACK_NORMAL][weapon].shots > weaponData[weapon].attack[ATTACK_NORMAL].clipSize)
        {
            x1c += ((float)(stats->weapon[ATTACK_NORMAL][weapon].hits + stats->weapon[ATTACK_ALTERNATE][weapon].hits) / (stats->weapon[ATTACK_NORMAL][weapon].shots + stats->weapon[ATTACK_ALTERNATE][weapon].shots)) * osp_3a78[weapon];
            x10 += osp_3a78[weapon];
        }

        if (stats->weapon[ATTACK_NORMAL][weapon].shots > weaponData[weapon].attack[ATTACK_NORMAL].clipSize && (stats->weapon[ATTACK_NORMAL][weapon].headhits || stats->weapon[ATTACK_ALTERNATE][weapon].headhits))
        {
            x18 += ((float)(stats->weapon[ATTACK_NORMAL][weapon].headhits + stats->weapon[ATTACK_ALTERNATE][weapon].headhits) / (stats->weapon[ATTACK_NORMAL][weapon].shots + stats->weapon[ATTACK_ALTERNATE][weapon].shots)) * osp_3a78[weapon];
            x14 += osp_3a78[weapon];
        }
    }

    if (x10)
    {
        x1c /= x10;
        x1c *= 50.0;
        x20 += x1c;
    }

    if (x14)
    {
        x18 /= x14;
        x18 *= 10.0;
        x20 += x18;
    }

    x24 = ((float)((stats->kills + 1) * stats->damageGiven)) /
          (((stats->teamKills + 1) * stats->teamDamage) +
           ((stats->kills + 1) * stats->damageGiven) +
           ((stats->deaths + 1) * stats->damageTaken));
    x24 *= 30.0;
    x20 += x24;

    if (osp.x6479ac)
    {
        x28 = (float)level.clients[clientNum].ps.persistant[PERS_SCORE] / osp.x6479ac;
        x28 *= 10.0;
        x20 += x28;
    }

    stats->x28 = x20;
    stats->x24 = level.time + 2500 + (rand() % 250);

    if (stats->x28 < 0)
    {
        stats->x28 = 0;
    }

    return stats->x28;
}
