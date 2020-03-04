// osp_chattokens.c
//
#include "g_local.h"

static char *ChatToken_Armor(gentity_t *e, int i, qboolean variant);
static char *ChatToken_Health(gentity_t *e, int i, qboolean variant);
static char *ChatToken_Bar(gentity_t *e, int i, qboolean variant);
static char *ChatToken_LastToHit(gentity_t *e, int i, qboolean variant);
static char *ChatToken_LastHitBy(gentity_t *e, int i, qboolean variant);
static char *ChatToken_ClosestTeammate(gentity_t *e, int i, qboolean variant);
static char *ChatToken_Location(gentity_t *e, int i, qboolean variant);
static char *ChatToken_Pistol(gentity_t *e, int i, qboolean variant);
static char *ChatToken_PrimaryWeapon(gentity_t *e, int i, qboolean variant);
static char *ChatToken_SecondaryWeapon(gentity_t *e, int i, qboolean variant);
static char *ChatToken_Grenade(gentity_t *e, int i, qboolean variant);
static char *ChatToken_Equipment(gentity_t *e, int i, qboolean variant);

typedef struct
{
    char *token;
    int rfu1;
    qboolean variant;
    char *(*func)(gentity_t *e, int i, qboolean variant);
    char *helptext;
} ospChatToken_t;

// osp: 0x0000387c
static const ospChatToken_t ospChatTokens[25] = {
    {"A", 0, qfalse, ChatToken_Armor, ":^7 Current armor"},
    {"a", 0, qtrue, ChatToken_Armor, ":^7 Current armor with colorcoding"},
    {"H", 0, qfalse, ChatToken_Health, ":^7 Current health"},
    {"h", 0, qtrue, ChatToken_Health, ":^7 Current health with colorcoding"},
    {"B", 0, qfalse, ChatToken_Bar, ":^7 Current health in bar code"},
    {"b", 0, qtrue, ChatToken_Bar, ":^7 Current armor in bar code"},
    {"D", 0, qfalse, ChatToken_LastHitBy, ":^7 Last player to hurt you"},
    {"d", 0, qtrue, ChatToken_LastHitBy, ":^7 Last player to hurt you (with colors)"},
    {"T", 0, qfalse, ChatToken_LastToHit, ":^7 Last player you hurt"},
    {"t", 0, qtrue, ChatToken_LastToHit, ":^7 Last player you hurt (with colors)"},
    {"F", 0, qfalse, ChatToken_ClosestTeammate, ":^7 Closest teammate"},
    {"f", 0, qtrue, ChatToken_ClosestTeammate, ":^7 Closest teammate (with colors)"},
    {"L", 0, qfalse, ChatToken_Location, ":^7 Location in level"},
    {"l", 0, qtrue, ChatToken_Location, ":^7 Location in level"},
    {"I", 0, qfalse, ChatToken_Pistol, ":^7 Current pistol"},
    {"i", 0, qtrue, ChatToken_Pistol, ":^7 Current pistol (short)"},
    {"P", 0, qfalse, ChatToken_PrimaryWeapon, ":^7 Current primary weapon"},
    {"p", 0, qtrue, ChatToken_PrimaryWeapon, ":^7 Current primary weapon (short)"},
    {"S", 0, qfalse, ChatToken_SecondaryWeapon, ":^7 Current secondary weapon"},
    {"s", 0, qtrue, ChatToken_SecondaryWeapon, ":^7 Current secondary weapon (short)"},
    {"G", 0, qfalse, ChatToken_Grenade, ":^7 Current grenades"},
    {"g", 0, qtrue, ChatToken_Grenade, ":^7 Current grenades (short)"},
    {"E", 0, qfalse, ChatToken_Equipment, ":^7 Current equipment"},
    {"e", 0, qtrue, ChatToken_Equipment, ":^7 Current equipment"},
    {"0", 0, qfalse, 0, NULL},
};

// osp: 0x00003a70
static int ospNumTokens = (sizeof(ospChatTokens) / sizeof(ospChatTokens[0])) - 1;

// osp: 0x0002a6f4
static char *OSP_ColorForValue(int value)
{
    if (value > 90)
        return "^v";
    else if (value > 80)
        return "^g";
    else if (value > 70)
        return "^'";
    else if (value > 60)
        return "^3";
    else if (value > 50)
        return "^k";
    else if (value > 40)
        return "^N";
    else if (value > 30)
        return "^b";
    else if (value > 20)
        return "^C";
    else if (value > 10)
        return "^T";
    else
        return "^1";
}

// osp: 0x0002a741
static char *ChatToken_Armor(gentity_t *e, int i, qboolean variant)
{
    gentity_t *ent = e;

    if (variant)
        return va("%s%d^2", OSP_ColorForValue(ent->client->ps.stats[STAT_ARMOR]), ent->client->ps.stats[STAT_ARMOR]);
    else
        return va("%d", ent->client->ps.stats[STAT_ARMOR]);
}

// osp: 0x0002a782
static char *ChatToken_Health(gentity_t *e, int i, qboolean variant)
{
    gentity_t *ent = e;

    if (variant)
        return va("%s%d^2", OSP_ColorForValue(ent->client->ps.stats[STAT_HEALTH]), ent->client->ps.stats[STAT_HEALTH]);
    else
        return va("%d", ent->client->ps.stats[STAT_HEALTH]);
}

// osp: 0x0002a7c3
static char *ChatToken_Bar(gentity_t *e, int idx, qboolean variant)
{
    gentity_t *ent;
    char bar[32];
    int value;
    int i;

    ent = e;

    memset(bar, 0, sizeof(bar));

    if (variant)
    {
        value = ent->client->ps.stats[STAT_ARMOR];
    }
    else
    {
        value = ent->client->ps.stats[STAT_HEALTH];
    }

    for (i = 0; i < 10; i++)
    {
        if (i * 10 <= value)
        {
            if (variant)
            {
                strcat(bar, "^4-");
            }
            else
            {
                strcat(bar, "^1-");
            }
        }
        else
        {
            strcat(bar, "^0-");
        }
    }

    return va("%s^2", bar);
}

// osp: 0x0002a829
static char *ChatToken_LastToHit(gentity_t *e, int i, qboolean variant)
{
    int clientNum;
    gentity_t *ent = e;

    variant = qtrue;

    if (!ent->client->ospClient->lastHitClient)
    {
        return "none";
    }

    clientNum = ent->client->ospClient->lastHitClient - 1;

    if (level.clients[clientNum].pers.connected != CON_CONNECTED)
    {
        return "none";
    }

    if (variant)
        return va("%s^2", level.clients[clientNum].pers.netname);
    else
        return va("%s", osp.clients[clientNum].cleanname);
}

// osp: 0x0002a88f
static char *ChatToken_LastHitBy(gentity_t *e, int i, qboolean variant)
{
    int clientNum;
    gentity_t *ent = e;

    variant = qtrue;

    if (!ent->client->ospClient->lastHitByClient)
    {
        return "none";
    }

    clientNum = ent->client->ospClient->lastHitByClient - 1;

    if (level.clients[clientNum].pers.connected != CON_CONNECTED)
    {
        return "none";
    }

    if (variant)
        return va("%s^2", level.clients[clientNum].pers.netname);
    else
        return va("%s", osp.clients[clientNum].cleanname);
}

// osp: 0x0002a8f5
static char *ChatToken_ClosestTeammate(gentity_t *e, int idx, qboolean variant)
{
    int i;
    gentity_t *other;
    gentity_t *ent;
    float nearest;
    int clientNum;

    ent = e;
    clientNum = -1;
    nearest = 999999;
    variant = qtrue;

    for (i = 0; i < level.maxclients; i++)
    {
        other = &g_entities[i];

        if (other == ent)
        {
            continue;
        }

        if (OSP_IsClientOnline(i) && OnSameTeam(ent, other))
        {
            float dist;
            dist = Distance(ent->r.currentOrigin, other->r.currentOrigin);

            if (dist < nearest)
            {
                nearest = dist;
                clientNum = i;
            }
        }
    }

    if (clientNum >= 0)
    {
        if (variant)
            return va("%s^2", level.clients[clientNum].pers.netname);
        else
            return va("%s", osp.clients[clientNum].cleanname);
    }
    else
    {
        return "none";
    }
}

// osp: 0x0002a995
static char *ChatToken_Location(gentity_t *e, int i, qboolean variant)
{
    char location[64];
    gentity_t *ent;

    ent = e;

    if (Team_GetLocationMsg(ent, location, sizeof(location)))
        return va("%s", location);
    else
        return "unknown";
}

// osp: 0x0002a9ba
static char *ChatToken_Pistol(gentity_t *e, int i, qboolean variant)
{
    weapon_t weapon;
    gentity_t *ent;

    ent = e;

    for (weapon = WP_M1911A1_PISTOL; weapon < WP_M590_SHOTGUN; weapon++)
    {
        if (ent->client->ps.stats[STAT_WEAPONS] & (1 << weapon))
        {
            if (variant)
                return osp_shortWeaponNames[weapon];
            else
                return bg_weaponNames[weapon];
        }
    }

    return "none";
}

// osp: 0x0002a9f7
static char *ChatToken_PrimaryWeapon(gentity_t *e, int i, qboolean variant)
{
    weapon_t weapon;
    gentity_t *ent;

    ent = e;

    for (weapon = WP_MP5; weapon < WP_M84_GRENADE; weapon++)
    {
        if (ent->client->ps.stats[STAT_WEAPONS] & (1 << weapon))
        {
            if (variant)
                return osp_shortWeaponNames[weapon];
            else
                return bg_weaponNames[weapon];
        }
    }

    return "none";
}

// osp: 0x0002aa34
static char *ChatToken_SecondaryWeapon(gentity_t *e, int i, qboolean variant)
{
    weapon_t weapon;
    gentity_t *ent;

    ent = e;

    for (weapon = WP_M590_SHOTGUN; weapon < WP_MP5; weapon++)
    {
        if (ent->client->ps.stats[STAT_WEAPONS] & (1 << weapon))
        {
            if (variant)
                return osp_shortWeaponNames[weapon];
            else
                return bg_weaponNames[weapon];
        }
    }

    return "none";
}

// osp: 0x0002aa71
static char *ChatToken_Grenade(gentity_t *e, int i, qboolean variant)
{
    weapon_t weapon;
    gentity_t *ent;

    ent = e;

    for (weapon = WP_M84_GRENADE; weapon < WP_NUM_WEAPONS; weapon++)
    {
        if (weapon == WP_OICW || weapon == WP_USSOCOM_SILENCED)
        {
            continue;
        }

        if (ent->client->ps.stats[STAT_WEAPONS] & (1 << weapon))
        {
            if (variant)
                return osp_shortWeaponNames[weapon];
            else
                return bg_weaponNames[weapon];
        }
    }

    return "none";
}

// osp: 0x0002aab8
static char *ChatToken_Equipment(gentity_t *e, int i, qboolean variant)
{
    gentity_t *ent = e;

    switch (bg_outfittingGroups[OUTFITTING_GROUP_ACCESSORY][ent->client->pers.outfitting.items[OUTFITTING_GROUP_ACCESSORY]])
    {
    case MODELINDEX_ARMOR:
        return bg_itemlist[MODELINDEX_ARMOR].pickup_name;
    case MODELINDEX_THERMAL:
        return bg_itemlist[MODELINDEX_THERMAL].pickup_name;
    case MODELINDEX_NIGHTVISION:
        return bg_itemlist[MODELINDEX_NIGHTVISION].pickup_name;
    default:
        return "none";
    }
}

// osp: 0x0002aaed
static char *OSP_ExpandToken(gentity_t *ent, char token)
{
    int i;

    for (i = 0; i < ospNumTokens; i++)
    {
        if (ospChatTokens[i].token[0] == token)
        {
            return ospChatTokens[i].func(ent, i, ospChatTokens[i].variant);
        }
    }

    return NULL;
}

// osp: 0x0002ab32
void OSP_ParseTokens(gentity_t *e, char *text, int size)
{
    gentity_t *ent;
    char newText[MAX_SAY_TEXT];
    int i;

    ent = e;

    if (G_IsClientSpectating(ent->client))
    {
        return;
    }

    memset(newText, 0, sizeof(newText));

    for (i = 0; i < strlen(text) && strlen(newText) < MAX_SAY_TEXT - 1; i++)
    {
        char *buf = NULL;

        if (text[i] == '#' && text[i + 1] != '\0' && (buf = OSP_ExpandToken(ent, text[i + 1])))
        {
            strcat(newText, buf);
            i++;
            continue;
        }

        newText[strlen(newText)] = text[i];
    }

    memset(text, 0, size);
    Q_strncpyz(text, newText, size);
}

// osp: 0x0002abdb
void OSP_Cmd_Tokens(gentity_t *e, int idx, int arg)
{
    gentity_t *ent;
    int i;

    ent = e;

    trap_SendServerCommand(ent - g_entities, "print \"^1\nAvailable chat tokens:\n----------------------------\n");

    for (i = 0; i < ospNumTokens; i++)
    {
        trap_SendServerCommand(ent - g_entities, va("print \"^1%c%s %s\n\"", '#', ospChatTokens[i].token, ospChatTokens[i].helptext));
    }
}
