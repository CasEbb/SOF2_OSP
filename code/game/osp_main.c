// osp_main.c
//
#include "g_local.h"

ospClient_t osp_clients[MAX_CLIENTS];
ospStats_t osp_stats[MAX_CLIENTS];
osp_locals_t osp;

typedef struct
{
    vmCvar_t *vmCvar;
    char *cvarName;
    char *defaultString;
    int cvarFlags;
    float mMinValue, mMaxValue;
    int modificationCount;
    qboolean trackChange;
    qboolean teamShader;
} ospCvarTable_t;

vmCvar_t g_spectatorInactivity;
vmCvar_t g_coloredTeamplayNames;
vmCvar_t g_alternateSort;
vmCvar_t g_radarFix;
vmCvar_t g_teamTokensOnly;
vmCvar_t g_awards;
vmCvar_t g_alternateMap;
vmCvar_t g_killingspree;
vmCvar_t osp_friendlyFireMultiplier;
vmCvar_t osp_modversion;
vmCvar_t osp_femaleVIP;
vmCvar_t osp_defaultJailTime;
vmCvar_t osp_weaponsFile;
vmCvar_t osp_realisticMode;
vmCvar_t osp_enemyKillReport;
vmCvar_t osp_noM203nades;
vmCvar_t osp_maxM203PerTeam;
vmCvar_t osp_realisticRespawnInt;
vmCvar_t sv_enableAdminPowers;
vmCvar_t sv_adminfile;
vmCvar_t sv_extracommands;
vmCvar_t sv_refereePassword;
vmCvar_t sv_nolagGlassBreaks;
vmCvar_t sv_messageFiltering;
vmCvar_t sv_ospClient;
vmCvar_t sv_modifiedfiles;
vmCvar_t match_latejoin;
vmCvar_t match_mutespecs;
vmCvar_t server_autoconfig;
vmCvar_t match_swap;
vmCvar_t match_shuffle;
vmCvar_t match_fastRestart;
vmCvar_t match_minplayers;
vmCvar_t match_readypercent;
vmCvar_t match_timeoutcount;
vmCvar_t match_timeoutlength;
vmCvar_t server_motd0;
vmCvar_t server_motd1;
vmCvar_t server_motd2;
vmCvar_t server_motd3;
vmCvar_t server_motd4;
vmCvar_t server_motd5;
vmCvar_t team_maxplayers;
vmCvar_t team_controls;
vmCvar_t team_redName;
vmCvar_t team_blueName;
vmCvar_t vote_limit;
vmCvar_t vote_percent;
vmCvar_t shotgun_debugWeapons;
vmCvar_t shotgun_debugAdjBBox;
vmCvar_t shotgun_adjLeanOffset;
vmCvar_t shotgun_adjLBMaxsZ;
vmCvar_t shotgun_adjLBMinsZ;
vmCvar_t x62c190;
vmCvar_t shotgun_adjDuckedLBMinsZ;
vmCvar_t shotgun_adjPLBMaxsZ;

static ospCvarTable_t ospCvarTable[] = {
    {NULL, "gameversion", "OSP v1.1", CVAR_SERVERINFO | CVAR_INIT | CVAR_ROM, 0.0f, 0.0f, 0, qfalse},

    {&osp_modversion, "osp_modversion", "0", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_SYSTEMINFO | CVAR_LATCH, 0.0f, 2.0f, 0, qfalse},
    {&osp_femaleVIP, "osp_femaleVIP", "0", CVAR_ARCHIVE | CVAR_LATCH, 0.0f, 0.0f, 0, qfalse},
    {&osp_defaultJailTime, "osp_defaultJailTime", "60", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qtrue},
    {&osp_friendlyFireMultiplier, "osp_friendlyFireMultiplier", "1", CVAR_ARCHIVE | CVAR_SERVERINFO, 0.01f, 100.0f, 0, qtrue},
    {&osp_weaponsFile, "osp_weaponsFile", "ext_data/sof2.wpn", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_SYSTEMINFO, 0.0f, 0.0f, 0, qtrue},
    {&osp_realisticMode, "osp_realisticMode", "0", CVAR_ARCHIVE | CVAR_SERVERINFO, 0.0f, 1.0f, 0, qtrue},
    {&osp_enemyKillReport, "osp_enemyKillReport", "1", CVAR_ARCHIVE, 0.0f, 1.0f, 0, qtrue},
    {&osp_noM203nades, "osp_noM203nades", "1", CVAR_ARCHIVE | CVAR_LATCH, 0.0f, 0.0f, 0, qtrue},
    {&osp_maxM203PerTeam, "osp_maxM203PerTeam", "2", CVAR_ARCHIVE | CVAR_LATCH, 0.0f, 0.0f, 0, qtrue},
    {&osp_realisticRespawnInt, "osp_realisticRespawnInt", "7", CVAR_ARCHIVE | CVAR_LATCH, 0.0f, 0.0f, 0, qtrue},

    {&g_spectatorInactivity, "g_spectatorInactivity", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&g_coloredTeamplayNames, "g_coloredTeamplayNames", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&g_alternateSort, "g_alternateSort", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&g_radarFix, "g_radarFix", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qtrue},
    {&g_teamTokensOnly, "g_teamTokensOnly", "0", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qtrue},

    {&sv_modifiedfiles, "sv_modifiedfiles", "0", CVAR_SERVERINFO | CVAR_ROM | CVAR_TEMP, 0.0f, 0.0f, 0, qfalse},

    {&g_alternateMap, "g_alternateMap", "0", CVAR_LATCH, 0.0f, 0.0f, 0, qfalse},
    {&g_awards, "g_awards", "2", CVAR_ARCHIVE | CVAR_LATCH, 0.0f, 0.0f, 0, qfalse},
    {&g_killingspree, "g_killingspree", "0", CVAR_ARCHIVE | CVAR_LATCH, 0.0f, 0.0f, 0, qfalse},

    {&sv_enableAdminPowers, "sv_enableAdminPowers", "1", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_LATCH, 0.0f, 0.0f, 0, qtrue},
    {&sv_adminfile, "sv_adminfile", "osp_admins.cfg", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&sv_refereePassword, "sv_refereePassword", "none", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&sv_extracommands, "sv_extracommands", "osp_extracommands.cfg", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&sv_nolagGlassBreaks, "sv_nolagGlassBreaks", "1", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qtrue},
    {&sv_messageFiltering, "sv_messageFiltering", "0", CVAR_ARCHIVE | CVAR_LATCH, 0.0f, 0.0f, 0, qfalse},
    {&sv_ospClient, "sv_ospClient", "1", CVAR_ARCHIVE | CVAR_SERVERINFO | CVAR_LATCH, 0.0f, 0.0f, 0, qfalse},

    {&match_swap, "match_swap", "0", 0, 0.0f, 0.0f, 0, qtrue},
    {&match_shuffle, "match_shuffle", "0", 0, 0.0f, 0.0f, 0, qtrue},
    {&match_fastRestart, "match_fastRestart", "0", 0, 0.0f, 0.0f, 0, qtrue},
    {&match_minplayers, "match_minplayers", "2", 0, 0.0f, 0.0f, 0, qtrue},
    {&match_readypercent, "match_readypercent", "100", 0, 0.0f, 0.0f, 0, qtrue},
    {&match_timeoutcount, "match_timeoutcount", "3", 0, 0.0f, 0.0f, 0, qtrue},
    {&match_timeoutlength, "match_timeoutlength", "180", 0, 0.0f, 0.0f, 0, qtrue},
    {&match_latejoin, "match_latejoin", "1", 0, 0.0f, 1.0f, 0, qfalse},
    {&match_mutespecs, "match_mutespecs", "0", 0, 0.0f, 1.0f, 0, qfalse},

    {&server_autoconfig, "server_autoconfig", "0", 0, 0.0f, 0.0f, 0, qfalse},
    {&server_motd0, "server_motd0", "", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&server_motd1, "server_motd1", "", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&server_motd2, "server_motd2", "", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&server_motd3, "server_motd3", "", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&server_motd4, "server_motd4", "", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},
    {&server_motd5, "server_motd5", "", CVAR_ARCHIVE, 0.0f, 0.0f, 0, qfalse},

    {&team_maxplayers, "team_maxplayers", "0", 0, 0.0f, 0.0f, 0, qtrue},
    {&team_controls, "team_controls", "0", 0, 0.0f, 0.0f, 0, qtrue},
    {&team_redName, "team_redName", "", CVAR_SERVERINFO | CVAR_TEMP, 0.0f, 0.0f, 0, qfalse},
    {&team_blueName, "team_blueName", "", CVAR_SERVERINFO | CVAR_TEMP, 0.0f, 0.0f, 0, qfalse},

    {&vote_limit, "vote_limit", "3", 0, 0.0f, 0.0f, 0, qtrue},
    {&vote_percent, "vote_percent", "50", 0, 0.0f, 0.0f, 0, qtrue},

    {&shotgun_debugWeapons, "shotgun_debugWeapons", "0", CVAR_INTERNAL, 0.0f, 0.0f, 0, qfalse},
    {&shotgun_debugAdjBBox, "shotgun_debugAdjBBox", "0", CVAR_INTERNAL, 0.0f, 0.0f, 0, qfalse},
    {&shotgun_adjLeanOffset, "shotgun_adjLeanOffset", "0.65", CVAR_INTERNAL, 0.0f, 0.0f, 0, qfalse},
    {&shotgun_adjLBMaxsZ, "shotgun_adjLBMaxsZ", "-3", CVAR_INTERNAL, 0.0f, 0.0f, 0, qfalse},
    {&shotgun_adjLBMinsZ, "shotgun_adjLBMinsZ", "54", CVAR_INTERNAL, 0.0f, 0.0f, 0, qfalse},
    {&shotgun_adjDuckedLBMinsZ, "shotgun_adjDuckedLBMinsZ", "25", CVAR_INTERNAL, 0.0f, 0.0f, 0, qfalse},
    {&shotgun_adjPLBMaxsZ, "shotgun_adjPLBMaxsZ", "-39", CVAR_INTERNAL, 0.0f, 0.0f, 0, qfalse},
};

static int ospCvarTableSize = sizeof(ospCvarTable) / sizeof(ospCvarTable[0]);

// osp: 0x0002e375
qboolean OSP_RegisterCvars(void)
{
    ospCvarTable_t *cv;
    int i;
    qboolean remapped = qfalse;

    for (i = 0, cv = ospCvarTable; i < ospCvarTableSize; i++, cv++)
    {
        trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags, cv->mMinValue, cv->mMaxValue);

        if (cv->vmCvar)
        {
            cv->modificationCount = cv->vmCvar->modificationCount;
        }

        if (cv->teamShader)
        {
            remapped = qtrue;
        }
    }

    return remapped;
}

// osp 0x0002e3d6
qboolean OSP_UpdateCvars(void)
{
    static int x46a8 = 0;
    ospCvarTable_t *cv;
    int i;
    int max;
    qboolean remapped = qfalse;

    if (x46a8 >= ospCvarTableSize)
        x46a8 = 0;

    if (x46a8 + 5 >= ospCvarTableSize)
        max = ospCvarTableSize;
    else
        max = x46a8 + 5;

    for (i = x46a8, cv = &ospCvarTable[x46a8]; i < max; i++, cv++)
    {
        if (cv->vmCvar)
        {
            trap_Cvar_Update(cv->vmCvar);

            if (cv->modificationCount != cv->vmCvar->modificationCount)
            {
                if (Q_stricmp(cv->cvarName, "team_redName") == 0)
                {
                    trap_SetConfigstring(CS_RED_TEAM_NAME, team_redName.string);
                }

                if (Q_stricmp(cv->cvarName, "team_blueName") == 0)
                {
                    trap_SetConfigstring(CS_BLUE_TEAM_NAME, team_blueName.string);
                }

                if (Q_stricmp(cv->cvarName, "team_controls") == 0)
                {
                    OSP_UpdateTeaminfo();
                }

                if (Q_stricmp(cv->cvarName, "osp_modversion") == 0)
                {
                    char mapname[64];
                    trap_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));
                    trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
                }

                cv->modificationCount = cv->vmCvar->modificationCount;

                if (cv->trackChange)
                {
                    trap_SendServerCommand(-1, va("print \"^6CVAR change^7: ^5%s ^7changed to ^3%s^7\n\"", cv->cvarName, cv->vmCvar->string));
                }

                if (cv->teamShader)
                {
                    remapped = qtrue;
                }
            }
        }
    }

    x46a8 = max;
    return remapped;
}

// osp: 0x0002e4d0
static qboolean OSP_LoadSpeclock(void)
{
    char *p;
    {
        fileHandle_t f;
        {
            char mapname[64];
            {
                char buf[2000];

                trap_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));

                if (trap_FS_FOpenFile("data/speclocks.cfg", &f, FS_READ))
                {
                    trap_FS_Read(buf, sizeof(buf), f);
                    trap_FS_FCloseFile(f);
                    if (p = strstr(buf, mapname))
                    {
                        if (p = strstr(p, " "))
                        {
                            osp.speclockOrigin[0] = atof(p);
                            p++;

                            if (p && (p = strstr(p, " ")))
                            {
                                osp.speclockOrigin[1] = atof(p);
                                p++;

                                if (p && (p = strstr(p, " ")))
                                {
                                    osp.speclockOrigin[2] = atof(p);
                                    p++;

                                    if (p && (p = strstr(p, " ")))
                                    {
                                        osp.speclockYaw = atof(p);
                                        return qtrue;
                                    }
                                }
                            }
                        }
                    }
                }

                return qfalse;
            }
        }
    }
}

// osp: 0x0002e5aa
static int OSP_Checksum(char *filename)
{
    fileHandle_t f;
    int pos, result, len;
    char buf[4], checksum[4];

    pos = 0;
    result = 0;

    if ((len = trap_FS_FOpenFile(filename, &f, FS_READ)) >= 4)
    {
        memset(checksum, 0, sizeof(checksum));

        while (pos < len)
        {
            memset(buf, 0, sizeof(buf));
            trap_FS_Read(buf, sizeof(buf), f);

            checksum[0] ^= buf[0];
            checksum[1] ^= buf[1];
            checksum[2] ^= buf[2];
            checksum[3] ^= buf[3];

            pos += 4;
        }

        trap_FS_FCloseFile(f);

        result += checksum[0] << 24;
        result += checksum[1] << 16;
        result += checksum[2] << 8;
        result += checksum[3];
    }

    return result;
}

// osp: 0x0002e642
void OSP_InitGame(void)
{
    team_t team;

    // Reset teams and load session data
    for (team = TEAM_RED; team <= TEAM_BLUE; team++)
    {
        OSP_ResetTeam(team, qtrue);

        if (team_controls.integer)
        {
            OSP_ReadTeamSessionData(team);
        }
    }

    // update the configstrings
    trap_SetConfigstring(CS_RED_TEAM_NAME, team_redName.string);
    trap_SetConfigstring(CS_BLUE_TEAM_NAME, team_blueName.string);
    OSP_UpdateTeaminfo();

    // load the adminlist
    OSP_LoadAdmins();

    // load swear words
    if (sv_messageFiltering.integer)
    {
        OSP_LoadBadwords();
    }

    // initialize our main structs
    memset(osp_stats, 0, MAX_CLIENTS * sizeof(osp_stats[0]));
    osp.stats = osp_stats;
    memset(osp_clients, 0, MAX_CLIENTS * sizeof(osp_clients[0]));
    osp.clients = osp_clients;

    // set match phase
    OSP_SetMatchphase(-1, 0);

    // load speclocks
    if (OSP_LoadSpeclock())
    {
        Com_Printf("^5speclock load successful\n");
    }
    else
    {
        Com_Printf("^1speclock load failed\n");
    }

    // cache for adm bbq
    G_EffectIndex("explosions/incendiary_explosion_mp.efx");

    // checksums
    {
        qboolean changed;
        int modifiedFiles;
        modifiedFiles = 0;

        changed = (OSP_Checksum("ext_data/sof2.wpn") != CHECKSUM_SOF2_WPN) ? qtrue : qfalse;
        if (changed || strcmp(osp_weaponsFile.string, "ext_data/sof2.wpn") != 0)
        {
            modifiedFiles |= 1;
        }

        changed = (OSP_Checksum("ext_data/sof2.ammo") != CHECKSUM_SOF2_AMMO) ? qtrue : qfalse;
        if (changed)
        {
            modifiedFiles |= 2;
        }

        changed = (OSP_Checksum("inview/sof2.inview") != CHECKSUM_SOF2_INVIEW) ? qtrue : qfalse;
        if (changed)
        {
            modifiedFiles |= 4;
        }

        // sof2++ enabled?
        if (osp_modversion.integer >= 2)
        {
            // result of sof2.wpn is irrelevant, discard
            modifiedFiles &= ~1;

            changed = (OSP_Checksum("data/sof2++.wpn") != CHECKSUM_SOF2PP_WPN) ? qtrue : qfalse;
            if (changed)
            {
                modifiedFiles |= 0x1;
            }

            changed = (OSP_Checksum("data/sof2++.inview") != CHECKSUM_SOF2PP_INVIEW) ? qtrue : qfalse;

            if (changed)
            {
                modifiedFiles |= 4;
            }
        }

        trap_Cvar_Set("sv_modifiedfiles", va("%d", modifiedFiles));
    }
    // osp++
    if (osp_modversion.integer == 1)
    {
        weaponData[WP_M590_SHOTGUN].attack[ATTACK_NORMAL].inaccuracy = 1.0f;
        weaponData[WP_M590_SHOTGUN].attack[ATTACK_NORMAL].maxInaccuracy = 25.0f;
        weaponData[WP_M590_SHOTGUN].attack[ATTACK_NORMAL].damage = 30;

        weaponData[WP_USAS_12_SHOTGUN].attack[ATTACK_NORMAL].inaccuracy = 5.0f;
        weaponData[WP_USAS_12_SHOTGUN].attack[ATTACK_NORMAL].maxInaccuracy = 50.0f;
        weaponData[WP_USAS_12_SHOTGUN].attack[ATTACK_NORMAL].damage = 25;

        weaponData[WP_SIG551].attack[ATTACK_NORMAL].zoomInaccuracy = 130.0f;
        weaponData[WP_SIG551].attack[ATTACK_NORMAL].maxKickAngles[0] = 2.9f;
        weaponData[WP_SIG551].attack[ATTACK_NORMAL].damage = 40;
    }

    // sof2++
    if (osp_modversion.integer == 2)
    {
        weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].damage = 150;
        weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].weaponFlags |= 0x800;
        weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].extraClips = qfalse;
        weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].rV.range = 100;
        weaponData[WP_RPG7_LAUNCHER].attack[ATTACK_NORMAL].fireDelay = 50;

        weaponData[WP_MM1_GRENADE_LAUNCHER].attack[ATTACK_NORMAL].weaponFlags |= PROJECTILE_TIMED;
        weaponData[WP_MM1_GRENADE_LAUNCHER].attack[ATTACK_NORMAL].clipSize = 3;
        weaponData[WP_MM1_GRENADE_LAUNCHER].attack[ATTACK_NORMAL].projectileLifetime = 4000;
        weaponData[WP_MM1_GRENADE_LAUNCHER].attack[ATTACK_NORMAL].bounceScale = 0.25f;

        bg_itemlist[MODELINDEX_WEAPON_M15].outfittingGroup = OUTFITTING_GROUP_ACCESSORY;
        bg_itemlist[MODELINDEX_WEAPON_ANM14].outfittingGroup = OUTFITTING_GROUP_ACCESSORY;
        bg_itemlist[MODELINDEX_WEAPON_M84].outfittingGroup = OUTFITTING_GROUP_ACCESSORY;

        bg_outfittingGroups[OUTFITTING_GROUP_PISTOL][0] = MODELINDEX_WEAPON_M19;
        bg_outfittingGroups[OUTFITTING_GROUP_PISTOL][1] = MODELINDEX_WEAPON_SOCOM;
        bg_outfittingGroups[OUTFITTING_GROUP_PISTOL][2] = MODELINDEX_WEAPON_SILVERTALON;
        bg_outfittingGroups[OUTFITTING_GROUP_PISTOL][3] = MODELINDEX_WEAPON_SOCOM_SILENCED;

        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][0] = MODELINDEX_WEAPON_AK74;
        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][1] = MODELINDEX_WEAPON_M4;
        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][2] = MODELINDEX_WEAPON_SIG551;
        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][3] = MODELINDEX_WEAPON_USAS12;
        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][4] = MODELINDEX_WEAPON_MSG90A1;
        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][5] = MODELINDEX_WEAPON_M60;
        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][6] = MODELINDEX_WEAPON_MP5;
        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][7] = MODELINDEX_WEAPON_RPG7;
        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][8] = MODELINDEX_WEAPON_MM1;
        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][9] = MODELINDEX_WEAPON_OICW;
        bg_outfittingGroups[OUTFITTING_GROUP_PRIMARY][10] = -1;

        bg_outfittingGroups[OUTFITTING_GROUP_GRENADE][0] = MODELINDEX_WEAPON_SMOHG92;
        bg_outfittingGroups[OUTFITTING_GROUP_GRENADE][1] = MODELINDEX_WEAPON_M67;
        bg_outfittingGroups[OUTFITTING_GROUP_GRENADE][2] = MODELINDEX_WEAPON_MDN11;
        bg_outfittingGroups[OUTFITTING_GROUP_GRENADE][3] = MODELINDEX_WEAPON_F1;
        bg_outfittingGroups[OUTFITTING_GROUP_GRENADE][4] = MODELINDEX_WEAPON_L2A2;
        bg_outfittingGroups[OUTFITTING_GROUP_GRENADE][5] = -1;

        bg_outfittingGroups[OUTFITTING_GROUP_ACCESSORY][0] = MODELINDEX_WEAPON_M15;
        bg_outfittingGroups[OUTFITTING_GROUP_ACCESSORY][1] = MODELINDEX_WEAPON_M84;
        bg_outfittingGroups[OUTFITTING_GROUP_ACCESSORY][2] = MODELINDEX_WEAPON_ANM14;
        bg_outfittingGroups[OUTFITTING_GROUP_ACCESSORY][3] = MODELINDEX_AMMO;
        bg_outfittingGroups[OUTFITTING_GROUP_ACCESSORY][4] = MODELINDEX_THERMAL;
        bg_outfittingGroups[OUTFITTING_GROUP_ACCESSORY][5] = MODELINDEX_NIGHTVISION;
        bg_outfittingGroups[OUTFITTING_GROUP_ACCESSORY][6] = -1;
    }
}

// osp: 0x0002e82c
void OSP_GlobalSound(char *path)
{
    gentity_t *te;

    te = G_TempEntity(level.intermission_origin, EV_GLOBAL_SOUND);
    te->s.eventParm = G_SoundIndex(path);
    te->r.svFlags |= SVF_BROADCAST;
}

// osp: 0x0002e857
void OSP_ClientSound(int clientNum, char *path)
{
    gentity_t *te;

    te = G_TempEntity(g_entities[clientNum].r.currentOrigin, EV_ENTITY_SOUND);
    te->r.svFlags = SVF_SINGLECLIENT;
    te->r.singleClient = clientNum;
    te->s.eventParm = G_SoundIndex(path);
    te->s.weapon = clientNum;
}

// osp: 0x0002e88d
static void sub_0002e88d(int clientNum, int arg1, char *path)
{
    gentity_t *te;

    te = G_TempEntity(g_entities[clientNum].r.currentOrigin, EV_GLOBAL_SOUND);
    te->r.svFlags = SVF_SINGLECLIENT;
    te->r.singleClient = clientNum;
    te->s.eventParm = G_SoundIndex(path);
    te->s.weapon = arg1;
}

// osp: 0x0002e8c3
void OSP_ClearStats(int clientNum)
{
    ospStats_t *stats = &osp.stats[clientNum];
    memset(stats, 0, sizeof(ospStats_t));
}

// osp: 0x0002e8d9
qboolean OSP_CanFollow(gentity_t *e, team_t team)
{
    gentity_t *ent = e;

    if (OSP_IsClientAdmin(ent - g_entities, qtrue))
    {
        return qtrue;
    }

    if ((level.time - level.startTime) > 2500)
    {
        if (ospTeams[TEAM_RED].speclocked && TeamCount(-1, TEAM_RED, qfalse) == 0)
        {
            ospTeams[TEAM_RED].speclocked = qfalse;
        }

        if (ospTeams[TEAM_BLUE].speclocked && TeamCount(-1, TEAM_BLUE, qfalse) == 0)
        {
            ospTeams[TEAM_BLUE].speclocked = qfalse;
        }
    }

    return (!ospTeams[team].speclocked ||
            (ent->client->sess.team != TEAM_SPECTATOR && ent->client->sess.team == team) ||
            (ent->client->ospClient->specinvites & team))
               ? qtrue
               : qfalse;
}

// osp: 0x0002e961
int Callvote_StartMatch(gentity_t *e, unsigned int i, char *arg1, char *arg2, int adminAction);
void OSP_CheckVote(void)
{
    if (level.voteExecuteTime && level.voteExecuteTime < level.time)
    {
        level.voteExecuteTime = 0;

        if (osp.callvoteFunc)
        {
            osp.callvoteFunc(0, 0, NULL, NULL, 0);
            osp.callvoteFunc = 0;
        }
    }

    if (!level.voteTime || level.time - level.voteTime < 1000)
    {
        return;
    }

    if (level.time - level.voteTime >= g_voteDuration.integer * 1000)
    {
        trap_SendServerCommand(-1, va("print \"^2Vote FAILED! ^3(%s)\n\"", level.voteString));
        G_LogPrintf("Vote Failed: %s\n", level.voteString);
        level.clients[level.voteClient].voteDelayTime = level.time + g_failedVoteDelay.integer * 60000;
    }
    else
    {
        int percentage = (osp.callvoteFunc == Callvote_StartMatch) ? 75 : vote_percent.integer;
        if (percentage > 99)
            percentage = 99;
        if (percentage < 1)
            percentage = 1;
        trap_SetConfigstring(CS_VOTE_NEEDED, va("%i", ((percentage * level.numVotingClients) / 100) + 1));

        if (level.voteYes > (percentage * level.numVotingClients) / 100)
        {
            if (level.voteYes > level.numVotingClients + 1)
            {
                G_LogPrintf("Referee Setting: %s\n", level.voteString);
            }
            else if (!osp.callvoteFunc)
            {
                char buf[128];
                trap_GetConfigstring(CS_VOTE_STRING, buf, sizeof(buf));
                trap_SendServerCommand(-1, "print \"^5Vote passed!\n\"");
                trap_SendServerCommand(-1, va("cp \"Everyone agrees!\n%s\n\"", buf));
            }
            else
            {
                trap_SendServerCommand(-1, "print \"^5Vote passed!\n\"");
                level.clients[level.voteClient].pers.voteCount--;
                G_LogPrintf("Vote Passed: %s\n", level.voteString);
            }

            level.voteExecuteTime = level.time + 500;
        }
        else if (level.voteNo && level.voteNo >= ((100 - percentage) * level.numVotingClients) / 100)
        {
            if (!osp.callvoteFunc)
            {
                char buf[128];
                trap_GetConfigstring(CS_VOTE_STRING, buf, sizeof(buf));
                trap_SendServerCommand(-1, va("print \"^2Vote FAILED! ^3(%s)\n\"", buf));
            }
            else
            {
                trap_SendServerCommand(-1, va("print \"^2Vote FAILED! ^3(%s)\n\"", level.voteString));
                G_LogPrintf("Vote Failed: %s\n", level.voteString);
                level.clients[level.voteClient].voteDelayTime = level.time + g_failedVoteDelay.integer * 60000;
            }
        }
        else
        {
            return;
        }
    }

    level.voteTime = 0;
    trap_SetConfigstring(CS_VOTE_TIME, "");
}

// osp: 0x0002ead1
int OSP_VoteDisabled(char *vote)
{
    int novote;
    int vote_allow;

    novote = trap_Cvar_VariableIntegerValue(va("novote_%s", vote));
    vote_allow = trap_Cvar_VariableIntegerValue(va("vote_allow_%s", vote));

    return novote;
}

// osp: 0x0002eaff
float OSP_GetKDR(gclient_t *client)
{
    gclient_t *cl = client;

    if (cl->ospStats->kills + cl->ospStats->deaths == 0)
    {
        return 0;
    }

    if (cl->ospStats->kills + cl->ospStats->deaths < 5)
    {
        return 0;
    }

    return (float)cl->ospStats->kills / (cl->ospStats->kills + cl->ospStats->deaths);
}

// osp: 0x0002eb52
int OSP_KDRSort(const void *a, const void *b)
{
    gclient_t *ca;
    gclient_t *cb;

    ca = &level.clients[*(int *)a];
    cb = &level.clients[*(int *)b];

    // sort special clients last
    if (ca->sess.spectatorState == SPECTATOR_SCOREBOARD)
    {
        return 1;
    }

    if (cb->sess.spectatorState == SPECTATOR_SCOREBOARD)
    {
        return -1;
    }

    // then connecting clients
    if (ca->pers.connected == CON_CONNECTING)
    {
        return 1;
    }

    if (cb->pers.connected == CON_CONNECTING)
    {
        return -1;
    }

    // then spectators
    if (ca->sess.team == TEAM_SPECTATOR && cb->sess.team == TEAM_SPECTATOR)
    {
        if (ca->sess.spectatorTime < cb->sess.spectatorTime)
        {
            return -1;
        }

        if (ca->sess.spectatorTime > cb->sess.spectatorTime)
        {
            return 1;
        }

        return 0;
    }

    if (ca->sess.team == TEAM_SPECTATOR)
    {
        return 1;
    }

    if (cb->sess.team == TEAM_SPECTATOR)
    {
        return -1;
    }

    // Round-based gametype? Show alive players first
    if (level.gametypeData->respawnType == RT_NONE)
    {
        if (!G_IsClientDead(ca) && G_IsClientDead(cb))
        {
            return -1;
        }

        if (G_IsClientDead(ca) && !G_IsClientDead(cb))
        {
            return 1;
        }
    }

    // Invalid ping?
    if (ca->ps.ping >= 0 && ca->ps.ping < 999 && (cb->ps.ping < 0 || cb->ps.ping >= 999))
    {
        return -1;
    }

    if ((ca->ps.ping < 0 || ca->ps.ping >= 999) && cb->ps.ping >= 0 && cb->ps.ping < 999)
    {
        return 1;
    }

    // then sort by KDR
    if (OSP_GetKDR(ca) > OSP_GetKDR(cb))
    {
        return -1;
    }

    if (OSP_GetKDR(ca) < OSP_GetKDR(cb))
    {
        return 1;
    }

    // then sort by kills
    if (ca->sess.kills > cb->sess.kills)
    {
        return -1;
    }

    if (ca->sess.kills < cb->sess.kills)
    {
        return 1;
    }

    // then sort by deaths
    if (ca->sess.deaths > cb->sess.deaths)
    {
        return 1;
    }

    if (ca->sess.deaths < cb->sess.deaths)
    {
        return -1;
    }

    return 0;
}

// osp: 0x0002ecfd
float OSP_GetKPM(gclient_t *client)
{
    gclient_t *cl = client;

    if (!cl->sess.kills)
    {
        return 0;
    }

    return (float)cl->sess.kills / ((level.time - cl->pers.enterTime) / 1000);
}

// osp: 0x0002ed22
int OSP_KPMSort(const void *a, const void *b)
{
    gclient_t *ca;
    gclient_t *cb;

    ca = &level.clients[*(int *)a];
    cb = &level.clients[*(int *)b];

    // sort special clients last
    if (ca->sess.spectatorState == SPECTATOR_SCOREBOARD)
    {
        return 1;
    }

    if (cb->sess.spectatorState == SPECTATOR_SCOREBOARD)
    {
        return -1;
    }

    // then connecting clients
    if (ca->pers.connected == CON_CONNECTING)
    {
        return 1;
    }

    if (cb->pers.connected == CON_CONNECTING)
    {
        return -1;
    }

    // then spectators
    if (ca->sess.team == TEAM_SPECTATOR && cb->sess.team == TEAM_SPECTATOR)
    {
        if (ca->sess.spectatorTime < cb->sess.spectatorTime)
        {
            return -1;
        }

        if (ca->sess.spectatorTime > cb->sess.spectatorTime)
        {
            return 1;
        }

        return 0;
    }

    if (ca->sess.team == TEAM_SPECTATOR)
    {
        return 1;
    }

    if (cb->sess.team == TEAM_SPECTATOR)
    {
        return -1;
    }

    // Round-based gametype? Show alive players first
    if (level.gametypeData->respawnType == RT_NONE)
    {
        if (!G_IsClientDead(ca) && G_IsClientDead(cb))
        {
            return -1;
        }

        if (G_IsClientDead(ca) && !G_IsClientDead(cb))
        {
            return 1;
        }
    }

    // Invalid ping?
    if (ca->ps.ping >= 0 && ca->ps.ping < 999 && (cb->ps.ping < 0 || cb->ps.ping >= 999))
    {
        return -1;
    }

    if ((ca->ps.ping < 0 || ca->ps.ping >= 999) && cb->ps.ping >= 0 && cb->ps.ping < 999)
    {
        return 1;
    }

    // then sort by KPM
    if (OSP_GetKPM(ca) > OSP_GetKPM(cb))
    {
        return -1;
    }

    if (OSP_GetKPM(ca) < OSP_GetKPM(cb))
    {
        return 1;
    }

    // then sort by kills
    if (ca->sess.kills > cb->sess.kills)
    {
        return -1;
    }

    if (ca->sess.kills < cb->sess.kills)
    {
        return 1;
    }

    // then sort by deaths
    if (ca->sess.deaths > cb->sess.deaths)
    {
        return 1;
    }

    if (ca->sess.deaths < cb->sess.deaths)
    {
        return -1;
    }

    return 0;
}

// osp: 0x0002eecd
static void sub_0002eecd(void)
{
    qtime_t time;
    char *s;

    trap_RealTime(&time);
    s = va("shoutcast/dc%i@[%d-%d-%d][%d-%d-%d]",
           rand() % 1000,
           time.tm_year + 1900,
           time.tm_mon + 1,
           time.tm_mday,
           time.tm_hour,
           time.tm_min,
           time.tm_sec);
    trap_SendServerCommand(-1, va("print \"Democast filename: %s\n\"", "shoutcast/tsn"));
    OSP_GlobalSound("shoutcast/tsn");
}

// osp: 0x0002ef13
void OSP_SetMatchphase(int oldPhase, int newPhase)
{
    osp.matchPhase = newPhase;
    trap_SetConfigstring(CS_MATCH_PHASE, va("%d", osp.matchPhase));

    if (oldPhase == 0)
    {
        if (newPhase == 1)
        {
            level.warmupTime = 0;
            trap_SetConfigstring(CS_WARMUP, "0");
            osp.x6479a0 = level.gametypeData->respawnType;
            level.gametypeData->respawnType = RT_NORMAL;
            osp.x6479a4 = level.bodySinkTime;
            level.bodySinkTime = 10000;
        }

        if (newPhase == 2 && osp.x6479a0 < 0)
        {
            osp.x6479a0 = level.gametypeData->respawnType;
            level.gametypeData->respawnType = RT_NORMAL;
            osp.x6479a4 = level.bodySinkTime;
            level.bodySinkTime = 10000;
        }
    }

    if (oldPhase == 1 && newPhase == 2)
    {
        level.warmupTime = -1;
        trap_SetConfigstring(CS_WARMUP, "-1");
        G_LogPrintf("Warmup starts\n");
    }

    if (newPhase == 3)
    {
        G_LogPrintf("Match starts\n");

        if (sv_ospClient.integer)
        {
            trap_SendServerCommand(-1, "ar_start");
        }
    }

    if (newPhase == 4)
    {
        G_LogPrintf("Match ends\n");
        osp.x6479b0 = level.time + 1500;

        if (g_awards.integer)
        {
            OSP_CalculateAwards();
        }
    }
}

// osp: 0x0002efae
void OSP_CheckMatchphase(void)
{
    static int x46ac = 0;
    static int x46b0 = 0;

    if (osp.x6479a8 && osp.x6479a8 < level.time)
    {
        trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
    }

    switch (osp.matchPhase)
    {
    case 0:
    {
        osp.x6479a0 = -1;
        osp.x6479a4 = -1;

        if (g_doWarmup.integer == 2)
            OSP_SetMatchphase(0, 1);
        else if (g_doWarmup.integer == 1)
            OSP_SetMatchphase(0, 2);
        else
            OSP_SetMatchphase(0, 3);

        break;
    }

    case 1:
    {
        if (OSP_PlayersReady())
        {
            if (!x46ac)
                x46ac = level.time + 3000;

            if (x46ac && x46ac < level.time)
                OSP_SetMatchphase(1, 2);
        }
        else
        {
            x46ac = 0;
        }

        break;
    }

    case 2:
    {
        if (!x46b0 && g_doWarmup.integer == 2)
        {
            x46b0 = 1;
            trap_SetConfigstring(CS_GAMETYPE_MESSAGE,
                                 va("%i,%s ^7(^1RED^7) VS %s ^7(^4BLUE^7)",
                                    level.warmupTime > level.time + 3000 ? level.warmupTime : level.time + 3000,
                                    team_redName.string,
                                    team_blueName.string));
        }

        if (level.warmupTime > 0 && level.gametypeData->teams)
            OSP_CheckEmptyTeams();

        if (!level.warmupTime)
            OSP_SetMatchphase(2, 3);

        break;
    }

    case 3:
    {
        if (g_doWarmup.integer && level.gametypeData->teams)
            OSP_CheckEmptyTeams();

        if (level.intermissiontime)
            OSP_SetMatchphase(3, 4);

        break;
    }

    case 4:
    {
        if (osp.x6479b0 && osp.x6479b0 < level.time)
        {
            osp.x6479b0 = 0;

            if (sv_ospClient.integer)
            {
                trap_SendServerCommand(-1, "ar_end");
                trap_SendServerCommand(-1, "autoss");
            }
        }
    }
    }
}

// osp: 0x0002f0bc
qboolean OSP_CheckPause(int msec)
{
    static int scoreboardMessageTime = 0;
    int i;
    gentity_t *ent;

    // skip if not paused
    if (!osp.paused)
    {
        return qfalse;
    }

    // update scores once a sec
    if (scoreboardMessageTime < level.time)
    {
        scoreboardMessageTime = level.time + 1000;
        CalculateRanks();
        SendScoreboardMessageToAllClients();
    }

    // advance game timers
    level.startTime += msec;
    if (level.gametypeStartTime)
        level.gametypeStartTime += msec;
    if (level.gametypeDelayTime)
        level.gametypeDelayTime += msec;
    if (level.gametypeJoinTime)
        level.gametypeJoinTime += msec;
    if (level.gametypeResetTime)
        level.gametypeResetTime += msec;
    if (level.gametypeRespawnTime[0])
        level.gametypeRespawnTime[0] += msec;
    if (level.gametypeRespawnTime[1])
        level.gametypeRespawnTime[1] += msec;
    if (level.gametypeRespawnTime[2])
        level.gametypeRespawnTime[2] += msec;
    if (level.gametypeRoundTime)
        level.gametypeRoundTime += msec;
    if (level.warmupTime > 0)
        level.warmupTime += msec;

    // advance entity timers
    for (ent = g_entities, i = 0; i < level.num_entities; i++, ent++)
    {
        if (!ent->inuse)
            continue;

        if (ent->s.event)
            ent->eventTime += msec;

        if (ent->s.eType != ET_GENERAL)
            ent->s.pos.trTime += msec;

        if (ent->think && ent->nextthink > level.time)
            ent->nextthink += msec;

        if (ent->client && ent->client->ospClient)
        {
            ent->client->inactivityTime += msec;
            ent->client->airOutTime += msec;
            ent->pain_debounce_time += msec;
            ent->client->pers.enterTime += msec;
            if (ent->client->ps.respawnTimer > level.time)
                ent->client->ps.respawnTimer += msec;
            if (ent->client->ospClient->jailTime)
                ent->client->ospClient->jailTime += msec;
            if (ent->client->ospStats->x3c)
                ent->client->ospStats->x3c += msec;
        }

        if (ent->timestamp)
            ent->timestamp += msec;
    }

    // countdown to zero? back to work
    if (osp.unpauseTime && osp.unpauseTime < level.time)
    {
        OSP_Resume();
        return qfalse;
    }

    // counting down
    if (osp.unpauseTime)
    {
        OSP_UnpauseCountdown();
    }

    // timeout out of time?
    if (osp.timeoutTimeout && osp.timeoutTimeout < level.time)
    {
        OSP_Unpause(NULL, qtrue);
        return qtrue;
    }

    return qtrue;
}

// osp: 0x0002f2e0
void OSP_Pause(gentity_t *e, qboolean timeout)
{
    int i;
    gentity_t *ent;
    team_t team;
    int x28;

    ent = e;
    team = ent->client->sess.team;
    x28 = (team == TEAM_RED) ? PP_TIMEOUT_RED : PP_TIMEOUT_BLUE;

    if (timeout)
    {
        osp.paused = x28;
    }
    else
    {
        osp.paused = PP_ADMIN;
    }

    trap_Cvar_Set("gt_paused", "1");
    trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.time));

    if (timeout)
    {
        trap_SetConfigstring(CS_GAMETYPE_MESSAGE,
                             va("%i,%s calls a %stimeout (%d timeouts remain)",
                                level.time + 5000,
                                ent->client->pers.netname,
                                match_timeoutlength.integer ? va("%d seconds ", match_timeoutlength.integer) : "",
                                ospTeams[team].timeoutsLeft));
        trap_SetConfigstring(CS_GAMETYPE_TIMER, va("%i", level.time + (1000 * match_timeoutlength.integer)));

        if (!match_timeoutlength.integer)
            osp.timeoutTimeout = level.time + (4 * 60 * 60 * 1000);
        else
            osp.timeoutTimeout = level.time + (1000 * match_timeoutlength.integer);
    }
    else
    {
        trap_SetConfigstring(CS_GAMETYPE_MESSAGE, va("%i,^5Admin pauses the game", level.time + 5000));
        trap_SetConfigstring(CS_GAMETYPE_TIMER, "0");
    }

    for (i = 0; i < level.maxclients; i++)
    {
        if (level.clients[i].pers.connected != CON_CONNECTED)
        {
            continue;
        }

        VectorCopy(level.clients[i].ps.viewangles, osp.x646e70[i]);
    }

    OSP_GlobalSound("sound/misc/events/buzz01.wav");
}

// osp: 0x0002f3e0
void OSP_UnpauseCountdown(void)
{
    if (osp.unpauseCountdown <= (osp.unpauseTime - level.time) / 1000)
    {
        return;
    }

    osp.unpauseCountdown = (osp.unpauseTime - level.time - 1) / 1000;
    OSP_GlobalSound("sound/misc/events/buzz02.wav");

    switch (osp.unpauseType)
    {
    case 0:
        trap_SetConfigstring(CS_GAMETYPE_MESSAGE,
                             va("%i,@Time out has elapsed, restart in %d seconds",
                                level.time + 2000,
                                osp.unpauseCountdown + 1));
        break;
    case 1:
        trap_SetConfigstring(CS_GAMETYPE_MESSAGE,
                             va("%i,@%s resumes the match, restart in %d seconds",
                                level.time + 2000,
                                level.clients[osp.unpauseClient].pers.netname,
                                osp.unpauseCountdown + 1));
        break;
    case 2:
        trap_SetConfigstring(CS_GAMETYPE_MESSAGE,
                             va("%i,@^5Admin resumes the game, restart in %d seconds",
                                level.time + 2000,
                                osp.unpauseCountdown + 1));
        break;
    }
}

// osp: 0x0002f467
void OSP_Unpause(gentity_t *e, qboolean timeout)
{
    int i;
    gentity_t *ent;

    ent = e;

    osp.paused = PP_UNPAUSING;
    osp.unpauseTime = level.time + 5000;
    osp.timeoutTimeout = 0;

    if (!ent)
    {
        osp.unpauseType = 0;
    }
    else if (timeout)
    {
        osp.unpauseType = 1;
        osp.unpauseClient = ent - g_entities;
    }
    else
    {
        osp.unpauseType = 2;
    }

    osp.unpauseCountdown = 6;
    OSP_UnpauseCountdown();
    trap_SetConfigstring(CS_GAMETYPE_TIMER, va("%i", level.time + 5000));

    for (i = 0; i < level.maxclients; i++)
    {
        if (level.clients[i].pers.connected != CON_CONNECTED)
        {
            continue;
        }

        SetClientViewAngle(&g_entities[i], osp.x646e70[i]);
    }
}

// osp: 0x002f4df
void OSP_Resume(void)
{
    osp.paused = PP_UNPAUSED;
    osp.unpauseTime = 0;

    trap_Cvar_Set("gt_paused", "0");

    trap_SetConfigstring(CS_LEVEL_START_TIME, va("%i", level.startTime));
    trap_SetConfigstring(CS_GAMETYPE_TIMER, va("%i", level.gametypeRoundTime));

    if (level.warmupTime > 0)
        trap_SetConfigstring(CS_WARMUP, va("%i", level.warmupTime));

    trap_SetConfigstring(CS_GAMETYPE_MESSAGE, va("%i,GO!", level.time + 1000));
}

// osp: 0x0002f539
void OSP_LoadBadwords(void)
{
    fileHandle_t f = 0;
    int len = 0;

    len = trap_FS_FOpenFile("osp_messagefilter.cfg", &f, FS_READ);

    if (!f)
    {
        return;
    }

    if (len > 2095)
    {
        len = 2095;
    }

    memset(osp.badwords, 0, sizeof(osp.badwords));
    trap_FS_Read(osp.badwords, len, f);
    trap_FS_FCloseFile(f);
}

// osp: 0x0002f577
char *sub_0002f577(char *s1, char *s2)
{
    char c1;
    char c2;

    while (*s1)
    {
        int i;

        for (i = 0; s2[i] != '\0'; i++)
        {
            c1 = tolower(s1[i]);
            c2 = tolower(s2[i]);

            if (c1 != c2)
            {
                break;
            }
        }

        if (s2[i] == '\0')
        {
            return s1;
        }

        s1++;
    }

    return NULL;
}

// osp: 0x0002f5d5
void OSP_UpdateTeamOverlay(void)
{
    static int lastUpdateTime = 0;

    gentity_t *ent;
    int i;
    char redInfo[1024];
    char blueInfo[1024];
    gentity_t *location;
    int redCount;
    int blueCount;

    redCount = 0;
    blueCount = 0;

    // skip if serverside only
    if (!sv_ospClient.integer)
    {
        return;
    }

    // skip if there aren't teams
    if (!level.gametypeData->teams)
    {
        return;
    }

    // update once in 2.5s
    if (level.time - lastUpdateTime < 2500)
        return;
    lastUpdateTime = level.time;

    memset(redInfo, 0, sizeof(redInfo));
    memset(blueInfo, 0, sizeof(blueInfo));

    // update client locations
    for (i = 0; i < level.maxclients; i++)
    {
        ent = g_entities + i;

        if (ent->client->pers.connected != CON_CONNECTED)
            continue;

        if (ent->inuse && (ent->client->sess.team == TEAM_RED || ent->client->sess.team == TEAM_BLUE))
        {
            location = Team_GetLocation(ent, qtrue);

            if (location && location->health >= 0 && location->health < MAX_LOCATIONS)
                ent->client->pers.teamState.location = location->health;
            else
                ent->client->pers.teamState.location = 0;
        }
    }

    CalculateRanks();

    // build string for each team
    for (i = 0; i < level.numConnectedClients; i++)
    {
        ent = &g_entities[level.sortedClients[i]];

        if (ent->client->pers.connected != CON_CONNECTED)
            continue;

        if (ent->client->sess.team == TEAM_RED && redCount < 8)
        {
            redCount++;
            Com_sprintf(strlen(redInfo) + redInfo, sizeof(redInfo) - strlen(redInfo),
                        "%i %i %i %i %i %i ",
                        level.sortedClients[i],
                        (G_IsClientSpectating(ent->client) || ent->client->ps.stats[STAT_HEALTH] <= 0) ? 0 : ent->client->ps.stats[STAT_HEALTH],
                        ent->client->ps.stats[STAT_ARMOR] > 0 ? ent->client->ps.stats[STAT_ARMOR] : 0,
                        ent->client->pers.teamState.location > 0 ? ent->client->pers.teamState.location : 0,
                        ent->client->ps.weapon,
                        ent->s.gametypeitems);
        }

        if (ent->client->sess.team == TEAM_BLUE && blueCount < 8)
        {
            blueCount++;
            Com_sprintf(strlen(blueInfo) + blueInfo, sizeof(blueInfo) - strlen(blueInfo),
                        "%i %i %i %i %i %i ",
                        level.sortedClients[i],
                        (G_IsClientSpectating(ent->client) || ent->client->ps.stats[STAT_HEALTH] <= 0) ? 0 : ent->client->ps.stats[STAT_HEALTH],
                        ent->client->ps.stats[STAT_ARMOR] > 0 ? ent->client->ps.stats[STAT_ARMOR] : 0,
                        ent->client->pers.teamState.location > 0 ? ent->client->pers.teamState.location : 0,
                        ent->client->ps.weapon,
                        ent->s.gametypeitems);
        }
    }

    // dispatch to the correct clients
    for (i = 0; i < level.maxclients; i++)
    {
        ent = g_entities + i;

        if (osp.clients[i].clientVersion <= 0)
            continue;

        if (!osp.clients[i].teamOverlay)
            continue;

        if (ent->client->pers.connected == CON_CONNECTED)
        {
            if (ent->client->sess.team == TEAM_RED ||
                (G_IsClientSpectating(ent->client) &&
                 ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
                 g_entities[ent->client->sess.spectatorClient].client->sess.team == TEAM_RED))
            {
                trap_SendServerCommand(ent - g_entities, va("ti %s", redInfo));
            }

            if (ent->client->sess.team == TEAM_BLUE ||
                (G_IsClientSpectating(ent->client) &&
                 ent->client->sess.spectatorState == SPECTATOR_FOLLOW &&
                 g_entities[ent->client->sess.spectatorClient].client->sess.team == TEAM_BLUE))
            {
                trap_SendServerCommand(ent - g_entities, va("ti %s", blueInfo));
            }
        }
    }
}

// osp: 0x0002f8e0
const char *OSP_ReadyMask(void)
{
    int i;
    gentity_t *ent;
    int readyMask[2];

    readyMask[0] = 0;
    readyMask[1] = 0;

    for (i = 0; i < level.maxclients; i++)
    {
        ent = g_entities + i;

        if (ent->client->pers.connected != CON_CONNECTED)
        {
            continue;
        }

        if (ent->client->sess.team != TEAM_RED && ent->client->sess.team != TEAM_BLUE)
        {
            continue;
        }

        if (i < 32)
        {
            readyMask[0] |= ent->client->ospClient->ready << i;
        }
        else
        {
            readyMask[1] |= ent->client->ospClient->ready << (i - 32);
        }
    }

    return va("%d %d", readyMask[0], readyMask[1]);
}

// osp: 0x0002f95e
void OSP_UpdateTeaminfo(void)
{
    char buf[128];

    memset(buf, 0, sizeof(buf));
    Com_sprintf(buf, sizeof(buf),
                "\\paused\\%d\\teamc\\%d\\rtj\\%d\\btj\\%d\\rsl\\%d\\bsl\\%d",
                osp.paused,
                team_controls.integer,
                ospTeams[TEAM_RED].locked,
                ospTeams[TEAM_BLUE].locked,
                ospTeams[TEAM_RED].speclocked,
                ospTeams[TEAM_BLUE].speclocked);

    trap_SetConfigstring(CS_OSP_TEAMINFO, buf);
}

// osp: 0x0002f98c
void OSP_CalculateAwards(void)
{
    int val;
    int i;
    int best[MAX_AWARDS] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
    float temp;

    memset(osp.awards, 0, sizeof(osp.awards));

    for (i = 0; i < level.maxclients; i++)
    {
        gentity_t *ent;
        ospStats_t *stats;

        ent = g_entities + i;

        if (!ent->client)
        {
            continue;
        }

        if (!ent->client->ospStats)
        {
            continue;
        }

        if (!ent->client->pers.connected == CON_CONNECTED)
        {
            continue;
        }

        stats = ent->client->ospStats;

        if (stats->kills && stats->kills >= osp.awards[AWARD_LETHAL].score)
        {
            if (stats->kills == osp.awards[AWARD_LETHAL].score)
            {
                if (stats->damageGiven > osp.awards[AWARD_LETHAL].tiebreaker)
                {
                    best[AWARD_LETHAL] = i;
                    osp.awards[AWARD_LETHAL].score = stats->kills;
                    osp.awards[AWARD_LETHAL].tiebreaker = stats->damageGiven;
                }
            }
            else
            {
                best[AWARD_LETHAL] = i;
                osp.awards[AWARD_LETHAL].score = stats->kills;
                osp.awards[AWARD_LETHAL].tiebreaker = stats->damageGiven;
            }
        }

        if (stats->damageGiven && stats->damageGiven >= osp.awards[AWARD_DAMAGE].score)
        {
            if (stats->damageGiven == osp.awards[AWARD_DAMAGE].score)
            {
                if (osp.awards[AWARD_DAMAGE].tiebreaker && stats->damageTaken < osp.awards[AWARD_DAMAGE].tiebreaker)
                {
                    best[AWARD_DAMAGE] = i;
                    osp.awards[AWARD_DAMAGE].score = stats->damageGiven;
                    osp.awards[AWARD_DAMAGE].tiebreaker = stats->damageTaken;
                }
            }
            else
            {
                best[AWARD_DAMAGE] = i;
                osp.awards[AWARD_DAMAGE].score = stats->damageGiven;
                osp.awards[AWARD_DAMAGE].tiebreaker = stats->damageTaken;
            }
        }

        val = OSP_NumHeadshots(stats);
        if (val && val >= osp.awards[AWARD_HEADSHOTS].score)
        {
            if (val == osp.awards[AWARD_HEADSHOTS].score)
            {
                if (stats->kills > osp.awards[AWARD_HEADSHOTS].tiebreaker)
                {
                    best[AWARD_HEADSHOTS] = i;
                    osp.awards[AWARD_HEADSHOTS].score = val;
                    osp.awards[AWARD_HEADSHOTS].tiebreaker = stats->kills;
                }
            }
            else
            {
                best[AWARD_HEADSHOTS] = i;
                osp.awards[AWARD_HEADSHOTS].score = val;
                osp.awards[AWARD_HEADSHOTS].tiebreaker = stats->kills;
            }
        }

        val = OSP_ExplosiveKills(stats);
        if (val && val >= osp.awards[AWARD_EXPLOSIVES].score)
        {
            if (val == osp.awards[AWARD_EXPLOSIVES].score)
            {
                if (stats->kills > osp.awards[AWARD_EXPLOSIVES].tiebreaker)
                {
                    best[AWARD_EXPLOSIVES] = i;
                    osp.awards[AWARD_EXPLOSIVES].score = val;
                    osp.awards[AWARD_EXPLOSIVES].tiebreaker = stats->kills;
                }
            }
            else
            {
                best[AWARD_EXPLOSIVES] = i;
                osp.awards[AWARD_EXPLOSIVES].score = val;
                osp.awards[AWARD_EXPLOSIVES].tiebreaker = stats->kills;
            }
        }

        val = OSP_MeleeKills(stats);
        if (val && val >= osp.awards[AWARD_CRAZIEST].score)
        {
            if (val == osp.awards[AWARD_CRAZIEST].score)
            {
                if (stats->kills > osp.awards[AWARD_CRAZIEST].tiebreaker)
                {
                    best[AWARD_CRAZIEST] = i;
                    osp.awards[AWARD_CRAZIEST].score = val;
                    osp.awards[AWARD_CRAZIEST].tiebreaker = stats->kills;
                }
            }
            else
            {
                best[AWARD_CRAZIEST] = i;
                osp.awards[AWARD_CRAZIEST].score = val;
                osp.awards[AWARD_CRAZIEST].tiebreaker = stats->kills;
            }
        }

        val = ent->client->sess.score;
        if (val && val >= osp.awards[AWARD_SCORE].score)
        {
            if (val == osp.awards[AWARD_SCORE].score)
            {
                if (stats->kills > osp.awards[AWARD_SCORE].tiebreaker)
                {
                    best[AWARD_SCORE] = i;
                    osp.awards[AWARD_SCORE].score = val;
                    osp.awards[AWARD_SCORE].tiebreaker = stats->kills;
                }
            }
            else
            {
                best[AWARD_SCORE] = i;
                osp.awards[AWARD_SCORE].score = val;
                osp.awards[AWARD_SCORE].tiebreaker = stats->kills;
            }
        }

        val = OSP_GetKDR(ent->client) * 10000;
        if (val / 100 >= 1 && val >= osp.awards[AWARD_SURVIVOR].score)
        {
            if (val == osp.awards[AWARD_SURVIVOR].score)
            {
                if (stats->damageGiven > osp.awards[AWARD_SURVIVOR].tiebreaker)
                {
                    best[AWARD_SURVIVOR] = i;
                    osp.awards[AWARD_SURVIVOR].score = val;
                    osp.awards[AWARD_SURVIVOR].tiebreaker = stats->damageGiven;
                }
            }
            else
            {
                best[AWARD_SURVIVOR] = i;
                osp.awards[AWARD_SURVIVOR].score = val;
                osp.awards[AWARD_SURVIVOR].tiebreaker = stats->damageGiven;
            }
        }

        val = ent->client->ospStats->suicides;
        if (val && val >= osp.awards[BADAWARD_QUITTER].score)
        {
            if (val == osp.awards[BADAWARD_QUITTER].score)
            {
                if (stats->damageTaken > osp.awards[BADAWARD_QUITTER].tiebreaker)
                {
                    best[BADAWARD_QUITTER] = i;
                    osp.awards[BADAWARD_QUITTER].score = val;
                    osp.awards[BADAWARD_QUITTER].tiebreaker = stats->damageTaken;
                }
            }
            else
            {
                best[BADAWARD_QUITTER] = i;
                osp.awards[BADAWARD_QUITTER].score = val;
                osp.awards[BADAWARD_QUITTER].tiebreaker = stats->damageTaken;
            }
        }

        val = ent->client->ospStats->teamKills;
        if (val && val >= osp.awards[BADAWARD_LEMON].score)
        {
            if (val == osp.awards[BADAWARD_LEMON].score)
            {
                if (stats->teamDamage > osp.awards[BADAWARD_LEMON].tiebreaker)
                {
                    best[BADAWARD_LEMON] = i;
                    osp.awards[BADAWARD_LEMON].score = val;
                    osp.awards[BADAWARD_LEMON].tiebreaker = stats->teamDamage;
                }
            }
            else
            {
                best[BADAWARD_LEMON] = i;
                osp.awards[BADAWARD_LEMON].score = val;
                osp.awards[BADAWARD_LEMON].tiebreaker = stats->teamDamage;
            }
        }

        if ((stats->kills + stats->deaths) >= 5)
        {
            val = (1 - OSP_GetKDR(ent->client)) * 10000.0;

            if ((val / 100) >= 1 && val >= osp.awards[BADAWARD_NEWBIE].score)
            {
                if (val == osp.awards[BADAWARD_NEWBIE].score)
                {
                    if (stats->damageTaken > osp.awards[BADAWARD_NEWBIE].tiebreaker)
                    {
                        best[BADAWARD_NEWBIE] = i;
                        osp.awards[BADAWARD_NEWBIE].score = val;
                        osp.awards[BADAWARD_NEWBIE].tiebreaker = stats->damageTaken;
                    }
                }
                else
                {
                    best[BADAWARD_NEWBIE] = i;
                    osp.awards[BADAWARD_NEWBIE].score = val;
                    osp.awards[BADAWARD_NEWBIE].tiebreaker = stats->damageTaken;
                }
            }
        }

        val = ent->client->ospStats->deaths;
        if (val && val >= osp.awards[BADAWARD_FREEKILL].score)
        {
            if (val == osp.awards[BADAWARD_FREEKILL].score)
            {
                if (stats->damageTaken > osp.awards[BADAWARD_FREEKILL].tiebreaker)
                {
                    best[BADAWARD_FREEKILL] = i;
                    osp.awards[BADAWARD_FREEKILL].score = val;
                    osp.awards[BADAWARD_FREEKILL].tiebreaker = stats->damageTaken;
                }
            }
            else
            {
                best[BADAWARD_FREEKILL] = i;
                osp.awards[BADAWARD_FREEKILL].score = val;
                osp.awards[BADAWARD_FREEKILL].tiebreaker = stats->damageTaken;
            }
        }

        if ((temp = OSP_Accuracy(stats)) != -1.0)
        {
            val = (1 - temp) * 10000.0;

            if (val != 10000 && val >= osp.awards[BADAWARD_CROSSEYED].score)
            {
                if (val == osp.awards[BADAWARD_CROSSEYED].score)
                {
                    if (stats->damageTaken > osp.awards[BADAWARD_CROSSEYED].tiebreaker)
                    {
                        best[BADAWARD_CROSSEYED] = i;
                        osp.awards[BADAWARD_CROSSEYED].score = val;
                        osp.awards[BADAWARD_CROSSEYED].tiebreaker = stats->damageTaken;
                    }
                }
                else
                {
                    best[BADAWARD_CROSSEYED] = i;
                    osp.awards[BADAWARD_CROSSEYED].score = val;
                    osp.awards[BADAWARD_CROSSEYED].tiebreaker = stats->damageTaken;
                }
            }
        }

        val = OSP_NumShots(stats);
        if (val > 50 && val >= osp.awards[BADAWARD_SPAMMER].score)
        {
            if (val == osp.awards[BADAWARD_SPAMMER].score)
            {
                if (stats->damageTaken > osp.awards[BADAWARD_SPAMMER].tiebreaker)
                {
                    best[BADAWARD_SPAMMER] = i;
                    osp.awards[BADAWARD_SPAMMER].score = val;
                    osp.awards[BADAWARD_SPAMMER].tiebreaker = stats->damageTaken;
                }
            }
            else
            {
                best[BADAWARD_SPAMMER] = i;
                osp.awards[BADAWARD_SPAMMER].score = val;
                osp.awards[BADAWARD_SPAMMER].tiebreaker = stats->damageTaken;
            }
        }

        val = stats->numChats;
        if (val > 5 && val >= osp.awards[BADAWARD_CHATTYBITCH].score)
        {
            if (val == osp.awards[BADAWARD_CHATTYBITCH].score)
            {
                if (stats->damageTaken > osp.awards[BADAWARD_CHATTYBITCH].tiebreaker)
                {
                    best[BADAWARD_CHATTYBITCH] = i;
                    osp.awards[BADAWARD_CHATTYBITCH].score = val;
                    osp.awards[BADAWARD_CHATTYBITCH].tiebreaker = stats->damageTaken;
                }
            }
            else
            {
                best[BADAWARD_CHATTYBITCH] = i;
                osp.awards[BADAWARD_CHATTYBITCH].score = val;
                osp.awards[BADAWARD_CHATTYBITCH].tiebreaker = stats->damageTaken;
            }
        }
    }

    // Copy over names and clientnums
    for (i = 0; i < MAX_AWARDS; i++)
    {
        gentity_t *ent;

        if (best[i] < 0 || best[i] >= level.maxclients)
        {
            osp.awards[i].winnerNum = -1;
            Q_strncpyz(osp.awards[i].winnerName, "", 32);
            continue;
        }

        ent = &g_entities[best[i]];

        if (!ent->client)
        {
            continue;
        }

        if (!ent->client->ospStats)
        {
            continue;
        }

        if (!ent->client->pers.connected == CON_CONNECTED)
        {
            continue;
        }

        osp.awards[i].winnerNum = best[i];
        Q_strncpyz(osp.awards[i].winnerName, ent->client->pers.netname, 32);
    }

    // Build configstrings for clients, if needed
    if (sv_ospClient.integer)
    {
        char buf[1024];

        memset(buf, 0, sizeof(buf));

        if (osp.awards[AWARD_LETHAL].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\kN\\%s\\kC\\%i", osp.awards[AWARD_LETHAL].winnerName, osp.awards[AWARD_LETHAL].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\kN\\\\kC\\%i", -1);
        }

        if (osp.awards[AWARD_DAMAGE].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\dN\\%s\\dC\\%i", osp.awards[AWARD_DAMAGE].winnerName, osp.awards[AWARD_DAMAGE].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\dN\\\\dC\\%i", -1);
        }

        if (osp.awards[AWARD_HEADSHOTS].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\hN\\%s\\hC\\%i", osp.awards[AWARD_HEADSHOTS].winnerName, osp.awards[AWARD_HEADSHOTS].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\hN\\\\hC\\%i", -1);
        }

        if (osp.awards[AWARD_EXPLOSIVES].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\gN\\%s\\gC\\%i", osp.awards[AWARD_EXPLOSIVES].winnerName, osp.awards[AWARD_EXPLOSIVES].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\gN\\\\gC\\%i", -1);
        }

        if (osp.awards[AWARD_CRAZIEST].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\knN\\%s\\knC\\%i", osp.awards[AWARD_CRAZIEST].winnerName, osp.awards[AWARD_CRAZIEST].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\knN\\\\knC\\%i", -1);
        }

        if (osp.awards[AWARD_SCORE].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\sN\\%s\\sC\\%i", osp.awards[AWARD_SCORE].winnerName, osp.awards[AWARD_SCORE].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\sN\\\\sC\\%i", -1);
        }

        if (osp.awards[AWARD_SURVIVOR].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\rN\\%s\\rC\\%i", osp.awards[AWARD_SURVIVOR].winnerName, osp.awards[AWARD_SURVIVOR].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\rN\\\\rC\\%i", -1);
        }

        trap_SetConfigstring(CS_AWARDS, buf);
        memset(buf, 0, sizeof(buf));

        if (osp.awards[BADAWARD_QUITTER].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\suN\\%s\\suC\\%i", osp.awards[BADAWARD_QUITTER].winnerName, osp.awards[BADAWARD_QUITTER].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\suN\\\\suC\\%i", -1);
        }

        if (osp.awards[BADAWARD_LEMON].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\lN\\%s\\lC\\%i", osp.awards[BADAWARD_LEMON].winnerName, osp.awards[BADAWARD_LEMON].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\lN\\\\lC\\%i", -1);
        }

        if (osp.awards[BADAWARD_NEWBIE].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\nN\\%s\\nC\\%i", osp.awards[BADAWARD_NEWBIE].winnerName, osp.awards[BADAWARD_NEWBIE].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\nN\\\\nC\\%i", -1);
        }

        if (osp.awards[BADAWARD_FREEKILL].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\etN\\%s\\etC\\%i", osp.awards[BADAWARD_FREEKILL].winnerName, osp.awards[BADAWARD_FREEKILL].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\etN\\\\etC\\%i", -1);
        }

        if (osp.awards[BADAWARD_CROSSEYED].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\ceN\\%s\\ceC\\%i", osp.awards[BADAWARD_CROSSEYED].winnerName, osp.awards[BADAWARD_CROSSEYED].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\ceN\\\\ceC\\%i", -1);
        }

        if (osp.awards[BADAWARD_SPAMMER].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\spN\\%s\\spC\\%i", osp.awards[BADAWARD_SPAMMER].winnerName, osp.awards[BADAWARD_SPAMMER].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\spN\\\\spC\\%i", -1);
        }

        if (osp.awards[BADAWARD_CHATTYBITCH].winnerNum >= 0)
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\cbN\\%s\\cbC\\%i", osp.awards[BADAWARD_CHATTYBITCH].winnerName, osp.awards[BADAWARD_CHATTYBITCH].score);
        }
        else
        {
            Com_sprintf(strlen(buf) + buf, sizeof(buf) - strlen(buf), "\\cbN\\\\cbC\\%i", -1);
        }

        trap_SetConfigstring(CS_BADAWARDS, buf);
    }
}

// osp: 0x00030200
void OSP_PrintAwards(void)
{
    static qboolean awardsPrepared = qfalse;
    static int lastPrintTime = 0;

    int i;

    if (!awardsPrepared && level.intermissiontime + 10000 < level.time)
    {
        int j;

        awardsPrepared = qtrue;

        for (j = 0; j < level.maxclients; j++)
        {
            gentity_t *ent;
            ent = g_entities + j;

            if (ent->client && ent->client->pers.connected == CON_CONNECTED)
            {
                ent->client->ps.pm_type = PM_FREEZE;
                ent->client->ps.stats[STAT_HEALTH] = -1;
            }
        }

        osp.awards[AWARD_LETHAL].shown = qfalse;
        osp.awards[AWARD_LETHAL].showTime = level.intermissiontime + 10500;
        osp.awards[AWARD_HEADSHOTS].shown = qfalse;
        osp.awards[AWARD_HEADSHOTS].showTime = level.intermissiontime + 11000;
        osp.awards[AWARD_EXPLOSIVES].shown = qfalse;
        osp.awards[AWARD_EXPLOSIVES].showTime = level.intermissiontime + 11500;
        osp.awards[AWARD_CRAZIEST].shown = qfalse;
        osp.awards[AWARD_CRAZIEST].showTime = level.intermissiontime + 12000;
        osp.awards[AWARD_SURVIVOR].shown = qfalse;
        osp.awards[AWARD_SURVIVOR].showTime = level.intermissiontime + 12500;
        osp.awards[AWARD_DAMAGE].shown = qfalse;
        osp.awards[AWARD_DAMAGE].showTime = level.intermissiontime + 13000;
        osp.awards[AWARD_SCORE].shown = qfalse;
        osp.awards[AWARD_SCORE].showTime = level.intermissiontime + 13500;

        memset(osp.awardMsg, 0, sizeof(osp.awardMsg));
        osp.awardsStarted = qtrue;
    }

    if (awardsPrepared)
    {
        for (i = AWARD_LETHAL; i < BADAWARD_QUITTER; i++)
        {
            if (!osp.awards[i].shown && osp.awards[i].showTime < level.time)
            {
                osp.awards[i].shown = qtrue;

                if (sv_ospClient.integer)
                {
                    continue;
                }

                OSP_GlobalSound("sound/misc/events/rifle_shot.mp3");

                lastPrintTime = 0;

                switch (i)
                {
                case AWARD_LETHAL:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Lethal] %-32s^7 (%d)\n",
                                osp.awards[AWARD_LETHAL].winnerName,
                                osp.awards[AWARD_LETHAL].score);
                    break;
                case AWARD_HEADSHOTS:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Head Shots] %-32s^7 (%d)\n",
                                osp.awards[AWARD_HEADSHOTS].winnerName,
                                osp.awards[AWARD_HEADSHOTS].score);
                    break;
                case AWARD_EXPLOSIVES:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Explosives] %-32s^7 (%d)\n",
                                osp.awards[AWARD_EXPLOSIVES].winnerName,
                                osp.awards[AWARD_EXPLOSIVES].score);
                    break;
                case AWARD_CRAZIEST:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Craziest] %-32s^7 (%d)\n",
                                osp.awards[AWARD_CRAZIEST].winnerName,
                                osp.awards[AWARD_CRAZIEST].score);
                    break;
                case AWARD_SURVIVOR:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Survivor] %-32s^7 (%.2f)\n",
                                osp.awards[AWARD_SURVIVOR].winnerName,
                                (float)osp.awards[AWARD_SURVIVOR].score / 100.0);
                    break;
                case AWARD_DAMAGE:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Damage] %-32s^7 (%d)\n",
                                osp.awards[AWARD_DAMAGE].winnerName,
                                osp.awards[AWARD_DAMAGE].score);
                    break;
                case AWARD_SCORE:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Score] %-32s^7 (%d)\n",
                                osp.awards[AWARD_SCORE].winnerName,
                                osp.awards[AWARD_SCORE].score);
                    break;
                }
            }
        }

        if (lastPrintTime < level.time && !sv_ospClient.integer)
        {
            lastPrintTime = level.time + 2000;
            trap_SendServerCommand(-1, va("cp \"%s\"", osp.awardMsg));
        }

        if (!lastPrintTime && sv_ospClient.integer)
        {
            lastPrintTime = 1;
            trap_SetConfigstring(CS_INTERMISSION, "2");
        }
    }
}

// osp: 0x0003040e
void OSP_PrintBadAwards(void)
{
    static qboolean awardsPrepared = qfalse;
    static int lastPrintTime = 0;

    int i;

    if (!awardsPrepared && level.intermissiontime + 20000 < level.time)
    {
        awardsPrepared = qtrue;

        osp.awards[BADAWARD_QUITTER].shown = qfalse;
        osp.awards[BADAWARD_QUITTER].showTime = level.intermissiontime + 20500;
        osp.awards[BADAWARD_LEMON].shown = qfalse;
        osp.awards[BADAWARD_LEMON].showTime = level.intermissiontime + 21000;
        osp.awards[BADAWARD_NEWBIE].shown = qfalse;
        osp.awards[BADAWARD_NEWBIE].showTime = level.intermissiontime + 21500;
        osp.awards[BADAWARD_FREEKILL].shown = qfalse;
        osp.awards[BADAWARD_FREEKILL].showTime = level.intermissiontime + 22000;
        osp.awards[BADAWARD_CROSSEYED].shown = qfalse;
        osp.awards[BADAWARD_CROSSEYED].showTime = level.intermissiontime + 22500;
        osp.awards[BADAWARD_SPAMMER].shown = qfalse;
        osp.awards[BADAWARD_SPAMMER].showTime = level.intermissiontime + 23000;
        osp.awards[BADAWARD_CHATTYBITCH].shown = qfalse;
        osp.awards[BADAWARD_CHATTYBITCH].showTime = level.intermissiontime + 23500;

        memset(osp.awardMsg, 0, sizeof(osp.awardMsg));
    }

    if (awardsPrepared)
    {
        for (i = BADAWARD_QUITTER; i < MAX_AWARDS; i++)
        {
            if (!osp.awards[i].shown && osp.awards[i].showTime < level.time)
            {
                osp.awards[i].shown = qtrue;

                if (sv_ospClient.integer)
                {
                    continue;
                }

                OSP_GlobalSound("sound/misc/confused/moose.mp3");

                lastPrintTime = 0;

                switch (i)
                {
                case BADAWARD_QUITTER:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Quitter] %-32s^7 (%d)\n",
                                osp.awards[BADAWARD_QUITTER].winnerName,
                                osp.awards[BADAWARD_QUITTER].score);
                    break;
                case BADAWARD_LEMON:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Lemon] %-32s^7 (%d)\n",
                                osp.awards[BADAWARD_LEMON].winnerName,
                                osp.awards[BADAWARD_LEMON].score);
                    break;
                case BADAWARD_NEWBIE:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Newbie] %-32s^7 (%d)\n",
                                osp.awards[BADAWARD_NEWBIE].winnerName,
                                osp.awards[BADAWARD_NEWBIE].score);
                    break;
                case BADAWARD_FREEKILL:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[FreeKill] %-32s^7 (%d)\n",
                                osp.awards[BADAWARD_FREEKILL].winnerName,
                                osp.awards[BADAWARD_FREEKILL].score);
                    break;
                case BADAWARD_CROSSEYED:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[CrossEyed] %-32s^7 (%.2f)\n",
                                osp.awards[BADAWARD_CROSSEYED].winnerName,
                                (float)osp.awards[BADAWARD_CROSSEYED].score / 100.0);
                    break;
                case BADAWARD_SPAMMER:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[Spammer] %-32s^7 (%d)\n",
                                osp.awards[BADAWARD_SPAMMER].winnerName,
                                osp.awards[BADAWARD_SPAMMER].score);
                    break;
                case BADAWARD_CHATTYBITCH:
                    Com_sprintf(strlen(osp.awardMsg) + osp.awardMsg,
                                1024 - strlen(osp.awardMsg),
                                "[ChattyB*tch] %-32s^7 (%d)\n",
                                osp.awards[BADAWARD_CHATTYBITCH].winnerName,
                                osp.awards[BADAWARD_CHATTYBITCH].score);
                    break;
                }
            }
        }

        if (lastPrintTime < level.time && !sv_ospClient.integer)
        {
            lastPrintTime = level.time + 2000;
            trap_SendServerCommand(-1, va("cp \"%s\"", osp.awardMsg));
        }

        if (!lastPrintTime && sv_ospClient.integer)
        {
            lastPrintTime = 1;
            trap_SetConfigstring(CS_INTERMISSION, "3");
        }
    }
}

// osp 0x000305d8
static void OSP_AltmapFlags(void)
{
    TGenericParser2 GP2;
    TGPGroup *basegroup;
    TGPGroup *group;
    char temp[256];

    if (!g_alternateMap.integer || !OSP_AltmapExists())
    {
        return;
    }

    GP2 = trap_GP_ParseFile(osp.altmapPath, qtrue, qfalse);
    if (!GP2)
    {
        return;
    }

    osp.altmapFlags = 0;
    basegroup = trap_GP_GetBaseParseGroup(GP2);
    group = trap_GPG_GetSubGroups(basegroup);

    while (group)
    {
        trap_GPG_FindPairValue(group, "classname", "", temp);

        if (strcmp(temp, "gametype_item") == 0)
        {
            osp.altmapFlags |= ALTMAP_GT_ITEM;
        }
        else if (strcmp(temp, "gametype_trigger") == 0)
        {
            osp.altmapFlags |= ALTMAP_GT_TRIGGER;
        }
        else if (strcmp(temp, "gametype_player") == 0)
        {
            trap_GPG_FindPairValue(group, "spawnflags", "", temp);

            if (strcmp(temp, "1") == 0)
                osp.altmapFlags |= ALTMAP_GT_PLAYER_RED;
            else if (strcmp(temp, "2") == 0)
                osp.altmapFlags |= ALTMAP_GT_PLAYER_BLUE;
        }
        else if (strcmp(temp, "target_location") == 0)
        {
            osp.altmapFlags |= ALTMAP_TARGET_LOCATION;
        }
        else if (strcmp(temp, "info_player_intermission") == 0)
        {
            osp.altmapFlags |= ALTMAP_INTERMISSION;
        }
        else if (strcmp(temp, "pickup_backpack") == 0)
        {
            osp.altmapFlags |= ALTMAP_BACKPACK;
        }
        else if (Q_strncmp(temp, "pickup_", 7) == 0)
        {
            osp.altmapFlags |= ALTMAP_PICKUP;
        }
        else if (strcmp(temp, "func_static") == 0)
        {
            osp.altmapFlags |= ALTMAP_STATIC;
        }
        else if (strcmp(temp, "func_glass") == 0)
        {
            osp.altmapFlags |= ALTMAP_STATIC;
        }
        else if (strcmp(temp, "target_speaker") == 0)
        {
            osp.altmapFlags |= ALTMAP_SPEAKER;
        }

        group = trap_GPG_GetNext(group);
    }

    trap_GP_Delete(&GP2);
}

// osp: 0x00030763
void OSP_LoadAltmap(void)
{
    const char *filename;

    if (!g_alternateMap.integer)
    {
        return;
    }
    else
    {
        char mapname[MAX_QPATH];

        trap_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));
        filename = va("data/alt%d_%s_%s.cfg", g_alternateMap.integer, mapname, g_gametype.string);
        Q_strncpyz(osp.altmapPath, filename, sizeof(osp.altmapPath));

        OSP_AltmapFlags();
    }
}
