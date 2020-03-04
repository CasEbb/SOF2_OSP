// osp_comp.c
//
#include "g_local.h"

static char *osp_compSettings[][2] = {
    {"g_doWarmup", "2"},
    {"g_warmup", "20"},
    {"g_speed", "280"},
    {"g_roundtimelimit", "4"},
    {"g_roundstartdelay", "5"},
    {"g_scorelimit", "7"},
    {"g_timelimit", "0"},
    {"match_swap", "1"},
    {"g_radarFix", "0"},
    {"g_forceespawn", "3"},
    {"g_inactivity", "0"},
    {"g_spectatorInactivity", "0"},
    {"team_controls", "1"},
    {"team_maxplayers", "0"},
    {"match_minplayers", "2"},
    {"match_readypercent", "100"},
    {"g_teamForceBalance", "0"},
    {"g_teamAutoJoin ", "0"},
    {"g_timeouttospec ", "60"},
    {"g_friendlyfire", "1"},
    {"g_teamkillDamageMax", "0"},
    {"g_forceFollow", "2"},
    {"g_followEnemy", "0"},
    {"vote_limit", "0"},
    {"g_failedVoteDelay", "0"},
};

static char *osp_pubSettings[][2] = {
    {"g_doWarmup", "1"},
};

// osp: 0x0002e018
qboolean OSP_LoadSettings(qboolean comp, char *name)
{
    char **settings;
    int i;
    unsigned int numSettings;
    fileHandle_t f;
    char *path;
    int len;

    if (name[0])
        path = va("configs/osp_%s_%s.%s.cfg", g_gametype.string, comp ? "comp" : "pub", name);
    else
        path = va("configs/osp_%s_%s.cfg", g_gametype.string, comp ? "comp" : "pub");

    len = trap_FS_FOpenFile(path, &f, FS_READ);

    if (!f)
    {
        // no file, load hardcoded settings
        if (comp)
        {
            settings = osp_compSettings[0];
            numSettings = sizeof(osp_compSettings) / sizeof(osp_compSettings[0]);
        }
        else
        {
            settings = osp_pubSettings[0];
            numSettings = sizeof(osp_pubSettings) / sizeof(osp_pubSettings[0]);
        }

        for (i = 0; i < numSettings; i++, settings += 2)
        {
            trap_SendConsoleCommand(EXEC_APPEND, va("set %s %s\n", settings[0], settings[1]));
        }

        trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
    }
    else
    {
        trap_SendConsoleCommand(EXEC_APPEND, va("exec %s;map_restart\n", path));
    }

    trap_FS_FCloseFile(f);
    return qtrue;
}
