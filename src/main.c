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

    setPolyF4(&poly);
    setRGB0(&poly, 120, 120, 120);

    setXY4(
        &poly,
        80, 60,
        240, 60,
        240, 180,
        80, 180
    );

    while (1)
    {
        DrawSync(0);
        VSync(0);

        ClearOTagR(ot, OT_LEN);

        addPrim(&ot[0], &poly);

        DrawOTag(&ot[0]);
    }

    return 0;
}
