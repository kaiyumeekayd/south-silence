#include <stdint.h>
#include <psxgpu.h>
#include <psxetc.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define FOCAL_LENGTH 256

typedef struct
{
    long x;
    long y;
    long z;
} Vertex3D;

static int project_x(Vertex3D v)
{
    return 160 + (int)((v.x * FOCAL_LENGTH) / v.z);
}

static int project_y(Vertex3D v)
{
    return 120 - (int)((v.y * FOCAL_LENGTH) / v.z);
}

static void draw_triangle(
    uint32_t *ot,
    Vertex3D a,
    Vertex3D b,
    Vertex3D c
)
{
    POLY_F3 poly;

    setPolyF3(&poly);

    setRGB0(&poly, 130, 130, 130);

    setXY3(
        &poly,
        project_x(a), project_y(a),
        project_x(b), project_y(b),
        project_x(c), project_y(c)
    );

    addPrim(&ot[0], &poly);
}

int main(void)
{
    DISPENV disp;
    DRAWENV draw;

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
     * QUADRADO 3D
     *
     * Os quatro pontos estão no espaço 3D.
     *
     * Os pontos superiores estão mais longe
     * do centro da câmera.
     */

    Vertex3D v0 = { -90,  70, 350 };
    Vertex3D v1 = {  90,  70, 350 };
    Vertex3D v2 = {  70, -70, 500 };
    Vertex3D v3 = { -70, -70, 500 };

    uint32_t ot[1];

    while (1)
    {
        ClearOTagR(ot, 1);

        /*
         * Primeira metade do quadrado
         */
        draw_triangle(
            ot,
            v0,
            v1,
            v2
        );

        /*
         * Segunda metade do quadrado
         */
        draw_triangle(
            ot,
            v0,
            v2,
            v3
        );

        DrawOTag(&ot[0]);

        VSync(0);
    }

    return 0;
}
