#define CHECKSUM_SOF2_WPN 0x0a413c58
#define CHECKSUM_SOF2_AMMO 0x0f53362f
#define CHECKSUM_SOF2_INVIEW 0x74694f79
#define CHECKSUM_SOF2PP_WPN 0x417d0227
#define CHECKSUM_SOF2PP_INVIEW 0x0c7b7524

typedef enum
{
    PP_UNPAUSED,
    PP_UNPAUSING,
    PP_ADMIN,
    PP_TIMEOUT_RED,
    PP_TIMEOUT_BLUE,
} ospPausePhase_t;

typedef struct ospAward_s
{
    int showTime;
    qboolean shown;
    int score;
    int tiebreaker;
    int winnerNum;
    char winnerName[32];
} ospAward_t;

typedef struct
{
    ospStats_t *stats;                                                   // 0x6450f0
    ospClient_t *clients;                                                // 0x6450f4
    int x6450f8;                                                         // 0x6450f8
    ospPausePhase_t paused;                                              // 0x6450fc
    int unpauseTime;                                                     // 0x645100
    int timeoutTimeout;                                                  // 0x645104
    int unpauseType;                                                     // 0x645108
    int unpauseClient;                                                   // 0x64510c
    int unpauseCountdown;                                                // 0x645110
    int x645114;                                                         // 0x645114
    int x645118;                                                         // 0x645118
    int forceReady;                                                      // 0x64511c
    int x645120;                                                         // 0x645120
    char x645124[256];                                                   // 0s645124
    int (*callvoteFunc)(gentity_t *, unsigned int, char *, char *, int); // 0x645224
    char callvoteArgs[3][256];                                           // 0x645228
    char adminList[MAX_ADMINS][MAX_GUID];                                // 0x645528
    int numAdmins;                                                       // 0x646e28
    int x646e2c[3];                                                      // 0x646e2c
    int matchPhase;                                                      // 0x646e38
    int x646e3c[TEAM_NUM_TEAMS][2];                                      // 0x646e3c
    vec3_t speclockOrigin;                                               // 0x646e5c
    float speclockYaw;                                                   // 0x646e68
    qboolean x646e6c;                                                    // 0x646e6c
    vec3_t x646e70[MAX_CLIENTS];                                         // 0x646e70
    char badwords[2096];                                                 // 0x647170
    respawnType_t x6479a0;                                               // 0x6479a0
    int x6479a4;                                                         // 0x6479a4
    int x6479a8;                                                         // 0x6479a8
    int x6479ac;                                                         // 0x6479ac
    int x6479b0;                                                         // 0x6479b0
    int pistolsOnly;                                                     // 0x6479b4
    int knivesOnly;                                                      // 0x6479b8
    int weaponOnly;                                                      // 0x6479bc
    int weaponOnly_weapon;                                               // 0x6479c0
    char awardMsg[1024];                                                 // 0x6479c4
    ospAward_t awards[14];                                               // 0x647dc4
    qboolean awardsStarted;                                              // 0x64809c
    int x6480a0;                                                         // 0x6480a0
    char altmapPath[MAX_QPATH];                                          // 0x6480a4
    unsigned int altmapFlags;                                            // 0x6480e4
    int x6480e8;                                                         // 0x6480e8
    char altinfo1[256];                                                  // 0x6480ec
    char altinfo2[256];                                                  // 0x6481ec
    int x6482ec[4];                                                      // 0x6482ec
} osp_locals_t;

typedef struct
{
    qboolean speclocked;
    qboolean locked;
    qboolean hasCustomName;
    int x0c;
    int timeoutsLeft;
} ospTeamData_t;

//
// osp_active.c
//
void OSP_DoWarmup(gentity_t *ent);
void OSP_StopMotd(gentity_t *ent);
qboolean OSP_SpectatorInactivity(gentity_t *ent);
void OSP_SpectatorThink(gentity_t *ent, int x64809c);
void OSP_ClientEndFrame(int clientNum);

//
// osp_admin.c
//
void OSP_Cmd_Adm(gentity_t *ent, int idx, int arg);

//
// osp_chattokens.c
//
void OSP_ParseTokens(gentity_t *ent, char *chatText, int size);
void OSP_Cmd_Tokens(gentity_t *ent, int idx, int arg);

//
// osp_client.c
//
qboolean OSP_IsClientOnline(int clientNum);
char *OSP_GetClientName(int clientNum, int len);
char *OSP_GetGuid(int clientNum);
void OSP_SetCachedGuid(int clientNum);
void OSP_ClearCachedGuid(int clientNum);
char *OSP_GetCachedGuid(int clientNum);
qboolean OSP_LoadAdmin(int clientNum);
qboolean OSP_IsClientAdmin(int clientNum, qboolean withPowers);
qboolean OSP_IsClientReferee(int clientNum);
qboolean OSP_IsClientCaptain(int clientNum);
qboolean OSP_ClientConnect(int clientNum);
char *OSP_ClientUserinfoChanged(int clientNum);
void OSP_ClientSpawn(int clientNum);
void OSP_ClientDisconnect(int clientNum);
int OSP_ClientForString(gentity_t *ent, char *s);
void sub_0002b4b4(team_t team);
int OSP_ClientFlags(int clientNum);

//
// osp_cmds.c
//
qboolean OSP_ClientCommand(gentity_t *ent, const char *cmd, qboolean noIntermission);
void OSP_PrintStatsToAllClients(void);
qboolean OSP_FilterChattext(char *text);
void OSP_ScoreboardMessage(gentity_t *ent);

//
// osp_combat.c
//
void OSP_Damage(gentity_t *attacker, gentity_t *target, int damage, int location, int mod, qboolean logHits);
void OSP_PlayerDie(gentity_t *attacker, gentity_t *self, int hitLocation, int mod);
int OSP_DeathAnim(gentity_t *self, int hitLocation, vec3_t hitDir);

//
// osp_gametype.c
//
qboolean OSP_LoadSettings(qboolean comp, char *name);
void OSP_LastOneAliveWarning(team_t team);
void OSP_MatchSwap(void);
void OSP_MatchShuffle(void);
void OSP_SelectVIP(gclient_t *client);
qboolean OSP_CheckMaxM203(gentity_t *ent);

//
// osp_main.c
//
qboolean OSP_RegisterCvars(void);
qboolean OSP_UpdateCvars(void);
void OSP_InitGame(void);
void OSP_GlobalSound(char *path);
void OSP_ClientSound(int clientNum, char *path);
void OSP_ClearStats(int clientNum);
qboolean OSP_CanFollow(gentity_t *ent, team_t team);
void OSP_CheckVote(void);
int OSP_VoteDisabled(char *vote);

//
// osp_misc.c
//
float OSP_GetKDR(gclient_t *client);
int OSP_KDRSort(const void *a, const void *b);
float OSP_GetKPM(gclient_t *client);
int OSP_KPMSort(const void *a, const void *b);
void OSP_SetMatchphase(int oldPhase, int newPhase);
void OSP_CheckMatchphase(void);
qboolean OSP_CheckPause(int msec);
void OSP_Pause(gentity_t *ent, qboolean timeout);
void OSP_UnpauseCountdown(void);
void OSP_Unpause(gentity_t *ent, qboolean timeout);
void OSP_Resume(void);
void OSP_LoadBadwords(void);
char *sub_0002f577(char *s1, char *s2);
void OSP_UpdateTeamOverlay(void);
const char *OSP_ReadyMask(void);
void OSP_UpdateTeaminfo(void);
void OSP_CalculateAwards(void);
void OSP_PrintAwards(void);
void OSP_PrintBadAwards(void);
void OSP_LoadAltmap(void);

//
// osp_referee.c
//
void OSP_Cmd_Ref(gentity_t *ent, int index, int admin);
void OSP_Ref_Ready(gentity_t *ent, unsigned int index, int param);

//
// osp_svcmds.c
//
void OSP_LoadAdmins(void);
void OSP_ShuffleTeams(void);
void OSP_BalanceTeams(void);
void OSP_SwapTeams(void);
qboolean OSP_ConsoleCommand(const char *cmd);

//
// osp_spawn.c
//
qboolean OSP_FindSafeSpawnpoint(team_t team);
qboolean OSP_AltmapExists(void);
void OSP_SpawnAltmap(void);

//
// osp_tcmd.c
//
void OSP_Cmd_Tcmd(gentity_t *ent, int index, int param);

//
// osp_team.c
//
void OSP_ResetTeam(team_t team, qboolean disableSpeclock);
void OSP_ReadTeamSessionData(team_t team);
void OSP_WriteTeamSessionData(team_t team);
void OSP_InviteSpectator(team_t team, qboolean invite, int clientNum);
void OSP_LockSpectators(team_t team, qboolean lock);
void OSP_SetTeamname(team_t team, const char *name);
const char *OSP_GetTeamname(int team);
qboolean OSP_PlayersReady(void);
void OSP_CheckEmptyTeams(void);
qboolean OSP_TeamchangeAllowed(team_t team, team_t oldTeam, gentity_t *ent);
void OSP_ResetSpecinvites(team_t team, qboolean locked);
void OSP_SwapTeamSettings(void);
void OSP_PrintTeaminfo(gentity_t *ent, team_t team);

//
// osp_voting.c
//
qboolean OSP_Callvote(gentity_t *e, unsigned int index, int adminAction);

//
// osp_weapon.c
//
weapon_t OSP_ModToWeapon(int mod);
qboolean OSP_HeadshotPossible(int mod, attackType_t attack);
void OSP_LogShot(gentity_t *ent, int attack);
int OSP_GetPistol(gentity_t *ent);
int OSP_GetPrimaryWeapon(gentity_t *ent);
int OSP_GetSecondaryWeapon(gentity_t *ent);
int OSP_GetGrenade(gentity_t *ent);
float OSP_Accuracy(ospStats_t *stats);
int OSP_AccuracyInt(ospStats_t *stats);
void OSP_RunMissile(gentity_t *ent);
int OSP_NumHeadshots(ospStats_t *stats);
int OSP_ExplosiveKills(ospStats_t *stats);
int OSP_NumShots(ospStats_t *stats);
int OSP_MeleeKills(ospStats_t *stats);

//
// zg_bboxadj.c
//
void G_AdjustClientBBox(gentity_t *other);
void G_AdjustClientBBoxs(void);
void G_UndoAdjustedClientBBox(gentity_t *other);
void G_UndoAdjustedClientBBoxs(void);
void G_SetClientPreLeaningBBox(gentity_t *ent);
void G_SetClientLeaningBBox(gentity_t *ent);
void G_ShowClientBBox(gentity_t *ent);

extern char *osp_alternateAttackNames[WP_NUM_WEAPONS];
extern char *osp_shortWeaponNames[WP_NUM_WEAPONS];

extern osp_locals_t osp;
extern ospStats_t osp_stats[MAX_CLIENTS];
extern ospClient_t osp_clients[MAX_CLIENTS];
extern ospTeamData_t ospTeams[TEAM_NUM_TEAMS];
extern vmCvar_t g_spectatorInactivity;
extern vmCvar_t g_coloredTeamplayNames;
extern vmCvar_t g_alternateSort;
extern vmCvar_t g_radarFix;
extern vmCvar_t g_teamTokensOnly;
extern vmCvar_t g_awards;
extern vmCvar_t g_alternateMap;
extern vmCvar_t g_killingspree;
extern vmCvar_t osp_friendlyFireMultiplier;
extern vmCvar_t osp_modversion;
extern vmCvar_t osp_femaleVIP;
extern vmCvar_t osp_defaultJailTime;
extern vmCvar_t osp_weaponsFile;
extern vmCvar_t osp_realisticMode;
extern vmCvar_t osp_enemyKillReport;
extern vmCvar_t osp_noM203nades;
extern vmCvar_t osp_maxM203PerTeam;
extern vmCvar_t osp_realisticRespawnInt;
extern vmCvar_t sv_enableAdminPowers;
extern vmCvar_t sv_adminfile;
extern vmCvar_t sv_extracommands;
extern vmCvar_t sv_refereePassword;
extern vmCvar_t sv_nolagGlassBreaks;
extern vmCvar_t sv_messageFiltering;
extern vmCvar_t sv_ospClient;
extern vmCvar_t sv_modifiedfiles;
extern vmCvar_t match_latejoin;
extern vmCvar_t match_mutespecs;
extern vmCvar_t server_autoconfig;
extern vmCvar_t match_swap;
extern vmCvar_t match_shuffle;
extern vmCvar_t match_fastRestart;
extern vmCvar_t match_minplayers;
extern vmCvar_t match_readypercent;
extern vmCvar_t match_timeoutcount;
extern vmCvar_t match_timeoutlength;
extern vmCvar_t server_motd0;
extern vmCvar_t server_motd1;
extern vmCvar_t server_motd2;
extern vmCvar_t server_motd3;
extern vmCvar_t server_motd4;
extern vmCvar_t server_motd5;
extern vmCvar_t team_maxplayers;
extern vmCvar_t team_controls;
extern vmCvar_t team_redName;
extern vmCvar_t team_blueName;
extern vmCvar_t vote_limit;
extern vmCvar_t vote_percent;
extern vmCvar_t shotgun_debugWeapons;
extern vmCvar_t shotgun_debugAdjBBox;
extern vmCvar_t shotgun_adjLeanOffset;
extern vmCvar_t shotgun_adjLBMaxsZ;
extern vmCvar_t shotgun_adjLBMinsZ;
extern vmCvar_t x62c190;
extern vmCvar_t shotgun_adjDuckedLBMinsZ;
extern vmCvar_t shotgun_adjPLBMaxsZ;
