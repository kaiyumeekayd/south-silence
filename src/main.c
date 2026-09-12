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
    draw.r0 = 100;
    draw.g0 = 0;
    draw.b0 = 0;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);

    while (1)
    {
        VSync(0);
    }

    return 0;
}
