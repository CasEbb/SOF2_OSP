// osp_combat.c
//
#include "g_local.h"

// osp: 0x0002d364
static void OSP_Obituary(gentity_t *attacker, gentity_t *target, int mod, int hitLocation, attackType_t attack, int arg6, int killstreak)
{
    int attackerNum;
    int targetNum;
    char *message;
    const char *targetColor;
    char *message2;
    const char *attackerColor;
    gender_t gender;

    targetNum = target - g_entities;

    if (attacker)
    {
        attackerNum = attacker - g_entities;
    }
    else
    {
        attackerNum = -1;
    }

    attackerColor = S_COLOR_WHITE;
    targetColor = S_COLOR_WHITE;

    if (targetNum < 0 || targetNum >= MAX_CLIENTS)
    {
        Com_Error(ERR_FATAL, "OSP_Obituary: target out of range (%d)", targetNum);
    }

    if (mod == MOD_WATER)
    {
        G_Sound(target, CHAN_AUTO, G_SoundIndex("sound/pain_death/mullins/drown_dead.mp3"));
    }
    else
    {
        switch (level.time % 3)
        {
        case 0:
            G_Sound(target, CHAN_VOICE, G_SoundIndex("sound/pain_death/male/die01.mp3"));
            break;
        case 1:
            G_Sound(target, CHAN_VOICE, G_SoundIndex("sound/pain_death/male/die03.mp3"));
            break;
        case 2:
            G_Sound(target, CHAN_VOICE, G_SoundIndex("sound/pain_death/male/die04.mp3"));
            break;
        }
    }

    if (attackerNum == targetNum)
    {
        OSP_ClientSound(attackerNum, "sound/self_frag.mp3");
    }
    else if (attackerNum >= 0)
    {
        if (level.gametypeData->teams)
        {
            if (OnSameTeam(attacker, target))
            {
                OSP_ClientSound(attackerNum, "sound/self_frag.mp3");
            }
            else
            {
                OSP_ClientSound(attackerNum, "sound/frag.mp3");
            }
        }
        else
        {
            OSP_ClientSound(attackerNum, "sound/frag.mp3");
        }
    }

    if (attackerNum < 0 || attackerNum >= MAX_CLIENTS)
    {
        attackerNum = ENTITYNUM_WORLD;
    }

    switch (target->client->sess.team)
    {
    case TEAM_RED:
        targetColor = S_COLOR_RED;
        break;

    case TEAM_BLUE:
        targetColor = S_COLOR_BLUE;
        break;
    }

    message2 = "";
    gender = GENDER_MALE;

    switch (mod)
    {
    case MOD_SUICIDE:
        message = "suicides";
        break;
    case MOD_FALLING:
        if (gender == GENDER_FEMALE)
            message = "fell to her death";
        else
            message = "fell to his death";
        break;
    case MOD_CRUSH:
        message = "was squished";
        break;
    case MOD_WATER:
        message = "sank like a rock";
        break;
    case MOD_TRIGGER_HURT:
    case MOD_TRIGGER_HURT_NOSUICIDE:
        message = "was in the wrong place";
        break;
    case MOD_TEAMCHANGE:
        message = "switched teams";
        return;
    default:
        message = NULL;
    }

    if (attackerNum == targetNum)
    {
        switch (mod)
        {
        case MOD_MM1_GRENADE_LAUNCHER:
        case MOD_RPG7_LAUNCHER:
        case MOD_M84_GRENADE:
        case MOD_SMOHG92_GRENADE:
        case MOD_ANM14_GRENADE:
        case MOD_M15_GRENADE:
        case MOD_MDN11_GRENADE:
        case MOD_F1_GRENADE:
        case MOD_L2A2_GRENADE:
        case MOD_M67_GRENADE:
            if (gender == GENDER_FEMALE)
                message = "blew herself up";
            else if (gender == GENDER_NEUTER)
                message = "blew itself up";
            else
                message = "blew himself up";
            break;

        default:
            if (gender == GENDER_FEMALE)
                message = "killed herself";
            else if (gender == GENDER_NEUTER)
                message = "killed itself";
            else
                message = "killed himself";
            break;
        }
    }

    if (message)
    {
        trap_SendServerCommand(-1, va("print \"%s%s^7 %s.\n\"", targetColor, target->client->pers.netname, message));
        return;
    }

    if (level.gametypeData->showKills)
    {
        char *s;

        if (!level.gametypeData->teams)
        {
            s = va("You killed %s%s\n%d%s place with %i",
                   targetColor,
                   target->client->pers.netname,
                   attacker->client->ps.persistant[PERS_RANK] + 1,
                   (attacker->client->ps.persistant[PERS_RANK] + 1) > 3
                       ? "th"
                       : ((attacker->client->ps.persistant[PERS_RANK] + 1) > 2
                              ? "rd"
                              : ((attacker->client->ps.persistant[PERS_RANK] + 1) > 1
                                     ? "nd"
                                     : "st")),
                   attacker->client->ps.persistant[PERS_SCORE]);
        }
        else
        {
            s = va("You killed %s%s", targetColor, target->client->pers.netname);
        }

        trap_SendServerCommand(attackerNum, va("cp \"@%s\n\"", s));
    }

    if (attackerNum != ENTITYNUM_WORLD)
    {
        switch (attacker->client->sess.team)
        {
        case TEAM_RED:
            attackerColor = S_COLOR_RED;
            break;
        case TEAM_BLUE:
            attackerColor = S_COLOR_BLUE;
            break;
        }

        switch (mod)
        {
        case MOD_KNIFE:
            message = "was sliced by";
            break;
        case MOD_USAS_12_SHOTGUN:
        case MOD_M590_SHOTGUN:
            if (attack == ATTACK_ALTERNATE)
            {
                message = "was bludgeoned by";
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            else
            {
                message = "was pumped full of lead by";
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            break;
        case MOD_M1911A1_PISTOL:
        case MOD_USSOCOM_PISTOL:
        case MOD_SILVER_TALON:
            if (attack == ATTACK_ALTERNATE)
            {
                message = "was pistol whipped by";
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            else
            {
                message = "was shot by";
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            break;
        case MOD_USSOCOM_SILENCED:
            if (attack == ATTACK_ALTERNATE)
            {
                message = "was pistol whipped by";
                message2 = va("'s %s", weaponParseInfo[mod - 9].mName);
            }
            else
            {
                message = "was shot by";
                message2 = va("'s %s", weaponParseInfo[mod - 9].mName);
            }
            break;
        case MOD_AK74_ASSAULT_RIFLE:
            if (attack == ATTACK_ALTERNATE)
            {
                message = "was stabbed by";
            }
            else
            {
                message = "was shot by";
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            break;
        case MOD_M4_ASSAULT_RIFLE:
            if (attack == ATTACK_ALTERNATE)
            {
                message = "was detonated by";
                message2 = va("'s %s", "M203");
            }
            else
            {
                message = "was shot by";
                message2 = va("'s %s", weaponParseInfo[mod].mName);
            }
            break;
        case MOD_OICW:
            if (attack == ATTACK_ALTERNATE)
            {
                message = "was detonated by";
                message2 = va("'s %s", "20mm ammo");
            }
            else
            {
                message = "was shot by";
                message2 = va("'s %s", weaponParseInfo[mod - 9].mName);
            }
            break;
        case MOD_M60_MACHINEGUN:
        case MOD_MICRO_UZI_SUBMACHINEGUN:
        case MOD_MP5:
        case MOD_M3A1_SUBMACHINEGUN:
        case MOD_SIG551:
            message = "was shot by";
            message2 = va("'s %s", weaponParseInfo[mod].mName);
            break;
        case MOD_MSG90A1_SNIPER_RIFLE:
            message = "was sniped by";
            message2 = va("'s %s", weaponParseInfo[mod].mName);
            break;
        case MOD_MM1_GRENADE_LAUNCHER:
        case MOD_RPG7_LAUNCHER:
        case MOD_M84_GRENADE:
        case MOD_SMOHG92_GRENADE:
        case MOD_ANM14_GRENADE:
        case MOD_M15_GRENADE:
            message = "was detonated by";
            message2 = va("'s %s", weaponParseInfo[mod].mName);
            break;
        case MOD_MDN11_GRENADE:
        case MOD_F1_GRENADE:
        case MOD_L2A2_GRENADE:
        case MOD_M67_GRENADE:
            message = "was detonated by";
            message2 = va("'s %s", weaponParseInfo[mod - 9].mName);
            break;
        case MOD_TELEFRAG:
            message = "tried to invade";
            message2 = "'s personal space";
            break;
        case MOD_TARGET_LASER:
            message = "was struck by";
            message2 = "'s punitive bolt of lightning!";
            break;
        default:
            message = "was killed by";
            break;
        }

        if (message)
        {
            if (!killstreak || killstreak % 5)
                trap_SendServerCommand(
                    -1,
                    va("print \"%s%s^7 %s %s%s^7%s%s\n\"",
                       targetColor,
                       target->client->pers.netname,
                       message,
                       attackerColor,
                       attacker->client->pers.netname,
                       message2,
                       (hitLocation & HL_HEAD) && arg6 ? " ^1[ ^3HeaDShoT ^1]" : ""));
            else
                trap_SendServerCommand(
                    -1,
                    va("print \"%s%s^7 %s %s%s^7%s%s%s\n\"",
                       targetColor,
                       target->client->pers.netname,
                       message,
                       attackerColor,
                       attacker->client->pers.netname,
                       message2,
                       (hitLocation & HL_HEAD) && arg6 ? " ^1[ ^3HeaDShoT ^1]" : "",
                       killstreak == 5
                           ? " ^4[ ^5KillinGSpreE^4 ]"
                           : (killstreak == 10
                                  ? " ^4[^5 DominatinG ^4]"
                                  : (killstreak == 15
                                         ? " ^4[ ^5UnstoppablE ^4]"
                                         : (killstreak >= 20
                                                ? " ^4[ ^5GodLikE ^4]"
                                                : "")))));

            return;
        }
    }

    trap_SendServerCommand(-1, va("print \"%s%s^7 died.\n\"", targetColor, target->client->pers.netname));
}

// osp: 0x0002d887
void OSP_Damage(gentity_t *attacker, gentity_t *target, int damage, int location, int mod, qboolean logHits)
{
    gentity_t *other;
    gentity_t *ent;
    int meansOfDeath;

    ent = attacker;
    other = target;

    if (other->s.number < 0 || other->s.number > MAX_CLIENTS)
    {
        return;
    }

    meansOfDeath = mod & 0xFF;

    if (other && other->client && other->client->ospStats)
        other->client->ospStats->damageTaken += damage;

    if (ent && ent->client && ent->client->ospStats && other && other->client && other->client->ospStats)
    {
        attackType_t attack;

        if (!OnSameTeam(ent, other))
        {
            ent->client->ospStats->damageGiven += damage;
        }
        else if (ent != other)
        {
            ent->client->ospClient->lastHitClient = (other - g_entities) + 1;
            other->client->ospClient->lastHitByClient = (ent - g_entities) + 1;
            ent->client->ospStats->teamDamage += damage;
            return;
        }
        else
        {
            return;
        }

        ent->client->ospClient->lastHitClient = (other - g_entities) + 1;
        other->client->ospClient->lastHitByClient = (ent - g_entities) + 1;

        if (logHits)
        {
            weapon_t weapon;

            weapon = OSP_ModToWeapon(meansOfDeath);

            if (!OnSameTeam(ent, other))
            {
                OSP_LogHit(ent->client->ospStats, other->client->ospStats, location);
            }

            if (other->client->ospClient->vip)
            {
                ent->client->ospStats->x3c = level.time + 5000;
            }

            attack = (mod >> 8) & 0xFF;

            if (attack == ATTACK_ALTERNATE)
                ent->client->ospStats->weapon[ATTACK_ALTERNATE][weapon].hits++;
            else
                ent->client->ospStats->weapon[ATTACK_NORMAL][weapon].hits++;

            if ((location & HL_HEAD) && OSP_HeadshotPossible(meansOfDeath, attack))
            {
                if (attack == ATTACK_ALTERNATE)
                    ent->client->ospStats->weapon[ATTACK_ALTERNATE][weapon].headhits++;
                else
                    ent->client->ospStats->weapon[ATTACK_NORMAL][weapon].headhits++;
            }
        }
    }
}

// osp: 0x0002da91
void OSP_PlayerDie(gentity_t *a, gentity_t *s, int hitLocation, int mod)
{
    gentity_t *self;
    gentity_t *attacker;
    int meansOfDeath;
    attackType_t attack;
    qboolean logHeadshots;

    attacker = a;
    self = s;

    if (self->s.number < 0 || self->s.number > MAX_CLIENTS)
    {
        return;
    }

    attack = (mod >> 8) & 0xFF;
    meansOfDeath = mod & 0xFF;
    logHeadshots = OSP_HeadshotPossible(meansOfDeath, attack);

    if (self && self->client && self->client->ospStats)
    {
        if (meansOfDeath == MOD_SUICIDE ||
            meansOfDeath == MOD_FALLING ||
            meansOfDeath == MOD_WATER ||
            meansOfDeath == MOD_CRUSH ||
            meansOfDeath == MOD_TRIGGER_HURT)
            self->client->ospStats->suicides++;
        else
            self->client->ospStats->deaths++;

        if (g_killingspree.integer)
            self->client->ospStats->killstreak = 0;
    }

    if (attacker && attacker->client && attacker->client->ospStats &&
        self && self->client && self->client->ospStats)
    {
        weapon_t weapon;

        weapon = OSP_ModToWeapon(meansOfDeath);

        if (g_killingspree.integer)
        {
            if (attacker != self && !OnSameTeam(attacker, self))
            {
                attacker->client->ospStats->killstreak++;

                if (attacker->client->ospStats->killstreak % 5 == 0)
                {
                    attacker->client->ospStats->x38++;

                    if (attacker->client->ospStats->killstreak == 5)
                    {
                        trap_SendServerCommand(-1, va("cp \"^7%s ^7is on a\n^4[ ^5KillinG SpreE ^4]\n\"", attacker->client->pers.netname));
                        trap_SendServerCommand(attacker - g_entities, "cp \"@^4[ ^5KillinG SpreE ^4]\n\"");
                        OSP_GlobalSound("sound/enemy/english/male/dangerous.mp3");
                    }
                    else if (attacker->client->ospStats->killstreak == 10)
                    {
                        trap_SendServerCommand(-1, va("cp \"^7%s ^7is\n^4[ ^5DominatinG ^4]\n\"", attacker->client->pers.netname));
                        trap_SendServerCommand(attacker - g_entities, "cp \"@^4[ ^5DominatinG ^4]\n\"");
                        OSP_GlobalSound("sound/enemy/english/male/underfire03.mp3.mp3");
                    }
                    else if (attacker->client->ospStats->killstreak == 15)
                    {
                        trap_SendServerCommand(-1, va("cp \"^7%s ^7is\n^4[ ^5UnstoppablE ^4]\n\"", attacker->client->pers.netname));
                        trap_SendServerCommand(attacker - g_entities, "cp \"@^4[ ^5UnstoppablE ^4]\n\"");
                        OSP_GlobalSound("sound/enemy/english/male/man_down03.mp3");
                    }
                    else if (attacker->client->ospStats->killstreak >= 20)
                    {
                        trap_SendServerCommand(-1, va("cp \"^7%s ^7is\n^4[ ^5GodLikE ^4]\n\"", attacker->client->pers.netname));
                        trap_SendServerCommand(attacker - g_entities, "cp \"@^4[ ^5GodLikE ^4]\n\"");
                        OSP_GlobalSound("sound/enemy/english/male/slaughtered.mp3");
                    }
                }
            }
        }

        if (OnSameTeam(attacker, self))
        {
            if (g_killingspree.integer)
            {
                attacker->client->ospStats->killstreak = 0;
            }
        }

        if (attacker != self && (sv_ospClient.integer || !(hitLocation & HL_HEAD) || !logHeadshots) && (attacker->client->ospStats->killstreak % 5 != 0))
        {
            gentity_t *tent;

            tent = G_TempEntity(self->r.currentOrigin, EV_OBITUARY);
            tent->s.eventParm = mod;
            tent->s.otherEntityNum = self->s.number;
            tent->s.otherEntityNum2 = attacker->s.number;
            tent->s.time2 = hitLocation & ~HL_DISMEMBERBIT;
            tent->r.svFlags = SVF_BROADCAST;
        }
        else
        {
            OSP_Obituary(attacker, self, meansOfDeath, hitLocation, attack, logHeadshots, attacker->client->ospStats->killstreak);
        }

        if (!OnSameTeam(attacker, self))
        {
            attacker->client->ospStats->kills++;

            if (osp_enemyKillReport.integer && g_doWarmup.integer < 2)
            {
                trap_SendServerCommand(self - g_entities, va("print \"%s ^7had ^3%i HEALTH ^7and ^5%i ARMOR ^7left.\n\"", attacker->client->pers.netname, attacker->client->ps.stats[STAT_HEALTH], attacker->client->ps.stats[STAT_ARMOR]));
            }
        }
        else if (attacker != self)
        {
            attacker->client->ospStats->teamKills++;
            self->client->ospStats->killstreak = 0;
            return;
        }
        else
        {
            attacker->client->ospStats->suicides++;
            if (g_killingspree.integer)
            {
                self->client->ospStats->killstreak = 0;
            }
            return;
        }

        if (attack == ATTACK_ALTERNATE)
            attacker->client->ospStats->weapon[ATTACK_ALTERNATE][weapon].kills++;
        else
            attacker->client->ospStats->weapon[ATTACK_NORMAL][weapon].kills++;

        if (attack == ATTACK_ALTERNATE)
            self->client->ospStats->weapon[ATTACK_ALTERNATE][weapon].deaths++;
        else
            self->client->ospStats->weapon[ATTACK_NORMAL][weapon].deaths++;

        if ((hitLocation & HL_HEAD) && logHeadshots)
        {
            trap_SendServerCommand(attacker - g_entities, "cp \"@^1[ ^3HeaDShoT ^1]\n\"");
        }
    }
}

// osp 0x0002de74
int OSP_DeathAnim(gentity_t *self, int hitLocation, vec3_t hitDir)
{
    vec3_t fwd;
    int variant;
    int anim;
    gentity_t *ent;

    ent = self;
    variant = 0;

    AngleVectors(ent->client->ps.viewangles, fwd, NULL, NULL);

    if (hitDir && DotProduct(fwd, hitDir) >= 0)
        variant = 1;
    else
        variant = 0;

    switch (hitLocation)
    {
    case HL_WAIST:
        if (rand() % 2)
            anim = BOTH_DEATH_GROIN_1 + variant;
        else
            anim = BOTH_DEATH_GUT_1 + variant;
        break;
    default:
    case HL_CHEST:
        anim = BOTH_DEATH_CHEST_1 + variant;
        break;
    case HL_CHEST_RT:
        if (irand(1, 10) < 8)
            anim = BOTH_DEATH_SHOULDER_RIGHT_1 + variant;
        else
            anim = BOTH_DEATH_CHEST_1 + variant;
        break;
    case HL_CHEST_LT:
        if (irand(1, 10) < 8)
            anim = BOTH_DEATH_SHOULDER_LEFT_1 + variant;
        else
            anim = BOTH_DEATH_CHEST_1 + variant;
        break;
    case HL_NECK:
        anim = BOTH_DEATH_NECK;
        break;
    case HL_HEAD:
        anim = BOTH_DEATH_HEAD_1 + variant;
        break;
    case HL_LEG_UPPER_LT:
        anim = BOTH_DEATH_THIGH_LEFT_1 + variant;
        break;
    case HL_LEG_LOWER_LT:
    case HL_FOOT_LT:
        anim = BOTH_DEATH_LEGS_LEFT_1 + (rand() % 3);
        break;
    case HL_ARM_LT:
        if (rand() % 2)
            anim = BOTH_DEATH_ARMS_LEFT_1 + variant;
        else
            anim = BOTH_DEATH_SHOULDER_LEFT_1 + variant;
        break;
    case HL_ARM_RT:
        if (rand() % 2)
            anim = BOTH_DEATH_ARMS_RIGHT_1 + variant;
        else
            anim = BOTH_DEATH_SHOULDER_RIGHT_1 + variant;
        break;
    case HL_LEG_UPPER_RT:
        anim = BOTH_DEATH_THIGH_RIGHT_1 + variant;
        break;
    case HL_LEG_LOWER_RT:
    case HL_FOOT_RT:
        anim = BOTH_DEATH_LEGS_RIGHT_1 + (rand() % 3);
        break;
    }

    return anim;
}
