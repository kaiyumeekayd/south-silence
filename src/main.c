#include <sys/types.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxapi.h>

int main(void)
{
    DISPENV disp;
    DRAWENV draw;

    ResetGraph(0);

    SetDefDispEnv(&disp, 0, 0, 320, 240);
    SetDefDrawEnv(&draw, 0, 0, 320, 240);

    draw.r0 = 80;
draw.g0 = 0;
draw.b0 = 0;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    DrawSync(0);

    while (1)
    {
        VSync(0);
    }

    return 0;
}
