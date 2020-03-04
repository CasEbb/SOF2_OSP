// osp_spawn.c
//
#include "g_local.h"

static vec3_t osp_playerMins = {-15, -15, -46};
static vec3_t osp_playerMaxs = {15, 15, 48};

// osp: 0x00031b0e
static qboolean sub_00031b0e(gspawn_t *start, gspawn_t *end)
{
    vec3_t mins;
    vec3_t maxs;
    trace_t tr;

    VectorCopy(osp_playerMins, mins);
    mins[2] += 10.0;
    VectorCopy(osp_playerMaxs, maxs);
    trap_Trace(&tr, start->origin, mins, maxs, end->origin, -1, MASK_SOLID);

    if (tr.fraction < 1.0)
        return qtrue;
    else
        return qfalse;
}

// osp: 0x00031b43
static qboolean sub_00031b43(gspawn_t *arg0)
{
    vec3_t end;
    trace_t tr;

    VectorCopy(arg0->origin, end);
    end[2] -= 100.0;
    trap_Trace(&tr, arg0->origin, NULL, NULL, end, -1, MASK_SOLID);

    if (tr.fraction < 1.0)
        return qfalse;
    else
        return qtrue;
}

// osp: 0x00031b75
qboolean OSP_FindSafeSpawnpoint(team_t team)
{
    gspawn_t spawn;
    int i;

    for (i = 0; i < level.spawnCount; i++)
    {
        memcpy(&spawn, &level.spawns[i], sizeof(spawn));

        if (team != -1 && team != spawn.team)
        {
            continue;
        }

        switch (rand() % 8)
        {
        case 0:
            spawn.origin[0] += 64;
            spawn.origin[1] += 64;
            break;
        case 1:
            spawn.origin[0] += 64;
            spawn.origin[1] += 0;
            break;
        case 2:
            spawn.origin[0] += 64;
            spawn.origin[1] += -64;
            break;
        case 3:
            spawn.origin[0] += 0;
            spawn.origin[1] += 64;
            break;
        case 4:
            spawn.origin[0] += 0;
            spawn.origin[1] += -64;
            break;
        case 5:
            spawn.origin[0] += -64;
            spawn.origin[1] += 64;
            break;
        case 6:
            spawn.origin[0] += -64;
            spawn.origin[1] += 0;
            break;
        case 7:
            spawn.origin[0] += -64;
            spawn.origin[1] += -64;
            break;
        }

        if (!G_SpotWouldTelefrag(&spawn) &&
            !sub_00031b0e(&level.spawns[i], &spawn) &&
            !sub_00031b43(&spawn))
        {
            memcpy(&level.x191c5c, &spawn, sizeof(level.x191c5c));
            return qtrue;
        }
    }

    return qfalse;
}

// osp: 0x00031c66
qboolean OSP_AltmapExists(void)
{
    static qboolean result = -1;
    fileHandle_t f;

    if (!g_alternateMap.integer)
    {
        return qfalse;
    }

    if (result != -1)
    {
        return result;
    }

    if (trap_FS_FOpenFile(osp.altmapPath, &f, FS_READ) < 0)
    {
        result = qfalse;
    }
    else
    {
        result = qtrue;
        trap_FS_FCloseFile(f);
    }

    return result;
}

// osp: 0x00031c99
static void OSP_ParseAltmapGroup(TGPGroup group)
{
    char temp[256];

    level.numSpawnVars = 0;
    level.numSpawnVarChars = 0;

    trap_GPG_FindPairValue(group, "classname", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("classname", temp);
    }

    trap_GPG_FindPairValue(group, "targetname", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("targetname", temp);
    }

    trap_GPG_FindPairValue(group, "target", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("target", temp);
    }

    trap_GPG_FindPairValue(group, "gametype", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("gametype", temp);
    }

    trap_GPG_FindPairValue(group, "origin", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("origin", temp);
    }

    trap_GPG_FindPairValue(group, "angle", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("angle", temp);
    }

    trap_GPG_FindPairValue(group, "angles", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("angles", temp);
    }

    trap_GPG_FindPairValue(group, "spawnflags", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("spawnflags", temp);
    }

    trap_GPG_FindPairValue(group, "maxs", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("maxs", temp);
    }

    trap_GPG_FindPairValue(group, "mins", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("mins", temp);
    }

    trap_GPG_FindPairValue(group, "model", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("model", temp);
    }

    trap_GPG_FindPairValue(group, "model2", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("model2", temp);
    }

    trap_GPG_FindPairValue(group, "message", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("message", temp);
    }

    trap_GPG_FindPairValue(group, "effect", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("effect", temp);
    }

    trap_GPG_FindPairValue(group, "delay", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("delay", temp);
    }

    trap_GPG_FindPairValue(group, "dmg", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("dmg", temp);
    }

    trap_GPG_FindPairValue(group, "speed", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("speed", temp);
    }

    trap_GPG_FindPairValue(group, "_color", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("_color", temp);
    }

    trap_GPG_FindPairValue(group, "light", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("light", temp);
    }

    trap_GPG_FindPairValue(group, "scale", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("scale", temp);
    }

    trap_GPG_FindPairValue(group, "wait", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("wait", temp);
    }

    trap_GPG_FindPairValue(group, "random", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("random", temp);
    }

    trap_GPG_FindPairValue(group, "team", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("team", temp);
    }

    trap_GPG_FindPairValue(group, "count", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("count", temp);
    }

    trap_GPG_FindPairValue(group, "health", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("health", temp);
    }

    trap_GPG_FindPairValue(group, "targetShaderName", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("targetShaderName", temp);
    }

    trap_GPG_FindPairValue(group, "targetShaderNewName", "", temp);
    if (strlen(temp) > 0)
    {
        AddSpawnField("targetShaderNewName", temp);
    }

    G_SpawnGEntityFromSpawnVars(qfalse);
}

// osp: 0x00031fb6
void OSP_SpawnAltmap(void)
{
    TGPGroup subGroup;
    char temp[256];
    char name[64];
    TGenericParser2 GP2;
    TGPGroup baseGroup;

    if (!g_alternateMap.integer || !OSP_AltmapExists())
    {
        return;
    }

    GP2 = trap_GP_ParseFile(osp.altmapPath, qtrue, qfalse);

    if (!GP2)
    {
        return;
    }

    osp.x6480a0 = qtrue;
    baseGroup = trap_GP_GetBaseParseGroup(GP2);
    subGroup = trap_GPG_GetSubGroups(baseGroup);

    while (subGroup)
    {
        trap_GPG_GetName(subGroup, name);
        if (strcmp(name, "altinfo") == 0)
        {
            trap_GPG_FindPairValue(subGroup, "altinfo1", "", temp);
            memset(osp.altinfo1, 0, sizeof(osp.altinfo1));
            if (strlen(temp) > 0)
            {
                strncpy(osp.altinfo1, temp, sizeof(osp.altinfo1));
            }

            trap_GPG_FindPairValue(subGroup, "altinfo2", "", temp);
            memset(osp.altinfo2, 0, sizeof(osp.altinfo2));
            if (strlen(temp) > 0)
            {
                strncpy(osp.altinfo2, temp, sizeof(osp.altinfo2));
            }
        }
        else
        {
            OSP_ParseAltmapGroup(subGroup);
        }

        subGroup = trap_GPG_GetNext(subGroup);
    }

    trap_GP_Delete(&GP2);
    osp.x6480a0 = qfalse;
}
