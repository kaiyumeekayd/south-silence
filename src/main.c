#include <stdint.h>
#include <psxgpu.h>
#include <psxetc.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

int main(void)
{
    DISPENV disp;
    DRAWENV draw;

    SetDefDispEnv(&disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    draw.isbg = 1;
    draw.r0 = 12;
    draw.g0 = 12;
    draw.b0 = 16;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);

    uint32_t ot[1];

    ClearOTagR(ot, 1);

    POLY_F3 triangle;

    setPolyF3(&triangle);

    setRGB0(&triangle, 100, 100, 100);

    setXY3(
        &triangle,
        80, 180,
        160, 60,
        240, 180
    );

    addPrim(&ot[0], &triangle);

    DrawOTag(&ot[0]);

    while (1)
    {
        VSync(0);
    }

    return 0;
}
