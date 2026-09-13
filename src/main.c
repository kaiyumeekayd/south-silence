#include <stdint.h>

#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define OT_LEN 256
#define PACKET_LEN 4096

typedef struct
{
    DISPENV disp;
    DRAWENV draw;
    uint32_t ot[OT_LEN];
    char packet[PACKET_LEN];
} RenderBuffer;

RenderBuffer db[2];

int db_active = 0;
char *db_nextpri;


/*
 * ============================================================
 * Inicialização gráfica
 * ============================================================
 */

void init_graphics(void)
{
    ResetGraph(0);

    /*
     * Framebuffer 0
     */
    SetDefDispEnv(
        &db[0].disp,
        0,
        0,
        SCREEN_XRES,
        SCREEN_YRES
    );

    SetDefDrawEnv(
        &db[0].draw,
        SCREEN_XRES,
        0,
        SCREEN_XRES,
        SCREEN_YRES
    );

    setRGB0(
        &db[0].draw,
        8,
        8,
        12
    );

    db[0].draw.isbg = 1;


    /*
     * Framebuffer 1
     */
    SetDefDispEnv(
        &db[1].disp,
        SCREEN_XRES,
        0,
        SCREEN_XRES,
        SCREEN_YRES
    );

    SetDefDrawEnv(
        &db[1].draw,
        0,
        0,
        SCREEN_XRES,
        SCREEN_YRES
    );

    setRGB0(
        &db[1].draw,
        8,
        8,
        12
    );

    db[1].draw.isbg = 1;


    /*
     * Ordering Tables
     */
    ClearOTagR(
        db[0].ot,
        OT_LEN
    );

    ClearOTagR(
        db[1].ot,
        OT_LEN
    );


    db_nextpri = db[0].packet;


    /*
     * GTE
     */
    InitGeom();

    gte_SetGeomOffset(
        SCREEN_XRES >> 1,
        SCREEN_YRES >> 1
    );

    gte_SetGeomScreen(
        SCREEN_XRES >> 1
    );


    PutDrawEnv(
        &db[0].draw
    );

    PutDispEnv(
        &db[0].disp
    );

    SetDispMask(1);
}


/*
 * ============================================================
 * Troca de framebuffer
 * ============================================================
 */

void display(void)
{
    DrawSync(0);

    VSync(0);

    db_active ^= 1;

    db_nextpri =
        db[db_active].packet;

    ClearOTagR(
        db[db_active].ot,
        OT_LEN
    );

    PutDrawEnv(
        &db[db_active].draw
    );

    PutDispEnv(
        &db[db_active].disp
    );

    DrawOTag(
        db[1 - db_active].ot + (OT_LEN - 1)
    );
}


/*
 * ============================================================
 * South Silence - primeiro chão
 * ============================================================
 *
 * O chão é um retângulo formado por dois triângulos.
 *
 *        A----------------B
 *         \              /
 *          \            /
 *           \          /
 *            \        /
 *             C------D
 *
 * A e B ficam mais distantes.
 * C e D ficam mais próximos da câmera.
 *
 * Isso cria a perspectiva de um chão.
 */

int main(void)
{
    MATRIX matrix;

    SVECTOR rotation =
    {
        0,
        0,
        0,
        0
    };

    VECTOR position =
    {
        0,
        0,
        600
    };


    /*
     * Quatro cantos do chão.
     *
     * Y positivo = mais para cima.
     * Y negativo = mais para baixo.
     *
     * Z positivo = mais distante.
     */

    SVECTOR A =
    {
        -250,
         100,
         300,
         0
    };

    SVECTOR B =
    {
         250,
         100,
         300,
         0
    };

    SVECTOR C =
    {
        -250,
        -120,
        -250,
        0
    };

    SVECTOR D =
    {
         250,
        -120,
        -250,
        0
    };


    init_graphics();


    while (1)
    {
        POLY_F3 *poly;

        long depth;
        long depth2;


        /*
         * ====================================================
         * Matriz
         * ====================================================
         */

        RotMatrix(
            &rotation,
            &matrix
        );

        TransMatrix(
            &matrix,
            &position
        );

        gte_SetRotMatrix(
            &matrix
        );

        gte_SetTransMatrix(
            &matrix
        );


        /*
         * ====================================================
         * Primeiro triângulo
         *
         * A -> B -> C
         * ====================================================
         */

        poly =
            (POLY_F3 *)db_nextpri;

        setPolyF3(poly);

        setRGB0(
            poly,
            65,
            65,
            70
        );


        gte_ldv3(
            &A,
            &B,
            &C
        );

        gte_rtpt();


        gte_stsxy0(
            &poly->x0
        );

        gte_stsxy1(
            &poly->x1
        );

        gte_stsxy2(
            &poly->x2
        );


        gte_avsz3();

        gte_stotz(
            &depth
        );

        depth >>= 2;


        if (depth < 0)
            depth = 0;

        if (depth >= OT_LEN)
            depth = OT_LEN - 1;


        addPrim(
            db[db_active].ot + depth,
            poly
        );

        poly++;


        /*
         * ====================================================
         * Segundo triângulo
         *
         * B -> D -> C
         * ====================================================
         */

        setPolyF3(poly);

        setRGB0(
            poly,
            48,
            48,
            53
        );


        gte_ldv3(
            &B,
            &D,
            &C
        );

        gte_rtpt();


        gte_stsxy0(
            &poly->x0
        );

        gte_stsxy1(
            &poly->x1
        );

        gte_stsxy2(
            &poly->x2
        );


        gte_avsz3();

        gte_stotz(
            &depth2
        );

        depth2 >>= 2;


        if (depth2 < 0)
            depth2 = 0;

        if (depth2 >= OT_LEN)
            depth2 = OT_LEN - 1;


        addPrim(
            db[db_active].ot + depth2,
            poly
        );

        poly++;


        /*
         * Próximo espaço do packet buffer.
         */
        db_nextpri =
            (char *)poly;


        /*
         * Troca framebuffer.
         */
        display();
    }


    return 0;
}
