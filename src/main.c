#include <stdint.h>
#include <psxgpu.h>
#include <psxetc.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define OT_LENGTH 16

typedef struct
{
    POLY_F3 polygons[3];
    uint32_t ot[OT_LENGTH];
} RenderContext;

static void make_triangle(
    POLY_F3 *poly,
    int x0, int y0,
    int x1, int y1,
    int x2, int y2,
    int r, int g, int b
)
{
    setPolyF3(poly);

    setRGB0(poly, r, g, b);

    setXY3(
        poly,
        x0, y0,
        x1, y1,
        x2, y2
    );
}

int main(void)
{
    DISPENV disp;
    DRAWENV draw;

    RenderContext ctx;

    ResetGraph(0);

    SetDefDispEnv(
        &disp,
        0,
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

    SetDefDrawEnv(
        &draw,
        0,
        0,
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );

    setRGB0(&draw, 12, 12, 16);

    draw.isbg = 1;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);

    while (1)
    {
        ClearOTagR(ctx.ot, OT_LENGTH);

        /*
         * TRIÂNGULO 1
         */
        make_triangle(
            &ctx.polygons[0],
            40, 40,
            120, 40,
            80, 100,
            180, 180, 180
        );

        /*
         * TRIÂNGULO 2
         */
        make_triangle(
            &ctx.polygons[1],
            120, 100,
            200, 100,
            160, 160,
            100, 100, 100
        );

        /*
         * TRIÂNGULO 3
         */
        make_triangle(
            &ctx.polygons[2],
            200, 40,
            280, 40,
            240, 100,
            60, 60, 60
        );

        addPrim(&ctx.ot[12], &ctx.polygons[0]);
        addPrim(&ctx.ot[11], &ctx.polygons[1]);
        addPrim(&ctx.ot[10], &ctx.polygons[2]);

        DrawOTag(&ctx.ot[OT_LENGTH - 1]);

        VSync(0);
    }

    return 0;
}
