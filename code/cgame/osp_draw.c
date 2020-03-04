// osp_draw.c
//
#include "cg_local.h"

static int x3f08[WP_NUM_WEAPONS][2] = {
    {0, 0},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {128, 64},
    {64, 64},
    {64, 64},
    {64, 64},
    {64, 64},
    {128, 64},
    {128, 64},
    {64, 64},
    {64, 64},
    {64, 64},
    {64, 64},
};

// osp: 0x00025a18
void OSP_DrawClock(void)
{
    qtime_t time;
    const char *s;
    int w;
    int y;
    int h;

    if (!osp_drawClock.integer || cg.showStats)
    {
        return;
    }

    trap_RealTime(&time);

    y = 160;
    s = va("%2d:%s%d%s",
           time.tm_hour > 12 ? time.tm_hour - 12 : time.tm_hour,
           time.tm_min < 10 ? "0" : "",
           time.tm_min,
           time.tm_hour >= 12 ? "pm" : "am");
    w = trap_R_GetTextWidth(s, cgs.media.hudFont, 0.5f, 0);
    h = trap_R_GetTextHeight(s, cgs.media.hudFont, 0.5f, 0);

    CG_DrawText(630 - w, y, cgs.media.hudFont, 0.5f, g_color_table[2], s, 0, DT_OUTLINE);
}

// osp: 0x00025aaf
void OSP_DrawCrosshairInfo(void)
{
    float *color;
    char *name;
    float w;
    int y;
    qboolean onSameTeam;

    onSameTeam = qfalse;

    if (!cg_drawCrosshair.integer)
    {
        return;
    }

    if (!cg_drawCrosshairNames.integer)
    {
        return;
    }

    if (cg.renderingThirdPerson)
    {
        return;
    }

    if (cgs.gametypeData->teams)
    {
        if (cg.predictedPlayerState.pm_type != PM_SPECTATOR && !(cg.predictedPlayerState.pm_flags & PMF_GHOST) && !(cg.predictedPlayerState.pm_flags & PMF_FOLLOW))
        {
            if (cgs.clientinfo[cg.crosshairClientNum].team != cg.predictedPlayerState.persistant[PERS_TEAM])
            {
                return;
            }
        }
    }

    if (cgs.clientinfo[cg.crosshairClientNum].team == cg.predictedPlayerState.persistant[PERS_TEAM])
    {
        onSameTeam = qtrue;
    }

    color = CG_FadeColor(cg.crosshairClientTime, 1000);
    if (!color)
    {
        trap_R_SetColor(NULL);
        return;
    }

    name = cgs.clientinfo[cg.crosshairClientNum].name;
    y = (SCREEN_HEIGHT / 2) + 20;

    color[3] *= 0.5f;
    w = trap_R_GetTextWidth(name, cgs.media.hudFont, 0.43f, 0);
    CG_DrawText(320 - w / 2, y, cgs.media.hudFont, 0.43f, color, name, 0, DT_OUTLINE);

    if (onSameTeam &&
        cgs.clientinfo[cg.crosshairClientNum].weapon > 0 &&
        cgs.clientinfo[cg.crosshairClientNum].weapon < WP_NUM_WEAPONS)
    {
        float dy;

        dy = trap_R_GetTextHeight(name, cgs.media.hudFont, 0.35f, 0);
        dy += 3.0;

        w = trap_R_GetTextWidth(va("^2[H^7%3d^2] [A^7%3d^2]", cgs.clientinfo[cg.crosshairClientNum].health, cgs.clientinfo[cg.crosshairClientNum].armor), cgs.media.hudFont, 0.35f, 0);
        CG_DrawText(320 - w / 2, y + dy, cgs.media.hudFont, 0.35f, color, va("^2[H^7%3d^2] [A^7%3d^2]", cgs.clientinfo[cg.crosshairClientNum].health, cgs.clientinfo[cg.crosshairClientNum].armor), 0, DT_OUTLINE);

        {
            float scale = 0.5f;
            int weapon = cgs.clientinfo[cg.crosshairClientNum].weapon;
            int itemIndex = BG_FindWeaponItem(cgs.clientinfo[cg.crosshairClientNum].weapon) - bg_itemlist;
            dy += 4.0;
            CG_DrawStretchPic(320 - (scale * (x3f08[weapon][0] / 2)), y + dy, scale * x3f08[weapon][0], scale * x3f08[weapon][1], 0, 0, 1, 1, color, cg_items[itemIndex].mSimpleIcon);
        }
    }

    trap_R_SetColor(NULL);
}

// osp: 0x00025c7e
static void OSP_DrawTeamoverlayClient(int x, int y, int w, int h, float scale, int clientNum)
{
    static int maxLocationWidth = -1;

    clientInfo_t *ci;
    vec4_t color;
    vec4_t bgColor;
    int i;

    ci = &cgs.clientinfo[clientNum];

    if (maxLocationWidth < 0)
    {
        int max;

        for (i = 0; i < MAX_LOCATIONS; i++)
        {
            max = trap_R_GetTextWidth(CG_ConfigString(CS_LOCATIONS + i), cgs.media.hudFont, scale, 20);

            if (max > maxLocationWidth)
            {
                maxLocationWidth = max;
            }
        }
    }

    if (cg.snap->ps.clientNum == clientNum)
        Vector4Copy(colorWhite, color);
    else
        Vector4Copy(colorLtGrey, color);

    if (cgs.clientinfo[clientNum].team == TEAM_RED)
        Vector4Copy(g_color_table[1], bgColor);
    else
        Vector4Copy(g_color_table[4], bgColor);

    color[3] = 0.95f;
    bgColor[3] = 0.35;

    CG_FillRect(x, y, maxLocationWidth + 137 + h, h, bgColor);
    CG_FillRect(x + 1, y + 1, (maxLocationWidth + 137 + h) - 2, h - 2, bgColor);

    if (trap_R_GetTextWidth(ci->name, cgs.media.hudFont, scale, 16) < 75)
        CG_DrawText(x, y, cgs.media.hudFont, scale, color, ci->name, 16, DT_FORCECOLOR);
    else if (trap_R_GetTextWidth(ci->name, cgs.media.hudFont, scale, 15) < 75)
        CG_DrawText(x, y, cgs.media.hudFont, scale, color, ci->name, 15, DT_FORCECOLOR);
    else
        CG_DrawText(x, y, cgs.media.hudFont, scale, color, ci->name, 14, DT_FORCECOLOR);

    CG_DrawText(x + 75, y, cgs.media.hudFont, scale, color, va("%3d", ci->health), 3, DT_FORCECOLOR);
    CG_DrawText(x + 75 + 20, y, cgs.media.hudFont, scale, color, va("%3d", ci->armor), 3, DT_FORCECOLOR);

    if (ci->weapon)
    {
        float weaponScale = 0.6 * scale;
        int item = BG_FindWeaponItem(ci->weapon) - bg_itemlist;

        CG_DrawStretchPic(
            x + 75 + 20 + 20 + ((ci->weapon >= WP_M84_GRENADE && ci->weapon != WP_OICW && ci->weapon != WP_USSOCOM_SILENCED) ? weaponScale * 32.0 : 0.0),
            y,
            weaponScale * x3f08[ci->weapon][0],
            weaponScale * x3f08[ci->weapon][1],
            0, 0,
            1, 1,
            color,
            cg_items[item].mSimpleIcon);
    }

    CG_DrawText(x + 75 + 20 + 20 + 22, y, cgs.media.hudFont, scale, color, CG_ConfigString(CS_LOCATIONS + ci->location), 20, DT_FORCECOLOR);

    {
        int x80;
        int x84;

        x84 = 0;

        for (x80 = 0; x80 < MAX_GAMETYPE_ITEMS; x80++)
        {
            if (!(ci->gametypeitems & (1 << x80)))
            {
                continue;
            }

            if (!cg_items[MODELINDEX_GAMETYPE_ITEM + x80].icon)
            {
                continue;
            }

            CG_DrawStretchPic(
                x + 75 + 20 + 20 + 22 + maxLocationWidth + (x84 * h),
                y,
                h - 1,
                h - 1,
                0,
                0,
                1,
                1,
                color,
                cg_items[MODELINDEX_GAMETYPE_ITEM + x80].icon);
            x84++;
        }
    }
}

// osp: 0x00025f80
void OSP_DrawTeamoverlay(void)
{
    clientInfo_t *ci;
    int i;
    int numClients;
    int maxClients;
    int h;
    int y;
    int x;
    int w;

    numClients = 0;
    maxClients = 0;

    if (!osp_drawTeamOverlay.integer)
    {
        return;
    }

    if (!cgs.gametypeData->teams)
    {
        return;
    }

    if (cg.predictedPlayerState.pm_type == PM_SPECTATOR && !(cg.predictedPlayerState.pm_flags & PMF_FOLLOW))
    {
        return;
    }

    w = 180;
    h = 9;
    x = 0;

    for (i = 0; i < cgs.maxclients; i++)
    {
        ci = &cgs.clientinfo[i];

        if (!ci->infoValid)
        {
            continue;
        }

        if (ci->team != cgs.clientinfo[cg.snap->ps.clientNum].team)
        {
            continue;
        }

        if (ci->weapon <= 0 || ci->weapon >= WP_NUM_WEAPONS)
        {
            continue;
        }

        if (cgs.gametypeData->respawnType == RT_NONE && ci->health <= 0)
        {
            continue;
        }

        if (ci->health == 0)
        {
            continue;
        }

        maxClients++;
    }

    if (osp_drawTeamOverlay.integer > 1 && maxClients > osp_drawTeamOverlay.integer)
    {
        maxClients = osp_drawTeamOverlay.integer;
    }

    for (i = 0; i < cgs.maxclients; i++)
    {
        ci = &cgs.clientinfo[i];

        if (!ci->infoValid)
        {
            continue;
        }

        if (ci->team != cgs.clientinfo[cg.snap->ps.clientNum].team)
        {
            continue;
        }

        if (ci->weapon <= 0 || ci->weapon >= WP_NUM_WEAPONS)
        {
            continue;
        }

        if (cgs.gametypeData->respawnType == RT_NONE && ci->health <= 0)
        {
            continue;
        }

        if (ci->health == 0)
        {
            continue;
        }

        if (osp_drawTeamOverlay.integer > 1 && numClients >= osp_drawTeamOverlay.integer)
        {
            return;
        }

        y = 453 - ((maxClients - numClients) * h);
        OSP_DrawTeamoverlayClient(x, y, w, h, 0.28f, i);
        numClients++;
    }
}

// osp: 0x000260a7
void OSP_DrawInformation(void)
{
    int diff;

    if (diff = strcmp(ospCg.gameversion, "1.1"))
    {
        if (diff > 0)
        {
            CG_DrawText(0, 462, cgs.media.hudFont, 0.35f, colorWhite,
                        va("Server runs ^1NEWER^7 version %s", ospCg.gameversion),
                        0, DT_OUTLINE);
        }

        if (diff < 0)
        {
            CG_DrawText(0, 462, cgs.media.hudFont, 0.35f, colorWhite,
                        va("Server runs ^1OLDER^7 version %s", ospCg.gameversion),
                        0, DT_OUTLINE);
        }
    }

    CG_DrawText(576, 460, cgs.media.hudFont, 0.65f, colorWhite, "1.1", 0, DT_OUTLINE);

    if (osp_modversion.integer == 2)
    {
        CG_DrawStretchPic(566, 396, 64, 64, 0, 0, 1, 1, NULL, ospMedia.sof2logopp);
    }
    else if (osp_modversion.integer == 1)
    {
        CG_DrawStretchPic(566, 396, 64, 64, 0, 0, 1, 1, NULL, ospMedia.osplogopp);
    }
    else
    {
        CG_DrawStretchPic(566, 396, 64, 64, 0, 0, 1, 1, NULL, ospMedia.osplogo);
    }

    if (ospCg.modifiedFiles)
    {
        CG_DrawText(0, 444, cgs.media.hudFont, 0.35f, colorYellow,
                    "Server runs modified weapons stats", 0, DT_OUTLINE);
    }
}

// osp: 0x000261a9
static qboolean OSP_ShouldDrawWeaponMode(int weapon)
{
    switch (weapon)
    {
    case WP_KNIFE:
        return qfalse;
    case WP_M1911A1_PISTOL:
        return qfalse;
    case WP_USSOCOM_PISTOL:
        return qfalse;
    case WP_SILVER_TALON:
        return qfalse;
    case WP_M590_SHOTGUN:
        return qfalse;
    case WP_MICRO_UZI_SUBMACHINEGUN:
        return qtrue;
    case WP_M3A1_SUBMACHINEGUN:
        return qfalse;
    case WP_MP5:
        return qtrue;
    case WP_USAS_12_SHOTGUN:
        return qtrue;
    case WP_M4_ASSAULT_RIFLE:
        return qtrue;
    case WP_AK74_ASSAULT_RIFLE:
        return qtrue;
    case WP_SIG551:
        return qtrue;
    case WP_OICW:
        return qtrue;
    case WP_MSG90A1:
        return qfalse;
    case WP_M60_MACHINEGUN:
        return qfalse;
    case WP_MM1_GRENADE_LAUNCHER:
        return qfalse;
    case WP_RPG7_LAUNCHER:
        return qfalse;
    case WP_M84_GRENADE:
        return qfalse;
    case WP_SMOHG92_GRENADE:
        return qfalse;
    case WP_MDN11_GRENADE:
        return qfalse;
    case WP_F1_GRENADE:
        return qfalse;
    case WP_L2A2_GRENADE:
        return qfalse;
    case WP_M67_GRENADE:
        return qfalse;
    case WP_ANM14_GRENADE:
        return qfalse;
    case WP_M15_GRENADE:
        return qfalse;
    }

    return qfalse;
}

// osp: 0x00026226
static void OSP_DrawHUD_Weapon(int x, int y, int w, int h, float scale, int weapon, vec4_t color)
{
    vec4_t x30;
    vec4_t iconColor;
    float x50;
    vec4_t x54;
    float dh;
    float x68;
    int clip;
    float x70;
    float dw;
    int ammo;
    int curMode;
    int itemIndex;
    int x84;
    const char *mode;

    x68 = 0.5 * scale;
    x70 = 0.7 * scale;
    x50 = 0.4 * scale;

    itemIndex = BG_FindWeaponItem(weapon) - bg_itemlist;
    color[3] = 0.1;

    CG_FillRect(x + 1, y + 1, w - 2, h - 2, color);

    Vector4Copy(colorWhite, iconColor);
    Vector4Copy(colorWhite, x30);
    Vector4Copy(colorWhite, x54);

    if (weapon == cg.snap->ps.weapon)
    {
        iconColor[3] = 1.0;
        x30[3] = 0.95;
        x54[3] = 0.85;
    }
    else
    {
        iconColor[3] = 0.4;
        x30[3] = 0.35;
        x54[3] = 0.3;
    }

    if (weapon == WP_AK74_ASSAULT_RIFLE)
    {
        x84 = y - 5;
    }
    else if (weapon == WP_M4_ASSAULT_RIFLE ||
             weapon == WP_MSG90A1 ||
             weapon == WP_M590_SHOTGUN ||
             weapon == WP_M60_MACHINEGUN ||
             weapon == WP_RPG7_LAUNCHER)
    {
        x84 = y - 3;
    }
    else if (weapon == WP_MP5 || weapon == WP_KNIFE)
    {
        x84 = y - 2;
    }
    else
    {
        x84 = y;
    }

    if (w < 60)
    {
        x68 *= w / 60.0;
    }

    CG_DrawStretchPic(x + ((w - (x68 * x3f08[weapon][0])) / 2), x84, x68 * x3f08[weapon][0],
                      x68 * x3f08[weapon][1], 0, 0, 1, 1, iconColor,
                      cg_items[itemIndex].mSimpleIcon);

    clip = cg.predictedPlayerState.clip[ATTACK_NORMAL][weapon];
    ammo = AMMOWRAP(cg.predictedPlayerState, weaponData[weapon].attack[ATTACK_NORMAL].ammoIndex);
    curMode = cg.predictedPlayerState.firemode[weapon];

    switch (curMode)
    {
    case WP_FIREMODE_NONE:
        mode = "";
        break;
    case WP_FIREMODE_AUTO:
        if (w < 60)
            mode = "a";
        else
            mode = "auto";
        break;
    case WP_FIREMODE_BURST:
        if (w < 60)
            mode = "b";
        else
            mode = "burst";
        break;
    case WP_FIREMODE_SINGLE:
        if (w < 60)
            mode = "s";
        else
            mode = "single";
        break;
    case WP_FIREMODE_20MM:
        if (w < 60)
            mode = "x";
        else
            mode = "alt";
        break;
    default:
        mode = "";
    }

    dh = trap_R_GetTextHeight("A", cgs.media.hudFont, x70, 0);
    CG_DrawText(x + 2, (y + h) - dh, cgs.media.hudFont, x70, x30, va("%d", clip), 0, DT_OUTLINE);

    if (weapon == WP_M4_ASSAULT_RIFLE || weapon == WP_OICW)
    {
        clip = cg.predictedPlayerState.clip[ATTACK_ALTERNATE][weapon] + AMMOWRAP(cg.predictedPlayerState, weaponData[weapon].attack[ATTACK_ALTERNATE].ammoIndex);

        if (clip == -1)
        {
            clip = cg.predictedPlayerState.generic1;
            CG_DrawText(x + 2, y + 1, cgs.media.hudFont, x50, x30, va("^1%d", clip), 0, DT_OUTLINE);
        }
        else if (clip > 0)
        {
            CG_DrawText(x + 2, y + 1, cgs.media.hudFont, x50, x30, va("^2%d", clip), 0, DT_OUTLINE);
        }
    }

    dh = trap_R_GetTextHeight("A", cgs.media.hudFont, x50, 0);
    dw = trap_R_GetTextWidth(va("%2d", ammo), cgs.media.hudFont, x50, 0);
    CG_DrawText((x + w) - (dw + 2), (y + h) - (dh + 1), cgs.media.hudFont, x50, x30, va("%2d", ammo), 0, DT_OUTLINE);

    if (OSP_ShouldDrawWeaponMode(weapon))
    {
        dw = trap_R_GetTextWidth(va("%s", mode), cgs.media.hudFont, x50, 0);
        dh = trap_R_GetTextHeight("A", cgs.media.hudFont, x50, 0);
        CG_DrawText(x + ((w - dw) / 2) + 3, (y + h) - (dh + 1), cgs.media.hudFont, x50, x54, va("%s", mode), 0, DT_OUTLINE);
    }
}

// osp: 0x000265bd
static void OSP_DrawHUD_Teaminfo(int x, int y, int w, int h, int team, float scale)
{
    float dw;
    float dh;
    float small;
    float large;
    char score[16];
    const char *s;

    small = 0.4 * scale;
    large = 0.8 * scale;

    if (!cgs.gametypeData->teams)
    {
        return;
    }

    if (cgs.scores1 == SCORE_NOT_PRESENT)
    {
        Com_sprintf(score, sizeof(score), "-");
    }
    else
    {
        if (team == TEAM_RED)
        {
            Com_sprintf(score, sizeof(score), "%i", cgs.scores1);
        }
        else if (team == TEAM_BLUE)
        {
            Com_sprintf(score, sizeof(score), "%i", cgs.scores2);
        }
    }

    if (team == TEAM_RED)
    {
        CG_DrawPic(x, y, w / 2, h, cgs.media.redFriendShader);
    }
    else if (team == TEAM_BLUE)
    {
        CG_DrawPic(x, y, w / 2, h, cgs.media.blueFriendShader);
    }

    dw = trap_R_GetTextWidth(score, cgs.media.hudFont, large, 0);
    dh = trap_R_GetTextHeight("0", cgs.media.hudFont, large, 0);
    CG_DrawText(x + (((w / 2) - dw) / 2), y + h - dh, cgs.media.hudFont, large, colorWhite, score, 0, DT_OUTLINE);

    if (cgs.gametypeData->respawnType == RT_NONE)
    {
        if (team == TEAM_RED)
            s = va("%d/%d", cg.predictedPlayerState.persistant[PERS_RED_ALIVE_COUNT], CG_TeamCount(team));
        else
            s = va("%d/%d", cg.predictedPlayerState.persistant[PERS_BLUE_ALIVE_COUNT], CG_TeamCount(team));
    }
    else
    {
        s = va("%d", CG_TeamCount(team));
    }

    dw = trap_R_GetTextWidth(s, cgs.media.hudFont, small, 0);
    dh = trap_R_GetTextHeight("0", cgs.media.hudFont, small, 0);
    CG_DrawText(x + (w / 2) + (((w / 2) - dw) / 2), (y + h) - dh - 1, cgs.media.hudFont, small, colorLtGrey, s, 0, DT_OUTLINE);

    if (strcmp(cgs.gametypeData->name, "ctf") == 0)
    {
        if (team == TEAM_RED && cgs.hudIcons[0])
            CG_DrawPic(x + h, y, h - 3, h - 3, cgs.gameIcons[cgs.hudIcons[0]]);
        else if (team == TEAM_BLUE && cgs.hudIcons[1])
            CG_DrawPic(x + h, y, h - 3, h - 3, cgs.gameIcons[cgs.hudIcons[1]]);
    }
    else if (strcmp(cgs.gametypeData->name, "inf") == 0)
    {
        if (team == TEAM_RED && cgs.hudIcons[1] == TEAM_RED)
        {
            CG_DrawPic(x + h + 2, y, h / 2, h / 2, cgs.gameIcons[cgs.hudIcons[0]]);
        }

        if (team == TEAM_BLUE && cgs.hudIcons[1] == TEAM_BLUE)
        {
            CG_DrawPic(x + h + 2, y, h / 2, h / 2, cgs.gameIcons[cgs.hudIcons[0]]);
        }
    }
    else if (strcmp(cgs.gametypeData->name, "dem") == 0)
    {
        if (team == TEAM_RED && cgs.hudIcons[0])
            CG_DrawPic(x + h + 2, y, h / 2, h / 2, cgs.gameIcons[cgs.hudIcons[0]]);
        else if (team == TEAM_BLUE && !cgs.hudIcons[0])
            CG_DrawPic(x + h + 2, y, h / 2, h / 2, cg_items[MODELINDEX_GAMETYPE_ITEM].icon);
    }
}

// osp: 0x000268b6
void OSP_DrawHUD(void)
{
    vec4_t color;

    //
    // backdrop
    //
    if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_RED)
    {
        Vector4Copy(g_color_table[ColorIndex(COLOR_RED)], color);
    }
    else if (cgs.clientinfo[cg.snap->ps.clientNum].team == TEAM_BLUE)
    {
        Vector4Copy(g_color_table[ColorIndex(COLOR_BLUE)], color);
    }
    else
    {
        Vector4Copy(g_color_table[ColorIndex(COLOR_GREEN)], color);
    }

    color[3] = 0.45f;
    VectorScale(color, 0.75f, color);

    CG_FillRect(0, 458, 640, 22, color);
    CG_FillRect(1, 459, 638, 20, color);

    //
    // timer
    //
    if (cg.predictedPlayerState.stats[STAT_FROZEN])
    {
        CG_DrawTimer(5, 458, cgs.media.hudFont, 0.65f, colorGreen, DT_OUTLINE, -cg.predictedPlayerState.stats[STAT_FROZEN]);
    }
    else if (cgs.gametypeTimerTime != 0 && cgs.gametypeTimerTime >= cg.time)
    {
        CG_DrawTimer(5, 458, cgs.media.hudFont, 0.65f, colorGreen, DT_OUTLINE, cgs.gametypeTimerTime - cg.time);
    }

    //
    // health/armor
    //
    if (osp_numericalHealth.integer)
    {
        if (cg.predictedPlayerState.stats[STAT_HEALTH] > 0)
        {
            CG_DrawStretchPic(64, 460, 18, 18, 0, 0, 1, 1, colorRed, ospMedia.statusbarHealth);
            CG_DrawText(82, 458, cgs.media.hudFont, 0.65f, colorWhite, va("%3d", cg.predictedPlayerState.stats[STAT_HEALTH]), 3, DT_OUTLINE);
        }
        else
        {
            CG_DrawStretchPic(64, 460, 18, 18, 0, 0, 1, 1, colorWhite, cgs.media.deadShader);
        }

        if (cg.predictedPlayerState.stats[STAT_ARMOR] > 0)
        {
            CG_DrawPic(126, 460, 18, 18, ospMedia.statusbarArmor);
            CG_DrawText(144, 458, cgs.media.hudFont, 0.65f, colorLtGrey, va("%3d", cg.predictedPlayerState.stats[STAT_ARMOR]), 3, DT_OUTLINE);
        }
    }

    //
    // weaponlist
    //
    {
        int weapon;
        int offset;
        float w;
        int numDisplayed;

        offset = 0;
        numDisplayed = 0;

        for (weapon = WP_KNIFE; weapon < WP_NUM_WEAPONS; weapon++)
        {
            if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << weapon))
            {
                numDisplayed++;
            }
        }

        if (numDisplayed < 7)
        {
            w = 60;
        }
        else
        {
            w = 360 / numDisplayed;
        }

        for (weapon = WP_KNIFE; weapon <= WP_SILVER_TALON; weapon++)
        {
            if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << weapon))
            {
                OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, weapon, color);
                offset++;
            }
        }

        if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_USSOCOM_SILENCED))
        {
            OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, WP_USSOCOM_SILENCED, color);
            offset++;
        }

        for (weapon = WP_M590_SHOTGUN; weapon <= WP_M60_MACHINEGUN; weapon++)
        {
            if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << weapon))
            {
                OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, weapon, color);
                offset++;
            }
        }

        if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_OICW))
        {
            OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, WP_OICW, color);
            offset++;
        }

        for (weapon = WP_MM1_GRENADE_LAUNCHER; weapon <= WP_RPG7_LAUNCHER; weapon++)
        {
            if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << weapon))
            {
                OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, weapon, color);
                offset++;
            }
        }

        if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_SMOHG92_GRENADE))
        {
            OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, WP_SMOHG92_GRENADE, color);
            offset++;
        }

        if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_MDN11_GRENADE))
        {
            OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, WP_MDN11_GRENADE, color);
            offset++;
        }

        if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_F1_GRENADE))
        {
            OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, WP_F1_GRENADE, color);
            offset++;
        }

        if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_L2A2_GRENADE))
        {
            OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, WP_L2A2_GRENADE, color);
            offset++;
        }

        if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_M67_GRENADE))
        {
            OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, WP_M67_GRENADE, color);
            offset++;
        }

        if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_M84_GRENADE))
        {
            OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, WP_M84_GRENADE, color);
            offset++;
        }

        if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_ANM14_GRENADE))
        {
            OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, WP_ANM14_GRENADE, color);
            offset++;
        }

        if (cg.snap->ps.stats[STAT_WEAPONS] & (1 << WP_M15_GRENADE))
        {
            OSP_DrawHUD_Weapon((offset * w) + 190, 458, w, 22, 0.65f, WP_M15_GRENADE, color);
            offset++;
        }
    }

    //
    // team scores
    //
    if (cgs.gametypeData->teams)
    {
        OSP_DrawHUD_Teaminfo(550, 458, 44, 22, TEAM_RED, 0.65f);
        OSP_DrawHUD_Teaminfo(595, 458, 44, 22, TEAM_BLUE, 0.65f);
    }
}

// osp: 0x00026cd1
static void OSP_DrawAward_Result(int award, int x, int y, float scale, vec4_t bgColor, vec4_t color)
{
    int medal_w;
    int medal_h;
    int medal_x;
    int medal_y;

    CG_FillRect(x, y, 300, 62, bgColor);

    if (scale > 1)
    {
        medal_w = 30 * scale;
        medal_h = 60 * scale;
        medal_x = (x + 1) - ((medal_w - 30) / 2);
        medal_y = (y + 1) - ((medal_h - 60) / 2);
    }
    else
    {
        medal_x = x + 1;
        medal_y = y + 1;
        medal_w = 30;
        medal_h = 60;
    }

    CG_FillRect(x + 1, y + 1, 30, 60, bgColor);
    CG_DrawPic(medal_x, medal_y, medal_w, medal_h, ospMedia.awardMedals[award]);
    CG_FillRect(x + 1 + 30, y + 15, 267, 31, bgColor);

    switch (award)
    {
    case AWARD_LETHAL:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Lethal", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case AWARD_DAMAGE:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Damage", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case AWARD_HEADSHOTS:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Head Shots", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case AWARD_EXPLOSIVES:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Explosives", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case AWARD_CRAZIEST:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Craziest", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case AWARD_SCORE:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Score", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case AWARD_SURVIVOR:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Survivor", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case BADAWARD_QUITTER:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Quitter", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case BADAWARD_LEMON:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Lemon", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case BADAWARD_NEWBIE:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Newbie", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case BADAWARD_FREEKILL:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Free Kill", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case BADAWARD_CROSSEYED:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Cross-Eyed", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case BADAWARD_SPAMMER:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Spammer", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    case BADAWARD_CHATTYBITCH:
        CG_DrawText(x + 1 + 30, y + 15, cgs.media.hudFont, 0.4f, color, "Chatty B*tch", 0, color[3] == 1 ? DT_DROPSHADOW : 0);
        break;
    }

    if (ospCg.awards[award].winnerName[0])
    {
        switch (award)
        {
        case AWARD_LETHAL:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i kill%s", ospCg.awards[AWARD_LETHAL].winnerScore, ospCg.awards[AWARD_LETHAL].winnerScore > 1 ? "s" : ""), 0, DT_OUTLINE);
            break;
        case AWARD_DAMAGE:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i dmg", ospCg.awards[AWARD_DAMAGE].winnerScore), 0, DT_OUTLINE);
            break;
        case AWARD_HEADSHOTS:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i headshot%s", ospCg.awards[AWARD_HEADSHOTS].winnerScore, ospCg.awards[AWARD_HEADSHOTS].winnerScore > 1 ? "s" : ""), 0, DT_OUTLINE);
            break;
        case AWARD_EXPLOSIVES:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i kill%s", ospCg.awards[AWARD_EXPLOSIVES].winnerScore, ospCg.awards[AWARD_EXPLOSIVES].winnerScore > 1 ? "s" : ""), 0, DT_OUTLINE);
            break;
        case AWARD_CRAZIEST:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i kill%s", ospCg.awards[AWARD_CRAZIEST].winnerScore, ospCg.awards[AWARD_CRAZIEST].winnerScore > 1 ? "s" : ""), 0, DT_OUTLINE);
            break;
        case AWARD_SCORE:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i", ospCg.awards[AWARD_SCORE].winnerScore), 0, DT_OUTLINE);
            break;
        case AWARD_SURVIVOR:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%.2f%%", ospCg.awards[AWARD_SURVIVOR].winnerScore / 100.0f), 0, DT_OUTLINE);
            break;
        case BADAWARD_QUITTER:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i suicide%s", ospCg.awards[BADAWARD_QUITTER].winnerScore, ospCg.awards[BADAWARD_QUITTER].winnerScore > 1 ? "s" : ""), 0, DT_OUTLINE);
            break;
        case BADAWARD_LEMON:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i team kill%s", ospCg.awards[BADAWARD_LEMON].winnerScore, ospCg.awards[BADAWARD_LEMON].winnerScore > 1 ? "s" : ""), 0, DT_OUTLINE);
            break;
        case BADAWARD_NEWBIE:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%.2f%% ratio", 100.0f - (ospCg.awards[BADAWARD_NEWBIE].winnerScore / 100.0f)), 0, DT_OUTLINE);
            break;
        case BADAWARD_FREEKILL:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i death%s", ospCg.awards[BADAWARD_FREEKILL].winnerScore, ospCg.awards[BADAWARD_FREEKILL].winnerScore > 1 ? "s" : ""), 0, DT_OUTLINE);
            break;
        case BADAWARD_CROSSEYED:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%.2f%% acc.", 100.0f - (ospCg.awards[BADAWARD_CROSSEYED].winnerScore / 100.0f)), 0, DT_OUTLINE);
            break;
        case BADAWARD_SPAMMER:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i shot%s", ospCg.awards[BADAWARD_SPAMMER].winnerScore, ospCg.awards[BADAWARD_SPAMMER].winnerScore > 1 ? "s" : ""), 0, DT_OUTLINE);
            break;
        case BADAWARD_CHATTYBITCH:
            CG_DrawText(x + 300 - 100 - 1, y + 31, cgs.media.hudFont, 0.4f, color, va("%i chat%s", ospCg.awards[BADAWARD_CHATTYBITCH].winnerScore, ospCg.awards[BADAWARD_CHATTYBITCH].winnerScore > 1 ? "s" : ""), 0, DT_OUTLINE);
            break;
        }
    }

    CG_DrawText(x + 1 + 30 + 15, y + 15 + 15 - 2 + 2, cgs.media.hudFont, 0.4f, color, ospCg.awards[award].winnerName, 0, DT_OUTLINE);
}

// osp: 0x000273d4
static void OSP_DrawAward(int award, int pos)
{
    float progress;
    vec4_t color;
    vec4_t bgColor;
    float scale;

    if (ospCg.awards[award].displayTime < cg.time)
    {
        scale = 1.0f;
        bgColor[0] = 0;
        bgColor[1] = 0;
        bgColor[2] = 0;
        bgColor[3] = 0.5f;
        Vector4Copy(colorWhite, color);

        if (!ospCg.awards[award].soundPlayed)
        {
            ospCg.awards[award].soundPlayed = qtrue;

            if (award < 7)
            {
                trap_S_StartLocalSound(ospMedia.awardSound, CHAN_AUTO);
            }
            else
            {
                trap_S_StartLocalSound(ospMedia.badawardSound, CHAN_AUTO);
            }
        }

        progress = (cg.time - ospCg.awards[award].displayTime) / 1000.0f;

        // fade background between 0 and 500 ms
        if (progress > 0.5)
        {
            bgColor[3] = pos % 2 ? 0.5 : 0.55;
        }
        else
        {
            bgColor[3] = progress;
        }

        // fade text between 500 and 1000ms
        if (progress < 0.5)
        {
            color[3] = 0;
        }
        else if (progress > 1.0)
        {
            color[3] = 1.0;
        }
        else
        {
            color[3] = 2.0 * (progress - 0.5);
        }

        if (progress <= 0)
        {
            progress = 0.001;
        }

        if (progress < 0.5)
        {
            scale = -2.0 * (progress - 0.5) + 1.0;
        }

        OSP_DrawAward_Result(award, 170, (pos * 63) + 23, scale, bgColor, color);
    }
}

// osp: 0x00027497
void OSP_DrawAwards(void)
{
    int pos = 0;

    OSP_DrawAward(AWARD_LETHAL, pos);
    pos++;
    OSP_DrawAward(AWARD_HEADSHOTS, pos);
    pos++;
    OSP_DrawAward(AWARD_EXPLOSIVES, pos);
    pos++;
    OSP_DrawAward(AWARD_CRAZIEST, pos);
    pos++;
    OSP_DrawAward(AWARD_SURVIVOR, pos);
    pos++;
    OSP_DrawAward(AWARD_DAMAGE, pos);
    pos++;
    OSP_DrawAward(AWARD_SCORE, pos);
}

// osp: 0x000274f9
void OSP_DrawBadAwards(void)
{
    int pos = 0;

    OSP_DrawAward(BADAWARD_QUITTER, pos);
    pos++;
    OSP_DrawAward(BADAWARD_LEMON, pos);
    pos++;
    OSP_DrawAward(BADAWARD_NEWBIE, pos);
    pos++;
    OSP_DrawAward(BADAWARD_FREEKILL, pos);
    pos++;
    OSP_DrawAward(BADAWARD_CROSSEYED, pos);
    pos++;
    OSP_DrawAward(BADAWARD_SPAMMER, pos);
    pos++;
    OSP_DrawAward(BADAWARD_CHATTYBITCH, pos);
}

// osp: 0x0002755b
static void OSP_DrawHitlocations(int x, int y, qboolean myHits, float scale)
{
    int total;
    int *stats;
    vec4_t color;
    int w;
    int h;

    w = 200 * scale;
    h = 400 * scale;

    color[0] = 0;
    color[1] = 0;
    color[2] = 0;
    color[3] = 0.25;

    CG_FillRect(x, y, w, h, color);
    CG_FillRect(x + (2 * scale), y + (2 * scale), w - (4 * scale), h - (4 * scale), color);
    CG_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, colorWhite, ospMedia.locations);

    if (myHits)
    {
        CG_DrawText(x + 5, y + 5, cgs.media.hudFont, 0.9 * scale, colorWhite, "You hit them", 0, DT_OUTLINE);
        total = ospCg.x4581bc;
        stats = ospCg.x457d54;
    }
    else
    {
        CG_DrawText(x + 5, y + 5, cgs.media.hudFont, 0.9 * scale, colorWhite, "They hit you", 0, DT_OUTLINE);
        total = ospCg.x4581c4;
        stats = ospCg.x457d6c;
    }

    // percentages
    CG_DrawText(x + (115 * scale), y + (78 * scale), cgs.media.hudFont, 0.35f * scale, colorWhite, va("%2.1f%%", total > 0 ? stats[1] * 100.0 / total : 0), 0, DT_OUTLINE);
    CG_DrawText(x + (134 * scale), y + (152 * scale), cgs.media.hudFont, 0.35f * scale, colorWhite, va("%2.1f%%", total > 0 ? stats[2] * 100.0 / total : 0), 0, DT_OUTLINE);
    CG_DrawText(x + (2 * scale), y + (152 * scale), cgs.media.hudFont, 0.35f * scale, colorWhite, va("%2.1f%%", total > 0 ? stats[3] * 100.0 / total : 0), 0, DT_OUTLINE);
    CG_DrawText(x + (18 * scale), y + (217 * scale), cgs.media.hudFont, 0.35f * scale, colorWhite, va("%2.1f%%", total > 0 ? stats[4] * 100.0 / total : 0), 0, DT_OUTLINE);
    CG_DrawText(x + (140 * scale), y + (285 * scale), cgs.media.hudFont, 0.35f * scale, colorWhite, va("%2.1f%%", total > 0 ? stats[5] * 100.0 / total : 0), 0, DT_OUTLINE);

    // overall
    CG_DrawText(x + (50 * scale), y + (330 * scale), cgs.media.hudFont, 0.45 * scale, colorWhite, va("%5i hits", total), 0, DT_OUTLINE);

    if (myHits && ospCg.x4581c0)
    {
        CG_DrawText(x + (50 * scale), y + (350 * scale), cgs.media.hudFont, 0.45 * scale, colorWhite, va("%5i shots", ospCg.x4581c0), 0, DT_OUTLINE);
        CG_DrawText(x + (50 * scale), y + (370 * scale), cgs.media.hudFont, 0.45 * scale, colorWhite, va("%2.1f%% accuracy", total * 100.0 / ospCg.x4581c0), 0, DT_OUTLINE);
    }
}

/*
0x2c --
0x30 total
0x34 stats[]
0x38 color
0x48 w
0x4c h
0x50 temp

0xbc --
0xc0 --
0xc4 x
0xc8 y
0xcc myHits
0xd0 scale
*/

// osp: 0x00027863
qboolean OSP_DrawStats(void)
{
    if (cg.intermissionStarted < 2 && !cg.showStats)
    {
        return qfalse;
    }

    if (cg.statsRequestTime + 5000 < cg.time)
    {
        cg.statsRequestTime = cg.time;
        trap_SendClientCommand("osp_stats");
    }

    OSP_DrawHitlocations(10, 100, qtrue, 0.65f);
    OSP_DrawHitlocations(500, 100, qfalse, 0.65f);
    return qtrue;
}
