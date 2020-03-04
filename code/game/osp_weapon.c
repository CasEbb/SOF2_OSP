// osp_weapon.c
//
#include "g_local.h"

void G_MissileImpact(gentity_t *ent, trace_t *trace);

char *osp_alternateAttackNames[WP_NUM_WEAPONS] = {
    "No Weapon", // WP_NONE
    "Thrown",    // WP_KNIFE
    "Whip",      // WP_M1911A1_PISTOL
    "Whip",      // WP_USSOCOM_PISTOL
    "Whip",      // WP_SILVER_TALON
    "Bash",      // WP_M590_SHOTGUN
    "",          // WP_MICRO_UZI_SUBMACHINEGUN
    "",          // WP_M3A1_SUBMACHINEGUN
    "",          // WP_MP5
    "",          // WP_USAS_12_SHOTGUN
    "M203",      // WP_M4_ASSAULT_RIFLE
    "Bayonnet",  // WP_AK74_ASSAULT_RIFLE
    "",          // WP_SIG551
    "",          // WP_MSG90A1
    "",          // WP_M60_MACHINEGUN
    "",          // WP_MM1_GRENADE_LAUNCHER
    "",          // WP_RPG7_LAUNCHER
    "Lobbed",    // WP_M84_GRENADE
    "Lobbed",    // WP_SMOHG92_GRENADE
    "Lobbed",    // WP_ANM14_GRENADE
    "Lobbed",    // WP_M15_GRENADE
    "Whip",      // WP_USSOCOM_SILENCED
    "20mm",      // WP_OICW
    "Lobbed",    // WP_MDN11_GRENADE
    "Lobbed",    // WP_F1_GRENADE
    "Lobbed",    // WP_L2A2_GRENADE
    "Lobbed",    // WP_M67_GRENADE
};

char *osp_shortWeaponNames[WP_NUM_WEAPONS] = {
    "No Weapon", // WP_NONE,
    "Knif",      // WP_KNIFE,
    "M19",       // WP_M1911A1_PISTOL,
    "SOCM",      // WP_US_SOCOM_PISTOL,
    "STal",      // WP_SILVER_TALON,
    "M590",      // WP_M590_SHOTGUN,
    "MUzi",      // WP_MICRO_UZI_SUBMACHINEGUN,
    "M3A1",      // WP_M3A1_SUBMACHINEGUN,
    "MP5",       // WP_MP5
    "USAS",      // WP_USAS_12_SHOTGUN,
    "M4",        // WP_M4_ASSAULT_RIFLE,
    "AK74",      // WP_AK74_ASSAULT_RIFLE,
    "SIG",       // WP_SIG551
    "MSG",       // WP_MSG90A1_SNIPER_RIFLE,
    "M60",       // WP_M60_MACHINEGUN,
    "MM1",       // WP_MM1_GRENADE_LAUNCHER,
    "RPG7",      // WP_RPG7_LAUNCHER,
    "M84",       // WP_M84_GRENADE,
    "SMOH",      // WP_SMOHG92_GRENADE,
    "ANM",       // WP_ANM14_GRENADE,
    "M15",       // WP_M15_GRENADE,
    "SSOC",      // WP_USSOCOM_SILENCED,
    "OICW",      // WP_OICW,
    "MDN11",     // WP_MDN11_GRENADE,
    "F1",        // WP_F1_GRENADE,
    "L2A2",      // WP_L2A2_GRENADE,
    "M67",       // WP_M67_GRENADE,
};

int osp_modToWeapon[][2] = {
    {MOD_UNKNOWN, WP_NONE},
    {MOD_KNIFE, WP_KNIFE},
    {MOD_M1911A1_PISTOL, WP_M1911A1_PISTOL},
    {MOD_USSOCOM_PISTOL, WP_USSOCOM_PISTOL},
    {MOD_SILVER_TALON, WP_SILVER_TALON},
    {MOD_M590_SHOTGUN, WP_M590_SHOTGUN},
    {MOD_MICRO_UZI_SUBMACHINEGUN, WP_MICRO_UZI_SUBMACHINEGUN},
    {MOD_M3A1_SUBMACHINEGUN, WP_M3A1_SUBMACHINEGUN},
    {MOD_MP5, WP_MP5},
    {MOD_USAS_12_SHOTGUN, WP_USAS_12_SHOTGUN},
    {MOD_M4_ASSAULT_RIFLE, WP_M4_ASSAULT_RIFLE},
    {MOD_AK74_ASSAULT_RIFLE, WP_AK74_ASSAULT_RIFLE},
    {MOD_SIG551, WP_SIG551},
    {MOD_MSG90A1_SNIPER_RIFLE, WP_MSG90A1},
    {MOD_M60_MACHINEGUN, WP_M60_MACHINEGUN},
    {MOD_MM1_GRENADE_LAUNCHER, WP_MM1_GRENADE_LAUNCHER},
    {MOD_RPG7_LAUNCHER, WP_RPG7_LAUNCHER},
    {MOD_M84_GRENADE, WP_M84_GRENADE},
    {MOD_SMOHG92_GRENADE, WP_SMOHG92_GRENADE},
    {MOD_ANM14_GRENADE, WP_ANM14_GRENADE},
    {MOD_M15_GRENADE, WP_M15_GRENADE},
    {MOD_WATER, WP_NONE},
    {MOD_CRUSH, WP_NONE},
    {MOD_TELEFRAG, WP_NONE},
    {MOD_FALLING, WP_NONE},
    {MOD_SUICIDE, WP_NONE},
    {MOD_TEAMCHANGE, WP_NONE},
    {MOD_TARGET_LASER, WP_NONE},
    {MOD_TRIGGER_HURT, WP_NONE},
    {MOD_TRIGGER_HURT_NOSUICIDE, WP_NONE},
    {MOD_USSOCOM_SILENCED, WP_USSOCOM_SILENCED},
    {MOD_OICW, WP_OICW},
    {MOD_MDN11_GRENADE, WP_MDN11_GRENADE},
    {MOD_F1_GRENADE, WP_F1_GRENADE},
    {MOD_L2A2_GRENADE, WP_L2A2_GRENADE},
    {MOD_M67_GRENADE, WP_M67_GRENADE},
};

static int x4ee4 = 36;

int x4ee8[WP_NUM_WEAPONS][2] = {
    {WP_NONE, 0},
    {WP_KNIFE, 3},
    {WP_M1911A1_PISTOL, 3},
    {WP_USSOCOM_PISTOL, 3},
    {WP_SILVER_TALON, 3},
    {WP_M590_SHOTGUN, 3},
    {WP_MICRO_UZI_SUBMACHINEGUN, 3},
    {WP_M3A1_SUBMACHINEGUN, 3},
    {WP_MP5, 3},
    {WP_USAS_12_SHOTGUN, 3},
    {WP_M4_ASSAULT_RIFLE, 1},
    {WP_AK74_ASSAULT_RIFLE, 3},
    {WP_SIG551, 3},
    {WP_MSG90A1, 3},
    {WP_M60_MACHINEGUN, 3},
    {WP_MM1_GRENADE_LAUNCHER, 0},
    {WP_RPG7_LAUNCHER, 0},
    {WP_M84_GRENADE, 0},
    {WP_SMOHG92_GRENADE, 0},
    {WP_ANM14_GRENADE, 0},
    {WP_M15_GRENADE, 0},
    {WP_USSOCOM_SILENCED, 3},
    {WP_OICW, 1},
    {WP_MDN11_GRENADE, 0},
    {WP_F1_GRENADE, 0},
    {WP_L2A2_GRENADE, 0},
    {WP_M67_GRENADE, 0},
};

// osp: 0x00034c20
weapon_t OSP_ModToWeapon(int mod)
{
    mod = mod & 0xff;

    if (mod > MOD_TRIGGER_HURT_NOSUICIDE || mod < MOD_UNKNOWN)
        return 0;

    return osp_modToWeapon[mod][1];
}

// osp: 0x00034c41
qboolean OSP_HeadshotPossible(int mod, attackType_t attack)
{
    weapon_t i;
    weapon_t weapon;

    weapon = OSP_ModToWeapon(mod);

    for (i = 0; i < WP_NUM_WEAPONS; i++)
    {
        if (x4ee8[i][0] == weapon)
        {
            switch (x4ee8[i][1])
            {
            case 0:
                return qfalse;
            case 1:
                if (attack == ATTACK_ALTERNATE)
                    return qfalse;
                else
                    return qtrue;
            case 2:
                if (attack == ATTACK_ALTERNATE)
                    return qtrue;
                else
                    return qfalse;
            case 3:
                return qtrue;
            }

            break;
        }
    }

    return qfalse;
}

// osp: 0x00034ca1
void OSP_LogShot(gentity_t *e, int attack)
{
    gentity_t *ent;

    ent = e;

    if (ent && ent->client->ospStats && attack != ATTACK_ALTERNATE)
    {
        if (ent->s.weapon == WP_M590_SHOTGUN || ent->s.weapon == WP_USAS_12_SHOTGUN)
            ent->client->ospStats->weapon[ATTACK_NORMAL][ent->s.weapon].shots += 8;
        else
            ent->client->ospStats->weapon[ATTACK_NORMAL][ent->s.weapon].shots++;
    }
    else
    {
        ent->client->ospStats->weapon[ATTACK_ALTERNATE][ent->s.weapon].shots++;
    }
}

// osp: 0x00034d26
int OSP_GetPistol(gentity_t *e)
{
    gentity_t *ent = e;
    return bg_itemlist[bg_outfittingGroups[OUTFITTING_GROUP_PISTOL][ent->client->pers.outfitting.items[OUTFITTING_GROUP_PISTOL]]].giTag;
}

// osp: 0x00034d40
int OSP_GetPrimaryWeapon(gentity_t *e)
{
    gentity_t *ent = e;
    return bg_itemlist[bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][ent->client->pers.outfitting.items[OUTFITTING_GROUP_PRIMARY]]].giTag;
}

// osp: 0x00034d5a
int OSP_GetSecondaryWeapon(gentity_t *e)
{
    gentity_t *ent = e;
    return bg_itemlist[bg_outfittingGroups[OUTFITTING_GROUP_SECONDARY][ent->client->pers.outfitting.items[OUTFITTING_GROUP_SECONDARY]]].giTag;
}

// osp: 0x00034d74
int OSP_GetGrenade(gentity_t *e)
{
    gentity_t *ent = e;
    return bg_itemlist[bg_outfittingGroups[OUTFITTING_GROUP_GRENADE][ent->client->pers.outfitting.items[OUTFITTING_GROUP_GRENADE]]].giTag;
}

// osp: 0x00034d8e
float OSP_Accuracy(ospStats_t *stats)
{
    int i;
    int hits;
    int shots;

    hits = 0;
    shots = 0;

    for (i = 1; i < 6; i++)
    {
        hits += stats->x44[i];
    }

    shots = OSP_NumShots(stats);

    if (shots)
    {
        if (shots < 30)
        {
            return -1.0;
        }
        else
        {
            return (float)hits / shots;
        }
    }

    return -1.0;
}

// osp: 0x00034dd6
int OSP_AccuracyInt(ospStats_t *stats)
{
    int weapon;
    int shots;
    ospWeaponStats_t *primaryAttack;
    int hits;
    ospWeaponStats_t *altAttack;

    if (!stats)
    {
        return 0;
    }

    weapon = OSP_GetPrimaryWeapon(&g_entities[stats - osp.stats]);
    primaryAttack = &stats->weapon[ATTACK_NORMAL][weapon];
    altAttack = &stats->weapon[ATTACK_ALTERNATE][weapon];

    hits = primaryAttack->hits;
    shots = primaryAttack->shots;

    if (weapon == WP_MSG90A1 || weapon == WP_SIG551)
    {
        hits += altAttack->hits;
        shots += altAttack->shots;
    }

    return (shots > 0) ? ((float)hits / shots) * 1000.0 : 0;
}

// osp: 0x00034e49
static float sub_00034e49(int clientNum, int minHits, int minShots)
{
    weapon_t weapon;
    ospStats_t *stats;
    int shots;
    int hits;
    ospWeaponStats_t *primaryAttack;
    ospWeaponStats_t *altAttack;

    stats = osp.stats + clientNum;

    if (!stats)
    {
        return -1.0;
    }

    weapon = OSP_GetPrimaryWeapon(&g_entities[stats - osp.stats]);
    primaryAttack = &stats->weapon[ATTACK_NORMAL][weapon];
    altAttack = &stats->weapon[ATTACK_ALTERNATE][weapon];

    hits = primaryAttack->hits;
    shots = primaryAttack->shots;

    if (hits < minHits)
    {
        return -1.0;
    }

    if (shots < minShots)
    {
        return -1.0;
    }

    if (weapon == WP_MSG90A1 || weapon == WP_SIG551)
    {
        hits += altAttack->hits;
        shots += altAttack->shots;
    }

    return (shots > 0) ? (float)hits / shots : -1.0;
}

// osp: 0x00034ed4
static int sub_00034ed4(ospStats_t *stats)
{
    weapon_t weapon;
    int hits;
    ospWeaponStats_t *primaryAttack;
    int headhits;
    ospWeaponStats_t *altAttack;

    if (!stats)
    {
        return 0;
    }

    weapon = OSP_GetPrimaryWeapon(&g_entities[stats - osp.stats]);
    primaryAttack = &stats->weapon[ATTACK_NORMAL][weapon];
    altAttack = &stats->weapon[ATTACK_ALTERNATE][weapon];

    hits = primaryAttack->hits;
    headhits = primaryAttack->headhits;

    if (weapon == WP_MSG90A1 || weapon == WP_SIG551)
    {
        hits += altAttack->hits;
        headhits += altAttack->headhits;
    }

    return (hits > 0) ? 1000.0 * ((float)headhits / hits) : 0.0;
}

// osp: 0x00034f4b
static void sub_00034f4b(trace_t *tr, G2Trace_t G2Trace, vec3_t start, vec3_t end, vec3_t mins, vec3_t maxs, int passEntityNum, int contentmask)
{
    int unlinkCount;
    gentity_t *unlinked[64];

    unlinkCount = 0;
    G2Trace[0].mEntityNum = -1;

    while (1)
    {
        trap_Trace(tr, start, mins, maxs, end, passEntityNum, contentmask);

        if (tr->fraction != 1 && !Q_stricmp(g_entities[tr->entityNum].classname, "func_glass"))
        {
            g_entities[tr->entityNum].use(&g_entities[tr->entityNum], &g_entities[tr->entityNum], &g_entities[tr->entityNum]);
            continue;
        }

        if (tr->fraction != 1 && g_entities[tr->entityNum].client)
        {
            gentity_t *traceEnt;
            animation_t *anim;
            float dist;
            vec3_t x150;
            vec3_t x15c;
            trace_t vtr;

            {
                vec3_t diff;

                VectorSubtract(end, start, diff);
                dist = VectorLength(diff);
                VectorMA(start, -50.0, diff, x150);
                VectorMA(end, 50.0, diff, x15c);
            }

            traceEnt = &g_entities[tr->entityNum];

            anim = &level.ghoulAnimations[traceEnt->client->legs.anim & (~ANIM_TOGGLEBIT)];
            trap_G2API_SetBoneAnim(level.serverGhoul2, 0, "model_root", anim->firstFrame, anim->firstFrame + anim->numFrames, BONE_ANIM_OVERRIDE_LOOP, 50.0f / anim->frameLerp, traceEnt->client->legs.animTime, -1, 0);

            anim = &level.ghoulAnimations[traceEnt->client->torso.anim & (~ANIM_TOGGLEBIT)];
            trap_G2API_SetBoneAnim(level.serverGhoul2, 0, "lower_lumbar", anim->firstFrame, anim->firstFrame + anim->numFrames, BONE_ANIM_OVERRIDE_LOOP, 50.0f / anim->frameLerp, traceEnt->client->torso.animTime, -1, 0);

            trap_G2API_SetBoneAngles(level.serverGhoul2, 0, "upper_lumbar", traceEnt->client->ghoulUpperTorsoAngles, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, 0, 0, level.time);
            trap_G2API_SetBoneAngles(level.serverGhoul2, 0, "lower_lumbar", traceEnt->client->ghoulLowerTorsoAngles, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, 0, 0, level.time);
            trap_G2API_SetBoneAngles(level.serverGhoul2, 0, "cranium", traceEnt->client->ghoulHeadAngles, BONE_ANGLES_POSTMULT, POSITIVE_Z, NEGATIVE_Y, POSITIVE_X, 0, 0, level.time);

            {
                vec3_t scale;
                VectorSet(scale, 1.2, 1.2, 1.2);
                trap_G2API_CollisionDetect(G2Trace, level.serverGhoul2, traceEnt->client->ghoulLegsAngles, traceEnt->r.currentOrigin, level.time, traceEnt->s.number, x150, x15c, scale, 0, 0);
            }

            if (G2Trace[0].mEntityNum == -1)
            {
                unlinked[unlinkCount++] = traceEnt;
                trap_UnlinkEntity(traceEnt);
                continue;
            }

            if (G2Trace[0].mDistance < (50.0 * dist) && G2Trace[0].mDistance >= ((50.0 * dist) + dist))
            {
                unlinked[unlinkCount++] = traceEnt;
                trap_UnlinkEntity(traceEnt);
                continue;
            }

            trap_UnlinkEntity(traceEnt);

            trap_Trace(&vtr, tr->endpos, NULL, NULL, G2Trace[0].mCollisionPosition, passEntityNum, MASK_SHOT & (~CONTENTS_BODY));

            if (vtr.entityNum != G2Trace[0].mEntityNum)
            {
                if (vtr.entityNum != ENTITYNUM_NONE)
                {
                    unlinked[unlinkCount++] = traceEnt;
                    continue;
                }
            }

            trap_LinkEntity(traceEnt);
        }

        break;
    }

    while (unlinkCount > 0)
    {
        trap_LinkEntity(unlinked[--unlinkCount]);
    }
}

// osp: 0x00035268
static void sub_00035268(void)
{
    int i;

    for (i = 0; i < level.numConnectedClients; i++)
    {
        gentity_t *ent = &g_entities[level.sortedClients[i]];

        if (ent->client->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        if (!ent->inuse)
        {
            continue;
        }

        if (G_IsClientSpectating(ent->client) || G_IsClientDead(ent->client))
        {
            continue;
        }

        ent->client->minSave[0] = ent->r.mins[0];
        ent->client->minSave[1] = ent->r.mins[1];

        ent->client->maxSave[0] = ent->r.maxs[0];
        ent->client->maxSave[1] = ent->r.maxs[1];
        ent->client->maxSave[2] = ent->r.maxs[2];

        if (ent->client->ps.pm_flags & PMF_DUCKED)
        {
            ent->r.maxs[2] += 10.0;
        }

        if (ent->client->ps.pm_flags & PMF_LEANING)
        {
            ent->r.maxs[0] *= 3.0;
            ent->r.maxs[1] *= 3.0;
            ent->r.mins[0] *= 3.0;
            ent->r.mins[1] *= 3.0;
        }
        else
        {
            ent->r.maxs[0] *= 2.0;
            ent->r.maxs[1] *= 2.0;
            ent->r.mins[0] *= 2.0;
            ent->r.mins[1] *= 2.0;
        }

        ent->r.svFlags |= SVF_INFLATED_BBOX;
        trap_LinkEntity(ent);
    }
}

// osp: 0x000353a8
static void sub_000353a8(void)
{
    int i;

    for (i = 0; i < level.numConnectedClients; i++)
    {
        gentity_t *ent = &g_entities[level.sortedClients[i]];

        if (ent->client->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        if (G_IsClientSpectating(ent->client) || G_IsClientDead(ent->client))
        {
            continue;
        }

        if (ent->r.svFlags & SVF_INFLATED_BBOX)
        {
            ent->r.maxs[0] = ent->client->maxSave[0];
            ent->r.maxs[1] = ent->client->maxSave[1];
            ent->r.maxs[2] = ent->client->maxSave[2];

            ent->r.mins[0] = ent->client->minSave[0];
            ent->r.mins[1] = ent->client->minSave[1];

            ent->r.svFlags &= (~SVF_INFLATED_BBOX);
        }

        trap_LinkEntity(ent);
    }
}

// osp: 0x00035450
void OSP_RunMissile(gentity_t *e)
{
    gentity_t *ent;
    vec3_t origin;
    trace_t tr;
    int passent;
    G2Trace_t G2Trace;

    ent = e;

    BG_EvaluateTrajectory(&ent->s.pos, level.time, origin);

    if (ent->target_ent)
    {
        passent = ent->target_ent->s.number;
    }
    else
    {
        passent = ent->r.ownerNum;
    }

    if (ent->s.eFlags & EF_INSKY)
    {
        if (origin[0] < level.worldMins[0] ||
            origin[1] < level.worldMins[1] ||
            origin[0] > level.worldMaxs[0] ||
            origin[1] > level.worldMaxs[1] ||
            origin[2] < level.worldMins[2])
        {
            G_FreeEntity(ent);
            return;
        }

        if (origin[2] > level.worldMaxs[2] && ent->s.pos.trType != TR_GRAVITY && ent->s.pos.trType != TR_LIGHTGRAVITY)
        {
            G_FreeEntity(ent);
            return;
        }

        trap_Trace(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passent, ent->clipmask);

        if (tr.fraction == 1.0f)
        {
            ent->s.eFlags &= ~EF_INSKY;
            VectorCopy(origin, ent->r.currentOrigin);
        }

        VectorCopy(origin, ent->r.currentOrigin);
        trap_LinkEntity(ent);
    }
    else
    {
        vec3_t start;
        vec3_t end;

        VectorCopy(ent->r.currentOrigin, start);
        VectorCopy(origin, end);

        sub_00035268();
        sub_00034f4b(&tr, G2Trace, start, end, ent->r.mins, ent->r.maxs, passent, ent->clipmask);
        VectorCopy(tr.endpos, ent->r.currentOrigin);
        trap_LinkEntity(ent);

        if (tr.fraction != 1.0)
        {
            if ((tr.surfaceFlags & SURF_NOIMPACT) && (tr.surfaceFlags & SURF_SKY))
            {
                ent->s.eFlags |= EF_INSKY;
                ent->r.svFlags |= SVF_BROADCAST;
                VectorCopy(origin, ent->r.currentOrigin);
                trap_LinkEntity(ent);
            }
            else if (tr.entityNum >= 0 && tr.entityNum < MAX_CLIENTS && G2Trace[0].mEntityNum == -1)
            {
                if (tr.fraction != 1.0)
                {
                    G_MissileImpact(ent, &tr);

                    if (ent->s.eFlags & EF_EXPLODE)
                    {
                        ent->s.eFlags &= ~(EF_EXPLODE);
                        G_ExplodeMissile(ent);
                        sub_000353a8();
                        return;
                    }

                    if (ent->s.eType != ET_MISSILE)
                    {
                        sub_000353a8();
                        return;
                    }
                }
                else
                {
                    VectorCopy(origin, ent->r.currentOrigin);
                    trap_LinkEntity(ent);
                }
            }
            else
            {
                G_MissileImpact(ent, &tr);

                if (ent->s.eFlags & EF_EXPLODE)
                {
                    ent->s.eFlags &= (~EF_EXPLODE);
                    G_ExplodeMissile(ent);
                    sub_000353a8();
                    return;
                }

                if (ent->s.eType != ET_MISSILE)
                {
                    sub_000353a8();
                    return;
                }
            }
        }
        else if (
            (osp_modversion.integer >= 2 && ent->s.weapon != WP_ANM14_GRENADE && ent->s.weapon != WP_M15_GRENADE && (ent->s.eFlags & EF_EXPLODE)) ||
            (ent->s.pos.trType == TR_STATIONARY && (ent->s.eFlags & EF_EXPLODE)))
        {
            ent->s.eFlags &= (~EF_EXPLODE);
            G_ExplodeMissile(ent);
            sub_000353a8();
            return;
        }

        sub_000353a8();
    }

    if (ent->s.weapon == WP_KNIFE)
    {
        vec3_t vel;

        BG_EvaluateTrajectoryDelta(&ent->s.pos, level.time, vel);
        vectoangles(vel, ent->s.angles);

        ent->s.angles[YAW] += 90;
        ent->s.angles[ROLL] = 0;
        ent->s.angles[PITCH] = 0;
    }

    G_RunThink(ent);
}

/*
0x024 --
0x028 ent
0x02c origin
0x038 tr
0x070 passent
0x074 G2Trace
    0x474 start
    0x480 end


0x4a8 --
0x4ac --
0x4b0 ent
*/

// osp: 0x0003569a
int OSP_NumHeadshots(ospStats_t *stats)
{
    weapon_t weapon;
    int result;

    result = 0;

    for (weapon = WP_NONE; weapon < WP_NUM_WEAPONS; weapon++)
    {
        result += stats->weapon[ATTACK_NORMAL][weapon].headhits;
        result += stats->weapon[ATTACK_ALTERNATE][weapon].headhits;
    }

    return result;
}

// osp: 0x000356d2
int OSP_ExplosiveKills(ospStats_t *stats)
{
    weapon_t weapon;
    int result;

    result = 0;

    for (weapon = WP_M84_GRENADE; weapon < WP_NUM_WEAPONS; weapon++)
    {
        if (weapon == WP_OICW || weapon == WP_USSOCOM_SILENCED)
        {
            continue;
        }

        result += stats->weapon[ATTACK_NORMAL][weapon].kills;
        result += stats->weapon[ATTACK_ALTERNATE][weapon].kills;
    }

    result += stats->weapon[ATTACK_ALTERNATE][WP_M4_ASSAULT_RIFLE].kills;
    result += stats->weapon[ATTACK_ALTERNATE][WP_OICW].kills;

    return result;
}

// osp: 0x00035728
static int sub_00035728(ospStats_t *stats)
{
    weapon_t weapon;
    int shots;
    int hits;

    shots = 0;
    hits = 0;

    for (weapon = WP_M1911A1_PISTOL; weapon <= WP_OICW; weapon++)
    {
        if (weapon == WP_M84_GRENADE ||
            weapon == WP_SMOHG92_GRENADE ||
            weapon == WP_ANM14_GRENADE ||
            weapon == WP_M15_GRENADE)
        {
            continue;
        }

        shots += stats->weapon[ATTACK_NORMAL][weapon].shots;
        shots += stats->weapon[ATTACK_ALTERNATE][weapon].shots;
        hits += stats->weapon[ATTACK_NORMAL][weapon].hits;
        hits += stats->weapon[ATTACK_ALTERNATE][weapon].hits;
    }

    if (shots > 0)
    {
        return (hits / shots) * 10000;
    }

    return 0;
}

// osp: 0x000357a0
int OSP_NumShots(ospStats_t *stats)
{
    int i;
    int result = 0;

    if (!stats)
        return 0;

    for (i = WP_M1911A1_PISTOL; i <= WP_OICW; i++)
    {
        result += stats->weapon[ATTACK_NORMAL][i].shots;
        result += stats->weapon[ATTACK_ALTERNATE][i].shots;
    }

    return result;
}

// osp: 0x000357dc
int OSP_MeleeKills(ospStats_t *stats)
{
    int result = 0;

    result += stats->weapon[ATTACK_NORMAL][WP_KNIFE].kills;
    result += stats->weapon[ATTACK_ALTERNATE][WP_KNIFE].kills;
    result += stats->weapon[ATTACK_ALTERNATE][WP_USSOCOM_PISTOL].kills;
    result += stats->weapon[ATTACK_ALTERNATE][WP_M1911A1_PISTOL].kills;
    result += stats->weapon[ATTACK_ALTERNATE][WP_SILVER_TALON].kills;
    result += stats->weapon[ATTACK_ALTERNATE][WP_M590_SHOTGUN].kills;
    result += stats->weapon[ATTACK_ALTERNATE][WP_AK74_ASSAULT_RIFLE].kills;

    return result;
}
