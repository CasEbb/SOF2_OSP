// osp_view.c
//
#include "cg_local.h"

static int x459678;
static int x459674;
static float x459670;
static float x45966c;
static float x459668;

// osp: 0x000295e3
static void sub_000295e3(void)
{
    if (!cg.hyperspace)
    {
        CG_AddPacketEntities();
        CG_AddLocalEntities();
        CG_DrawMiscEnts();
    }
}

// osp: 0x000295f3
void sub_000295f3(refdef_t *fd, centity_t *cent, int i)
{
    int x10;

    fd->fov_x /= osp_mvFov.value;
    fd->fov_y /= osp_mvFov.value;

    fd->width = (640.0f * cgs.screenXScale) / osp_mvViewRatio.integer;
    fd->height = (480.0f * cgs.screenYScale) / osp_mvViewRatio.integer;

    fd->x = (i % osp_mvViewRatio.integer == 0) ? 0 : (fd->width * i) % (fd->width * osp_mvViewRatio.integer);

    x10 = (i < osp_mvViewRatio.integer) ? 0 : i / osp_mvViewRatio.integer;

    fd->y = !x10 ? 0 : fd->height * x10;

    AnglesToAxis(cent->lerpAngles, fd->viewaxis);
    VectorCopy(cent->lerpOrigin, fd->vieworg);
    VectorCopy(cent->lerpAngles, fd->viewangles);
}

// osp: 0x000296a3
void sub_00003467(void);
void sub_000296a3(refdef_t *fd)
{
    int i;
    centity_t *cent;
    const char *s;
    int x34;
    int x38;
    refdef_t x3c;
    int x1b8;

    x38 = cg.snap->ps.clientNum;
    i = 0;
    x34 = 0;

    if (!cg.multiview)
    {
        return;
    }

    memcpy(&x3c, fd, sizeof(refdef_t));

    for (i = 0; i < cgs.maxclients; i++)
    {
        if (cgs.clientinfo[i].team == cgs.clientinfo[cg.snap->ps.clientNum].team)
        {
            x34++;
        }
    }

    x1b8 = osp_mvFollow.integer;
    if (osp_mvFollow.integer < 0 || osp_mvFollow.integer > 63)
    {
        x1b8 = 0;
    }

    i = 0;
    x34 = 0;

    while (x34 < (osp_mvViewRatio.integer * osp_mvViewRatio.integer) && i < MAX_CLIENTS)
    {
        int rfu;
        
        if (i >= osp_mvMaxViews.integer)
        {
            break;
        }

        cent = &cg_entities[i];
        i++;

        if (!cent->currentValid)
        {
            continue;
        }

        if (cent->currentState.number == cg.snap->ps.clientNum)
        {
            continue;
        }

        sub_000295f3(fd, cent, x34);
        x34++;
        x45966c = fd->vieworg[2];
        fd->vieworg[2] += (cent->currentState.eFlags & EF_DUCKED) ? 8 : 37;
        x459668 = fd->vieworg[2];

        if (x45966c != x459668)
        {
            x459670 = x45966c - x459668;
            x459674 = cg.time;
        }

        x459678 = cg.time - cg.duckTime;
        if (x459678 < DUCK_TIME)
            fd->vieworg[2] -= x459670 * (DUCK_TIME - x459678) / DUCK_TIME;

        cg.snap->ps.clientNum = cent->currentState.number;
        sub_000295e3();
        trap_R_RenderScene(fd);

        s = va("Following: %s", cgs.clientinfo[cent->currentState.number].name);

        CG_DrawText(
            ((fd->x + (fd->width / 2)) / cgs.screenXScale) - (trap_R_GetTextWidth(s, cgs.media.hudFont, 0.2f, 0) / 2),
            (fd->y / cgs.screenYScale) + trap_R_GetTextHeight(s, cgs.media.hudFont, 0.25f, 0),
            cgs.media.hudFont,
            0.25f,
            colorWhite,
            s,
            0, 0);
        sub_00003467();
    }

    memcpy(fd, &x3c, sizeof(refdef_t));
    cg.snap->ps.clientNum = x38;
}

/*
0x024 --
0x028 i
0x02c cent
0x030 s
0x034
0x038
0x03c
0x1b8 

0x1e0 --
0x1e4 --
0x1e8 fd
*/