// osp_svcmds.c
//
#include "g_local.h"

// osp: 0x000311ec
static qboolean OSP_AddToList(int clientNum)
{
	int i;

	if (OSP_LoadAdmin(clientNum))
	{
		Com_Printf("already in list\n");
		return qfalse;
	}

	for (i = 0; i < MAX_ADMINS; i++)
	{
		if (strlen(osp.adminList[i]) < 32)
		{
			memset(osp.adminList[i], 0, 64);
			strncpy(osp.adminList[i], va("%s //%s", OSP_GetGuid(clientNum), OSP_GetClientName(clientNum, 29)), 64);
			osp.adminList[i][63] = '\0';
			osp.numAdmins++;
			trap_SendServerCommand(-1, "print \"^6An admin was just added.\n\"");
			return qtrue;
		}
	}

	Com_Printf("list is full\n");
	return qfalse;
}

// osp: 0x0003127d
static qboolean OSP_RemoveFromList(int clientNum)
{
	int i;
	char *guid;

	if (!OSP_LoadAdmin(clientNum))
	{
		Com_Printf("client not in list\n");
		return qfalse;
	}

	guid = OSP_GetGuid(clientNum);

	for (i = 0; i < MAX_ADMINS; i++)
	{
		if (Q_strncmp(osp.adminList[i], guid, 32) == 0)
		{
			osp.adminList[i][0] = '\0';
			osp.numAdmins--;
			return qtrue;
		}
	}

	Com_Printf("client not in list(2)\n");
	return qfalse;
}

// osp: 0x000312de
static void OSP_RefreshAdmins(void)
{
	int i;

	for (i = 0; i < level.maxclients; i++)
	{
		qboolean isAdmin;

		if (level.clients[i].pers.connected != CON_DISCONNECTED)
		{
			isAdmin = OSP_LoadAdmin(i);

			if (osp.clients[i].admin != isAdmin)
			{
				osp.clients[i].admin = isAdmin;
				ClientUserinfoChanged(i);
			}
		}
	}
}

// osp: 0x00031327
static void OSP_SaveAdmins(void)
{
	int i;
	fileHandle_t f;
	int len;

	len = trap_FS_FOpenFile(sv_adminfile.string, &f, FS_WRITE);

	for (i = 0; i < MAX_ADMINS; i++)
	{
		if (osp.adminList[i][0])
		{
			trap_FS_Write(va("%s\n", osp.adminList[i]), strlen(va("%s\n", osp.adminList[i])), f);
		}
	}

	trap_FS_FCloseFile(f);
}

// osp: 0x00031382
void OSP_LoadAdmins(void)
{
	char buf[MAX_ADMINS * MAX_GUID];
	fileHandle_t f;
	int len;
	int numAdmins;
	int line;
	int pos;

	f = 0;
	len = 0;
	numAdmins = 0;
	line = 0;

	len = trap_FS_FOpenFile(sv_adminfile.string, &f, FS_READ);

	if (!f)
	{
		return;
	}

	if (len > (MAX_ADMINS * MAX_GUID))
	{
		len = MAX_ADMINS * MAX_GUID;
	}

	memset(buf, 0, sizeof(buf));
	trap_FS_Read(buf, len, f);

	for (pos = 0; pos < len; pos++, line++)
	{
		if (buf[pos] == '\n' || buf[pos] == '\r')
		{
			numAdmins++;
			line = -1;
			continue;
		}

		if (line > (MAX_GUID - 1))
		{
			continue;
		}

		osp.adminList[numAdmins][line] = buf[pos];
	}

	osp.numAdmins = numAdmins;
	trap_FS_FCloseFile(f);
}

// osp: 0x00031415
static void OSP_AddAdmin(void)
{
	int clientNum;
	gentity_t *ent;
	char arg[512];

	if (trap_Argc() < 2)
	{
		Com_Printf("Usage:  addadmin <client_id>\n");
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	clientNum = atoi(arg);

	if (clientNum < 0 || clientNum > level.maxclients)
	{
		Com_Printf("Bad client id %d\n", clientNum);
		return;
	}

	ent = g_entities + clientNum;

	if (ent && ent->client && ent->client->ospClient)
	{
		if (OSP_AddToList(clientNum))
		{
			OSP_SaveAdmins();
			G_LogPrintf("added %s to admins list\n", ent->client->pers.netname);
		}
		else
		{
			Com_Printf("%s could not be added to list\n", ent->client->pers.netname);
		}
	}
	else
	{
		Com_Printf("Bad client\n");
	}
}

// osp: 0x000314a2
static void OSP_RemoveAdmin(void)
{
	int clientNum;
	gentity_t *ent;
	char arg[512];

	if (trap_Argc() < 2)
	{
		Com_Printf("Usage:  removeadmin <client_id>\n");
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	clientNum = atoi(arg);

	if (clientNum < 0 || clientNum > level.maxclients)
	{
		Com_Printf("Bad client id %d\n", clientNum);
		return;
	}

	ent = g_entities + clientNum;

	if (ent && ent->client)
	{
		if (OSP_RemoveFromList(clientNum))
		{
			OSP_SaveAdmins();
			G_LogPrintf("removed %s from admins list\n", ent->client->pers.netname);
		}
		else
		{
			Com_Printf("%s could not be removed from list\n", ent->client->pers.netname);
		}
	}
	else
	{
		Com_Printf("Bad client\n");
	}
}

// osp: 0x00031523
void OSP_ShuffleTeams(void)
{
	gclient_t *cl;
	int i;
	team_t team;
	int alternateSort;

	if (level.numConnectedClients <= 0)
	{
		return;
	}

	alternateSort = trap_Cvar_VariableIntegerValue("g_alternateSort");
	trap_Cvar_Set("g_alternateSort", va("%d", 1));
	CalculateRanks();
	trap_Cvar_Set("g_alternateSort", va("%d", alternateSort));

	team = OtherTeam(level.clients[level.sortedClients[0]].sess.team);

	for (i = 0; i < level.numConnectedClients; i++)
	{
		cl = &level.clients[level.sortedClients[i]];

		if (cl && cl->ospStats)
		{
			if (cl->sess.team == TEAM_RED || cl->sess.team == TEAM_BLUE)
			{
				cl->sess.team = team;

				if (!level.intermissiontime)
				{
					ClientUserinfoChanged(level.sortedClients[i]);
					ClientBegin(level.sortedClients[i]);
				}

				cl->switchTeamTime = level.time + 60000;

				if (i != 1)
				{
					team = OtherTeam(team);
				}
			}
		}
	}
}

// osp: 0x000315e3
void OSP_BalanceTeams(void)
{
	gentity_t *ent;
	team_t *team;
	int j;
	int lastEnterTime;
	gentity_t *candidate;
	team_t biggerTeam;
	int i;
	int diff;
	int count[TEAM_NUM_TEAMS];
	int alternateSort;

	biggerTeam = 0;
	diff = 0;

	if (level.numConnectedClients <= 0)
	{
		return;
	}

	alternateSort = trap_Cvar_VariableIntegerValue("g_alternateSort");
	trap_Cvar_Set("g_alternateSort", va("%d", 0));
	CalculateRanks();
	trap_Cvar_Set("g_alternateSort", va("%d", alternateSort));

	// choose team
	count[TEAM_BLUE] = TeamCount(-1, TEAM_BLUE, NULL);
	count[TEAM_RED] = TeamCount(-1, TEAM_RED, NULL);

	if (count[TEAM_BLUE] > count[TEAM_RED])
	{
		biggerTeam = TEAM_BLUE;
		diff = count[TEAM_BLUE] - count[TEAM_RED];
	}
	else if (count[TEAM_BLUE] < count[TEAM_RED])
	{
		biggerTeam = TEAM_RED;
		diff = count[TEAM_RED] - count[TEAM_BLUE];
	}

	// difference big enough?
	if (diff < 2)
	{
		return;
	}

	diff /= 2;

	// move 'em
	for (i = 0; i < diff; i++)
	{
		lastEnterTime = 0;

		// pick some one who joined last
		for (j = 0; j < level.numConnectedClients; j++)
		{
			ent = g_entities + level.sortedClients[j];
			team = &ent->client->sess.team;

			if (ent->client->ospClient->admin ||
				ent->s.gametypeitems > 0 ||
				*team != biggerTeam ||
				!(*team == TEAM_RED || *team == TEAM_BLUE))
			{
				continue;
			}

			if (ent->client->pers.enterTime > lastEnterTime)
			{
				lastEnterTime = ent->client->pers.enterTime;
				candidate = ent;
			}
		}

		if (biggerTeam == TEAM_RED)
		{
			candidate->client->sess.team = TEAM_BLUE;
		}
		else
		{
			candidate->client->sess.team = TEAM_RED;
		}

		if (candidate->r.svFlags & SVF_BOT)
		{
			char userinfo[1024];

			trap_GetUserinfo(candidate->s.number, userinfo, sizeof(userinfo));
			Info_SetValueForKey(userinfo, "team", candidate->client->sess.team == TEAM_RED ? "red" : "blue");
			trap_SetUserinfo(candidate->s.number, userinfo);
		}

		candidate->client->pers.identity = NULL;
		ClientUserinfoChanged(candidate->s.number);
		ClientBegin(candidate->s.number);
	}

	CalculateRanks();
}

// osp: 0x00031752
void OSP_SwapTeams(void)
{
	gentity_t *ent;
	int i;

	OSP_SwapTeamSettings();

	for (i = 0; i < level.maxclients; i++)
	{
		ent = g_entities + i;

		if (ent && ent->client && ent->client->ospStats)
		{
			if ((ent->client->ospClient->specinvites & TEAM_RED) &&
				!(ent->client->ospClient->specinvites & TEAM_BLUE))
			{
				ent->client->ospClient->specinvites = TEAM_BLUE;
			}
			else if ((ent->client->ospClient->specinvites & TEAM_BLUE) &&
					 !(ent->client->ospClient->specinvites & TEAM_RED))
			{
				ent->client->ospClient->specinvites = TEAM_RED;
			}

			if (ent->client->sess.team == TEAM_RED || ent->client->sess.team == TEAM_BLUE)
			{
				if (ent->client->sess.team == TEAM_RED)
				{
					ent->client->sess.team = TEAM_BLUE;
				}
				else if (ent->client->sess.team == TEAM_BLUE)
				{
					ent->client->sess.team = TEAM_RED;
				}

				if (!level.intermissiontime)
				{
					ClientUserinfoChanged(i);
					ClientBegin(i);
				}

				ent->client->switchTeamTime = level.time + 60000;
			}
		}
	}
}

// osp: 0x00031838
static void OSP_Restart(void)
{
	int delay;
	char arg[512];

	if (trap_Argc() < 2)
	{
		trap_SendConsoleCommand(EXEC_APPEND, "map_restart\n");
		return;
	}

	trap_Argv(1, arg, sizeof(arg));
	delay = atoi(arg);

	if (delay < 0 || delay > 60)
	{
		Com_Printf("Invalid delay (<0 or >60)\n");
		return;
	}

	osp.x6479a8 = level.time + (1000 * delay);
}

// osp: 0x00031877
qboolean OSP_ConsoleCommand(const char *cmd)
{
	if (Q_stricmp(cmd, "shuffleteams") == 0)
	{
		trap_SendServerCommand(-1, "print \"Shuffling teams!\n\"");
		OSP_ShuffleTeams();
		return qtrue;
	}

	if (Q_stricmp(cmd, "balanceteams") == 0)
	{
		trap_SendServerCommand(-1, "print \"Balancing teams!\n\"");
		OSP_BalanceTeams();
		return qtrue;
	}

	if (Q_stricmp(cmd, "swapteams") == 0)
	{
		trap_SendServerCommand(-1, "print \"Swapping teams!\n\"");
		OSP_SwapTeams();
		return qtrue;
	}

	if (Q_stricmp(cmd, "addadmin") == 0)
	{
		OSP_AddAdmin();
		OSP_RefreshAdmins();
		return qtrue;
	}

	if (Q_stricmp(cmd, "removeadmin") == 0)
	{
		OSP_RemoveAdmin();
		OSP_RefreshAdmins();
		return qtrue;
	}

	if (Q_stricmp(cmd, "loadadmins") == 0)
	{
		OSP_LoadAdmins();
		OSP_RefreshAdmins();
		return qtrue;
	}

	if (Q_stricmp(cmd, "saveadmins") == 0)
	{
		OSP_SaveAdmins();
		return qtrue;
	}

	if (Q_stricmp(cmd, "restart") == 0)
	{
		OSP_Restart();
		return qtrue;
	}

	if (Q_stricmp(cmd, "comp") == 0)
	{
		if (trap_Argc() < 2)
		{
			if (OSP_LoadSettings(qtrue, ""))
			{
				trap_SendServerCommand(-1, "cp \"Competition Settings (default) Loaded!\n\"");
			}
			else
			{
				trap_SendServerCommand(-1, "cp \"Competition Settings ^1NOT^7 Loaded!\n\"");
			}
		}
		else
		{
			char arg[128];
			memset(arg, 0, sizeof(arg));
			trap_Argv(1, arg, sizeof(arg) - 1);

			if (OSP_LoadSettings(qtrue, arg))
			{
				trap_SendServerCommand(-1, va("cp \"Competition Settings (%s) Loaded!\n\"", arg));
			}
			else
			{
				trap_SendServerCommand(-1, "cp \"Competition Settings ^1NOT^7 Loaded!\n\"");
			}
		}

		return qtrue;
	}

	if (Q_stricmp(cmd, "pub") == 0)
	{
		if (trap_Argc() < 2)
		{
			if (OSP_LoadSettings(qfalse, ""))
			{
				trap_SendServerCommand(-1, "cp \"Public Settings (default) Loaded!\n\"");
			}
			else
			{
				trap_SendServerCommand(-1, "cp \"Public Settings ^1NOT^7 Loaded!\n\"");
			}
		}
		else
		{
			char arg[128];
			memset(arg, 0, sizeof(arg));
			trap_Argv(1, arg, sizeof(arg) - 1);

			if (OSP_LoadSettings(qfalse, arg))
			{
				trap_SendServerCommand(-1, va("cp \"Public Settings (%s) Loaded!\n\"", arg));
			}
			else
			{
				trap_SendServerCommand(-1, "cp \"Public Settings ^1NOT^7 Loaded!\n\"");
			}
		}

		return qtrue;
	}

	if (Q_stricmp(cmd, "smusic") == 0)
	{
		char arg[128];
		memset(arg, 0, sizeof(arg));
		trap_Argv(1, arg, sizeof(arg) - 1);
		trap_SetConfigstring(CS_MUSIC, arg);
		return qtrue;
	}

	if (Q_stricmp(cmd, "debugtrigger") == 0)
	{
		gentity_t *target;

		target = NULL;

		while (NULL != (target = G_Find(target, FOFS(classname), "gametype_trigger")))
		{
			int i;
			vec3_t corners[8];

			VectorCopy(target->r.maxs, corners[0]);

			VectorCopy(target->r.maxs, corners[1]);
			corners[1][2] = target->r.mins[2];

			VectorCopy(target->r.mins, corners[2]);

			VectorCopy(target->r.mins, corners[3]);
			corners[3][2] = target->r.maxs[2];

			VectorCopy(corners[0], corners[4]);
			corners[4][0] = corners[2][0];

			VectorCopy(corners[1], corners[5]);
			corners[5][0] = corners[3][0];

			VectorCopy(corners[2], corners[6]);
			corners[6][0] = corners[0][0];

			VectorCopy(corners[3], corners[7]);
			corners[7][0] = corners[1][0];

			for (i = 0; i < 8; i += 2)
			{
				gentity_t *tent;
				tent = G_TempEntity(corners[i], EV_BOTWAYPOINT);
				tent->r.svFlags = SVF_BROADCAST;
				VectorCopy(corners[i + 1], tent->s.angles);
				tent->s.time = 120000;

				if (i == 0)
					tent->s.time2 = 0xff;
				else if (i == 2)
					tent->s.time2 = 0xff00;
				else if (i == 4)
					tent->s.time2 = 0xffff00;
				else if (i == 6)
					tent->s.time2 = 0xff00ff;
			}
		}

		return qtrue;
	}

	return qfalse;
}
