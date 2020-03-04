// osp_voting.c
//
#include "g_local.h"

char *ConcatArgs(int start);

static char *x4a4c[] = {"ACTIVATED", "DEACTIVATED"};
static char *x4a54[] = {"ENABLE", "DISABLE"};
static char *x4a5c[] = {"ENABLED", "DISABLED"};

typedef struct
{
	int flags;
	char *name;
	int (*func)(gentity_t *, unsigned int, char *, char *, int);
	int numParameters;
	char *description;
	char *helptext;
} callvoteTable_t;

static int OSP_Callvote_Dispatch(gentity_t *ent, char *arg1, char *arg2, int adminAction);
static void OSP_Callvote_ListCmds(gentity_t *ent, qboolean arg);
static qboolean OSP_Callvote_Usage(gentity_t *ent, int adminAction, int index);
static void OSP_VoteDisabledError(gentity_t *ent, char *cmd);
static void OSP_PrintValue(gentity_t *ent, const char *key, const char *value);
static int OSP_Callvote_Setting(gentity_t *ent, char *cmd, char *arg, int adminAction, qboolean currentValue, qboolean disabled, int index);
static void OSP_ChangeSettingBool(char *desc, char *cvar);
static void OSP_ChangeSettingBool(char *desc, char *cvar);
static void OSP_ChangeSettingVotestring(char *desc);

static int Callvote_Comp(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_Pub(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_PistolsOnly(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_KnivesOnly(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_WeaponOnly(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_DoWarmup(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_Warmup(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_Scorelimit(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_Gametype(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_Kick(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_Map(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_RMGMap(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_MapRestart(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_RoundRestart(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_Mapcycle(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_Referee(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_ShuffleTeams(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_BalanceTeams(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_StartMatch(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_SwapTeams(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_FriendlyFire(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_RealisticMode(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_Timelimit(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_TimeExtension(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);
static int Callvote_Unreferee(gentity_t *ent, unsigned int index, char *cmd, char *arg, int adminAction);

static const callvoteTable_t callvoteCmds[] = {
	{0xFF, "comp", Callvote_Comp, 0, "Load Competition Settings", " <option configname>^7\n  Loads standard competition settings for the current mode"},
	{0xFF, "pub", Callvote_Pub, 0, "Load Public Settings", " <option configname>^7\n  Loads standard public settings for the current mode"},
	{0xFF, "map_restart", Callvote_MapRestart, 0, "Map Restart", "^7\n  Restarts the current map"},
	{0xFF, "gametype", Callvote_Gametype, 1, "Set Gametype to", " <value>^7\n  Changes the current gametype"},
	{0xFF, "start_match", Callvote_StartMatch, 0, "Start Match", " ^7\n  Sets all players to \"ready\" status to start the match"},
	{0xFF, "roundrestart", Callvote_RoundRestart, 0, "Restart Round", " ^7\n  Restarts the current round"},
	{0xFF, "scorelimit", Callvote_Scorelimit, 1, "Scorelimit", " <value>^7\n  Changes the current scorelimit"},
	{0xFF, "timelimit", Callvote_Timelimit, 1, "Timelimit", " <value>^7\n  Changes the current timelimit"},
	{0xFF, "timeextension", Callvote_TimeExtension, 0, "Extend time by", " <value>^7\n  Extends the current timelimit"},
	{0xFF, "kick", Callvote_Kick, 1, "Kick", " <player_name>^7\n  Attempts to kick player from server"},
	{0xFF, "clientkick", Callvote_Kick, 1, "Kick", " <player_name>^7\n  Attempts to kick player from server"},
	{0xFF, "g_friendlyfire", Callvote_FriendlyFire, 1, "Friendly fire", " <0|1>^7\n  Toggles ability to hurt teammates"},
	{0xFF, "g_doWarmup", Callvote_DoWarmup, 1, "Change warmup mode to:", " <0|1|2>^7\n  Change the warmup mode"},
	{0xFF, "g_warmup", Callvote_Warmup, 1, "Change warmup duration to:", " <value>^7\n  Change the warmup duration"},
	{0xFF, "pistolsonly", Callvote_PistolsOnly, 0, "Play next round using pistols only", "^7\n  Play the next round using only pistols"},
	{0xFF, "knivesonly", Callvote_KnivesOnly, 0, "Play next round using knives only", "^7\n  Play the next round using only knives"},
	{0xFF, "weapononly", Callvote_WeaponOnly, 1, "Play next round using the ", "^7\n  Play the next round using only the specified weapon\n 1-Knife\n 2-M1911A1\n 3-SOCOM\n 4-STalon\n 5-M590\n 6-Uzi\n 7-M3A1\n 8-MP5\n 9-USAS-12\n10-M4\n11-AK74\n12-Sig\n13-MSG\n14-M60\n15-MM1\n16-RPG7\n17-M84\n18-SMOHG92\n19-ANM14\n20-M15\n"},
	{0xFF, "realisticmode", Callvote_RealisticMode, 1, "Realistic Mode", " <0|1?^7\n  Toggles realistic mode"},
	{0xFF, "map", Callvote_Map, 1, "Change map to", " <mapname>^7\n  Votes for a new map to be loaded"},
	{0xFF, "mapcycle", Callvote_Mapcycle, 0, "Load next map", "^7\n  Loads the next map in the map queue"},
	{0xFF, "rmgmap", Callvote_RMGMap, 3, "Load randomly generated map", "^7\n  Load a randomly generated map"},
	{0xFF, "referee", Callvote_Referee, 1, "Referee", " <player_id>^7\n  Elects a player to have admin abilities"},
	{0xFF, "unreferee", Callvote_Unreferee, 1, "UNReferee", " <player_id>^7\n  Elects a player to have admin abilities removed"},
	{0xFF & ~0x2, "shuffleteams", Callvote_ShuffleTeams, 0, "Shuffle teams and restart", " ^7\n  Randomly place players on each team"},
	{0xFF & ~0x2, "swapteams", Callvote_SwapTeams, 0, "Swap teams and restart", " ^7\n  Switch the players on each team"},
	{0xFF & ~0x2, "balanceteams", Callvote_BalanceTeams, 0, "Balances teams", " ^7\n  Balance the players to each team"},
	{0},
};

// osp: 0x000332d4
static int OSP_Callvote_Dispatch(gentity_t *e, char *arg1, char *arg2, int adminAction)
{
	gentity_t *ent;
	unsigned int i, numCmds;

	numCmds = sizeof(callvoteCmds) / sizeof(callvoteCmds[0]);
	ent = e;

	for (i = 0; i < numCmds; i++)
	{
		if (trap_Argc() == 1)
		{
			OSP_Callvote_ListCmds(ent, (adminAction > 0 ? qtrue : qfalse));
			return -1;
		}

		if (Q_stricmp(arg1, callvoteCmds[i].name) == 0)
		{
			int result;
			result = callvoteCmds[i].func(ent, i, arg1, arg2, adminAction);

			if (!result)
			{
				Com_sprintf(arg1, 256, callvoteCmds[i].description);
				osp.callvoteFunc = callvoteCmds[i].func;
			}
			else
			{
				osp.callvoteFunc = 0;
			}

			return result;
		}

		if (Q_stricmp(arg1, "?") == 0)
		{
			OSP_Callvote_ListCmds(ent, (adminAction > 0 ? qtrue : qfalse));
			return -1;
		}
	}

	return -2;
}

// osp: 0x0003338e
static void OSP_Callvote_ListCmds(gentity_t *e, qboolean arg)
{
	int i;
	int numRows;
	int cmds[100];
	int numCmds;
	gentity_t *ent;

	numRows = 0;
	numCmds = (sizeof(callvoteCmds) / sizeof(callvoteCmds[0])) - 1;
	ent = e;

	if (arg)
	{
		trap_SendServerCommand(ent - g_entities, "print \"\nValid ^3callvote^7 commands are:\n^3----------------------------\n\"");
	}
	else
	{
		trap_SendServerCommand(ent - g_entities, "print \"\nValid settings change are:\n^3----------------------------\n\"");
	}

	for (i = 0; i < numCmds; i++)
	{
		if (callvoteCmds[i].flags & (1 << g_gametype.integer))
		{
			cmds[numRows++] = i;
		}
	}

	numCmds = numRows;
	numRows = numCmds / 4;
	if (numCmds % 4)
		numRows++;
	if (numRows < 0)
		return;

	for (i = 0; i < numRows; i++)
	{
		if (i + (3 * numRows) + 1 <= numCmds)
		{
			trap_SendServerCommand(ent - g_entities,
								   va("print \"%s%-17s%s%-17s%s%-17s%s%-17s\n\"",
									  OSP_VoteDisabled(callvoteCmds[cmds[i]].name) ? "^8" : "^5",
									  callvoteCmds[cmds[i]].name,
									  OSP_VoteDisabled(callvoteCmds[cmds[i + numRows]].name) ? "^8" : "^5",
									  callvoteCmds[cmds[i + numRows]].name,
									  OSP_VoteDisabled(callvoteCmds[cmds[i + (2 * numRows)]].name) ? "^8" : "^5",
									  callvoteCmds[cmds[i + (2 * numRows)]].name,
									  OSP_VoteDisabled(callvoteCmds[cmds[i + (3 * numRows)]].name) ? "^8" : "^5",
									  callvoteCmds[cmds[i + (3 * numRows)]].name));
		}
		else if (i + (2 * numRows) + 1 <= numCmds)
		{
			trap_SendServerCommand(ent - g_entities,
								   va("print \"%s%-17s%s%-17s%s%-17s\n\"",
									  OSP_VoteDisabled(callvoteCmds[cmds[i]].name) ? "^8" : "^5",
									  callvoteCmds[cmds[i]].name,
									  OSP_VoteDisabled(callvoteCmds[cmds[i + numRows]].name) ? "^8" : "^5",
									  callvoteCmds[cmds[i + numRows]].name,
									  OSP_VoteDisabled(callvoteCmds[cmds[i + (2 * numRows)]].name) ? "^8" : "^5",
									  callvoteCmds[cmds[i + (2 * numRows)]].name));
		}
		else if (i + (numRows) + 1 <= numCmds)
		{
			trap_SendServerCommand(ent - g_entities,
								   va("print \"%s%-17s%s%-17s\n\"",
									  OSP_VoteDisabled(callvoteCmds[cmds[i]].name) ? "^8" : "^5",
									  callvoteCmds[cmds[i]].name,
									  OSP_VoteDisabled(callvoteCmds[cmds[i + numRows]].name) ? "^8" : "^5",
									  callvoteCmds[cmds[i + numRows]].name));
		}
		else
		{
			trap_SendServerCommand(ent - g_entities,
								   va("print \"%s%-17s\n\"",
									  OSP_VoteDisabled(callvoteCmds[cmds[i]].name) ? "^8" : "^5",
									  callvoteCmds[cmds[i]].name));
		}
	}

	if (arg)
	{
		trap_SendServerCommand(ent - g_entities, "print \"\nUsage: ^3\\callvote <command> <params>\n^7For current settings/help, use: ^3\\callvote <command> ?\n\n\"");
	}
}

// osp: 0x000336b5
static qboolean OSP_Callvote_Usage(gentity_t *e, int adminAction, int index)
{
	char arg[512];
	gentity_t *ent;
	char *cmd;

	cmd = adminAction == 2 ? "\\adm" : (adminAction == 1 ? " \\ref" : "\\callvote");
	ent = e;

	if (!ent)
	{
		return qfalse;
	}

	trap_Argv(2, arg, sizeof(arg));

	if (Q_stricmp(arg, "?") == 0 || trap_Argc() == callvoteCmds[index].numParameters)
	{
		trap_Argv(1, arg, sizeof(arg));
		trap_SendServerCommand(ent - g_entities,
							   va("print \"\nUsage: ^3%s %s%s\n\n\"",
								  cmd,
								  arg,
								  callvoteCmds[index].helptext));
		return qtrue;
	}

	return qfalse;
}

// osp: 0x00033733
static void OSP_VoteDisabledError(gentity_t *e, char *cmd)
{
	gentity_t *ent = e;

	if (OSP_IsClientReferee(ent - g_entities))
		trap_SendServerCommand(ent - g_entities, va("print \"\nSorry, ^3%s^7 is not avalaible to referees\n\"", cmd));
	else
		trap_SendServerCommand(ent - g_entities, va("print \"\nSorry, ^3%s^7 voting has been disabled\n\"", cmd));
}

// osp: 0x00033777
static void sub_00033777(gentity_t *e)
{
	gentity_t *ent = e;
	trap_SendServerCommand(ent - g_entities, "print \"Use the ^3/players^7 command to find a valid player ID.\n\"");
}

// osp: 0x0003378a
static void OSP_PrintValue(gentity_t *e, const char *key, const char *value)
{
	gentity_t *ent = e;
	trap_SendServerCommand(ent - g_entities, va("print \"^2%s^7 is currently ^3%s\n\n\"", key, value));
}

// osp: 0x000337aa
static int OSP_Callvote_Setting(gentity_t *e, char *cmd, char *arg, int adminAction, qboolean currentValue, qboolean enabled, int index)
{
	gentity_t *ent = e;

	if (!enabled && adminAction < 2)
	{
		OSP_VoteDisabledError(ent, callvoteCmds[index].name);
		OSP_PrintValue(ent, callvoteCmds[index].name, currentValue ? x4a5c[0] : x4a5c[1]);
		return -1;
	}

	if (OSP_Callvote_Usage(ent, adminAction, index))
	{
		OSP_PrintValue(ent, callvoteCmds[index].name, currentValue ? x4a5c[0] : x4a5c[1]);
		return -1;
	}

	if ((atoi(arg) && currentValue) || (!atoi(arg) && !currentValue))
	{
		trap_SendServerCommand(ent - g_entities, va("print \"^3%s^5 is already %s!\n", callvoteCmds[index].name, currentValue ? x4a5c[0] : x4a5c[1]));
		return -1;
	}

	Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "%s", arg);
	Com_sprintf(arg, 256, "%s", atoi(arg) ? x4a4c[0] : x4a4c[1]);
	return 0;
}

// osp: 0x000338a6
static void OSP_ChangeSettingBool(char *desc, char *cvar)
{
	trap_SendServerCommand(-1, va("print \"^3%s is: ^5%s\n\"", desc, atoi(osp.callvoteArgs[0]) ? x4a5c[0] : x4a5c[1]));
	trap_SendConsoleCommand(EXEC_APPEND, va("set %s %s\n", cvar, osp.callvoteArgs[0]));
}

// osp: 0x000338e4
static void OSP_ChangeSetting(char *desc, char *cvar)
{
	trap_SendServerCommand(-1, va("print \"^3%s set to: ^5%s\n\"", desc, osp.callvoteArgs[0]));
	trap_SendConsoleCommand(EXEC_APPEND, va("set %s %s\n", cvar, osp.callvoteArgs[0]));
}

// osp: 0x0003390d
static void OSP_ChangeSettingVotestring(char *desc)
{
	trap_SendServerCommand(-1, va("print \"^3%s set to: ^5%s\n\"", desc, osp.callvoteArgs[0]));
	trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", level.voteString));
}

// osp: 0x00033933
static int Callvote_Comp(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if (arg)
		{
			Com_sprintf(osp.callvoteArgs[0], 256, "%s", arg);
			Com_sprintf(arg, 256, "(^3%s^7)", osp.callvoteArgs[0]);
		}
		else
		{
			osp.callvoteArgs[0][0] = '\0';
			Com_sprintf(arg, 256, "(^3DEFAULT^7)");
		}
	}
	else
	{
		if (OSP_LoadSettings(qtrue, osp.callvoteArgs[0]))
		{
			trap_SendServerCommand(-1, va("cp \"Competition Settings (%s) Loaded!\n\"", osp.callvoteArgs[0][0] ? osp.callvoteArgs[0] : "default"));
		}
		else
		{
			trap_SendServerCommand(-1, "cp \"Competition Settings ^1NOT^7 Loaded!\n\"");
		}
	}

	return 0;
}

// osp: 0x000339d1
static int Callvote_Pub(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if (arg)
		{
			Com_sprintf(osp.callvoteArgs[0], 256, "%s", arg);
			Com_sprintf(arg, 256, "(^3%s^7)", osp.callvoteArgs[0]);
		}
		else
		{
			osp.callvoteArgs[0][0] = '\0';
			Com_sprintf(arg, 256, "(^3DEFAULT^7)");
		}
	}
	else
	{
		if (OSP_LoadSettings(qfalse, osp.callvoteArgs[0]))
		{
			trap_SendServerCommand(-1, va("cp \"Public Settings (%s) Loaded!\n\"", osp.callvoteArgs[0][0] ? osp.callvoteArgs[0] : "default"));
		}
		else
		{
			trap_SendServerCommand(-1, "cp \"Public Settings ^1NOT^7 Loaded!\n\"");
		}
	}

	return 0;
}

// osp 0x00033a6f
static int Callvote_PistolsOnly(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if (level.gametypeData->respawnType != RT_NONE)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Pistols only is not available in regular respawn gametypes.\n\"");
			return -1;
		}
	}
	else
	{
		trap_SendServerCommand(-1, "cp \"The next round will be \nplayed using ^3PISTOLS ONLY^7!\"");
		osp.pistolsOnly = 2;
		osp.knivesOnly = 0;
		osp.weaponOnly = 0;
	}

	return 0;
}

// osp 0x00033ad4
static int Callvote_KnivesOnly(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if (level.gametypeData->respawnType != RT_NONE)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Knives only is not available in regular respawn gametypes.\n\"");
			return -1;
		}
	}
	else
	{
		trap_SendServerCommand(-1, "cp \"The next round will be \nplayed using ^3KNIVES ONLY^7!\"");
		osp.pistolsOnly = 0;
		osp.knivesOnly = 2;
		osp.weaponOnly = 0;
	}

	return 0;
}

// osp: 0x00033b39
static int Callvote_WeaponOnly(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if (!arg)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Invalid weapon type.\n\"");
			return -1;
		}

		if (strlen(arg) <= 0)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Invalid weapon type.\n\"");
			return -1;
		}

		if (atoi(arg) < 1 || (osp_modversion.integer >= 2 && atoi(arg) > 26) || (osp_modversion.integer < 2 && atoi(arg) > 20))
		{
			trap_SendServerCommand(ent - g_entities, "print \"Invalid weapon type.\n\"");
			return -1;
		}

		if (level.gametypeData->respawnType != RT_NONE)
		{
			trap_SendServerCommand(ent - g_entities, "print \"WeaponOnly vote is not available in regular respawn gametypes.\n\"");
			return -1;
		}

		osp.weaponOnly_weapon = atoi(arg);
		Com_sprintf(arg, 256, "^3%s", bg_weaponNames[osp.weaponOnly_weapon]);
	}
	else
	{
		char buf[256];
		memset(buf, 0, sizeof(buf));
		Com_sprintf(buf, sizeof(buf), "cp \"The next round will be \nplayed using ^3%s ONLY^7!\n\"", bg_weaponNames[osp.weaponOnly_weapon]);
		trap_SendServerCommand(-1, buf);
		osp.pistolsOnly = 0;
		osp.knivesOnly = 0;
		osp.weaponOnly = 2;
	}

	return 0;
}

// osp: 0x00033c3f
static int Callvote_DoWarmup(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if (strlen(arg) == 0)
		{
			OSP_PrintValue(ent, cmd, g_doWarmup.string);
			return -1;
		}

		if (atoi(arg) < 0 || atoi(arg) > 2)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Invalid warmup type.\n\"");
			return -1;
		}

		Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "%s", arg);
		Com_sprintf(arg, 256, "^3%s", atoi(arg) == 2 ? "READY-UP" : (atoi(arg) == 1 ? "COUNTDOWN" : "OFF"));
	}
	else
	{
		trap_SendServerCommand(-1, va("Warmup style set to : %s", atoi(osp.callvoteArgs[0]) == 2 ? "READY-UP" : (atoi(osp.callvoteArgs[0]) == 1 ? "COUNTDOWN" : "OFF")));
		trap_SendConsoleCommand(EXEC_APPEND, va("set g_dowarmup %d;map_restart\n", atoi(osp.callvoteArgs[0])));
	}

	return 0;
}

// osp: 0x00033d4c
static int Callvote_Warmup(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if (strlen(arg) == 0)
		{
			OSP_PrintValue(ent, cmd, g_warmup.string);
			return -1;
		}

		if (atoi(arg) <= 0)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Invalid warmup duration.\n\"");
			return -1;
		}

		Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "%s", arg);
		Com_sprintf(arg, 256, "^3%s", arg);
	}
	else
	{
		trap_SendServerCommand(-1, va("Warmup duration set to : %s seconds", osp.callvoteArgs[0]));
		trap_SendConsoleCommand(EXEC_APPEND, va("set g_warmup %s\n", osp.callvoteArgs[0]));
	}

	return 0;
}

// osp: 0x00033dfe
static int Callvote_Scorelimit(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			OSP_PrintValue(ent, cmd, g_scorelimit.string);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			OSP_PrintValue(ent, cmd, g_scorelimit.string);
			return -1;
		}

		if (strlen(arg) == 0)
		{
			OSP_PrintValue(ent, cmd, g_scorelimit.string);
			return -1;
		}

		if (atoi(arg) < 0)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Sorry, can't specify a scorelimit < 0!\n\"");
			return -1;
		}

		Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "%s", arg);
	}
	else
	{
		OSP_ChangeSettingVotestring("Scorelimit");
	}

	return 0;
}

// osp: 0x00033e9a
static int Callvote_Gametype(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		int newGametype;
		int currentGametype;

		newGametype = BG_FindGametype(arg);
		currentGametype = BG_FindGametype(g_gametype.string);

		if (newGametype < 0 || newGametype > MAX_GAMETYPES)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Invalid gametype.\n\"");
			return -1;
		}

		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			OSP_PrintValue(ent, cmd, va("%s (%s)", bg_gametypeData[currentGametype].name, g_gametype.string));
			return -1;
		}

		if (strlen(arg) == 0)
		{
			OSP_PrintValue(ent, cmd, bg_gametypeData[g_gametype.integer].displayName);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			int gametype = BG_FindGametype(g_gametype.string);
			OSP_PrintValue(ent, cmd, va("%s (%s)", bg_gametypeData[gametype].name, g_gametype.string));
			return -1;
		}

		Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "%s", arg);
		Com_sprintf(arg, 256, "%s", bg_gametypeData[newGametype].name);
	}
	else
	{
		OSP_ChangeSetting("Gametype", "g_gametype");
		trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
	}

	return 0;
}

// osp: 0x00033f99
static int Callvote_Kick(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		int clientNum;

		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if ((clientNum = OSP_ClientForString(ent, arg)) == -1)
		{
			return -1;
		}

		if (OSP_IsClientAdmin(clientNum, qfalse))
		{
			trap_SendServerCommand(ent - g_entities, "print \"Can't vote to kick admins!\n\"");
			return -1;
		}

		Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "%d", clientNum);

		if (g_teamkillDamageMax.integer)
			Com_sprintf(arg, 256, "^7%s ^3[^7tk:^1%i ^7afk:^5%i sec^3]",
						level.clients[clientNum].pers.netname,
						g_teamkillDamageMax.integer ? level.clients[clientNum].sess.teamkillDamage * 100 / g_teamkillDamageMax.integer : 0,
						(int)((level.time - osp.clients[clientNum].afkTime) / 1000.0));
		else
			Com_sprintf(arg, 256, "^7%s ^3[^7afk:^5%i sec^3]",
						level.clients[clientNum].pers.netname,
						(int)((level.time - osp.clients[clientNum].afkTime) / 1000.0));
	}
	else
	{
		if (g_voteKickBanTime.integer)
			trap_SendConsoleCommand(EXEC_APPEND, va("banclient %s %d voted off server\n", osp.callvoteArgs[0], g_voteKickBanTime.integer));
		else
			trap_SendConsoleCommand(EXEC_APPEND, va("clientkick %s\n", osp.callvoteArgs[0]));
	}

	return 0;
}

// osp: 0x000340b4
static int Callvote_Map(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		char serverinfo[MAX_INFO_STRING];

		trap_GetServerinfo(serverinfo, sizeof(serverinfo));

		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			OSP_PrintValue(ent, cmd, Info_ValueForKey(serverinfo, "mapname"));
			return -1;
		}

		if (!G_DoesMapExist(arg))
		{
			trap_SendServerCommand(ent - g_entities, "print \"Unknown mapname.\n\"");
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			OSP_PrintValue(ent, cmd, Info_ValueForKey(serverinfo, "mapname"));
			return -1;
		}

		Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "%s", arg);

		switch (trap_Argc())
		{
		default:
		case 3:
			memset(osp.callvoteArgs[1], 0, sizeof(osp.callvoteArgs[1]));
			memset(osp.callvoteArgs[2], 0, sizeof(osp.callvoteArgs[2]));
			break;
		case 4:
		{
			char arg3[1024];
			trap_Argv(3, arg3, sizeof(arg3));
			if (strlen(arg3) != 0)
			{
				Com_sprintf(osp.callvoteArgs[1], sizeof(osp.callvoteArgs[1]), "%s", arg3);
			}
			memset(osp.callvoteArgs[2], 0, sizeof(osp.callvoteArgs[2]));
			Com_sprintf(arg, 256, "%s, Alternate #%s", arg, arg3);
			break;
		}
		case 5:
		{
			char arg3[1024];
			char arg4[1024];

			trap_Argv(3, arg3, sizeof(arg3));
			if (strlen(arg3) != 0)
			{
				Com_sprintf(osp.callvoteArgs[1], sizeof(osp.callvoteArgs[1]), "%s", arg3);
			}

			trap_Argv(4, arg4, sizeof(arg4));
			if (strlen(arg4) != 0)
			{
				Com_sprintf(osp.callvoteArgs[2], sizeof(osp.callvoteArgs[2]), "%s", arg4);
			}
			Com_sprintf(arg, 256, "%s, Alternate #%s, Gametype %s", arg, arg3, arg4);
			break;
		}
		}
	}
	else
	{
		if (strlen(osp.callvoteArgs[1]) != 0 && strlen(osp.callvoteArgs[2]) != 0)
		{
			trap_SendConsoleCommand(EXEC_APPEND, va("set g_alternateMap %s;set g_gametype %s;map %s\n", osp.callvoteArgs[1], osp.callvoteArgs[2], osp.callvoteArgs[0]));
		}
		else if (strlen(osp.callvoteArgs[1]) != 0)
		{
			trap_SendConsoleCommand(EXEC_APPEND, va("set g_alternateMap %s;map %s\n", osp.callvoteArgs[1], osp.callvoteArgs[0]));
		}
		else
		{
			trap_SendConsoleCommand(EXEC_APPEND, va("map %s\n", osp.callvoteArgs[0]));
		}
	}

	return 0;
}

// osp: 0x00034259
static int Callvote_RMGMap(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		char arg3[1024];
		char arg4[1024];

		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		trap_Argv(3, arg3, sizeof(arg3));
		trap_Argv(4, arg4, sizeof(arg4));

		Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "1 %s 2 %s 3 %s 4 \"%s\" 0", arg, arg3, arg4, ConcatArgs(5));
		Com_sprintf(arg, 256, "\0");
	}
	else
	{
		trap_SendConsoleCommand(EXEC_APPEND, va("rmgmap %s\n", osp.callvoteArgs[0]));
	}

	return 0;
}

// osp: 0x000342de
static int Callvote_MapRestart(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}
	}
	else
	{
		trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
	}

	return 0;
}

// osp: 0x00034323
static int Callvote_RoundRestart(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}
	}
	else
	{
		G_ResetGametype(qfalse);
	}

	return 0;
}

// osp: 0x00034366
static int Callvote_Mapcycle(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if (!g_mapcycle.string[0] || Q_stricmp(g_mapcycle.string, "none") == 0)
		{
			trap_SendServerCommand(ent - g_entities, "print \"there is no map cycle currently set up.\n\"");
			return -1;
		}
	}
	else
	{
		trap_SendConsoleCommand(EXEC_APPEND, "mapcycle\n");
	}

	return 0;
}

// osp: 0x000343cc
static int Callvote_Referee(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		int clientNum;

		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (trap_Argc() == 2)
		{
			clientNum = ent - g_entities;
		}
		else if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}
		else if ((clientNum = OSP_ClientForString(ent, arg)) == -1)
		{
			return -1;
		}

		if (OSP_IsClientAdmin(clientNum, qfalse))
		{
			trap_SendServerCommand(ent - g_entities, va("print \"%s is already an admin!\n\"", level.clients[clientNum].pers.netname));
			return -1;
		}

		if (OSP_IsClientReferee(clientNum))
		{
			trap_SendServerCommand(ent - g_entities, va("print \"%s is already a referee!\n\"", level.clients[clientNum].pers.netname));
			return -1;
		}

		Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "%d", clientNum);
		Com_sprintf(arg, 256, "%s", level.clients[clientNum].pers.netname);
	}
	else
	{
		int clientNum;

		clientNum = atoi(osp.callvoteArgs[0]);

		if (level.clients[clientNum].pers.connected != CON_CONNECTED)
		{
			trap_SendServerCommand(-1, "print \"Player left before becoming referee\n\"");
		}
		else
		{
			osp.clients[clientNum].referee = 2;
			ClientUserinfoChanged(clientNum);
			trap_SendServerCommand(clientNum, "print \"Type: ^3ref ?^7 for a list of referee commands.\n\"");
			trap_SendServerCommand(-1, va("cp \"%s^7 is now a referee\n\"", level.clients[clientNum].pers.netname));
		}
	}

	return 0;
}

// osp: 0x000344fe
static int Callvote_ShuffleTeams(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}
	}
	else
	{
		OSP_ShuffleTeams();
		trap_SendConsoleCommand(EXEC_APPEND, "restart 1\n");
	}

	return 0;
}

// osp: 0x00034546
static int Callvote_BalanceTeams(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}
	}
	else
	{
		OSP_BalanceTeams();
	}

	return 0;
}

// osp: 0x00034587
static int Callvote_StartMatch(gentity_t *e, unsigned int index, char *cmd, char *arargg2, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if (osp.matchPhase != 1)
		{
			trap_SendServerCommand(ent - g_entities, "print \"^3Must be in match setup phase!\n\"");
			return -1;
		}

		if (level.numPlayingClients < match_minplayers.integer)
		{
			trap_SendServerCommand(ent - g_entities, "print \"^3Not enough players to start the match!\n\"");
			return -1;
		}
	}
	else
	{
		OSP_Ref_Ready(NULL, 0, 0);
	}

	return 0;
}

// osp: 0x000345f7
static int Callvote_SwapTeams(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}
	}
	else
	{
		OSP_SwapTeams();
		trap_SendConsoleCommand(EXEC_APPEND, "restart 1\n");
	}

	return 0;
}

// osp: 0x0003463f
static int Callvote_FriendlyFire(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		return OSP_Callvote_Setting(ent, cmd, arg, adminAction, g_friendlyFire.integer ? qtrue : qfalse, !OSP_VoteDisabled(cmd) ? qtrue : qfalse, index);
	}
	else
	{
		OSP_ChangeSettingBool("Hurt Teammates", "g_friendlyFire");
	}

	return 0;
}

// osp: 0x00034690
static int Callvote_RealisticMode(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		return OSP_Callvote_Setting(ent, cmd, arg, adminAction, g_friendlyFire.integer ? qtrue : qfalse, !OSP_VoteDisabled(cmd) ? qtrue : qfalse, index);
	}
	else
	{
		OSP_ChangeSettingBool("Realistic Mode", "osp_realisticMode");
	}

	return 0;
}

// osp: 0x000346e1
static int Callvote_Timelimit(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			OSP_PrintValue(ent, cmd, g_timelimit.string);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			OSP_PrintValue(ent, cmd, g_timelimit.string);
			return -1;
		}

		if (strlen(arg) == 0)
		{
			OSP_PrintValue(ent, cmd, g_timelimit.string);
			return -1;
		}

		if (atoi(arg) < 0)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Sorry, can't specify a timelimit < 0!\n\"");
			return -1;
		}

		Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "%s", arg);
	}
	else
	{
		OSP_ChangeSettingVotestring("Timelimit");
	}

	return 0;
}

// osp: 0x0003477d
static int Callvote_TimeExtension(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}

		if (g_timelimit.integer == 0)
		{
			trap_SendServerCommand(ent - g_entities, va("print \"There is no timelimit to extend.\n\""));
			return -1;
		}

		if (g_timeextension.integer == 0)
		{
			trap_SendServerCommand(ent - g_entities, va("print \"This server does not allow time extensions.\n\""));
			return -1;
		}

		if (atoi(arg) < 0)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Sorry, can't reduce timelimit!\n\"");
			return -1;
		}

		Com_sprintf(arg, 256, "%d minutes", g_timeextension.integer);
	}
	else
	{
		trap_SendConsoleCommand(EXEC_APPEND, va("extendtime %d", g_timeextension.integer));
	}

	return 0;
}

// osp: 0x0003482c
static int Callvote_Unreferee(gentity_t *e, unsigned int index, char *cmd, char *arg, int adminAction)
{
	gentity_t *ent = e;

	if (cmd)
	{
		int clientNum;

		if (OSP_VoteDisabled(cmd) && adminAction < 2)
		{
			OSP_VoteDisabledError(ent, cmd);
			return -1;
		}

		if (trap_Argc() == 2)
		{
			clientNum = ent - g_entities;
		}
		else if (OSP_Callvote_Usage(ent, adminAction, index))
		{
			return -1;
		}
		else if ((clientNum = OSP_ClientForString(ent, arg)) == -1)
		{
			return -1;
		}

		if (!OSP_IsClientReferee(clientNum))
		{
			trap_SendServerCommand(ent - g_entities, va("print \"%s isn't a referee!\n\"", level.clients[clientNum].pers.netname));
			return -1;
		}

		if (osp.clients[clientNum].referee != 2)
		{
			trap_SendServerCommand(ent - g_entities, va("print \"%s wasn't voted in! Cannot unreferee him.\n\"", level.clients[clientNum].pers.netname));
			return -1;
		}

		Com_sprintf(osp.callvoteArgs[0], sizeof(osp.callvoteArgs[0]), "%d", clientNum);
		Com_sprintf(arg, 256, "%s", level.clients[clientNum].pers.netname);
	}
	else
	{
		int clientNum;
		gclient_t *cl;

		clientNum = atoi(osp.callvoteArgs[0]);
		cl = level.clients + clientNum;

		osp.clients[clientNum].referee = 0;
		ClientUserinfoChanged(clientNum);
		trap_SendServerCommand(-1, va("cp \"%s^7\nis no longer a referee\n\"", cl->pers.netname));
	}

	return 0;
}

// osp: 0x00034944
qboolean OSP_Callvote(gentity_t *e, unsigned int index, int adminAction)
{
	int i;
	gentity_t *ent;
	char arg1[256];
	char arg2[256];

	ent = e;

	if (!adminAction)
	{
		if (level.voteTime)
		{
			trap_SendServerCommand(ent - g_entities, "print \"A vote is already in progress.\n\"");
			return qfalse;
		}

		if (!g_allowVote.integer)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Voting not enabled on this server.\n\"");
			return qfalse;
		}

		if (level.intermissiontime || level.intermissionQueued)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Voting not allowed during intermission.\n\"");
			return qfalse;
		}

		if (vote_limit.integer && ent->client->pers.voteCount >= vote_limit.integer)
		{
			trap_SendServerCommand(ent - g_entities, "print \"You have called the maximum number of votes.\n\"");
			return qfalse;
		}

		if (ent->client->voteDelayTime > level.time)
		{
			trap_SendServerCommand(ent - g_entities, va("print \"You are not allowed to vote within %d minute of a failed vote.\n\"", g_failedVoteDelay.integer));
			return qfalse;
		}

		if (ent->client->sess.team == TEAM_SPECTATOR)
		{
			trap_SendServerCommand(ent - g_entities, "print \"Not allowed to call a vote as a spectator.\n\"");
			return qfalse;
		}
	}

	trap_Argv(1, arg1, sizeof(arg1));
	trap_Argv(2, arg2, sizeof(arg2));

	if (strchr(arg1, ';') || strchr(arg2, ';'))
	{
		char *arg0;
		arg0 = (!adminAction) ? "vote" : "command";

		if (ent)
		{
			trap_SendServerCommand(ent - g_entities, va("print \"Invalid %s string.\n\"", arg0));
		}
		else
		{
			Com_Printf("Invalid %s string.\n", arg0);
		}

		return qfalse;
	}

	if (trap_Argc() > 1 && (i = OSP_Callvote_Dispatch(ent, arg1, arg2, adminAction)) != -2)
	{
		if (i != 0)
		{
			if (i == -2)
			{
				return qfalse;
			}
			else
			{
				return qtrue;
			}
		}
	}
	else
	{
		if (!adminAction)
		{
			trap_SendServerCommand(ent - g_entities, va("print \"\n^3>>> Unknown vote command: ^7%s %s\n\"", arg1, arg2));
			OSP_Callvote_ListCmds(ent, qtrue);
		}
		return qfalse;
	}

	if (osp.callvoteFunc == Callvote_Kick ||
		osp.callvoteFunc == Callvote_Gametype ||
		osp.callvoteFunc == Callvote_Map ||
		osp.callvoteFunc == Callvote_Timelimit ||
		osp.callvoteFunc == Callvote_Scorelimit ||
		osp.callvoteFunc == Callvote_TimeExtension ||
		osp.callvoteFunc == Callvote_FriendlyFire ||
		osp.callvoteFunc == Callvote_DoWarmup ||
		osp.callvoteFunc == Callvote_Referee ||
		osp.callvoteFunc == Callvote_Unreferee ||
		osp.callvoteFunc == Callvote_Comp ||
		osp.callvoteFunc == Callvote_Pub ||
		osp.callvoteFunc == Callvote_WeaponOnly)
	{
		Com_sprintf(level.voteString, sizeof(level.voteString), "%s %s", arg1, arg2);
	}
	else
	{
		Com_sprintf(level.voteString, sizeof(level.voteString), "%s", arg1);
	}

	if (adminAction)
	{
		level.voteYes = level.numVotingClients + 10;

		if (osp.callvoteFunc != Callvote_Kick)
		{
			if (adminAction == 1)
				trap_SendServerCommand(-1, va("cp \"^3** Referee Action **\n%s\n\"", level.voteString));
			else
				trap_SendServerCommand(-1, va("cp \"^5** Admin Action **\n%s\n\"", level.voteString));
		}

		OSP_GlobalSound("sound/misc/events/tut_lift02.mp3");

		level.voteTime = level.time;
		level.voteNo = 0;
	}
	else
	{
		level.voteYes = 1;
		ent->client->pers.voteCount++;
		trap_SendServerCommand(-1, va("print \"%s called a vote.\n\"", ent->client->pers.netname));
		OSP_GlobalSound("sound/misc/menus/quit_request.wav");

		level.voteTime = level.time;
		level.voteNo = 0;
		trap_SetConfigstring(CS_VOTE_TIME, va("%i,%i", level.voteTime, 1000 * g_voteDuration.integer));
		trap_SetConfigstring(CS_VOTE_STRING, level.voteString);
		trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteYes));
		trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteNo));

		{
			int votePercentage;
			votePercentage = (osp.callvoteFunc == Callvote_StartMatch) ? 75 : vote_percent.integer;

			if (votePercentage > 99)
			{
				votePercentage = 99;
			}

			if (votePercentage < 1)
			{
				votePercentage = 1;
			}

			trap_SetConfigstring(CS_VOTE_NEEDED, va("%i", ((votePercentage * level.numVotingClients) / 100) + 1));
		}
	}

	for (i = 0; i < level.numConnectedClients; i++)
		level.clients[level.sortedClients[i]].ps.eFlags &= ~EF_VOTED;

	ent->client->ps.eFlags |= EF_VOTED;

	return qtrue;
}
