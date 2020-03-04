// osp_gametype.c
//
#include "g_local.h"

// osp: 0x0002e0d1
static int OSP_GetAliveTeammember(team_t team)
{
    int i;

    for (i = 0; i < level.maxclients; i++)
    {
        if (g_entities[i].client->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        if (g_entities[i].client->sess.team != team)
        {
            continue;
        }

        if (G_IsClientDead(g_entities[i].client))
        {
            continue;
        }

        return i;
    }

    return 0;
}

// osp: 0x0002e11a
void OSP_LastOneAliveWarning(team_t team)
{
    int alive;

    alive = level.teamAliveCount[team];

    if (osp.x646e2c[team] > 1 && alive == 1)
    {
        int clientNum = OSP_GetAliveTeammember(team);
        trap_SendServerCommand(clientNum, "cp \"^3You're the ^3LAST ONE ALIVE ^3on your team!^7\n\"");
        OSP_ClientSound(clientNum, "sound/ambience/generic/alarm01.wav");
        osp.x646e2c[team] = 1;
    }

    if (alive > 1)
    {
        osp.x646e2c[team] = alive;
    }
}

// osp: 0x0002e168
void OSP_MatchSwap(void)
{
    OSP_SwapTeams();
}

// osp: 0x0002e16e
void OSP_MatchShuffle(void)
{
    OSP_ShuffleTeams();
}

// osp: 0x0002e174
void OSP_SelectVIP(gclient_t *client)
{
    int i;
    gclient_t *newVip;
    gclient_t *oldVip;

    oldVip = NULL;
    newVip = client;

    for (i = 0; i < level.maxclients; i++)
    {
        if (level.clients[i].ospClient &&
            level.clients[i].ospClient->vip &&
            level.clients[i].sess.team == newVip->sess.team)
        {
            oldVip = &level.clients[i];
            oldVip->ospClient->vip = qfalse;
            oldVip->ospClient->oldVip = qtrue;
            break;
        }
    }

    newVip->ospClient->vip = qtrue;
    newVip->ospClient->oldVip = qfalse;

    if (newVip != oldVip)
    {
        if (oldVip)
            ClientUserinfoChanged(oldVip - level.clients);

        ClientUserinfoChanged(newVip - level.clients);
    }

    if (newVip)
    {
        int ammoIndex;

        // clear everything
        newVip->ps.stats[STAT_WEAPONS] = 0;
        newVip->ps.stats[STAT_WEAPONS] = (1 << WP_NONE);
        memset(newVip->ps.ammo, 0, sizeof(newVip->ps.ammo));
        memset(newVip->ps.clip, 0, sizeof(newVip->ps.clip));
        newVip->ps.stats[STAT_GOGGLES] = 0;
        newVip->ps.stats[STAT_OSP_E] = -1;

        // give knife
        newVip->ps.stats[STAT_WEAPONS] |= (1 << WP_KNIFE);
        ammoIndex = weaponData[WP_KNIFE].attack[ATTACK_NORMAL].ammoIndex;
        (ammoIndex < MAX_AMMO)
            ? (newVip->ps.ammo[ammoIndex] = 0)
            : (newVip->ps.firemode[MAX_WEAPONS - 1 - (ammoIndex - MAX_AMMO)] = 0);
        newVip->ps.clip[ATTACK_NORMAL][WP_KNIFE] = weaponData[WP_KNIFE].attack[ATTACK_NORMAL].clipSize;

        // give pistol
        newVip->ps.stats[STAT_WEAPONS] |= (1 << WP_USSOCOM_PISTOL);
        ammoIndex = weaponData[WP_USSOCOM_PISTOL].attack[ATTACK_NORMAL].ammoIndex;
        (ammoIndex < MAX_AMMO)
            ? (newVip->ps.ammo[ammoIndex] = 3 * weaponData[WP_USSOCOM_PISTOL].attack[ATTACK_NORMAL].clipSize)
            : (newVip->ps.firemode[MAX_WEAPONS - 1 - (ammoIndex - MAX_AMMO)] = 3 * weaponData[WP_USSOCOM_PISTOL].attack[ATTACK_NORMAL].clipSize);
        newVip->ps.clip[ATTACK_NORMAL][WP_USSOCOM_PISTOL] = weaponData[WP_USSOCOM_PISTOL].attack[ATTACK_NORMAL].clipSize;

        newVip->ps.weapon = WP_USSOCOM_PISTOL;
        newVip->ps.stats[STAT_ARMOR] = 100;
        newVip->noOutfittingChange = qtrue;
    }

    trap_SendServerCommand(-1, "loaddeferred");
}

// osp: 0x0002e2cc
qboolean OSP_CheckMaxM203(gentity_t *e)
{
    int i;
    gentity_t *other;
    int x10;
    gentity_t *ent;

    ent = e;
    x10 = 0;

    if (osp_noM203nades.integer > 0)
    {
        return qfalse;
    }

    if (ent->client->hasM203 == 1 && (ent->client->ps.stats[STAT_WEAPONS] & (1 << WP_M4_ASSAULT_RIFLE)))
    {
        return qtrue;
    }

    for (i = 0; i < 4; i++)
    {
        osp.x6482ec[i] = 0;
    }

    for (i = 0; i < MAX_CLIENTS; i++)
    {
        other = g_entities + i;

        if (!other->client)
        {
            continue;
        }

        if (other->client->hasM203 == 1)
        {
            x10++;
            osp.x6482ec[other->client->sess.team]++;
        }
    }

    if (osp.x6482ec[ent->client->sess.team] < osp_maxM203PerTeam.integer &&
        (ent->client->ps.stats[STAT_WEAPONS] & (1 << WP_M4_ASSAULT_RIFLE)))
    {
        return qtrue;
    }

    return qfalse;
}
