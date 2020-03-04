// osp_admin.c
//
#include "g_local.h"

char *ConcatArgs(int start);
void G_ApplyKnockback(gentity_t *targ, vec3_t newDir, float knockback);

static void OSP_Adm_Help(gentity_t *ent, int idx, int rfu2);
static qboolean OSP_Adm_Usage(gentity_t *e, char *cmd, unsigned int idx);
static void OSP_Adm_ExtraCommand(gentity_t *ent);
static void OSP_Adm_Kick(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Ban(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Cancelvote(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Chat(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Talk(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Strike(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Bbq(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Phone(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Strip(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Jail(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Mute(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Cvarcheck(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Suspend(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Pause(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Showex(gentity_t *e, int idx, int rfu2);
static void OSP_Adm_Callvote(gentity_t *e, int idx, int rfu2);

typedef struct
{
	char *name;
	int rfu1;
	int rfu2;
	void (*func)(gentity_t *, int, int);
	char *helptext;
} ospAdminCmd_t;

static ospAdminCmd_t ospAdminCmds[] = {
	{"?", 1, 1, OSP_Adm_Help, ":^7 Gives a list of admin commands"},
	{"commands", 1, 1, OSP_Adm_Help, ":^7 Gives a list of admin commands"},
	{"kick", 1, 0, OSP_Adm_Kick, " <player_ID> <reason>:^7 Kick player with corresponding id giving specified reason"},
	{"ban", 1, 0, OSP_Adm_Ban, " <player_ID> <minutes>:^7 Ban player with corresponding id for specified time, or indefinitely"},
	{"cancelvote", 1, 0, OSP_Adm_Cancelvote, ":^7 Cancel current vote in progress"},
	{"chat", 1, 0, OSP_Adm_Chat, " <msg>:^7 Chat with other admins, only they will see the message"},
	{"talk", 1, 0, OSP_Adm_Talk, " <msg>:^7 Chat with everyone on the server, anytime"},
	{"strike", 1, 0, OSP_Adm_Strike, " <player_ID>:^7 Punish a player by killing him with a bolt of lightning!"},
	{"bbq", 1, 0, OSP_Adm_Bbq, " <player_ID>:^7 Punish a player by burning him with hellish flames!"},
	{"phone", 1, 0, OSP_Adm_Phone, " <player_ID>:^7 Punish a player by hitting him on the head with a telephone receiver!"},
	{"strip", 1, 0, OSP_Adm_Strip, " <player_ID>:^7 Punish a player by removing all his weapons!"},
	{"jail", 1, 0, OSP_Adm_Jail, " <player_ID> (seconds):^7 Jail a player for a certain amount of time!"},
	{"mute", 1, 0, OSP_Adm_Mute, " <player_ID>:^7 Mute a player so he can't talk anymore"},
	{"suspend", 1, 0, OSP_Adm_Suspend, ":^7 Suspend your admin privileges, useful when an admin play a match"},
	{"pause", 0, 1, OSP_Adm_Pause, ":^7 Pause the game"},
	{"unpause", 0, 0, OSP_Adm_Pause, ":^7 Unpause the game"},
	{"cvarcheck", 0, 0, OSP_Adm_Cvarcheck, " <player_ID> <cvar_name>:^7 Check cvar for specified player"},
	{"showex", 0, 0, OSP_Adm_Showex, ":^7 Show the extraction to everybody for 15 seconds"},
	{"callvote", 0, 0, OSP_Adm_Callvote, " <vote>:^7 Call a YES/NO vote"},
	{NULL, 0, 1, 0, NULL},
};

// osp: 0x0002964b
void OSP_Cmd_Adm(gentity_t *e, int arg1, int arg2)
{
	unsigned int i;
	ospAdminCmd_t *cmd;
	char arg[128];
	gentity_t *ent;
	unsigned int numCmds;

	numCmds = sizeof(ospAdminCmds) / sizeof(ospAdminCmds[0]);
	ent = e;

	memset(arg, 0, sizeof(arg));
	trap_Argv(1, arg, sizeof(arg) - 1);

	for (i = 0; i < numCmds; i++)
	{
		cmd = &ospAdminCmds[i];

		if (trap_Argc() == 1)
		{
			OSP_Adm_Help(ent, i, cmd->rfu2);
			return;
		}

		if (cmd->func != NULL && Q_stricmp(arg, cmd->name) == 0)
		{
			if (!OSP_Adm_Usage(ent, arg, i))
			{
				if (!OSP_IsClientAdmin(ent - g_entities, qfalse) && Q_stricmp(arg, "suspend") != 0)
				{
					trap_SendServerCommand(ent - g_entities, "print \"^3Only admins may use that command.\n\"");
				}
				else
				{
					cmd->func(ent, i, cmd->rfu2);
				}
			}

			return;
		}
	}

	if (!OSP_IsClientAdmin(ent - g_entities, qfalse))
	{
		return;
	}

	OSP_Cmd_Ref(ent, 0, qtrue);
	OSP_Adm_ExtraCommand(ent);
}

// osp: 0x00029720
static void OSP_Adm_Help(gentity_t *e, int idx, int rfu2)
{
	int i;
	int numRows;
	int numCmds;
	gentity_t *ent;

	numCmds = (sizeof(ospAdminCmds) / sizeof(ospAdminCmds[0])) - 1;
	ent = e;
	numRows = numCmds / 4;
	if (numCmds % 4)
		numRows++;
	if (numRows < 0)
		return;

	trap_SendServerCommand(ent - g_entities, "print \"^5\nAvailable OSP Admin Commands:\n----------------------------\n\"");

	for (i = 0; i < numRows; i++)
	{
		if (i + (3 * numRows) + 1 <= numCmds)
		{
			trap_SendServerCommand(ent - g_entities,
								   va("print \"%-17s%-17s%-17s%-17s\n\"",
									  ospAdminCmds[i].name,
									  ospAdminCmds[i + numRows].name,
									  ospAdminCmds[i + (2 * numRows)].name,
									  ospAdminCmds[i + (3 * numRows)].name));
		}
		else if (i + (2 * numRows) + 1 <= numCmds)
		{
			trap_SendServerCommand(ent - g_entities,
								   va("print \"%-17s%-17s%-17s\n\"",
									  ospAdminCmds[i].name,
									  ospAdminCmds[i + numRows].name,
									  ospAdminCmds[i + (2 * numRows)].name));
		}
		else if (i + (numRows) + 1 <= numCmds)
		{
			trap_SendServerCommand(ent - g_entities,
								   va("print \"%-17s%-17s\n\"",
									  ospAdminCmds[i].name,
									  ospAdminCmds[i + numRows].name));
		}
		else
		{
			trap_SendServerCommand(ent - g_entities,
								   va("print \"%-17s\n\"",
									  ospAdminCmds[i].name));
		}
	}

	trap_SendServerCommand(ent - g_entities, "print \"\nType: ^3\\command_name ?^7 for more information\n\"");
	trap_SendServerCommand(ent - g_entities, va("print \"\n^6%s\n\"", "SoF2 OSP v(1.1)"));
	trap_SendServerCommand(ent - g_entities, "print \"^5http://www.OrangeSmoothie.org/^7\n\n\"");
}

// osp: 0x00029890
static qboolean OSP_Adm_Usage(gentity_t *e, char *cmd, unsigned int idx)
{
	char arg[512];
	gentity_t *ent;

	ent = e;

	if (!ent)
	{
		return qfalse;
	}

	trap_Argv(2, arg, sizeof(arg));

	if (Q_stricmp(arg, "?") == 0)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"\n^5%s%s\n\n\"", cmd, ospAdminCmds[idx].helptext));
		return qtrue;
	}

	return qfalse;
}

// osp: 0x000298d8
static void OSP_Adm_ExtraCommand(gentity_t *e)
{
	char cmd[128];
	int len;
	char *p;
	fileHandle_t f;
	char buf[2048];
	gentity_t *ent;

	ent = e;

	if (strlen(sv_extracommands.string) == 0 || !strcmp(sv_extracommands.string, "none"))
	{
		return;
	}

	p = ConcatArgs(1);

	if (strstr(p, ";"))
	{
		return;
	}

	trap_Argv(1, cmd, sizeof(cmd));
	len = trap_FS_FOpenFile(sv_extracommands.string, &f, FS_READ);

	if (len <= 0)
	{
		return;
	}
	else
	{
		{
			char *token;
			const char *pos;

			if (len > 2047)
			{
				len = 2047;
			}

			memset(buf, 0, sizeof(buf));
			trap_FS_Read(buf, len, f);

			pos = buf;
			while (1)
			{
				token = COM_Parse(&pos);

				if (!token || !token[0])
				{
					return;
				}

				if (strcmp(Q_strlwr(token), Q_strlwr(cmd)) == 0)
				{
					break;
				}
			}
		}

		trap_FS_FCloseFile(f);
		trap_SendServerCommand(ent - g_entities, va("print \"Extra command successful: %s\n\"", p));
		trap_SendConsoleCommand(EXEC_APPEND, va("%s\n", p));
	}
}

// osp: 0x000299af
static void OSP_Adm_Kick(gentity_t *e, int idx, int rfu2)
{
	char *p;
	gentity_t *ent;
	int argc;

	ent = e;
	if ((argc = trap_Argc()) < 3)
	{
		trap_SendServerCommand(ent - g_entities, "print \"usage: /adm kick <clientid> <reason>\n\"");
		return;
	}

	p = ConcatArgs(2);
	if (((unsigned)p[0] - '0') > 9)
	{
		trap_SendServerCommand(ent - g_entities, "print \"usage: /adm kick <clientid> <reason>\n\"");
		return;
	}

	if (argc < 4)
		trap_SendConsoleCommand(EXEC_APPEND, va("clientkick %s kicked by admin\n", p));
	else
		trap_SendConsoleCommand(EXEC_APPEND, va("clientkick %s\n", p));

	G_LogPrintf("Admin Kick: %s\n", ent->client->pers.netname);
}

// osp: 0x00029a25
static void OSP_Adm_Ban(gentity_t *e, int idx, int rfu2)
{
	gentity_t *ent;
	char *p;

	ent = e;

	if (trap_Argc() < 3)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"usage: /adm ban <clientid> <time_minutes> <reason>\n\""));
		return;
	}

	p = ConcatArgs(2);

	trap_SendConsoleCommand(EXEC_APPEND, va("banclient %s\n", p));
	G_LogPrintf("Admin Ban: %s \n", ent->client->pers.netname);
}

// osp: 0x00029a71
static void OSP_Adm_Cancelvote(gentity_t *e, int idx, int rfu2)
{
	gentity_t *ent;

	ent = e;

	level.voteTime = 0;
	trap_SetConfigstring(CS_VOTE_TIME, "");
	osp.callvoteFunc = 0;
	level.clients[level.voteClient].voteDelayTime = level.time + (g_failedVoteDelay.integer * 60000);

	trap_SendServerCommand(-1, "print \"^5Vote cancelled by admin.\n\"");
	OSP_GlobalSound("sound/misc/events/flush.mp3");
	G_LogPrintf("Admin Cancel vote: %s\n", ent->client->pers.netname);
}

// osp: 0x00029aaf
static void OSP_Adm_Chat(gentity_t *e, int idx, int rfu2)
{
	int i;
	gentity_t *ent;
	char *p;

	ent = e;

	if (trap_Argc() <= 2)
	{
		trap_SendServerCommand(ent - g_entities, "print \"usage: /adm chat <message>\n\"");
		return;
	}

	p = ConcatArgs(2);

	for (i = 0; i < level.maxclients; i++)
	{
		if (!OSP_IsClientOnline(i))
		{
			continue;
		}

		if (!OSP_IsClientAdmin(i, qfalse))
		{
			continue;
		}

		trap_SendServerCommand(i, va("chat %d \"^6*admins only* ^5%s ^7: %s\"", ent->s.number, ent->client->pers.netname, p));
	}

	G_LogPrintf("Admin Chat: %s: %s\n", ent->client->pers.netname, p);
}

// osp: 0x00029b30
static void OSP_Adm_Talk(gentity_t *e, int idx, int rfu2)
{
	gentity_t *ent;
	char *p;

	ent = e;

	if (trap_Argc() <= 2)
	{
		trap_SendServerCommand(ent - g_entities, "print \"usage: /adm talk <message>\n\"");
		return;
	}

	p = ConcatArgs(2);
	trap_SendServerCommand(-1, va("chat %d \"^5*admin* ^7%s ^7: %s\"", ent->s.number, ent->client->pers.netname, p));
	OSP_GlobalSound("sound/misc/menus/click.wav");
	G_LogPrintf("admin talk: %s: %s\n", ent->client->pers.netname, p);
}

// osp: 0x00029b89
static void OSP_Adm_Strike(gentity_t *e, int idx, int rfu2)
{
	int clientNum;
	gentity_t *other;
	gentity_t *ent;
	char arg[8];

	ent = e;

	if (trap_Argc() < 3)
	{
		trap_SendServerCommand(ent - g_entities, "print \"usage: /adm strike <clientid>\n\"");
		return;
	}

	trap_Argv(2, arg, sizeof(arg));
	clientNum = atoi(arg);

	if (clientNum < 0 || clientNum >= level.maxclients)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
		return;
	}

	other = &g_entities[clientNum];

	if (other && other->client)
	{
		G_Damage(other, NULL, ent, NULL, NULL, 99999, DAMAGE_NO_PROTECTION, MOD_TARGET_LASER, HL_NONE);
		OSP_GlobalSound("sound/ambience/generic/thunder_loud.mp3");

		{
			gentity_t *tent;
			tent = G_TempEntity(other->r.currentOrigin, EV_MISSILE_MISS);
			tent->s.weapon = MOD_M84_GRENADE;
			tent->s.eventParm = 0;
		}
	}
	else
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
	}

	G_LogPrintf("Admin Strike: %s \n", ent->client->pers.netname);
}

// osp: 0x00029c53
static void OSP_Adm_Bbq(gentity_t *e, int idx, int rfu2)
{
	gentity_t *other;
	int clientNum;
	gentity_t *ent;
	char arg[8];

	ent = e;

	if (trap_Argc() < 3)
	{
		trap_SendServerCommand(ent - g_entities, "print \"usage: /adm bbq <clientid>\n\"");
		return;
	}

	trap_Argv(2, arg, sizeof(arg));
	clientNum = atoi(arg);

	if (clientNum < 0 || clientNum >= level.maxclients)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
		return;
	}

	other = &g_entities[clientNum];

	if (other && other->client && other->client->ps.stats[STAT_HEALTH] > 0 && (other->client->sess.team == TEAM_RED || other->client->sess.team == TEAM_BLUE || other->client->sess.team == TEAM_FREE))
	{
		vec3_t angles;
		VectorSet(angles, 0, 0, 0);
		other->client->ospClient->bbqTime = level.time + 1000;
		G_PlayEffect(G_EffectIndex("explosions/incendiary_explosion_mp.efx"), other->client->ps.origin, angles);

		{
			gentity_t *tent;
			vec3_t hitdir;

			tent = G_TempEntity(other->client->ps.origin, EV_EXPLOSION_HIT_FLESH);
			VectorSet(hitdir, 1, 0, 0);
			tent->s.eventParm = DirToByte(hitdir);
			tent->s.time = WP_ANM14_GRENADE + ((((int)other->s.apos.trBase[YAW] & 0x7FFF) % 360) << 16);
			tent->r.svFlags = SVF_BROADCAST;
			tent->s.otherEntityNum2 = other->s.number;
			VectorCopy(other->r.currentOrigin, tent->s.angles);
			SnapVector(tent->s.angles);
		}

		other->client->ospClient->bbqAnimTime = level.time + 3000;
		trap_SendServerCommand(-1, va("print \"%s lights up a matchstick under %s\'s butt!\n\"", ent->client->pers.netname, other->client->pers.netname));
	}
	else
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
	}

	G_LogPrintf("Admin BBQ: %s \n", ent->client->pers.netname);
}

// osp: 0x00029dea
static void OSP_Adm_Phone(gentity_t *e, int idx, int rfu2)
{
	gentity_t *other;
	int clientNum;
	gentity_t *ent;
	char arg[8];

	ent = e;

	if (trap_Argc() < 3)
	{
		trap_SendServerCommand(ent - g_entities, "print \"usage: /adm slap <clientid>\n\"");
		return;
	}

	trap_Argv(2, arg, sizeof(arg));
	clientNum = atoi(arg);

	if (clientNum < 0 || clientNum >= level.maxclients)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
		return;
	}

	other = &g_entities[clientNum];

	if (other && other->client)
	{
		vec3_t dir;
		qboolean wasAlive;

		wasAlive = qtrue;

		AngleVectors(other->client->ps.viewangles, dir, NULL, NULL);
		dir[0] *= -1;
		dir[1] *= -1;
		dir[2] = 0.1;
		G_ApplyKnockback(other, dir, 500);

		if (other->health <= 0)
		{
			wasAlive = qfalse;
		}

		other->health -= 5;
		other->client->ps.stats[STAT_ARMOR] -= 5;

		if (wasAlive && other->health < 0)
		{
			other->health = 1;
		}

		trap_SendServerCommand(-1, va("print \"%s schools %s with his telephone receiver!\n\"", ent->client->pers.netname, other->client->pers.netname));
		OSP_GlobalSound("sound/misc/confused/phone_shot.mp3");
	}
	else
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
	}

	G_LogPrintf("Admin Phone: %s \n", ent->client->pers.netname);
}

// osp: 0x00029f16
static void OSP_Adm_Strip(gentity_t *e, int idx, int rfu2)
{
	gentity_t *other;
	int clientNum;
	gentity_t *ent;
	char arg[8];

	ent = e;

	if (trap_Argc() < 3)
	{
		trap_SendServerCommand(ent - g_entities, "print \"usage: /adm slap <clientid>\n\"");
		return;
	}

	trap_Argv(2, arg, sizeof(arg));
	clientNum = atoi(arg);

	if (clientNum < 0 || clientNum >= level.maxclients)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
		return;
	}

	other = &g_entities[clientNum];

	if (other && other->client)
	{
		// Clear ammo
		memset(other->client->ps.ammo, 0, sizeof(other->client->ps.ammo));
		other->client->ps.firemode[27] = 0;
		other->client->ps.firemode[28] = 0;
		other->client->ps.firemode[29] = 0;
		other->client->ps.firemode[30] = 0;
		other->client->ps.firemode[31] = 0;
		memset(other->client->ps.clip, 0, sizeof(other->client->ps.clip));
		// Zoom out
		other->client->ps.zoomFov = 0;
		other->client->ps.pm_flags &= ~(PMF_ZOOM_FLAGS | PMF_GOGGLES_ON);
		// Set knife
		other->client->ps.stats[STAT_WEAPONS] = (1 << WP_KNIFE);
		other->client->ps.clip[ATTACK_NORMAL][WP_KNIFE] = 1;
		other->client->ps.weapon = WP_KNIFE;
		BG_GetInviewAnim(WP_KNIFE, "idle", &other->client->ps.weaponAnimId);

		trap_SendServerCommand(-1, va("print \"%s was just stripped of all his weapons by %s!\n\"", other->client->pers.netname, ent->client->pers.netname));
		OSP_GlobalSound("sound/npc/air1/guard02/laughs.mp3");
	}
	else
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
	}

	G_LogPrintf("Admin Strip: %s \n", ent->client->pers.netname);
}

// osp: 0x0002a060
static void OSP_Adm_Jail(gentity_t *e, int idx, int rfu2)
{
	int clientNum;
	gentity_t *ent;
	char arg[512];
	int seconds;

	seconds = 0;
	ent = e;

	if (trap_Argc() < 3)
	{
		trap_SendServerCommand(ent - g_entities, "print \"Usage: jail <clientid> (seconds)\n\"");
		return;
	}

	trap_Argv(2, arg, sizeof(arg));
	clientNum = atoi(arg);

	if (trap_Argc() == 4)
	{
		trap_Argv(3, arg, sizeof(arg));
		seconds = atoi(arg);
	}

	if (seconds <= 0)
	{
		seconds = osp_defaultJailTime.integer;
	}

	if (clientNum < 0 || clientNum > MAX_CLIENTS)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
		return;
	}

	if (level.clients[clientNum].pers.connected != CON_CONNECTED || !level.clients[clientNum].ospClient)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
		return;
	}

	if (!osp.clients[clientNum].jailTime)
	{
		G_Damage(&g_entities[clientNum], NULL, ent, NULL, NULL, 99999, DAMAGE_NO_PROTECTION, MOD_CRUSH, HL_NONE);
		OSP_GlobalSound("sound/movers/doors/jail_door/jail_doorstop.mp3");
		osp.clients[clientNum].jailTime = level.time + (1000 * seconds);
		trap_SendServerCommand(-1, va("cp \"%s has been ^1JAILED FOR %d SECONDS^7 by admin\n\"", level.clients[clientNum].pers.netname, seconds));
	}
	else
	{
		osp.clients[clientNum].jailTime = 0;
		trap_SendServerCommand(-1, va("print \"%s has been ^2LIBERATED^7 by admin\n\"", level.clients[clientNum].pers.netname));

		if (level.gametypeData->respawnType == RT_NORMAL)
		{
			respawn(ent);
			return;
		}
		else if (level.gametypeData->respawnType == RT_INTERVAL)
		{
			if (level.time > level.gametypeRespawnTime[ent->client->sess.team])
			{
				if (osp_realisticMode.integer)
				{
					level.gametypeRespawnTime[ent->client->sess.team] = level.time + (osp_realisticRespawnInt.integer * 1000);
				}
				else
				{
					level.gametypeRespawnTime[ent->client->sess.team] = level.time + (g_respawnInterval.integer * 1000);
				}
			}

			ent->client->ps.respawnTimer = level.gametypeRespawnTime[ent->client->sess.team] + 1000;
		}
	}

	G_LogPrintf("Admin Jail: %s \n", ent->client->pers.netname);
}

// osp: 0x0002a200
static void OSP_Adm_Mute(gentity_t *e, int idx, int rfu2)
{
	int clientNum;
	gentity_t *ent;
	char arg[512];

	ent = e;

	if (trap_Argc() < 3)
	{
		trap_SendServerCommand(ent - g_entities, "print \"Usage:  mute <clientid>\n\"");
		return;
	}

	trap_Argv(2, arg, sizeof(arg));
	clientNum = atoi(arg);

	if (clientNum < 0 || clientNum > MAX_CLIENTS)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
		return;
	}

	if (level.clients[clientNum].pers.connected != CON_CONNECTED)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
		return;
	}

	level.clients[clientNum].sess.muted = level.clients[clientNum].sess.muted ? qfalse : qtrue;

	if (level.clients[clientNum].sess.muted)
	{
		trap_SendServerCommand(-1, va("print \"%s has been ^1MUTED^7 by admin\n\"", level.clients[clientNum].pers.netname));
	}
	else
	{
		trap_SendServerCommand(-1, va("print \"%s has been ^2UNMUTED^7 by admin\n\"", level.clients[clientNum].pers.netname));
	}

	G_LogPrintf("admin mute: %s \n", ent->client->pers.netname);
}

// osp: 0x0002a2dc
static void OSP_Adm_Cvarcheck(gentity_t *e, int idx, int rfu2)
{
	int clientNum;
	gentity_t *ent;
	char arg2[512];
	char arg1[512];

	ent = e;

	if (trap_Argc() != 4)
	{
		trap_SendServerCommand(ent - g_entities, "print \"Usage: cvarcheck <clientid> <cvar>\n\"");
		return;
	}

	trap_Argv(2, arg1, sizeof(arg1));
	clientNum = atoi(arg1);

	if (clientNum < 0 || clientNum > MAX_CLIENTS)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
		return;
	}

	if (level.clients[clientNum].pers.connected != CON_CONNECTED)
	{
		trap_SendServerCommand(ent - g_entities, va("print \"Invalid client number %d.\n\"", clientNum));
		return;
	}

	trap_Argv(3, arg2, sizeof(arg2));

	if (sub_0002f577(arg2, "password"))
	{
		trap_SendServerCommand(ent - g_entities, "print \"Cvars with the word password in them are not checkable.\n\"");
		return;
	}

	trap_SendServerCommand(clientNum, va("cvarcheck %d \"%s\"\n", ent - g_entities, arg2));
	G_LogPrintf("Admin Cvar check: %s \n", ent->client->pers.netname);
}

// osp: 0x0002a398
static void OSP_Adm_Suspend(gentity_t *e, int idx, int rfu2)
{
	int i;
	char *guid;
	qboolean isAdmin;
	gentity_t *ent;

	ent = e;
	isAdmin = qfalse;
	guid = OSP_GetGuid(ent - g_entities);

	for (i = 0; i < MAX_ADMINS; i++)
	{
		if (Q_strncmp(guid, osp.adminList[i], 32) == 0)
		{
			isAdmin = qtrue;
		}
	}

	if (!isAdmin)
	{
		trap_SendServerCommand(ent - g_entities, "print \"^3Only admins can use that command.\n\"");
		return;
	}

	if (!ent->client->ospClient->adminPowerSuspended)
	{
		trap_SendServerCommand(-1, va("cp \"^3%s ^3has revoked admin privileges.\n\"", ent->client->pers.netname));
		OSP_GlobalSound("sound/movers/doors/airlock_door02/airlock02_doorstop.mp3");
		ent->client->ospClient->adminPowerSuspended = qtrue;
	}
	else if (ent->client->ospClient->adminPowerSuspended)
	{
		trap_SendServerCommand(-1, va("cp \"^3%s ^3has recovered admin privileges.\n\"", ent->client->pers.netname));
		OSP_GlobalSound("sound/movers/doors/airlock_door02/airlock02_open.mp3");
		ent->client->ospClient->adminPowerSuspended = qfalse;
	}

	G_LogPrintf("Admin Suspend: %s \n", ent->client->pers.netname);
}

// osp: 0x0002a45e
static void OSP_Adm_Pause(gentity_t *e, int idx, int pause)
{
	gentity_t *ent;

	ent = e;

	if (pause && osp.paused > 1)
	{
		trap_SendServerCommand(ent - g_entities, "print \"The match is already paused\n\"");
		return;
	}

	if (!pause && !osp.paused)
	{
		trap_SendServerCommand(ent - g_entities, "print \"The match is not currently paused\n\"");
		return;
	}

	if (osp.paused == 1)
	{
		trap_SendServerCommand(ent - g_entities, "print \"The match is currently resuming\n\"");
		return;
	}

	if (pause)
	{
		OSP_Pause(ent, qfalse);
	}
	else
	{
		OSP_Unpause(ent, qfalse);
	}

	OSP_UpdateTeaminfo();
	G_LogPrintf("Admin Pause: %s \n", ent->client->pers.netname);
}

// osp: 0x0002a4ce
static void OSP_Adm_Showex(gentity_t *e, int idx, int rfu2)
{
	gentity_t *target;
	gentity_t *ent;

	ent = e;

	if (strcmp(level.gametypeData->name, "inf") != 0)
	{
		trap_SendServerCommand(ent - g_entities, "print \"This command is only available in Infiltration gametype\n\"");
		return;
	}

	target = NULL;
	while (NULL != (target = G_Find(target, FOFS(classname), "gametype_trigger")))
	{
		gentity_t *tent;
		vec3_t corners[8];
		int i;

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
			tent = G_TempEntity(corners[i], EV_BOTWAYPOINT);
			tent->r.svFlags = SVF_BROADCAST;
			VectorCopy(corners[i + 1], tent->s.angles);
			tent->s.time = 15000;
			tent->s.time2 = 0xff3333;
		}

		tent = G_TempEntity(corners[0], EV_BOTWAYPOINT);
		tent->r.svFlags = SVF_BROADCAST;
		VectorCopy(corners[2], tent->s.angles);
		tent->s.time = 15000;
		tent->s.time2 = 0xff3333;

		tent = G_TempEntity(corners[5], EV_BOTWAYPOINT);
		tent->r.svFlags = SVF_BROADCAST;
		VectorCopy(corners[7], tent->s.angles);
		tent->s.time = 15000;
		tent->s.time2 = 0xff3333;

		tent = G_TempEntity(corners[1], EV_BOTWAYPOINT);
		tent->r.svFlags = SVF_BROADCAST;
		VectorCopy(corners[3], tent->s.angles);
		tent->s.time = 15000;
		tent->s.time2 = 0xff3333;

		tent = G_TempEntity(corners[4], EV_BOTWAYPOINT);
		tent->r.svFlags = SVF_BROADCAST;
		VectorCopy(corners[6], tent->s.angles);
		tent->s.time = 15000;
		tent->s.time2 = 0xff3333;

		break;
	}
}

// osp: 0x0002a618
static void OSP_Adm_Callvote(gentity_t *e, int idx, int rfu2)
{
	int i;
	char *p;
	gentity_t *ent;

	ent = e;
	p = ConcatArgs(2);

	if (strlen(p) < 1)
	{
		trap_SendServerCommand(ent - g_entities, "print \"You must give a vote\n\"");
		return;
	}

	G_LogPrintf("Admin Vote:\t%s\t%s", ent->client->pers.netname, level.voteString);
	trap_SendServerCommand(-1, "print \"An admin calls a vote.\n\"");
	OSP_GlobalSound("sound/misc/menus/quit_request.wav");

	level.voteTime = level.time;
	level.voteYes = 0;
	level.voteNo = 0;

	trap_SetConfigstring(CS_VOTE_TIME, va("%i,%i", level.voteTime, g_voteDuration.integer * 1000));
	trap_SetConfigstring(CS_VOTE_STRING, p);
	trap_SetConfigstring(CS_VOTE_YES, va("%i", level.voteYes));
	trap_SetConfigstring(CS_VOTE_NO, va("%i", level.voteNo));

	for (i = 0; i < level.maxclients; i++)
	{
		level.clients[i].ps.eFlags &= ~EF_VOTED;
	}

	{
		int voteNeeded;

		voteNeeded = vote_percent.integer;
		if (voteNeeded > 99)
			voteNeeded = 99;
		if (voteNeeded < 1)
			voteNeeded = 1;

		trap_SetConfigstring(CS_VOTE_NEEDED, va("%i", ((voteNeeded * level.numVotingClients) / 100) + 1));
	}
}
