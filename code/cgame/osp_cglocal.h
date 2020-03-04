typedef struct
{
    qhandle_t osplogo;                 // 0x4581cc
    qhandle_t osplogopp;               // 0x4581d0
    qhandle_t sof2logopp;              // 0x4581d4
    qhandle_t statusbarHealth;         // 0x4581d8
    qhandle_t statusbarArmor;          // 0x4581dc
    qhandle_t awardMedals[MAX_AWARDS]; // 0x4581e0
    qhandle_t locations;               // 0x458218
    qhandle_t awardSound;              // 0x45821c
    qhandle_t badawardSound;           // 0x458220
} ospMedia_t;

typedef struct
{
    int displayTime;
    int soundPlayed;
    int winnerScore;
    int x0c;
    int x10;
    char winnerName[32];
} ospAward_t;

typedef struct
{
    int matchphase;                     // 0x457818
    char teamNames[TEAM_NUM_TEAMS][64]; // 0x45781c
    int x45791c;                        // 0x45791c
    int x457920;                        // 0x457920
    char x457924[256];                  // 0x457924
    char gameversion[16];               // 0x457a24
    qboolean modifiedFiles;             // 0x457a34
    ospAward_t awards[MAX_AWARDS];      // 0x457a38
    int x457d10[17];                    // 0x457d10
    int x457d54[6]; // 0x457d54
    int x457d6c[6]; // 0x457d6c
    int x457d84[270]; // 0x457d84
    int x4581bc;            // 0x4581bc
    int x4581c0;            // 0x4581c0
    int x4581c4;            // 0x4581c4
    qboolean realisticMode; // 0x4581c8
} ospCgame_t;

//
// osp_consolecmds.c
//
void OSP_InitConsoleCommands(void);
void OSP_StartRecording_f(void);
void OSP_StopRecording_f(void);

//
// osp_draw.c
//
void OSP_DrawClock(void);
void OSP_DrawCrosshairInfo(void);
void OSP_DrawTeamoverlay(void);
void OSP_DrawInformation(void);
void OSP_DrawHUD(void);
void OSP_DrawAwards(void);
void OSP_DrawBadAwards(void);
qboolean OSP_DrawStats(void);

//
// osp_ents.c
//
qboolean sub_0002789a(centity_t *cent);
void sub_0002794a(refEntity_t *x18);

//
// osp_main.c
//
void OSP_RegisterCvars(void);
void OSP_UpdateCvars(void);
qboolean OSP_IsDigit(char c);
void OSP_Init(void);
qboolean OSP_Music(void);
void sub_00027c25(void);

//
// osp_scoreboard.c
//
qboolean OSP_DrawScoreboard(float y);

//
// osp_servercmds.c
//
void OSP_ParseReadyup(void);
void OSP_ParseMatchphase(void);
void OSP_ParseTeamname(team_t team);
void OSP_ParseTeaminfo(void);
void OSP_Record(qboolean start);
void OSP_Screenshot(void);
qboolean OSP_ServerCommand(const char *cmd);
qboolean OSP_ConfigStringModified(int num);

//
// osp_?
//
void sub_000296a3(refdef_t *fd);

extern vmCvar_t osp_drawCrosshairInfo;
extern vmCvar_t osp_drawClock;
extern vmCvar_t osp_drawTeamOverlay;
extern vmCvar_t osp_autoRecord;
extern vmCvar_t osp_autoScreenshot;
extern vmCvar_t cg_ospClient;
extern vmCvar_t osp_simpleHud;
extern vmCvar_t osp_numericalHealth;
extern vmCvar_t osp_oldScoreboard;
extern vmCvar_t osp_tracerDistance;
extern vmCvar_t osp_useKeyForLadders;
extern vmCvar_t osp_oldConsole;
extern vmCvar_t osp_weaponsFile;
extern vmCvar_t osp_modversion;
extern vmCvar_t x458664;
extern vmCvar_t osp_mvFollow;
extern vmCvar_t osp_mvFov;
extern vmCvar_t osp_mvMaxViews;
extern vmCvar_t osp_mvViewRatio;

extern ospMedia_t ospMedia;
extern ospCgame_t ospCg;
