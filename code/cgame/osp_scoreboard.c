// osp_scoreboard.c
//
#include "cg_local.h"

static int ospSb_lineHeight;
static int ospSb_maxClients;
static float ospSb_nameFontScale;
static float ospSb_numberFontScale;
static float ospSb_readyFontScale;

// osp: 0x00027c48
static void sub_00027c48(const char *in, char *out, int outSize, qboolean colors)
{
    int len;
    int colorlessLen;
    char ch;
    char *p;
    int spaces;

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
                // don't allow black in a name, period
                if (!colors || ColorIndex(*in) == 0)
                {
                    in++;
                    continue;
                }

                // make sure room in dest for both chars
                if (len > outSize - 2)
                {
                    break;
                }

                *out++ = ch;
                *out++ = *in++;
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

    // don't allow empty names
    if (*p == 0 || colorlessLen == 0)
    {
        Q_strncpyz(p, "UnnamedPlayer", outSize);
    }
}

// osp: 0x00027d9b
static void sub_00027d9b(int x, int y, int w, int h, float scale, clientInfo_t *client)
{
    clientInfo_t *ci = client;

    if (ospCg.matchphase != 1)
    {
        return;
    }

    if (ci->ready)
    {
        CG_DrawText(x + w - 100, y + 3, cgs.media.hudFont, 2 * scale, colorWhite, "READY", 0, DT_OUTLINE);
    }
}

// osp: 0x00027dd1
static qboolean OSP_DrawClientScore(float x, float y, float w, float h, float scale, score_t *score, vec4_t color)
{
    clientInfo_t *ci;
    vec4_t dataColor;
    vec4_t nameColor;
    vec4_t x4c;
    const char *s;
    float f;

    nameColor[3] = dataColor[3] = x4c[3] = 1.0;

    // Validate the score
    if (score->client < 0 || score->client >= cgs.maxclients)
    {
        Com_Printf("Bad score->client: %i\n", score->client);
        return qfalse;
    }

    // Convienience
    ci = &cgs.clientinfo[score->client];

    if (cgs.gametypeData->respawnType == RT_NONE &&
        (ci->ghost || score->ping < 0 || score->ping >= 999) &&
        cg.snap->ps.pm_type != PM_INTERMISSION)
    {
        y += 5.0;
    }

    CG_DrawPic(x - 5, y, w, h, cgs.media.scoreboardLine);

    // highlight your position
    if (score->client == cg.snap->ps.clientNum)
    {
        vec4_t hcolor;

        hcolor[0] = 1.0f;
        hcolor[1] = 1.0f;
        hcolor[2] = 1.0f;
        hcolor[3] = .10f;

        CG_FillRect(x - 5, y, w, h, hcolor);

        VectorSet(nameColor, 1.0f, 1.0f, 1.0f);
        VectorSet(dataColor, 0.5f, 0.5f, 0.5f);
        VectorSet(x4c, 0.45, 0.45, 0.45);
    }
    else if ((cg.snap->ps.pm_type == PM_DEAD) && score->client == cg.snap->ps.persistant[PERS_ATTACKER])
    {
        vec4_t hcolor;

        hcolor[0] = 1.0f;
        hcolor[1] = 1.0f;
        hcolor[2] = 1.0f;
        hcolor[3] = .10f;

        CG_FillRect(x - 5, y, w, h, hcolor);

        VectorCopy(color, nameColor);
        VectorSet(dataColor, 0.7f, 0.7f, 0.7f);
        VectorSet(x4c, 0.65f, 0.65f, 0.65f);
    }
    else
    {
        VectorCopy(color, nameColor);
        VectorSet(dataColor, 0.4f, 0.4f, 0.4f);
        VectorSet(x4c, 0.35f, 0.35f, 0.35f);
    }

    if (score->rank & 0x1)
    {
        char team_controls[2];
        trap_Cvar_VariableStringBuffer("ui_teamcontrols", team_controls, sizeof(team_controls));

        if (atoi(team_controls) > 0)
        {
            CG_DrawText(x - (0.05 * w), y, cgs.media.hudFont, 0.6 * scale, colorWhite, "C", 0, DT_OUTLINE);
        }
    }

    if (score->rank & 0x2)
    {
        if (score->rank & 0x4)
        {
            CG_DrawText(x - (0.05 * w), y + (0.33 * h), cgs.media.hudFont, 0.6 * scale, colorYellow, "A", 0, DT_OUTLINE);
        }
        else
        {
            CG_DrawText(x - (0.05 * w), y + (0.33 * h), cgs.media.hudFont, 0.6 * scale, colorCyan, "A", 0, DT_OUTLINE);
        }
    }
    else if (score->rank & 0x8)
    {
        CG_DrawText(x - (0.05 * w), y + (0.33 * h), cgs.media.hudFont, 0.6 * scale, colorYellow, "R", 0, DT_OUTLINE);
    }

    if (score->rank & 0x10)
    {
        CG_DrawText(x - (0.05 * w), y + (0.66 * h), cgs.media.hudFont, 0.6 * scale, colorLtGrey, "S", 0, DT_OUTLINE);
    }

    if (ci->ready && (ospCg.matchphase == 1 || ospCg.matchphase == 4))
    {
        CG_DrawText(x + (0.6 * w), y + 1 + (0 * h), cgs.media.hudFont, 0.8 * (0.8 * scale), nameColor, "READY", 24, DT_OUTLINE);
    }
    else if (ci->ghost)
    {
        CG_DrawPic(x + (0.85 * w), y + 1 + (0 * h), 0.4 * h, 0.4 * h, cgs.media.deadShader);
    }
    // Draw any gametype items the guy is carrying
    else
    {
        float xx = x + (0.85 * w);
        int i;

        for (i = 0; i < MAX_GAMETYPE_ITEMS; i++)
        {
            centity_t *cent;

            cent = CG_GetEntity(score->client);

            // No have item, no draw it
            if (!(ci->gametypeitems & (1 << i)))
            {
                continue;
            }

            if (!cg_items[MODELINDEX_GAMETYPE_ITEM + i].icon)
            {
                continue;
            }

            CG_DrawPic(xx, y + 1 + (0 * h), 0.4 * h, 0.4 * h, cg_items[MODELINDEX_GAMETYPE_ITEM + i].icon);

            xx += 0 * h;
        }
    }

    s = va("%i", score->score);
    f = trap_R_GetTextWidth(s, cgs.media.hudFont, 0.8 * scale, 0);
    CG_DrawText(x + w - f - 10, y + (0 * h), cgs.media.hudFont, 1.0 * scale, nameColor, s, 0, DT_OUTLINE);

    if (score->ping >= 0 && score->ping < 999)
    {
        CG_DrawText(x + (0 * w), y + (0.4 * h), cgs.media.hudFont, 0.6 * scale, x4c, "ping:", 24, 0);
        CG_DrawText(x + (0.08 * w), y + (0.4 * h), cgs.media.hudFont, 0.6 * scale, dataColor, va("%i", score->ping), 24, 0);

        CG_DrawText(x + (0.2 * w), y + (0.4 * h), cgs.media.hudFont, 0.6 * scale, x4c, "dmgD:", 24, 0);
        CG_DrawText(x + (0.32 * w), y + (0.4 * h), cgs.media.hudFont, 0.6 * scale, dataColor, va("%i", score->damageDone), 24, 0);

        CG_DrawText(x + (0.42 * w), y + (0.4 * h), cgs.media.hudFont, 0.6 * scale, x4c, "Acc:", 24, 0);
        CG_DrawText(x + (0.52 * w), y + (0.4 * h), cgs.media.hudFont, 0.6 * scale, dataColor, va("%.1f%%", score->accuracy), 24, 0);

        CG_DrawText(x + (0 * w), y + (0.7 * h), cgs.media.hudFont, 0.6 * scale, x4c, "id:", 24, 0);
        CG_DrawText(x + (0.08 * w), y + (0.7 * h), cgs.media.hudFont, 0.6 * scale, dataColor, va("%i", score->client), 24, 0);

        CG_DrawText(x + (0.2 * w), y + (0.7 * h), cgs.media.hudFont, 0.6 * scale, x4c, "time:", 24, 0);
        CG_DrawText(x + (0.32 * w), y + (0.7 * h), cgs.media.hudFont, 0.6 * scale, dataColor, va("%i", score->time), 24, 0);

        if (score->teamkillDamage)
        {
            CG_DrawText(x + (0.65 * w), y + (0.7 * h), cgs.media.hudFont, 0.6 * scale, x4c, "TKill:", 24, 0);
            CG_DrawText(x + (0.75 * w), y + (0.7 * h), cgs.media.hudFont, 0.6 * scale, dataColor, va("%i%%", score->teamkillDamage), 24, 0);
        }

        f = trap_R_GetTextWidth(va("%i/%i", score->kills, score->deaths), cgs.media.hudFont, 0.6 * scale, 0);
        CG_DrawText(x + w - f - 10, y + (0.7 * h) - 1, cgs.media.hudFont, 0.7f * scale, dataColor, va("%i/%i", score->kills, score->deaths), 24, 0);
    }
    else if (score->ping >= 999)
    {
        CG_DrawText(x + (0 * w), y + (0.55 * h), cgs.media.hudFont, 0.6 * scale, dataColor, "disconnected", 24, 0);
    }
    else
    {
        CG_DrawText(x + (0 * w), y + (0.55 * h), cgs.media.hudFont, 0.6 * scale, dataColor, "connecting", 24, 0);
    }

    if ((ci->ghost || score->ping < 0 || score->ping >= 999) && cg.snap->ps.pm_type != PM_INTERMISSION)
    {
        vec4_t hcolor;

        hcolor[0] = 0;
        hcolor[1] = 0;
        hcolor[2] = 0;
        hcolor[3] = 0.5f;

        VectorSet(nameColor, 0.5f, 0.5f, 0.5f);

        CG_FillRect(x - 5, y, w, h, hcolor);

        {
            char *xa0;
            char *x124;
            char name[128];
            strncpy(name, ci->name, sizeof(name));
            x124 = name;
            xa0 = name;
            Q_CleanStr(x124);

            while (*xa0)
            {
                if (*xa0 == '^')
                {
                    *xa0 = ' ';
                }

                xa0++;
            }

            CG_DrawText(x + (0 * w), y + (0 * h), cgs.media.hudFont, 0.8 * scale, nameColor, x124, 24, 0);
        }

        return qtrue;
    }
    else
    {
        CG_DrawText(x + (0 * w), y + (0 * h), cgs.media.hudFont, 0.8 * scale, nameColor, ci->name, 24, 0);
        return qfalse;
    }
}

// osp: 0x000284c3
static int OSP_DrawTeamScoreboard(float x, float y, float w, team_t team)
{
    int count;
    int i;
    qboolean drawnClient;
    vec4_t color;
    qboolean x44;
    clientInfo_t *ci;
    int skipped;
    const char *s;
    int players;

    x44 = qfalse;

    // Do we make sure the current client is drawn?
    drawnClient = qtrue;
    if (cg.scores[cg.snap->ps.clientNum].team == team)
    {
        drawnClient = qfalse;
    }

    // Determine the color for this team
    switch (team)
    {
    case TEAM_RED:
        VectorCopy4(g_color_table[ColorIndex(COLOR_RED)], color);
        break;

    case TEAM_BLUE:
        VectorCopy4(g_color_table[ColorIndex(COLOR_BLUE)], color);
        break;

    case TEAM_FREE:
        VectorCopy4(g_color_table[ColorIndex(COLOR_GREEN)], color);
        break;

    case TEAM_SPECTATOR:
    default:
        VectorCopy4(colorWhite, color);
        break;
    }

    // Draw as many clients as we can for this team
    for (skipped = -1, count = 0, i = 0; i < cg.numScores && count < ospSb_maxClients; i++)
    {
        score_t *score;

        score = &cg.scores[i];
        ci = &cgs.clientinfo[score->client];

        if (team != score->team)
        {
            continue;
        }

        if (count == ospSb_maxClients - 1 && !drawnClient)
        {
            if (score->client != cg.snap->ps.clientNum)
            {
                skipped = i;
                continue;
            }

            drawnClient = qtrue;
        }

        x44 = OSP_DrawClientScore(x, y + 30 + ospSb_lineHeight * count, w, ospSb_lineHeight, ospSb_nameFontScale, score, color);
        count++;
    }

    if (skipped != -1 && count < ospSb_maxClients)
    {
        x44 = OSP_DrawClientScore(x, y + 30 + ospSb_lineHeight * count, w, ospSb_lineHeight, ospSb_nameFontScale, &cg.scores[skipped], color);
        count++;
    }

    s = "";
    switch (team)
    {
    case TEAM_RED:
        s = va("%s", ospCg.teamNames[TEAM_RED]);
        players = ui_info_redcount.integer;
        break;

    case TEAM_BLUE:
        s = va("%s", ospCg.teamNames[TEAM_BLUE]);
        players = ui_info_bluecount.integer;
        break;

    case TEAM_FREE:
        s = "PLAYERS";
        players = ui_info_freecount.integer;
        break;

    default:
    case TEAM_SPECTATOR:
        s = "SPECTATORS";
        players = ui_info_speccount.integer;
        break;
    }

    // Use the same team color here, but alpha it a bit.
    color[3] = 0.6f;

    // Draw the header information for this team
    CG_DrawPic(x - 5, y, w, 25, cgs.media.scoreboardHeader);
    CG_FillRect(x - 5, y, w, 25, color);
    CG_DrawText(x, y, cgs.media.hudFont, 0.40f, colorWhite, va("%s", s), 0, 0);
    CG_DrawText(x, y + 13, cgs.media.hudFont, 0.30f, colorWhite, va("players: %d", players), 0, 0);

    // Draw the totals if this is the red or blue team
    if (team == TEAM_RED || team == TEAM_BLUE)
    {
        const char *s;
        float f;

        s = va("%d", (cg.teamScores[team - TEAM_RED]));
        f = trap_R_GetTextWidth(s, cgs.media.hudFont, 0.43f, 0);
        CG_DrawText(x + w - 10 - f, y, cgs.media.hudFont, 0.43f, colorWhite, s, 0, DT_OUTLINE);
    }

    if (count)
    {
        if (x44 && cgs.gametypeData->respawnType == RT_NONE)
            CG_DrawPic(x - 5, y + 35 + ospSb_lineHeight * count, w, ospSb_lineHeight, cgs.media.scoreboardFooter);
        else
            CG_DrawPic(x - 5, y + 30 + ospSb_lineHeight * count, w, ospSb_lineHeight, cgs.media.scoreboardFooter);
    }

    y = count * ospSb_lineHeight + y + 10;

    if (y > cg.scoreBoardBottom)
    {
        cg.scoreBoardBottom = y;
    }

    return y;
}

// osp: 0x00028779
static void OSP_DrawSpectators(void)
{
    score_t *score;
    clientInfo_t *ci;
    int i;
    int numDrawn;
    const char *s;
    int y;
    int h;
    int w;
    vec4_t color;
    int numLines;

    numDrawn = 0;
    w = 160;
    h = 10;
    numLines = (ui_info_speccount.integer + 4 - 1) / 4;
    y = 480 - (h * numLines);

    color[0] = 0.1;
    color[1] = 0.1;
    color[2] = 0.1;
    color[3] = 0.95;

    CG_FillRect(0, y, 640, 480 - y, color);

    for (i = 0; i < cg.numScores; i++)
    {
        score = &cg.scores[i];
        ci = &cgs.clientinfo[score->client];

        if (!ci->infoValid)
        {
            continue;
        }

        if (ci->team != TEAM_SPECTATOR)
        {
            continue;
        }

        if (score->ping >= 0 && score->ping < 999)
            s = va("^0[%s%s%s%s^0] ^7%s ^7(%d)",
                   score->rank & 0x2 ? (score->rank & 0x4 ? "^3A" : "^5A") : "",
                   (score->rank & 0x8 && !(score->rank & 0x2)) ? "^3R" : "",
                   ci->specinviteRed ? "^1Si" : "",
                   ci->specinviteBlue ? "^4Si" : "",
                   ci->name,
                   score->ping);
        else
            s = va("^0[%s%s%s%s^0] ^7%s ^7(%s)",
                   score->rank & 0x2 ? (score->rank & 0x4 ? "^3A" : "^5A") : "",
                   (score->rank & 0x8 && !(score->rank & 0x2)) ? "^3R" : "",
                   ci->specinviteRed ? "^1Si" : "",
                   ci->specinviteBlue ? "^4Si" : "",
                   ci->name,
                   score->ping < 0 ? "conn" : "disc");

        CG_DrawText(w * (numDrawn % 4), y + ((numDrawn / 4) * h), cgs.media.hudFont, 0.3f, colorWhite, s, 0, 0);
        numDrawn++;
    }
}

// osp: 0x00028944
qboolean OSP_DrawScoreboard(float y)
{
    qboolean redFirst = qfalse;

    cg.scoreBoardBottom = 0;

    // DRaw the game timer and the game type
    CG_DrawText(470, y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, "Game Time:", 0, DT_OUTLINE);
    CG_DrawTimer(540, y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, DT_OUTLINE, cg.time - cgs.levelStartTime);
    CG_DrawText(60, y - 14, cgs.media.hudFont, 0.38f, colorLtGrey, va("%s%s", cgs.gametypeData->displayName, ospCg.realisticMode ? " ^3[^2Realistic Mode^3]" : ""), 0, DT_OUTLINE);

    if (ui_info_speccount.integer)
    {
    }

    if (ui_info_redcount.integer > 10 || ui_info_bluecount.integer > 10)
    {
        ospSb_maxClients = 16;
        ospSb_lineHeight = 20;
        ospSb_nameFontScale = 0.35f;
        ospSb_readyFontScale = 0.30f;
        ospSb_numberFontScale = trap_R_GetTextHeight("W", cgs.media.hudFont, ospSb_nameFontScale, 0);
    }
    else
    {
        ospSb_maxClients = 10;
        ospSb_lineHeight = 30;
        ospSb_nameFontScale = 0.43f;
        ospSb_readyFontScale = 0.30f;
        ospSb_numberFontScale = trap_R_GetTextHeight("W", cgs.media.hudFont, ospSb_nameFontScale, 0) + 4;
    }

    // If there are more scores than the scoreboard can show then show the
    // players team first rather than the winning team
    if (cgs.clientinfo[cg.clientNum].team == TEAM_SPECTATOR)
    {
        if (cg.teamScores[0] >= cg.teamScores[1])
        {
            redFirst = qtrue;
        }
    }
    else if (cgs.clientinfo[cg.clientNum].team == TEAM_RED)
    {
        redFirst = qtrue;
    }

    if (redFirst)
    {
        OSP_DrawTeamScoreboard(50, y, 265, TEAM_RED);
        OSP_DrawTeamScoreboard(330, y, 265, TEAM_BLUE);
    }
    else
    {
        OSP_DrawTeamScoreboard(330, y, 265, TEAM_RED);
        OSP_DrawTeamScoreboard(50, y, 265, TEAM_BLUE);
    }

    if (ui_info_speccount.integer)
    {
        OSP_DrawSpectators();
    }

    return qtrue;
}

/*
0x24 --
0x28 redFirst
0x2c
0x30
0x34
0x38
0x3c --
0x40 --
0x44 y
*/