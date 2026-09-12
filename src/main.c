#include <stdint.h>
#include <psxgpu.h>
#include <psxetc.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define FOCAL_LENGTH 256

#define OT_LENGTH 16

typedef struct
{
    long x;
    long y;
    long z;
} Vertex3D;

typedef struct
{
    POLY_F3 polygons[2];
    uint32_t ot[OT_LENGTH];
} RenderContext;

static int project_x(Vertex3D v)
{
    return 160 + (int)((v.x * FOCAL_LENGTH) / v.z);
}

static int project_y(Vertex3D v)
{
    return 120 - (int)((v.y * FOCAL_LENGTH) / v.z);
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

    /*
     * QUATRO VÉRTICES 3D
     */

    Vertex3D v0 = { -90,  70, 350 };
    Vertex3D v1 = {  90,  70, 350 };
    Vertex3D v2 = {  70, -70, 500 };
    Vertex3D v3 = { -70, -70, 500 };

    while (1)
    {
        ClearOTagR(
            ctx.ot,
            OT_LENGTH
        );

        /*
         * TRIÂNGULO 1
         */

        setPolyF3(&ctx.polygons[0]);

        setRGB0(
            &ctx.polygons[0],
            140,
            140,
            140
        );

        setXY3(
            &ctx.polygons[0],

            project_x(v0),
            project_y(v0),

            project_x(v1),
            project_y(v1),

            project_x(v2),
            project_y(v2)
        );

        /*
         * TRIÂNGULO 2
         */

        setPolyF3(&ctx.polygons[1]);

        setRGB0(
            &ctx.polygons[1],
            100,
            100,
            100
        );

        setXY3(
            &ctx.polygons[1],

            project_x(v0),
            project_y(v0),

            project_x(v2),
            project_y(v2),

            project_x(v3),
            project_y(v3)
        );

        /*
         * Os dois polígonos entram
         * no nível 1 da Ordering Table.
         */

        addPrim(
            &ctx.ot[1],
            &ctx.polygons[0]
        );

        addPrim(
            &ctx.ot[1],
            &ctx.polygons[1]
        );

        /*
         * A partir daqui usamos o último
         * nível da Ordering Table como
         * ponto inicial da GPU.
         */

        DrawOTag(
            &ctx.ot[OT_LENGTH - 1]
        );

        VSync(0);
    }

    return 0;
}
