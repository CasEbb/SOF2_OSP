// osp_active.c
//
#include "g_local.h"

// osp: 0x000290d5
static void OSP_DoMotd(gentity_t *e)
{
    gentity_t *ent = e;

    if (!ent->client->ospClient->motdTime)
    {
        return;
    }

    if (ent->client->ospClient->motdTime < level.time)
    {
        trap_SendServerCommand(ent - g_entities, va("cp \"@%s\n%s\n%s\n\n%s\n%s\n%s\n%s\n%s\"\0\0", "^GSoldier of Fortune 2 OSP 1.1", "^H1.1 ^I- ^HJuly 13, 2006", server_motd0.string, server_motd1.string, server_motd2.string, server_motd3.string, server_motd4.string, server_motd5.string));
        ent->client->ospClient->motdTime = level.time + 3000;
    }
}

// osp: 0x0002912b
void OSP_DoWarmup(gentity_t *e)
{
    gentity_t *ent = e;

    if (ent->client->ospClient->motdTime)
    {
        return;
    }

    if (ent->client->ospClient->warmupTime < level.time)
    {
        if ((level.gametypeData->teams && ent->client->sess.team != TEAM_RED && ent->client->sess.team != TEAM_BLUE) || (!level.gametypeData->teams && ent->client->sess.team != TEAM_FREE))
        {
            trap_SendServerCommand(ent - g_entities, "cp \"@Waiting for players to be ready\n\"");
        }
        else
        {
            if (ent->client->ospClient->ready)
            {
                trap_SendServerCommand(ent - g_entities, "cp \"@^7You are ^3READY\n^7Type ^3/notready^7 in console to cancel\"");
            }
            else
            {
                trap_SendServerCommand(ent - g_entities, "cp \"@^7You are ^1NOT READY\n^7Type ^3/ready^7 in console to ready up\"");
            }
        }

        ent->client->ospClient->warmupTime = level.time + 3000;
    }
}

// osp: 0x000291c2
void OSP_StopMotd(gentity_t *e)
{
    gentity_t *ent = e;

    ent->client->ospClient->motdTime = 0;
}

// osp: 0x000291d5
static qboolean OSP_ClientInactivity(gentity_t *e)
{
    gentity_t *ent = e;

    if (ent->client->pers.cmd.forwardmove ||
        ent->client->pers.cmd.rightmove ||
        ent->client->pers.cmd.upmove ||
        (ent->client->pers.cmd.buttons & (BUTTON_ATTACK | BUTTON_USE | BUTTON_ALT_ATTACK)))
    {
        ent->client->ospClient->afkTime = level.time;
        return qtrue;
    }

    return qfalse;
}

// osp: 0x0002921e
qboolean OSP_SpectatorInactivity(gentity_t *e)
{
    gclient_t *client;
    gentity_t *ent;

    ent = e;
    client = ent->client;

    if (client->sess.team != TEAM_SPECTATOR)
    {
        return qtrue;
    }

    if (!g_spectatorInactivity.integer || osp.matchPhase == 1 || osp.paused)
    {
        client->inactivityTime = level.time + 60000;
        client->inactivityWarning = qfalse;
    }
    else if (!client->pers.localClient && !OSP_IsClientAdmin(ent - g_entities, qfalse))
    {
        if (level.time > client->inactivityTime)
        {
            trap_DropClient(client - level.clients, va("Disconnected due to spectating for too long (%d seconds)", g_spectatorInactivity.integer));
            ;
            return qfalse;
        }

        if (level.time > client->inactivityTime - 30000 && !client->inactivityWarning)
        {
            client->inactivityWarning = qtrue;
            trap_SendServerCommand(ent - g_entities, "cp \"30 seconds until spectator drop!\n\"");
        }
    }

    return qtrue;
}

// osp: 0x000292be
static void OSP_MoveClientToSpeclock(gentity_t *e)
{
    gentity_t *ent;
    vec3_t origin;
    vec3_t diff;
    vec3_t dir;
    gspawn_t *blueSpawn;
    gspawn_t *redSpawn;
    vec3_t angles;

    ent = e;

    if (!osp.speclockOrigin[0])
    {
        redSpawn = G_SelectRandomSpawnPoint(TEAM_RED);
        blueSpawn = G_SelectRandomSpawnPoint(TEAM_BLUE);

        VectorSubtract(redSpawn->origin, blueSpawn->origin, diff);
        diff[0] /= 2;
        diff[1] /= 2;
        diff[2] /= 2;
        VectorMA(blueSpawn->origin, 1, diff, origin);
        VectorCopy(origin, diff);
        origin[2] += 2000.0;

        // angles
        VectorSet(dir, 0, 0, -1);
        vectoangles(dir, angles);
    }
    else
    {
        // copy origin from speclock
        origin[0] = osp.speclockOrigin[0];
        origin[1] = osp.speclockOrigin[1];
        origin[2] = osp.speclockOrigin[2];
        
        // angles
        VectorSet(dir, 0, 0, -1);
        vectoangles(dir, angles);
        angles[YAW] = osp.speclockYaw;
    }
    

    if (osp.awardsStarted)
    {
        VectorCopy(level.intermission_origin, ent->s.origin);
        VectorCopy(level.intermission_origin, ent->r.currentOrigin);
        VectorCopy(level.intermission_origin, ent->client->ps.pvsOrigin);
        VectorCopy(level.intermission_origin, ent->client->ps.origin);
        SetClientViewAngle(ent, level.intermission_angle);
    }
    else
    {
        VectorCopy(origin, ent->s.origin);
        VectorCopy(origin, ent->r.currentOrigin);
        VectorCopy(origin, ent->client->ps.pvsOrigin);
        VectorCopy(origin, ent->client->ps.origin);
        SetClientViewAngle(ent, angles);
    }
}

// osp: 0x000293e9
void OSP_SpectatorThink(gentity_t *e, int x3c)
{
    gentity_t *ent;
    int x14;
    int x18;

    ent = e;
    x18 = ent->client->ospClient->x74;
    x14 = 0;

    if (!x3c)
    {
        if (OSP_IsClientAdmin(ent - g_entities, qtrue))
        {
            return;
        }

        if (ent->client->sess.ghost && ent->client->sess.team != TEAM_SPECTATOR)
        {
            Cmd_FollowCycle_f(ent, 1);
            return;
        }

        if (ent->client->sess.spectatorState == SPECTATOR_FREE || ent->client->sess.spectatorState == SPECTATOR_SCOREBOARD)
        {
            if (ospTeams[TEAM_RED].speclocked && !(ent->client->ospClient->specinvites & TEAM_RED))
            {
                x14 = 1;
            }

            if (ospTeams[TEAM_BLUE].speclocked && !(ent->client->ospClient->specinvites & TEAM_BLUE))
            {
                x14 = 1;
            }
        }
    }
    else
    {
        x14 = 1;
    }

    if (ent->client->ospClient->x74 && !x14)
    {
        ent->client->ospClient->x74 = 0;
    }
    else if (!ent->client->ospClient->x74 && x14)
    {
        OSP_MoveClientToSpeclock(ent);
        ent->client->ospClient->x74 = 1;
        ent->client->ps.pm_type = PM_SPECTATOR;
    }

    if (ent->client->ospClient->x74 && x18)
    {
        ent->client->ps.pm_type = PM_FREEZE;
    }

    ent->client->ps.stats[STAT_HEALTH] = -1;
}

// osp: 0x00029505
static void sub_00029505(int clientNum)
{
    ospStats_t *stats;

    stats = &osp.stats[clientNum];
    if (!stats)
    {
        return;
    }

    stats->x2c += level.time - level.previousTime;
}

// osp: 0x00029529
void OSP_ClientEndFrame(int clientNum)
{
    if (level.gametypeData->teams && level.clients[clientNum].pers.connected == CON_CONNECTED)
    {
        if (level.clients[clientNum].sess.team == TEAM_RED || level.clients[clientNum].sess.team == TEAM_BLUE)
        {
            sub_00029505(clientNum);
        }
    }

    if (level.clients[clientNum].ospClient->bbqTime && level.clients[clientNum].ospClient->bbqTime < level.time)
    {
        level.clients[clientNum].ospClient->bbqTime = level.time + 1000;
        G_Damage(&g_entities[clientNum],
                 NULL, NULL, NULL, NULL,
                 5,
                 DAMAGE_RADIUS | DAMAGE_NO_ARMOR | DAMAGE_NO_KNOCKBACK | DAMAGE_NO_PROTECTION,
                 MOD_ANM14_GRENADE,
                 HL_NONE);

        if (level.clients[clientNum].ospClient->bbqAnimTime < level.time)
        {
            gentity_t *ent, *tent;
            vec3_t hitdir;

            ent = &g_entities[clientNum];

            tent = G_TempEntity(ent->client->ps.origin, EV_EXPLOSION_HIT_FLESH);
            VectorSet(hitdir, 1, 0, 0);
            tent->s.eventParm = DirToByte(hitdir);
            tent->s.time = WP_ANM14_GRENADE + ((((int)ent->s.apos.trBase[YAW] & 0x7FFF) % 360) << 16);
            tent->r.svFlags = SVF_BROADCAST;
            tent->s.otherEntityNum2 = ent->s.number;
            VectorCopy(ent->r.currentOrigin, tent->s.angles);
            SnapVector(tent->s.angles);

            level.clients[clientNum].ospClient->bbqAnimTime = level.time + 3250;
        }
    }
}
