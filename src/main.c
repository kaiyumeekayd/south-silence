#include <stdint.h>
#include <psxgpu.h>
#include <psxetc.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define FOCAL_LENGTH 256
#define CAMERA_HEIGHT 0

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


/*
 * ============================================================
 * PROJEÇÃO 3D -> 2D
 * ============================================================
 *
 * Quanto maior o Z:
 *   - mais longe da câmera
 *   - menor o objeto na tela
 *
 * Quanto menor o Z:
 *   - mais perto da câmera
 *   - maior o objeto na tela
 */

static int project_x(Vertex3D v)
{
    return 160 + (int)((v.x * FOCAL_LENGTH) / v.z);
}

static int project_y(Vertex3D v)
{
    return 120
        - (int)((v.y * FOCAL_LENGTH) / v.z)
        + (int)((CAMERA_HEIGHT * FOCAL_LENGTH) / v.z);
}


/*
 * ============================================================
 * CRIA TRIÂNGULO
 * ============================================================
 */

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


/*
 * ============================================================
 * CUBO
 * ============================================================
 *
 * Frente: Z = 300
 * Trás:   Z = 500
 *
 * Isso significa que a parte de trás está realmente mais longe.
 *
 *
 *             v4 -------- v5
 *            /|           /|
 *           / |          / |
 *         v7--|---------v6 |
 *          |  |          | |
 *          |  v0 --------|-v1
 *          | /           | /
 *          |/            |/
 *         v3 ------------v2
 *
 */

int main(void)
{
    DISPENV disp;
    DRAWENV draw;

    RenderContext ctx;

    ResetGraph(0);

    /*
     * --------------------------------------------------------
     * DISPLAY
     * --------------------------------------------------------
     */

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

    /*
     * Fundo escuro
     */

    setRGB0(&draw, 12, 12, 16);

    draw.isbg = 1;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);


    /*
     * ========================================================
     * VÉRTICES DO CUBO
     * ========================================================
     *
     * Frente:
     * Z = 300
     *
     * Trás:
     * Z = 500
     *
     * A diferença de Z é o que cria a perspectiva.
     */

    Vertex3D v0 = { -60,  60, 300 };
    Vertex3D v1 = {  60,  60, 300 };
    Vertex3D v2 = {  60, -60, 300 };
    Vertex3D v3 = { -60, -60, 300 };

    Vertex3D v4 = { -60,  60, 500 };
    Vertex3D v5 = {  60,  60, 500 };
    Vertex3D v6 = {  60, -60, 500 };
    Vertex3D v7 = { -60, -60, 500 };


    while (1)
    {
        ClearOTagR(ctx.ot, OT_LENGTH);


        /*
         * ====================================================
         * FACE DA FRENTE
         * ====================================================
         */

        make_triangle(
            &ctx.polygons[0],
            v0, v1, v2,
            180, 180, 180
        );

        make_triangle(
            &ctx.polygons[1],
            v0, v2, v3,
            160, 160, 160
        );


        /*
         * ====================================================
         * FACE DE TRÁS
         * ====================================================
         */

        make_triangle(
            &ctx.polygons[2],
            v4, v6, v5,
            80, 80, 80
        );

        make_triangle(
            &ctx.polygons[3],
            v4, v7, v6,
            70, 70, 70
        );


        /*
         * ====================================================
         * LADO DIREITO
         * ====================================================
         */

        make_triangle(
            &ctx.polygons[4],
            v1, v5, v6,
            130, 130, 130
        );

        make_triangle(
            &ctx.polygons[5],
            v1, v6, v2,
            115, 115, 115
        );


        /*
         * ====================================================
         * LADO ESQUERDO
         * ====================================================
         */

        make_triangle(
            &ctx.polygons[6],
            v4, v0, v3,
            100, 100, 100
        );

        make_triangle(
            &ctx.polygons[7],
            v4, v3, v7,
            90, 90, 90
        );


        /*
         * ====================================================
         * TOPO
         * ====================================================
         */

        make_triangle(
            &ctx.polygons[8],
            v4, v5, v1,
            150, 150, 150
        );

        make_triangle(
            &ctx.polygons[9],
            v4, v1, v0,
            140, 140, 140
        );


        /*
         * ====================================================
         * BASE
         * ====================================================
         */

        make_triangle(
            &ctx.polygons[10],
            v3, v2, v6,
            60, 60, 60
        );

        make_triangle(
            &ctx.polygons[11],
            v3, v6, v7,
            50, 50, 50
        );


        /*
         * ====================================================
         * ORDERING TABLE
         * ====================================================
         */

        for (int i = 0; i < 12; i++)
        {
            addPrim(
                &ctx.ot[12],
                &ctx.polygons[i]
            );
        }


        /*
         * Desenha a Ordering Table
         */

        DrawOTag(&ctx.ot[OT_LENGTH - 1]);

        VSync(0);
    }

    return 0;
}
