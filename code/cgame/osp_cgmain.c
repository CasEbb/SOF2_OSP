// osp_main.c
//
#include "cg_local.h"
#include "../ui/ui_shared.h"

static const char *x40d0[] = {
    "*head_t",
    "*chestg",
    "*foot_l",
    "*foot_r",
    "*hand_r",
    "*hand_l",
};

// osp: 0x0002789a
qboolean sub_0002789a(centity_t *cent)
{
    trace_t trace;
    int i;
    vec3_t x68;
    mdxaBone_t boltMatrix;
    qboolean boltMatrixOK = qfalse;
    int boltIndex = -1;
    vec3_t mins = {6, 6, 5};
    vec3_t maxs = {-6, -6, -5};

    if (cent->currentState.eType != ET_PLAYER)
    {
        return qtrue;
    }

    if (cg.clientNum == cent->currentState.number)
    {
        return qtrue;
    }

    for (i = 0; i < 5; i++)
    {
        boltIndex = trap_G2API_AddBolt(cent->ghoul2, 0, x40d0[i]);
        boltMatrixOK = trap_G2API_GetBoltMatrix(cent->ghoul2, 0, boltIndex, &boltMatrix, cent->pe.ghoulLegsAngles, cent->lerpOrigin, cg.time, cgs.gameModels, cent->modelScale);

        if (boltMatrixOK)
        {
            x68[0] = boltMatrix.matrix[0][3];
            x68[1] = boltMatrix.matrix[1][3];
            x68[2] = boltMatrix.matrix[2][3];
        }

        CG_Trace(&trace, cg.refdef.vieworg, mins, maxs, x68, cg.clientNum, CONTENTS_SOLID);

        if (trace.entityNum >= 0 && trace.entityNum <= MAX_CLIENTS)
        {
            return qtrue;
        }

        if (trace.fraction == 1.0)
        {
            return qtrue;
        }
    }

    return qfalse;
}

// osp: 0x0002794a
void sub_0002794a(refEntity_t *x18)
{
    int xc;

    xc = x18->renderfx;
    trap_R_AddRefEntityToScene(x18);

    if (xc != x18->renderfx)
    {
        trap_Error("Attempted Hacking Detected");
    }
}


ospCgame_t ospCg;
ospMedia_t ospMedia;

vmCvar_t osp_drawCrosshairInfo;
vmCvar_t osp_drawClock;
vmCvar_t osp_drawTeamOverlay;
vmCvar_t osp_autoRecord;
vmCvar_t osp_autoScreenshot;
vmCvar_t cg_ospClient;
vmCvar_t osp_simpleHud;
vmCvar_t osp_oldScoreboard;
vmCvar_t osp_numericalHealth;
vmCvar_t osp_tracerDistance;
vmCvar_t osp_useKeyForLadders;
vmCvar_t osp_oldConsole;
vmCvar_t osp_weaponsFile;
vmCvar_t osp_modversion;
vmCvar_t x458664;
vmCvar_t osp_mvFollow;
vmCvar_t osp_mvFov;
vmCvar_t osp_mvMaxViews;
vmCvar_t osp_mvViewRatio;

typedef struct
{
    vmCvar_t *vmCvar;
    char *cvarName;
    char *defaultString;
    int cvarFlags;
    float mMinValue;
    float mMaxValue;
    int modificationCount;
} ospCvarTable_t;

static ospCvarTable_t ospCvarTable[] = {
    {&osp_drawCrosshairInfo, "osp_drawCrosshairInfo", "1", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 1.0, 0},
    {&osp_drawClock, "osp_drawClock", "1", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 1.0, 0},
    {&osp_drawTeamOverlay, "osp_drawTeamOverlay", "8", CVAR_ARCHIVE | CVAR_USERINFO | CVAR_LOCK_RANGE, 0.0, 16.0, 0},
    {&osp_autoRecord, "osp_autoRecord", "0", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 1.0, 0},
    {&osp_autoScreenshot, "osp_autoScreenshot", "0", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 1.0, 0},
    {&cg_ospClient, "cg_ospClient", "0.001", CVAR_USERINFO | CVAR_ROM, 0.0, 1.0, 0},
    {&osp_simpleHud, "osp_simpleHud", "1", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 1.0, 0},
    {&osp_oldScoreboard, "osp_oldScoreboard", "0", CVAR_ARCHIVE | CVAR_USERINFO | CVAR_LOCK_RANGE, 0.0, 1.0, 0},
    {&osp_numericalHealth, "osp_numericalHealth", "0", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 1.0, 0},
    {&osp_tracerDistance, "osp_tracerDistance", "500", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 0.0, 1024.0, 0},
    {&osp_useKeyForLadders, "osp_useKeyForLadders", "0", CVAR_ARCHIVE | CVAR_USERINFO | CVAR_LOCK_RANGE, 0.0, 1.0, 0},
    {&osp_oldConsole, "osp_oldConsole", "0", CVAR_ARCHIVE, 0.0, 1.0, 0},
    {&osp_weaponsFile, "osp_weaponsFile", "ext_data/sof2.wpn", CVAR_INIT | CVAR_ROM, 0.0, 0.0, 0},
    {&osp_modversion, "osp_modversion", "0", CVAR_INIT | CVAR_ROM, 0.0, 0.0, 0},
    {&osp_mvFollow, "osp_mvFollow", "0", CVAR_USERINFO, 0.0, 0.0, 0},
    {&osp_mvFov, "osp_mvFov", "1", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 1.0, 5.0, 0},
    {&osp_mvMaxViews, "osp_mvMaxViews", "4", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 1.0, 16.0, 0},
    {&osp_mvViewRatio, "osp_mvViewRatio", "1", CVAR_ARCHIVE | CVAR_LOCK_RANGE, 1.0, 4.0, 0},
};

// osp: 0x00027967
void OSP_RegisterCvars(void)
{
    unsigned int i;
    ospCvarTable_t *cv;

    for (i = 0, cv = ospCvarTable; i < sizeof(ospCvarTable) / sizeof(ospCvarTable[0]); i++, cv++)
    {
        trap_Cvar_Register(cv->vmCvar, cv->cvarName, cv->defaultString, cv->cvarFlags, cv->mMinValue, cv->mMaxValue);
    }
}

// osp: 0x000279a7
void OSP_UpdateCvars(void)
{
    unsigned int i;
    ospCvarTable_t *cv;

    for (i = 0, cv = ospCvarTable; i < sizeof(ospCvarTable) / sizeof(ospCvarTable[0]); i++, cv++)
    {
        trap_Cvar_Update(cv->vmCvar);

        if (cv->modificationCount != cv->vmCvar->modificationCount)
        {
            if (strcmp(cv->cvarName, "osp_oldConsole") == 0)
            {
                if (cv->vmCvar->integer == 1)
                {
                    trap_R_RemapShader("gfx/menus/console/console_mp", "gfx/menus/console/console_mp_old", "0");
                }
                else if (cv->vmCvar->integer == 0)
                {
                    trap_R_RemapShader("gfx/menus/console/console_mp", "gfx/menus/console/console_mp_new", "0");
                }
            }
            else if (strcmp(cv->cvarName, "osp_simpleHud") == 0)
            {
                if (cv->vmCvar->integer == 1)
                {
                    String_Init();
                    Menu_Reset();
                    CG_LoadMenus("ui/simplehud.txt");
                }
                else if (cv->vmCvar->integer == 0)
                {
                    String_Init();
                    Menu_Reset();
                    CG_LoadMenus("ui/hud.txt");
                }
            }

            cv->modificationCount = cv->vmCvar->modificationCount;
        }
    }
}

// osp: 0x00027a4e
qboolean OSP_IsDigit(char c)
{
    if (c >= '0' && c <= '9')
    {
        return qtrue;
    }

    return qfalse;
}

// osp: 0x00027a68
void OSP_Init(void)
{
    memset(&ospMedia, 0, sizeof(ospMedia));
    memset(&ospCg, 0, sizeof(ospCg));

    trap_Cvar_Set("clientnum", va("%i", cg.clientNum));

    ospMedia.osplogo = trap_R_RegisterShader("gfx/osplogo");
    ospMedia.osplogopp = trap_R_RegisterShader("gfx/osplogopp");
    ospMedia.sof2logopp = trap_R_RegisterShader("gfx/sof2logopp");

    ospMedia.statusbarHealth = trap_R_RegisterShader("osp/statusbar/health");
    ospMedia.statusbarArmor = trap_R_RegisterShader("osp/statusbar/armor");
    ospMedia.locations = trap_R_RegisterShader("gfx/locations");

    ospMedia.awardMedals[AWARD_LETHAL] = trap_R_RegisterShaderNoMip("gfx/osp_killsaward");
    ospMedia.awardMedals[AWARD_DAMAGE] = trap_R_RegisterShaderNoMip("gfx/osp_damageaward");
    ospMedia.awardMedals[AWARD_HEADSHOTS] = trap_R_RegisterShaderNoMip("gfx/osp_headshotsaward");
    ospMedia.awardMedals[AWARD_EXPLOSIVES] = trap_R_RegisterShaderNoMip("gfx/osp_grenadesaward");
    ospMedia.awardMedals[AWARD_CRAZIEST] = trap_R_RegisterShaderNoMip("gfx/osp_knifesaward");
    ospMedia.awardMedals[AWARD_SURVIVOR] = trap_R_RegisterShaderNoMip("gfx/osp_ratioaward");
    ospMedia.awardMedals[AWARD_SCORE] = trap_R_RegisterShaderNoMip("gfx/osp_skillaward");

    ospMedia.awardMedals[BADAWARD_QUITTER] = trap_R_RegisterShaderNoMip("gfx/osp_suicidalsaward");
    ospMedia.awardMedals[BADAWARD_LEMON] = trap_R_RegisterShaderNoMip("gfx/osp_lemonaward");
    ospMedia.awardMedals[BADAWARD_NEWBIE] = trap_R_RegisterShaderNoMip("gfx/osp_newbiesaward");
    ospMedia.awardMedals[BADAWARD_FREEKILL] = trap_R_RegisterShaderNoMip("gfx/osp_targetaward");
    ospMedia.awardMedals[BADAWARD_CROSSEYED] = trap_R_RegisterShaderNoMip("gfx/osp_crosseyedaward");
    ospMedia.awardMedals[BADAWARD_SPAMMER] = trap_R_RegisterShaderNoMip("gfx/osp_spammeraward");
    ospMedia.awardMedals[BADAWARD_CHATTYBITCH] = trap_R_RegisterShaderNoMip("gfx/osp_chattyaward");

    ospMedia.awardSound = trap_S_RegisterSound("sound/misc/events/rifle_shot.mp3");
    ospMedia.badawardSound = trap_S_RegisterSound("sound/misc/confused/moose.mp3");

    if (osp_oldConsole.integer)
    {
        trap_R_RemapShader("gfx/menus/console/console_mp", "gfx/menus/console/console_mp_old", "0");
    }

    if (osp_modversion.integer == 2)
    {
        bg_itemlist[MODELINDEX_WEAPON_M15].outfittingGroup = OUTFITTING_GROUP_ACCESSORY;
        bg_itemlist[MODELINDEX_WEAPON_ANM14].outfittingGroup = OUTFITTING_GROUP_ACCESSORY;
        bg_itemlist[MODELINDEX_WEAPON_M84].outfittingGroup = OUTFITTING_GROUP_ACCESSORY;

        bg_outfittingGroups[OUTFITTING_GROUP_PISTOL][0] = MODELINDEX_WEAPON_M19;
        bg_outfittingGroups[OUTFITTING_GROUP_PISTOL][1] = MODELINDEX_WEAPON_SOCOM;
        bg_outfittingGroups[OUTFITTING_GROUP_PISTOL][2] = MODELINDEX_WEAPON_SILVERTALON;
        bg_outfittingGroups[OUTFITTING_GROUP_PISTOL][3] = MODELINDEX_WEAPON_SOCOM_SILENCED;
        bg_outfittingGroups[OUTFITTING_GROUP_PISTOL][4] = -1;

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

// osp: 0x00027bdb
qboolean OSP_Music(void)
{
    char buf[64];

    if (!cg.demoPlayback)
    {
        return qfalse;
    }

    trap_Key_SetCatcher(8);
    trap_Cvar_VariableStringBuffer("mp3_file", buf, sizeof(buf));

    if (strlen(buf) <= 0)
    {
        return qfalse;
    }

    trap_S_StartBackgroundTrack(buf, buf, qtrue);
    trap_Cvar_VariableStringBuffer("s_musicvolume", buf, sizeof(buf));

    if (atof(buf) < 0.5)
    {
        trap_SendConsoleCommand("set s_musicvolume 1.0;\n");
    }

    return qtrue;
}

// osp: 0x00027c25
void sub_00027c25(void)
{
    if (ospCg.x45791c && ospCg.x45791c < cg.time)
    {
        OSP_Screenshot();
        ospCg.x45791c = 0;
    }

    if (ospCg.x457920 && ospCg.x457920 < cg.time)
    {
        ospCg.x457920 = 0;
        trap_SendConsoleCommand(ospCg.x457924);
    }
}
