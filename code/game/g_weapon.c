// Copyright (C) 2001-2002 Raven Software
//
// g_weapon.c
// perform the server side effects of a weapon firing

#include "g_local.h"
#include "be_aas.h"

extern qboolean G_BoxInBounds(vec3_t point, vec3_t mins, vec3_t maxs, vec3_t boundsMins, vec3_t boundsMaxs);

// OSP
#define SHOTGUN_JUMP_AIMCHANGE 2.5f

static int last_show = 0;
static int last_showBBox = 0;
// END

typedef struct hitLocationConversion_s
{
    int SOF2poses;
    int hitLocation;
    float damageMultiplier;
} hitLocationConversion_t;

#define HITLOC_HEAD_DM -1.0f
#define HITLOC_NECK_DM -2.0f

#define HL_CONV_MAX 18

hitLocationConversion_t hitLocationConversion[HL_CONV_MAX] = {
    {-1, HL_NONE, 0},             // UNKNOWN
    {0, HL_HEAD, HITLOC_HEAD_DM}, // "HEAD"
    {1, HL_LEG_LOWER_RT, 0.7f},   // "RIGHT LEG"
    {4, HL_NECK, HITLOC_NECK_DM}, // "NECK"
    {8, HL_CHEST, 1.0f},          // "CHEST"
    {12, HL_FOOT_LT, 0.4f},       // "RIGHT FOOT"
    {16, HL_CHEST_LT, 1.0f},      // "LEFT SHOULDER"
    {20, HL_ARM_LT, 0.7f},        // "LEFT ARM"
    {24, HL_HAND_LT, 0.4f},       // "LEFT HAND"
    {28, HL_CHEST_RT, 1.0f},      // "RIGHT SHOULDER"
    {32, HL_ARM_RT, 0.7f},        // "RIGHT ARM"
    {36, HL_HAND_RT, 0.4f},       // "RIGHT HAND"
    {40, HL_WAIST, 1.0f},         // "GUT"
    {44, HL_WAIST, 1.0f},         // "GROIN"
    {48, HL_LEG_UPPER_LT, 0.7f},  // "LEFT THIGH"
    {52, HL_LEG_LOWER_LT, 0.7f},  // "LEFT LEG"
    {56, HL_FOOT_RT, 0.4f},       // "LEFT FOOT"
    {60, HL_LEG_UPPER_RT, 0.7f},  // "RIGHT THIGH"
};

// OSP
hitLocationConversion_t ospHitLocationConversion[HL_CONV_MAX] = {
    {-1, HL_NONE, 0},            // UNKNOWN
    {0, HL_HEAD, 1.75f},         // "HEAD"
    {1, HL_LEG_LOWER_RT, 0.7f},  // "RIGHT LEG"
    {4, HL_NECK, 1.75f},         // "NECK"
    {8, HL_CHEST, 1.0f},         // "CHEST"
    {12, HL_FOOT_LT, 0.3f},      // "RIGHT FOOT"
    {16, HL_CHEST_LT, 1.0f},     // "LEFT SHOULDER"
    {20, HL_ARM_LT, 0.7f},       // "LEFT ARM"
    {24, HL_HAND_LT, 0.3f},      // "LEFT HAND"
    {28, HL_CHEST_RT, 1.0f},     // "RIGHT SHOULDER"
    {32, HL_ARM_RT, 0.7f},       // "RIGHT ARM"
    {36, HL_HAND_RT, 0.3f},      // "RIGHT HAND"
    {40, HL_WAIST, 1.0f},        // "GUT"
    {44, HL_WAIST, 1.0f},        // "GROIN"
    {48, HL_LEG_UPPER_LT, 0.7f}, // "LEFT THIGH"
    {52, HL_LEG_LOWER_LT, 0.7f}, // "LEFT LEG"
    {56, HL_FOOT_RT, 0.3f},      // "LEFT FOOT"
    {60, HL_LEG_UPPER_RT, 0.7f}, // "RIGHT THIGH"
};
// END

/*
===============
G_CheckSniperCall

Checks to see if the client or someone near him should yell "sniper"
===============
*/
void G_CheckSniperCall(weapon_t weapon, gentity_t *attacker, gentity_t *target, qboolean miss)
{
    vec3_t diff;

    // If it was a sniper that shot this guy then play the sniper voice
    if (!G_CanVoiceGlobal())
    {
        return;
    }

    // Needs to have been a snipe rifle and an enemy
    if (weapon != WP_MSG90A1 || OnSameTeam(attacker, target))
    {
        return;
    }

    // Make sure hes not close
    VectorSubtract(attacker->r.currentOrigin, target->r.currentOrigin, diff);
    if (VectorLengthSquared(diff) > 1500 * 1500)
    {
        gentity_t *nearby;

        // Find someone close
        nearby = G_FindNearbyClient(target->r.currentOrigin, target->client->sess.team, 800, target);

        // if it was a miss have the guy that was shot at say it
        if (miss)
        {
            if (nearby)
            {
                G_VoiceGlobal(target, "sniper", qfalse);
            }
        }
        // Look for someone around the guy being shot at to say sniper
        else if (nearby)
        {
            G_VoiceGlobal(nearby, "sniper", qfalse);
        }
    }
}

/*
===============
G_TraceBullet

Runs a trace for a fired bullet
===============
*/
void G_TraceBullet(weapon_t weapon, trace_t *tr, G2Trace_t G2Trace, vec3_t start, vec3_t end, int passent, int mask, qboolean detailed)
{
    int unlinkCount;
    gentity_t *unlinked[20];

    unlinkCount = 0;

    G2Trace[0].mEntityNum = -1;

    while (1)
    {
        // Run the trace as is
        trap_Trace(tr, start, NULL, NULL, end, passent, mask);

        // If the bullet hit glass then break it completely for now
        if (tr->fraction != 1 && !Q_stricmp(g_entities[tr->entityNum].classname, "func_glass"))
        {
            // break the glass
            g_entities[tr->entityNum].use(&g_entities[tr->entityNum], &g_entities[tr->entityNum], &g_entities[tr->entityNum]);
            continue;
        }

        // If we hit a client, do a detailed trace
        if (detailed && tr->fraction != 1 && g_entities[tr->entityNum].client)
        {
            animation_t *anim;
            gentity_t *traceEnt;
            trace_t vtr;

            traceEnt = &g_entities[tr->entityNum];

            anim = &level.ghoulAnimations[traceEnt->client->legs.anim & (~ANIM_TOGGLEBIT)];
            trap_G2API_SetBoneAnim(level.serverGhoul2, 0, "model_root", anim->firstFrame, anim->firstFrame + anim->numFrames, BONE_ANIM_OVERRIDE_LOOP, 50.0f / anim->frameLerp, traceEnt->client->legs.animTime, -1, 0);

            anim = &level.ghoulAnimations[traceEnt->client->torso.anim & (~ANIM_TOGGLEBIT)];
            trap_G2API_SetBoneAnim(level.serverGhoul2, 0, "lower_lumbar", anim->firstFrame, anim->firstFrame + anim->numFrames, BONE_ANIM_OVERRIDE_LOOP, 50.0f / anim->frameLerp, traceEnt->client->torso.animTime, -1, 0);

            trap_G2API_SetBoneAngles(level.serverGhoul2, 0, "upper_lumbar", traceEnt->client->ghoulUpperTorsoAngles, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, 0, 0, level.time);
            trap_G2API_SetBoneAngles(level.serverGhoul2, 0, "lower_lumbar", traceEnt->client->ghoulLowerTorsoAngles, BONE_ANGLES_POSTMULT, POSITIVE_X, NEGATIVE_Y, NEGATIVE_Z, 0, 0, level.time);
            trap_G2API_SetBoneAngles(level.serverGhoul2, 0, "cranium", traceEnt->client->ghoulHeadAngles, BONE_ANGLES_POSTMULT, POSITIVE_Z, NEGATIVE_Y, POSITIVE_X, 0, 0, level.time);

            // OSP
            //trap_G2API_CollisionDetect(G2Trace, level.serverGhoul2, traceEnt->client->ghoulLegsAngles, traceEnt->r.currentOrigin, level.time, traceEnt->s.number, start, end, vec3_identity, 0, 2);
            if (osp_modversion.integer >= 1)
            {
                vec3_t scale;

                VectorSet(scale, 1.1, 1.1, 1.1);
                trap_G2API_CollisionDetect(G2Trace, level.serverGhoul2, traceEnt->client->ghoulLegsAngles, traceEnt->r.currentOrigin, level.time, traceEnt->s.number, start, end, scale, 0, 0);
            }
            else
            {
                trap_G2API_CollisionDetect(G2Trace, level.serverGhoul2, traceEnt->client->ghoulLegsAngles, traceEnt->r.currentOrigin, level.time, traceEnt->s.number, start, end, vec3_identity, 0, 0);
            }
            // END

            // Check to see if anyone should yell SNIPER!
            G_CheckSniperCall(weapon, &g_entities[passent], traceEnt, G2Trace[0].mEntityNum != traceEnt->s.number ? qtrue : qfalse);

            // Did we hit?
            if (G2Trace[0].mEntityNum == -1)
            {
                unlinked[unlinkCount++] = traceEnt;
                trap_UnlinkEntity(traceEnt);
                continue;
            }

            trap_UnlinkEntity(traceEnt);

            // Unfortunately because the players bounding box is doubled we have to run
            // on more trace to make sure the player is actually able to be hit where they
            // were hit.  The bounding box is doubled to ensure that the entire ghoul model
            // is accounted for, but that means the player will stick through walls too.  To check
            // for this we run a trace from the hit location on the big bounding box to the
            // hit location of the ghoul model and make sure there is nothing solid in the way
            trap_Trace(&vtr, tr->endpos, NULL, NULL, G2Trace[0].mCollisionPosition, passent, MASK_SHOT & (~CONTENTS_BODY));
            if (vtr.entityNum != G2Trace[0].mEntityNum)
            {
                if (vtr.entityNum != ENTITYNUM_NONE)
                {
                    // Must have hit something, discount the trace
                    unlinked[unlinkCount++] = traceEnt;
                    continue;
                }
            }

            trap_LinkEntity(traceEnt);
        }
        // OSP
        else if (tr->fraction != 1 && g_entities[tr->entityNum].client && (g_entities[tr->entityNum].client->ps.pm_flags & PMF_LEANING))
        {
            gentity_t *other;
            vec3_t rOrigin;

            // ok, we've hit someone who is leaning...
            other = &g_entities[tr->entityNum];

            // adjust the standard bbox to compensate for
            // different head position and trace against that
            G_UndoAdjustedClientBBox(other);

            G_SetClientPreLeaningBBox(other);

            if (shotgun_debugAdjBBox.integer && (level.time - last_showBBox > 500))
            {
                G_ShowClientBBox(other);
            }

            trap_LinkEntity(other);

            // trace against slightly adjusted bbox (torso and head gone)
            trap_Trace(tr, start, NULL, NULL, end, passent, mask);

            // we hit the adjusted bbox, stop here
            if (tr->entityNum == other->s.number)
            {
                if (shotgun_debugAdjBBox.integer && (level.time - last_showBBox > 500))
                {
                    last_showBBox = level.time;
                }

                if (tr->endpos[2] > other->r.currentOrigin[2] + 4)
                {
                    G2Trace[0].mLocation = HL_WAIST;
                }
                else if (tr->endpos[2] > other->r.currentOrigin[2] - 24)
                {
                    G2Trace[0].mLocation = HL_FOOT_LT;
                }
                else if (tr->endpos[2] > other->r.currentOrigin[2] - 10)
                {
                    G2Trace[0].mLocation = HL_LEG_LOWER_LT;
                }
                else
                {
                    G2Trace[0].mLocation = HL_LEG_UPPER_LT;
                }

                break;
            }

            // failing the first check, move the bbox origin in
            // the direction of the lean and make it smaller
            VectorCopy(other->r.currentOrigin, rOrigin);

            G_UndoAdjustedClientBBox(other);

            // adjust bbox and origin while leaning
            G_SetClientLeaningBBox(other);

            if (shotgun_debugAdjBBox.integer && (level.time - last_showBBox > 500))
            {
                G_ShowClientBBox(other);
            }

            trap_LinkEntity(other);

            // trace against smaller, offset bbox
            trap_Trace(tr, start, NULL, NULL, end, passent, mask);

            if (shotgun_debugAdjBBox.integer && (level.time - last_showBBox > 500))
            {
                last_showBBox = level.time;
            }

            // good leaning hit, stop here
            if (tr->entityNum == other->s.number)
            {
                if (tr->endpos[2] > other->r.currentOrigin[2] + 8)
                {
                    G2Trace[0].mLocation = HL_HEAD;
                }
                else
                {
                    G2Trace[0].mLocation = HL_WAIST;
                }

                // reset origins
                VectorCopy(rOrigin, other->r.currentOrigin);
                break;
            }

            // reset origins
            VectorCopy(rOrigin, other->r.currentOrigin);

            // bad hit, ignore them and move on
            unlinked[unlinkCount++] = other;
            trap_UnlinkEntity(other);
            continue;
        }
        // END

        break;
    }

    // Relink all the unlinked entities
    while (unlinkCount > 0)
    {
        trap_LinkEntity(unlinked[--unlinkCount]);
    }
}

#define MAX_HITS 20

typedef struct gbullethit_s
{
    gentity_t *ent;
    vec3_t origin;
    int damage;
    int location;

} gbullethit_t;

/*
===============
G_FireBullet

Fires a bullet from the given player entity using the given weapon and attack
===============
*/
void G_FireBullet(gentity_t *ent, int weapon, int attack)
{
    trace_t tr;
    vec3_t end;
    gentity_t *tent;
    gentity_t *traceEnt;
    int i;
    vec3_t muzzlePoint;
    vec3_t fwd;
    vec3_t right;
    vec3_t up;
    vec3_t fireAngs;
    float damageMult;
    int hitcount;
    G2Trace_t G2Trace;

    int maxFx;
    weaponData_t *weaponDat;
    attackData_t *attackDat;
    float inaccuracy;
    qboolean detailed;
    int seed;

    gbullethit_t hit[MAX_HITS];

    // OSP
    float x69c = -1.0;
    // END

    // Grab the firing info
    weaponDat = &weaponData[ent->s.weapon];
    attackDat = &weaponDat->attack[attack];

    // OSP
    OSP_LogShot(ent, attack);
    // END

    // Maximum effects that can be spawned from this bullet
    maxFx = (attackDat->pellets / 2) + 1;
    hitcount = 0;

    // Detailed trace or not?
    // OSP
    //detailed = (attackDat->pellets > 1 || attackDat->melee) ? qfalse : qtrue;
    if (osp_modversion.integer >= 1)
    {
        detailed = qtrue;
    }
    else
    {
        detailed = (attackDat->pellets > 1 || attackDat->melee) ? qfalse : qtrue;
    }
    // END

    // Current inaccuracy
    inaccuracy = (float)ent->client->ps.inaccuracy / 1000.0f;

    // OSP
    //if (detailed)
    if (!attackDat->melee && attackDat->pellets == 1)
    // END
    {
        if (ent->client->ps.pm_flags & PMF_JUMPING)
        {
            inaccuracy *= JUMP_ACCURACY_MODIFIER;
        }
        else if (ent->client->ps.pm_flags & PMF_DUCKED)
        {
            inaccuracy *= DUCK_ACCURACY_MODIFIER;
        }
    }

    // Anti-lag
    G_ApplyAntiLag(ent, detailed);

    // where is gun muzzle?
    VectorCopy(ent->client->ps.origin, muzzlePoint);
    muzzlePoint[2] += ent->client->ps.viewheight;

    // Handle leaning
    VectorCopy(ent->client->ps.viewangles, fireAngs);

    AngleVectors(fireAngs, fwd, right, up);

    if (ent->client->ps.pm_flags & PMF_LEANING)
    {
        BG_ApplyLeanOffset(&ent->client->ps, muzzlePoint);
    }

    // Move the start trace back a bit to account for bumping up against someone
    VectorMA(muzzlePoint, -15, fwd, muzzlePoint);

    seed = ent->client->ps.stats[STAT_SEED];

    // OSP
    if (osp_modversion.integer >= 1)
    {
        x69c = inaccuracy;

        if (attackDat->mod == MOD_M590_SHOTGUN || attackDat->mod == MOD_USAS_12_SHOTGUN)
        {
            vec3_t x6c8;

            if (ent->client->ps.pm_flags & PMF_JUMPING)
            {
                inaccuracy = 1.0;
            }

            BG_CalculateBulletEndpoint(muzzlePoint, fireAngs, inaccuracy, attackDat->rV.range + 15, end, &seed);
            VectorSubtract(end, muzzlePoint, x6c8);
            vectoangles(x6c8, fireAngs);

            if (attackDat->mod == MOD_M590_SHOTGUN)
            {
                x69c = 1.75;
            }
            else
            {
                x69c = 2.0;
            }
        }
    }

    if ((ent->client->ps.pm_flags & PMF_JUMPING) && ((weapon == WP_M590_SHOTGUN) || (weapon == WP_USAS_12_SHOTGUN)))
    {
        BG_CalculateBulletEndpoint(muzzlePoint, fireAngs, SHOTGUN_JUMP_AIMCHANGE, attackDat->rV.range + 15, end, &seed);
        VectorSubtract(end, muzzlePoint, fwd);
        vectoangles(fwd, fireAngs);
    }
    // END

    // Run a trace for each pellet being fired
    for (i = 0; i < attackDat->pellets; i++)
    {
        int location = HL_NONE;

        // Determine the endpoint for the bullet
        // OSP
        //BG_CalculateBulletEndpoint(muzzlePoint, fireAngs, inaccuracy, attackDat->rV.range + 15, end, &seed);
        if (osp_modversion.integer >= 1)
        {
            BG_CalculateBulletEndpoint(muzzlePoint, fireAngs, x69c,
                                       (attackDat->melee) ? (attackDat->rV.range + 15) * 1.35 : attackDat->rV.range + 15,
                                       end, &seed);
        }
        else
        {
            BG_CalculateBulletEndpoint(muzzlePoint, fireAngs, inaccuracy, attackDat->rV.range + 15, end, &seed);
        }

        if (!detailed)
        {
            G_AdjustClientBBoxs();
        }
        // END

        // Trace the bullet
        G_TraceBullet(weapon, &tr, G2Trace, muzzlePoint, end, ent->s.number, MASK_SHOT, detailed);

        // OSP
        if (!detailed)
        {
            G_UndoAdjustedClientBBoxs();
        }

        if (shotgun_debugWeapons.integer && level.time - last_show > 500)
        {
            tent = G_TempEntity(muzzlePoint, EV_BOTWAYPOINT);
            VectorCopy(tr.endpos, tent->s.angles);
            tent->r.svFlags |= SVF_BROADCAST;
        }
        // END

        if ((tr.surfaceFlags & SURF_NOIMPACT) || tr.entityNum == ENTITYNUM_NONE)
        {
            // a big miss
            continue;
        }

        traceEnt = &g_entities[tr.entityNum];

        // snap the endpos to integers, but nudged towards the line
        SnapVectorTowards(tr.endpos, muzzlePoint);

        damageMult = 1.0f;
        if (traceEnt->takedamage)
        {
            // Where did the bullet hit?
            if (ent->client && traceEnt->client)
            {
                // Add or subtract some damage depending on where it hits
                if (G2Trace[0].mEntityNum == -1)
                {
                    vec3_t dir;

                    VectorSubtract(end, muzzlePoint, dir);
                    VectorNormalize(dir);

                    // OSP
                    //location = G_GetHitLocation(traceEnt, muzzlePoint, dir);
                    if (G2Trace[0].mLocation != 0)
                        location = G2Trace[0].mLocation;
                    else
                        location = G_GetHitLocation(traceEnt, muzzlePoint, dir);

                    switch (location)
                    {
                    case HL_FOOT_RT:
                    case HL_FOOT_LT:
                    case HL_HAND_RT:
                    case HL_HAND_LT:
                        damageMult = 0.3f;
                        break;

                    case HL_ARM_RT:
                    case HL_ARM_LT:
                    case HL_LEG_UPPER_RT:
                    case HL_LEG_UPPER_LT:
                    case HL_LEG_LOWER_RT:
                    case HL_LEG_LOWER_LT:
                        damageMult = 0.7f;
                        break;

                    case HL_HEAD:
                    case HL_NECK:
                        damageMult = 1.75f;
                        break;

                    default:
                    case HL_BACK_RT:
                    case HL_BACK_LT:
                    case HL_BACK:
                    case HL_CHEST_RT:
                    case HL_CHEST_LT:
                    case HL_CHEST:
                    case HL_WAIST:
                        damageMult = 1.0f;
                        break;

                    case HL_NONE:
                        damageMult = 0.0f;
                        break;
                    }

                    // OSP
                    if (((weapon == WP_M590_SHOTGUN) || (weapon == WP_USAS_12_SHOTGUN)) && (damageMult > 1.0f) && !(traceEnt->client->ps.pm_flags & PMF_LEANING))
                    {
                        vec3_t diff;
                        float dist;

                        VectorSubtract(muzzlePoint, tr.endpos, diff);
                        dist = VectorLength(diff);

                        // no headshots beyond close range
                        if (dist > 128)
                        {
                            damageMult = 1.0f;
                        }
                    }
                    // END
                }
                else
                {
                    int z;
                    float accuracyRatio;
                    float maxinaccuracy;
                    float addinaccuracy;

                    location = HL_NONE;
                    damageMult = 0;
                    accuracyRatio = 1.0f;
                    maxinaccuracy = attackDat->maxInaccuracy / 1000.0f;
                    addinaccuracy = attackDat->inaccuracy / 1000.0f;

                    if (maxinaccuracy)
                    {
                        // Calculate the ratio of how far along the inaccuracy spread they are
                        accuracyRatio = 1.0f - ((inaccuracy - addinaccuracy) / (maxinaccuracy));
                        accuracyRatio = Com_Clampf(0.0f, 1.0f, accuracyRatio);
                    }

                    for (z = 0; z < MAX_G2_COLLISIONS && G2Trace[z].mEntityNum != -1; z++)
                    {
                        int temp_location;
                        float temp_damageMult;
                        int pose;
                        int l;

                        pose = (G2Trace[z].mLocation >> 2);
                        temp_damageMult = 0.0f;
                        temp_location = HL_NONE;

                        // Convert hitregions (SOF2.poses) to hitlocation
                        for (l = 0; l < HL_CONV_MAX; l++)
                        {
                            // Found the SOF2pose
                            if (hitLocationConversion[l].SOF2poses == pose)
                            {
                                temp_location = hitLocationConversion[l].hitLocation;

                                // OSP
                                if (attackDat->pellets > 1 || attackDat->melee)
                                {
                                    temp_damageMult = ospHitLocationConversion[l].damageMultiplier;
                                }
                                // END
                                // Special cases
                                else if (hitLocationConversion[l].damageMultiplier == HITLOC_HEAD_DM) // Head
                                {
                                    temp_damageMult = 1.0f + 2.0f * accuracyRatio;
                                }
                                else if (hitLocationConversion[l].damageMultiplier == HITLOC_NECK_DM) // Neck
                                {
                                    temp_damageMult = 1.0f + 0.75f * accuracyRatio;
                                }
                                else
                                {
                                    temp_damageMult = hitLocationConversion[l].damageMultiplier;
                                }
                                break;
                            }
                        }

                        // Didn't find it? Default to waist
                        if (temp_location == HL_NONE)
                        {
                            temp_location = HL_WAIST;

                            // Search to find waist damageMultiplier
                            for (l = 0; l < HL_CONV_MAX; l++)
                            {
                                if (temp_location == hitLocationConversion[l].hitLocation)
                                {
                                    // OSP
                                    // temp_damageMult = hitLocationConversion[l].damageMultiplier;
                                    if (attackDat->pellets > 1 || attackDat->melee)
                                        temp_damageMult = ospHitLocationConversion[l].damageMultiplier;
                                    else
                                        temp_damageMult = hitLocationConversion[l].damageMultiplier;
                                    // END
                                    break;
                                }
                            }
                        }

                        if (temp_damageMult > damageMult)
                        {
                            location = temp_location;
                            damageMult = temp_damageMult;
                        }
                    }
                }
            }

            // If a range attack then do distance affected damage, but exclude close range attacks
            if (attackDat->rV.range > 100)
            {
                vec3_t diff;
                float dist;
                float checkrange;

                VectorSubtract(muzzlePoint, tr.endpos, diff);

                dist = VectorLength(diff);
                checkrange = attackDat->rV.range * 0.5f;

                // If half the way through its range then start falling off the damage to 1 quarter damage
                if (dist > attackDat->rV.range * 0.5f)
                {
                    dist -= (attackDat->rV.range * 0.5f);

                    // Scale oof the damage
                    damageMult *= (1.0f - (dist / (float)checkrange));
                }
            }

            // we hit something that noticed, so that is enough pellets
            if ((int)(attackDat->damage * damageMult) > 0)
            {
                if (!attackDat->melee)
                {
                    damageMult *= 1.15f;
                }

                hit[hitcount].ent = traceEnt;
                hit[hitcount].damage = attackDat->damage * damageMult;
                hit[hitcount].location = location;
                VectorCopy(tr.endpos, hit[hitcount].origin);

                hitcount++;
            }
        }

        // send bullet impact
        if (i < maxFx && attackDat->damage * damageMult > 0.0f)
        {
            qboolean flesh = qfalse;

            // Are they are a client?
            if (traceEnt->client)
            {
                tr.surfaceFlags = 0;

                // Invulnerable?
                if (level.time - traceEnt->client->invulnerableTime >= g_respawnInvulnerability.integer * 1000)
                {
                    // Shot my a teammate with ff off?
                    // OSP
                    //if (!level.gametypeData->teams || (ent->client && !(OnSameTeam(ent, traceEnt) && (!g_friendlyFire.integer || level.warmupTime))))
                    if (!level.gametypeData->teams || (ent->client && !(OnSameTeam(ent, traceEnt) && (!g_friendlyFire.integer || (level.warmupTime && g_friendlyFire.integer == 1)))))
                    // END
                    {
                        flesh = qtrue;
                    }
                }
            }

            // If it was a client that was hit and it wasnt by a teammate when friendly fire is off then send blood!
            if (flesh)
            {
                tent = G_TempEntity(tr.endpos, EV_BULLET_HIT_FLESH);

                // send entity and direction
                tent->s.eventParm = DirToByte(fwd);
                tent->s.otherEntityNum = ent->s.number;
                tent->s.otherEntityNum2 = traceEnt->s.number;

                // Pack the shot info into the temp end for gore
                tent->s.time = weapon + ((attack & 0xFF) << 8) + ((((int)traceEnt->s.apos.trBase[YAW] & 0x7FFF) % 360) << 16);
                VectorCopy(traceEnt->r.currentOrigin, tent->s.angles);
                SnapVector(tent->s.angles);

                // Some procedural gore should be ignored because it would look odd on the player. For
                // example, if someone gets shot in the head but doesnt die from it.
                if (traceEnt->client->ps.stats[STAT_HEALTH] > 0)
                {
                    if (location & (HL_HEAD))
                    {
                        tent->s.time |= GORE_NONE;
                    }
                }
            }
            else
            {
                tent = G_TempEntity(tr.endpos, EV_BULLET_HIT_WALL);
                // send direction and material
                tent->s.eventParm = DirToByte(tr.plane.normal);
                tent->s.eventParm <<= MATERIAL_BITS;
                tent->s.eventParm |= (tr.surfaceFlags & MATERIAL_MASK);
                tent->s.time = weapon + ((attack & 0xFF) << 8);
                tent->r.detailTime = level.time + rand() % 1000;
                tent->r.svFlags |= SVF_DETAIL;
            }

            tent->s.otherEntityNum = ent->s.number;
        }
    }

    // Push all the clients back to the real timeframe.  We need to do this
    // before damaging the client so the real bounding box and location are stored in
    // the body
    G_UndoAntiLag();

    // OSP
    if (shotgun_debugWeapons.integer && level.time - last_show > 500)
    {
        last_show = level.time;
    }
    // END

    if (hitcount)
    {
        int flags;
        int h;

        flags = 0;
        flags |= (attackDat->gore ? 0 : DAMAGE_NO_GORE);
        flags |= (attackDat->melee ? DAMAGE_NO_ARMOR : 0);
        flags |= (hitcount > 2 ? DAMAGE_FORCE_GORE : 0);

        for (h = 0; h < hitcount; h++)
        {
            // OSP
            //G_Damage(hit[h].ent, ent, ent, fwd, hit[h].origin, hit[h].damage, flags, attackDat->mod + (attack << 8), hit[h].location);
            vec3_t x6dc;
            VectorSubtract(hit[h].origin, muzzlePoint, x6dc);
            G_Damage(hit[h].ent, ent, ent, x6dc, hit[h].origin, hit[h].damage, flags, attackDat->mod + (attack << 8), hit[h].location);
            // END
        }
    }
}

/*
===============
G_FireProjectile

Fires a projectile from the given client entity
===============
*/
gentity_t *G_FireProjectile(gentity_t *ent, weapon_t weapon, attackType_t attack, int projectileLifetime, int flags)
{
    int i;
    vec3_t muzzlePoint;
    vec3_t fwd, right, up, fireAngs;
    gentity_t *missile = NULL;
    weaponData_t *weaponDat;
    attackData_t *attackDat;
    float inaccuracy;

    // Grab the firing info
    weaponDat = &weaponData[ent->s.weapon];
    attackDat = &weaponDat->attack[attack];

    // OSP
    OSP_LogShot(ent, attack);
    // END

    inaccuracy = attackDat->inaccuracy / 1000.0f;

    // where is gun muzzle?
    VectorCopy(ent->client->ps.origin, muzzlePoint);
    muzzlePoint[2] += ent->client->ps.viewheight;

    // Inform of the grenade toss if its a timed grenade
    // OSP
    //if (weapon >= WP_M84_GRENADE && weapon < WP_M15_GRENADE && (flags & PROJECTILE_TIMED) && (ent->client->ps.pm_type == PM_NORMAL))
    if (weapon >= WP_M84_GRENADE &&
        weapon <= WP_M67_GRENADE &&
        pm->ps->weapon != WP_OICW &&
        pm->ps->weapon != WP_USSOCOM_SILENCED &&
        (flags & PROJECTILE_TIMED) &&
        (ent->client->ps.pm_type == PM_NORMAL))
    // END
    {
        gentity_t *nearby;

        // Make sure there is someone nearby to hear you yell
        nearby = G_FindNearbyClient(ent->r.currentOrigin, ent->client->sess.team, 1200, ent);
        if (nearby)
        {
            G_VoiceGlobal(ent, "fire_hole", qtrue);
        }
    }

    // snap to integer coordinates for more efficient network bandwidth usage
    SnapVector(muzzlePoint);

    VectorCopy(ent->client->ps.viewangles, fireAngs);

    if (ent->client->ps.pm_flags & PMF_LEANING)
    {
        BG_ApplyLeanOffset(&ent->client->ps, muzzlePoint);
    }

    AngleVectors(fireAngs, fwd, right, up);

    for (i = 0; i < attackDat->pellets; i++)
    {
        vec3_t dir;
        VectorCopy(fwd, dir);
        if (inaccuracy != 0)
        {
            // add in some spread / scatter
            dir[0] += flrand(-0.1 * inaccuracy, 0.1 * inaccuracy);
            dir[1] += flrand(-0.1 * inaccuracy, 0.1 * inaccuracy);
            dir[2] += flrand(-0.1 * inaccuracy, 0.1 * inaccuracy);
        }

        missile = G_CreateMissile(muzzlePoint, dir, attackDat->rV.velocity, min(projectileLifetime, 10000), ent, attack);

        missile->classname = ammoData[attackDat->ammoIndex].name;
        missile->s.weapon = weapon;

        VectorSet(missile->r.maxs, 1, 1, 1);
        VectorScale(missile->r.maxs, -1, missile->r.mins);

        missile->damage = attackDat->damage;
        missile->dflags = DAMAGE_DEATH_KNOCKBACK | DAMAGE_NO_ARMOR;
        missile->dflags |= (attackDat->gore ? 0 : DAMAGE_NO_GORE);
        missile->methodOfDeath = attackDat->mod + (attack << 8);
        missile->clipmask = MASK_SHOT | CONTENTS_MISSILECLIP;
        if (attackDat->splashRadius)
        {
            missile->splashDamage = attackDat->damage;
            missile->splashRadius = attackDat->splashRadius;
            missile->splashMethodOfDeath = missile->methodOfDeath;
        }
        else
        {
            missile->splashDamage = 0;
            missile->splashRadius = 0;
            missile->splashMethodOfDeath = MOD_UNKNOWN; //??
        }

        if (flags & PROJECTILE_GRAVITY)
        {
            missile->s.pos.trType = TR_GRAVITY;
        }
        else if (flags & PROJECTILE_LIGHTGRAVITY)
        {
            missile->s.pos.trType = TR_LIGHTGRAVITY;
        }
        // OSP
        else if (flags & PROJECTILE_OSP)
        {
            missile->s.pos.trType = TR_XXX;
        }
        // END

        if (flags & PROJECTILE_TIMED)
        {
            missile->s.eFlags |= EF_BOUNCE_SCALE;
            missile->think = G_GrenadeThink;
            missile->bounceScale = attackDat->bounceScale;
        }
        else
        {
            // we don't want it to bounce, just blow up
            missile->bounceScale = 0;
            missile->think = G_ExplodeMissile;
        }

        if (flags & PROJECTILE_DAMAGE_AREA)
        {
            // add area damage over time
            missile->damage /= 2;
            missile->dflags |= DAMAGE_AREA_DAMAGE;
        }
    }

    // Angle override on the knife
    if (weapon == WP_KNIFE)
    {
        missile->s.eFlags |= EF_ANGLE_OVERRIDE;
    }

    return missile;
}

/*
======================
SnapVectorTowards

Round a vector to integers for more efficient network
transmission, but make sure that it rounds towards a given point
rather than blindly truncating.  This prevents it from truncating 
into a wall.
======================
*/
void SnapVectorTowards(vec3_t v, vec3_t to)
{
    int i;

    for (i = 0; i < 3; i++)
    {
        if (to[i] <= v[i])
        {
            v[i] = (int)v[i];
        }
        else
        {
            v[i] = (int)v[i] + 1;
        }
    }
}

/*
===============
G_FireWeapon

Fires either a bullet or a projectile
===============
*/
gentity_t *G_FireWeapon(gentity_t *ent, attackType_t attack)
{
    weaponData_t *weaponDat;
    attackData_t *attackDat;
    int projectileLifetime;
    int flags;

    weaponDat = &weaponData[ent->s.weapon];
    attackDat = &weaponDat->attack[attack];
    flags = attackDat->weaponFlags;

    // Determine lifetime of projectile. Modified by how long we've been holding it!
    if (!ent->client->ps.grenadeTimer)
    {
        projectileLifetime = attackDat->projectileLifetime;
    }
    else
    {
        projectileLifetime = ent->client->ps.grenadeTimer;

        // Less than 50 milliseconds will just cause it to blow up in your hand
        if (projectileLifetime < 50)
        {
            flags &= ~PROJECTILE_TIMED;
        }
    }

    // Clear the grenade timer
    ent->client->ps.grenadeTimer = 0;

    if (attackDat->weaponFlags & PROJECTILE_FIRE)
    {
        return G_FireProjectile(ent, ent->s.weapon, attack, projectileLifetime, flags);
    }
    else
    {
        G_FireBullet(ent, ent->s.weapon, attack);
    }

    return NULL;
}

/*
===============
G_InitHitModel
===============
*/
void *G_InitHitModel(void)
{
    void *ghoul2;
    char temp[20480];
    int numPairs;
    qhandle_t handle;

    ghoul2 = NULL;

    // Initialize the ghoul2 model
    trap_G2API_InitGhoul2Model(&ghoul2,
                               "models/characters/average_sleeves/average_sleeves.glm",
                               0, 0, 0, (1 << 4), 2);

    // Verify it
    if (!ghoul2)
    {
        return ghoul2;
    }

    //  Parse the skin file that will be used for hit information
    numPairs = BG_ParseSkin("models/characters/skins/col_rebel_h1.g2skin", temp, sizeof(temp));
    if (!numPairs)
    {
        trap_G2API_CleanGhoul2Models(&ghoul2);
        return NULL;
    }

    // Register the skin and attach it to the ghoul model
    handle = trap_G2API_RegisterSkin("hitmodel", numPairs, temp);
    trap_G2API_SetSkin(ghoul2, 0, handle);

    // Read in the animations for this model
    trap_G2API_GetAnimFileNameIndex(ghoul2, 0, temp);
    BG_ParseAnimationFile(va("%s_mp.cfg", temp), level.ghoulAnimations);

    // Hand the hit model back
    return ghoul2;
}

/*
===============
G_InitHitModels
===============
*/
void G_InitHitModels(void)
{
    level.serverGhoul2 = G_InitHitModel();
}
