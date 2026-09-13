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
     * QUADRILÁTERO 3D
     *
     * Os dois pontos da esquerda
     * estão mais próximos.
     *
     * Os dois pontos da direita
     * estão mais distantes.
     *
     * Isso deve produzir perspectiva
     * visível na tela.
     */

    Vertex3D v0 = { -100,  70, 250 };
    Vertex3D v1 = {  100,  70, 450 };
    Vertex3D v2 = {  100, -70, 450 };
    Vertex3D v3 = { -100, -70, 250 };

    while (1)
    {
        ClearOTagR(ctx.ot, OT_LENGTH);

        /*
         * Primeiro triângulo
         */
        make_triangle(
            &ctx.polygons[0],
            v0, v1, v2,
            180, 180, 180
        );

        /*
         * Segundo triângulo
         */
        make_triangle(
            &ctx.polygons[1],
            v0, v2, v3,
            100, 100, 100
        );

        addPrim(&ctx.ot[12], &ctx.polygons[0]);
        addPrim(&ctx.ot[12], &ctx.polygons[1]);

        DrawOTag(&ctx.ot[OT_LENGTH - 1]);

        VSync(0);
    }

    return 0;
}
