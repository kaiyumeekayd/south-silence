#include <sys/types.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxapi.h>

#define OT_LEN 8

static DISPENV disp;
static DRAWENV draw;

static u_long ot[OT_LEN];
static POLY_F3 poly;

static SVECTOR vertices[3] = {
    {-60,  50, 400, 0},
    { 60,  50, 400, 0},
    {  0, -60, 400, 0}
};

int main(void)
{
    MATRIX matrix;

    long sx0, sy0, sz0, flag0;
    long sx1, sy1, sz1, flag1;
    long sx2, sy2, sz2, flag2;

    ResetGraph(0);

    SetDefDispEnv(&disp, 0, 0, 320, 240);
    SetDefDrawEnv(&draw, 0, 0, 320, 240);

    draw.isbg = 1;
    draw.r0 = 15;
    draw.g0 = 15;
    draw.b0 = 15;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);

    InitGeom();

    gte_SetGeomOffset(160, 120);
    gte_SetGeomScreen(256);

    RotMatrix(&((SVECTOR){0, 0, 0, 0}), &matrix);

    gte_SetRotMatrix(&matrix);
    gte_SetTransMatrix(&matrix);

    while (1)
    {
        DrawSync(0);
        VSync(0);

        ClearOTagR(ot, OT_LEN);

        gte_RotTransPers(
            &vertices[0],
            &sx0, &sy0,
            &sz0,
            &flag0
        );

        gte_RotTransPers(
            &vertices[1],
            &sx1, &sy1,
            &sz1,
            &flag1
        );

        gte_RotTransPers(
            &vertices[2],
            &sx2, &sy2,
            &sz2,
            &flag2
        );

        setPolyF3(&poly);

        setRGB0(&poly, 180, 180, 180);

        setXY3(
            &poly,
            sx0, sy0,
            sx1, sy1,
            sx2, sy2
        );

        addPrim(&ot[0], &poly);

        DrawOTag(&ot[0]);
    }

    return 0;
}
