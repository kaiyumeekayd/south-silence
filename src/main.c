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
    POLY_F3 polygons[12];
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
     * CUBO 3D
     *
     * Frente: Z = 350
     * Trás:   Z = 500
     */

    Vertex3D v[8] =
    {
        /* Frente */
        { -70,  70, 350 },
        {  70,  70, 350 },
        {  70, -70, 350 },
        { -70, -70, 350 },

        /* Trás */
        { -70,  70, 500 },
        {  70,  70, 500 },
        {  70, -70, 500 },
        { -70, -70, 500 }
    };

    while (1)
    {
        ClearOTagR(ctx.ot, OT_LENGTH);

        /*
         * FACE DA FRENTE
         */

        make_triangle(
            &ctx.polygons[0],
            v[0], v[1], v[2],
            140, 140, 140
        );

        make_triangle(
            &ctx.polygons[1],
            v[0], v[2], v[3],
            140, 140, 140
        );

        /*
         * FACE DE TRÁS
         */

        make_triangle(
            &ctx.polygons[2],
            v[4], v[6], v[5],
            60, 60, 60
        );

        make_triangle(
            &ctx.polygons[3],
            v[4], v[7], v[6],
            60, 60, 60
        );

        /*
         * LADO ESQUERDO
         */

        make_triangle(
            &ctx.polygons[4],
            v[0], v[3], v[7],
            90, 90, 90
        );

        make_triangle(
            &ctx.polygons[5],
            v[0], v[7], v[4],
            90, 90, 90
        );

        /*
         * LADO DIREITO
         */

        make_triangle(
            &ctx.polygons[6],
            v[1], v[5], v[6],
            110, 110, 110
        );

        make_triangle(
            &ctx.polygons[7],
            v[1], v[6], v[2],
            110, 110, 110
        );

        /*
         * TOPO
         */

        make_triangle(
            &ctx.polygons[8],
            v[0], v[4], v[5],
            120, 120, 120
        );

        make_triangle(
            &ctx.polygons[9],
            v[0], v[5], v[1],
            120, 120, 120
        );

        /*
         * BASE
         */

        make_triangle(
            &ctx.polygons[10],
            v[3], v[2], v[6],
            70, 70, 70
        );

        make_triangle(
            &ctx.polygons[11],
            v[3], v[6], v[7],
            70, 70, 70
        );

        /*
         * Coloca os 12 polígonos na OT.
         */

        for (int i = 0; i < 12; i++)
        {
            addPrim(
                &ctx.ot[1],
                &ctx.polygons[i]
            );
        }

        DrawOTag(
            &ctx.ot[OT_LENGTH - 1]
        );

        VSync(0);
    }

    return 0;
}
