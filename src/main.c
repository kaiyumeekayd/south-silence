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
    POLY_F3 polygons[6];
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

static void make_triangle(
    POLY_F3 *poly,
    Vertex3D a,
    Vertex3D b,
    Vertex3D c,
    int r,
    int g,
    int bcol
)
{
    setPolyF3(poly);

    setRGB0(poly, r, g, bcol);

    setXY3(
        poly,
        project_x(a), project_y(a),
        project_x(b), project_y(b),
        project_x(c), project_y(c)
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

    /*
     * CUBO
     *
     *              4 -------- 5
     *             /|         /|
     *            / |        / |
     *           7 -------- 6  |
     *           |  |       |  |
     *           |  0 ------|--1
     *           | /        | /
     *           |/         |/
     *           3 -------- 2
     *
     * 0-1-2-3 = frente
     * 4-5-6-7 = trás
     */

    Vertex3D v[8] =
    {
        /* Frente */
        { -70,  70, 300 },  /* 0 */
        {  70,  70, 300 },  /* 1 */
        {  70, -70, 300 },  /* 2 */
        { -70, -70, 300 },  /* 3 */

        /* Trás */
        { -70,  70, 500 },  /* 4 */
        {  70,  70, 500 },  /* 5 */
        {  70, -70, 500 },  /* 6 */
        { -70, -70, 500 }   /* 7 */
    };

    while (1)
    {
        ClearOTagR(ctx.ot, OT_LENGTH);

        /*
         * TOPO
         *
         * Dois triângulos.
         */
        make_triangle(
            &ctx.polygons[0],
            v[0], v[5], v[4],
            110, 110, 110
        );

        make_triangle(
            &ctx.polygons[1],
            v[0], v[1], v[5],
            110, 110, 110
        );

        /*
         * LADO DIREITO
         *
         * Dois triângulos.
         */
        make_triangle(
            &ctx.polygons[2],
            v[1], v[6], v[5],
            70, 70, 70
        );

        make_triangle(
            &ctx.polygons[3],
            v[1], v[2], v[6],
            70, 70, 70
        );

        /*
         * FRENTE
         *
         * Dois triângulos.
         */
        make_triangle(
            &ctx.polygons[4],
            v[0], v[1], v[2],
            170, 170, 170
        );

        make_triangle(
            &ctx.polygons[5],
            v[0], v[2], v[3],
            170, 170, 170
        );

        /*
         * Faces mais distantes primeiro.
         */

        addPrim(&ctx.ot[14], &ctx.polygons[0]);
        addPrim(&ctx.ot[14], &ctx.polygons[1]);

        addPrim(&ctx.ot[13], &ctx.polygons[2]);
        addPrim(&ctx.ot[13], &ctx.polygons[3]);

        /*
         * Frente por último.
         */

        addPrim(&ctx.ot[12], &ctx.polygons[4]);
        addPrim(&ctx.ot[12], &ctx.polygons[5]);

        DrawOTag(&ctx.ot[OT_LENGTH - 1]);

        VSync(0);
    }

    return 0;
}
