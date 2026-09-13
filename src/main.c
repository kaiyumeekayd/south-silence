#include <stdint.h>

#include <psxgpu.h>
#include <psxgte.h>
#include <psxetc.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define OT_LENGTH 16

typedef struct
{
    POLY_F3 poly;
    uint32_t ot[OT_LENGTH];
} RenderContext;

static RenderContext ctx;

int main(void)
{
    DISPENV disp;
    DRAWENV draw;

    /* Inicializa GPU */
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

    /* Fundo escuro */
    setRGB0(&draw, 12, 12, 16);
    draw.isbg = 1;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);

    /*
     * Inicializa o GTE.
     *
     * O GTE é o responsável pelas transformações
     * 3D e pela perspectiva no PlayStation.
     */
    InitGeom();

    /*
     * Centro da tela.
     */
    SetGeomOffset(
        SCREEN_WIDTH / 2,
        SCREEN_HEIGHT / 2
    );

    /*
     * Distância focal.
     */
    SetGeomScreen(256);

    /*
     * Matriz de rotação = identidade.
     *
     * Portanto, nosso triângulo inicialmente
     * não possui rotação.
     */
    MATRIX matrix;

    matrix.m[0][0] = 4096;
    matrix.m[0][1] = 0;
    matrix.m[0][2] = 0;

    matrix.m[1][0] = 0;
    matrix.m[1][1] = 4096;
    matrix.m[1][2] = 0;

    matrix.m[2][0] = 0;
    matrix.m[2][1] = 0;
    matrix.m[2][2] = 4096;

    matrix.t[0] = 0;
    matrix.t[1] = 0;
    matrix.t[2] = 0;

    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);

    /*
     * Três vértices 3D.
     *
     * O eixo Z representa a distância da câmera.
     */
    SVECTOR v0 = { -100, -70, 500, 0 };
    SVECTOR v1 = {  100, -70, 500, 0 };
    SVECTOR v2 = {    0, 100, 700, 0 };

    long p;
    long flag;

    int16_t x0;
    int16_t y0;
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;

    while (1)
    {
        ClearOTagR(ctx.ot, OT_LENGTH);

        /*
         * Transformação 3D + perspectiva.
         *
         * O GTE transforma os três vértices
         * e gera suas coordenadas na tela.
         */
        gte_ldv3(
            &v0,
            &v1,
            &v2
        );

        gte_rtpt();

        gte_stsxy0(&ctx.poly.x0);
        gte_stsxy1(&ctx.poly.x1);
        gte_stsxy2(&ctx.poly.x2);

        /*
         * Triângulo flat-shaded.
         */
        setPolyF3(&ctx.poly);

        setRGB0(
            &ctx.poly,
            180,
            180,
            180
        );

        /*
         * Coloca o triângulo na Ordering Table.
         */
        addPrim(
            &ctx.ot[8],
            &ctx.poly
        );

        /*
         * Envia para a GPU.
         */
        DrawOTag(
            &ctx.ot[OT_LENGTH - 1]
        );

        VSync(0);
    }

    return 0;
}
