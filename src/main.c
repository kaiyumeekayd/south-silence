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

    /* -------------------------
       GPU
       ------------------------- */

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

    setRGB0(
        &draw,
        12,
        12,
        16
    );

    draw.isbg = 1;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);


    /* -------------------------
       GTE
       ------------------------- */

    InitGeom();

    SetGeomOffset(
        SCREEN_WIDTH / 2,
        SCREEN_HEIGHT / 2
    );

    SetGeomScreen(256);


    /* -------------------------
       3D transformation
       ------------------------- */

    SVECTOR rotation = {0, 0, 0, 0};

    VECTOR translation = {
        0,
        0,
        0,
        0
    };

    MATRIX matrix;

    RotMatrix(
        &rotation,
        &matrix
    );

    TransMatrix(
        &matrix,
        &translation
    );

    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);


    /* -------------------------
       Triangle vertices
       ------------------------- */

    SVECTOR v0 = {
        -100,
        -70,
        500,
        0
    };

    SVECTOR v1 = {
         100,
        -70,
        500,
        0
    };

    SVECTOR v2 = {
           0,
         100,
        700,
        0
    };


    while (1)
    {
        long p;
        long flag;

        long otz0;
        long otz1;
        long otz2;

        long x0;
        long x1;
        long x2;

        /* Limpa Ordering Table */
        ClearOTagR(
            ctx.ot,
            OT_LENGTH
        );


        /* Inicializa triângulo */
        setPolyF3(&ctx.poly);

        setRGB0(
            &ctx.poly,
            180,
            180,
            180
        );


        /*
         * Transformação 3D + perspectiva.
         *
         * O GTE transforma cada vértice
         * diretamente para coordenadas
         * de tela.
         */

        otz0 = RotTransPers(
            &v0,
            &x0,
            &p,
            &flag
        );

        otz1 = RotTransPers(
            &v1,
            &x1,
            &p,
            &flag
        );

        otz2 = RotTransPers(
            &v2,
            &x2,
            &p,
            &flag
        );


        /*
         * RotTransPers retorna as
         * coordenadas X/Y empacotadas
         * em um long.
         */

        ctx.poly.x0 = x0 & 0xFFFF;
        ctx.poly.y0 = (x0 >> 16) & 0xFFFF;

        ctx.poly.x1 = x1 & 0xFFFF;
        ctx.poly.y1 = (x1 >> 16) & 0xFFFF;

        ctx.poly.x2 = x2 & 0xFFFF;
        ctx.poly.y2 = (x2 >> 16) & 0xFFFF;


        /*
         * Usa a profundidade média
         * para ordenar o triângulo.
         */

        long otz =
            (otz0 + otz1 + otz2) / 3;

        if (otz < 1)
            otz = 1;

        if (otz >= OT_LENGTH)
            otz = OT_LENGTH - 1;


        addPrim(
            &ctx.ot[otz],
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
