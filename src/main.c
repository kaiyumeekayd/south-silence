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
    Vertex3D c,
    int r,
    int g,
    int bcol
)
{
    POLY_F3 poly;

    setPolyF3(&poly);

    setRGB0(&poly, r, g, bcol);

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
     * CUBO 3D
     *
     * Frente: Z = 300
     * Trás:   Z = 500
     *
     * O fundo está mais distante,
     * portanto aparece menor.
     */

    Vertex3D v[8] =
    {
        /* frente */
        { -70, -70, 300 },
        {  70, -70, 300 },
        {  70,  70, 300 },
        { -70,  70, 300 },

        /* trás */
        { -70, -70, 500 },
        {  70, -70, 500 },
        {  70,  70, 500 },
        { -70,  70, 500 }
    };

    uint32_t ot[1];

    while (1)
    {
        ClearOTagR(ot, 1);

        /*
         * Frente
         */

        draw_triangle(
            ot,
            v[0], v[1], v[2],
            120, 120, 120
        );

        draw_triangle(
            ot,
            v[0], v[2], v[3],
            120, 120, 120
        );

        /*
         * Trás
         */

        draw_triangle(
            ot,
            v[4], v[6], v[5],
            70, 70, 70
        );

        draw_triangle(
            ot,
            v[4], v[7], v[6],
            70, 70, 70
        );

        /*
         * Lado esquerdo
         */

        draw_triangle(
            ot,
            v[0], v[3], v[7],
            90, 90, 90
        );

        draw_triangle(
            ot,
            v[0], v[7], v[4],
            90, 90, 90
        );

        /*
         * Lado direito
         */

        draw_triangle(
            ot,
            v[1], v[5], v[6],
            100, 100, 100
        );

        draw_triangle(
            ot,
            v[1], v[6], v[2],
            100, 100, 100
        );

        /*
         * Topo
         */

        draw_triangle(
            ot,
            v[3], v[2], v[6],
            110, 110, 110
        );

        draw_triangle(
            ot,
            v[3], v[6], v[7],
            110, 110, 110
        );

        /*
         * Baixo
         */

        draw_triangle(
            ot,
            v[0], v[4], v[5],
            60, 60, 60
        );

        draw_triangle(
            ot,
            v[0], v[5], v[1],
            60, 60, 60
        );

        DrawOTag(&ot[0]);

        VSync(0);
    }

    return 0;
}
