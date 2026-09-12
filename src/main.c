#include <sys/types.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxapi.h>

#define OT_LEN 8

static DISPENV disp;
static DRAWENV draw;

static u_long ot[OT_LEN];
static POLY_F4 poly;

int main(void)
{
    ResetGraph(0);

    SetDefDispEnv(&disp, 0, 0, 320, 240);
    SetDefDrawEnv(&draw, 0, 0, 320, 240);

    draw.isbg = 1;
    draw.r0 = 20;
    draw.g0 = 20;
    draw.b0 = 20;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);

    ClearOTagR(ot, OT_LEN);

    setPolyF4(&poly);

    setRGB0(&poly, 120, 120, 120);

    setXY4(
        &poly,
        100, 70,
        220, 70,
        240, 190,
        80, 190
    );

    addPrim(&ot[OT_LEN - 1], &poly);

    while (1)
    {
        DrawSync(0);
        VSync(0);

        DrawOTag(&ot[0]);
    }

    return 0;
}
