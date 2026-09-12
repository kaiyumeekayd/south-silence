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

int main(void)
{
    DISPENV disp;
    DRAWENV draw;

    /*
     * Os polígonos precisam continuar existindo
     * até DrawOTag().
     */
    POLY_F3 poly0;
    POLY_F3 poly1;

    uint32_t ot[1];

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
     * QUATRO VÉRTICES EM 3D
     *
     * Frente:
     * v0 ---- v1
     *
     * Fundo:
     * v3 ---- v2
     *
     * O fundo está em Z=500,
     * portanto aparece menor.
     */

    Vertex3D v0 = { -90,  70, 350 };
    Vertex3D v1 = {  90,  70, 350 };
    Vertex3D v2 = {  70, -70, 500 };
    Vertex3D v3 = { -70, -70, 500 };

    while (1)
    {
        ClearOTagR(ot, 1);

        /*
         * Primeiro triângulo
         */

        setPolyF3(&poly0);

        setRGB0(
            &poly0,
            140,
            140,
            140
        );

        setXY3(
            &poly0,

            project_x(v0),
            project_y(v0),

            project_x(v1),
            project_y(v1),

            project_x(v2),
            project_y(v2)
        );

        /*
         * Segundo triângulo
         */

        setPolyF3(&poly1);

        setRGB0(
            &poly1,
            100,
            100,
            100
        );

        setXY3(
            &poly1,

            project_x(v0),
            project_y(v0),

            project_x(v2),
            project_y(v2),

            project_x(v3),
            project_y(v3)
        );

        /*
         * Agora os dois polígonos ainda existem
         * quando DrawOTag() for executado.
         */

        addPrim(&ot[0], &poly0);
        addPrim(&ot[0], &poly1);

        DrawOTag(&ot[0]);

        VSync(0);
    }

    return 0;
}
