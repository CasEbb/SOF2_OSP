#define MAX_ADMINS 100
#define MAX_GUID 64

#define ALTMAP_GT_ITEM (1 << 0)
#define ALTMAP_GT_TRIGGER (1 << 1)
#define ALTMAP_GT_PLAYER_RED (1 << 2)
#define ALTMAP_GT_PLAYER_BLUE (1 << 3)
#define ALTMAP_TARGET_LOCATION (1 << 4)
#define ALTMAP_PICKUP (1 << 5)
#define ALTMAP_STATIC (1 << 6)
#define ALTMAP_INTERMISSION (1 << 7)
#define ALTMAP_BACKPACK (1 << 8)
#define ALTMAP_SPEAKER (1 << 9)

typedef enum
{
    AWARD_LETHAL,
    AWARD_DAMAGE,
    AWARD_HEADSHOTS,
    AWARD_EXPLOSIVES,
    AWARD_CRAZIEST,
    AWARD_SCORE,
    AWARD_SURVIVOR,

    BADAWARD_QUITTER,
    BADAWARD_LEMON,
    BADAWARD_NEWBIE,
    BADAWARD_FREEKILL,
    BADAWARD_CROSSEYED,
    BADAWARD_SPAMMER,
    BADAWARD_CHATTYBITCH,

    MAX_AWARDS,
};

typedef struct ospClient_s
{
    int x0;                       // +0x00
    int x4;                       // +0x04
    int x8;                       // +0x08
    int xc;                       // +0x0c
    int cmdThrottle;              // +0x10
    unsigned int x14;             // +0x14
    qboolean ready;               // +0x18
    int rate;                     // +0x1c
    int snaps;                    // +0x20
    int motdTime;                 // +0x24
    int warmupTime;               // +0x28
    int referee;                  // +0x2c
    qboolean captain;             // +0x30
    int specinvites;              // +0x34
    int x38;                      // +0x38
    int x3c;                      // +0x3c
    char cleanname[36];           // +0x40
    qboolean admin;               // +0x64
    qboolean adminPowerSuspended; // +0x68
    int lastHitClient;            // +0x6c
    int lastHitByClient;          // +0x70
    int x74;                      // +0x74
    int afkTime;                  // +0x78
    qboolean teamOverlay;         // +0x7c
    float clientVersion;          // +0x80
    qboolean useKeyForLadders;    // +0x84
    qboolean ospScoreboard;       // +0x88
    int bbqTime;                  // +0x8c
    int bbqAnimTime;              // +0x90
    int x94;                      // +0x94
    int x98;                      // +0x98
    qboolean subbing;             // +0x9c
    int vip;                      // +0xa0
    int oldVip;                   // +0xa4
    int jailTime;                 // [+0xa8]
} ospClient_t;

typedef struct ospWeaponStats_s
{
    unsigned int shots;
    unsigned int deaths;
    unsigned int headhits;
    unsigned int hits;
    unsigned int kills;
} ospWeaponStats_t;

typedef struct ospStats_s
{
    int damageGiven; // 0x00
    int damageTaken; // 0x04
    int deaths;      // 0x08
    int x0c;         // 0x0c
    int kills;       // 0x10
    int x14;         // 0x14
    int suicides;    // 0x18
    int teamDamage;  // 0x1c
    int teamKills;   // 0x20
    int x24;         // 0x24
    float x28;       // 0x28
    int x2c;         // 0x2c
    int numChats;    // 0x30
    int killstreak;  // 0x34
    int x38;         // 0x38
    int x3c;         // 0x3c
    int x40;         // 0x40

    int x44[6];

    // int x48; // HEAD
    // int x4c; // CHEST
    // int x50; // ARM
    // int x54; // WAIST
    // int x58; // LEG

    int x5c[6];

    // int x60; // HEAD
    // int x64; // CHEST
    // int x68; // ARM
    // int x6c; // WAIST
    // int x70; // LEG

    ospWeaponStats_t weapon[ATTACK_MAX][WP_NUM_WEAPONS];
} ospStats_t;

#define AMMOWRAP(ps, ammoIndex) ((ammoIndex < MAX_AMMO) ? ((ps).ammo[ammoIndex]) : ((ps).firemode[MAX_WEAPONS - 1 - (ammoIndex - MAX_AMMO)]))
