#include <stdint.h>

#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define OT_LEN 256
#define PACKET_LEN 8192

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
     * Inicialização do GTE.
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
 * Desenha um triângulo 3D
 * ============================================================
 */

void draw_triangle(
    SVECTOR *a,
    SVECTOR *b,
    SVECTOR *c,
    int r,
    int g,
    int bcolor
)
{
    POLY_F3 *poly;
    long depth;


    poly =
        (POLY_F3 *)db_nextpri;


    setPolyF3(poly);

    setRGB0(
        poly,
        r,
        g,
        bcolor
    );


    /*
     * Carrega os três vértices no GTE.
     */
    gte_ldv3(
        a,
        b,
        c
    );


    /*
     * Rotação + translação + perspectiva.
     */
    gte_rtpt();


    /*
     * Coordenadas projetadas na tela.
     */
    gte_stsxy0(
        &poly->x0
    );

    gte_stsxy1(
        &poly->x1
    );

    gte_stsxy2(
        &poly->x2
    );


    /*
     * Calcula profundidade média.
     */
    gte_avsz3();

    gte_stotz(
        &depth
    );

    depth >>= 2;


    if (depth < 0)
        depth = 0;

    if (depth >= OT_LEN)
        depth = OT_LEN - 1;


    /*
     * Coloca o triângulo na Ordering Table.
     */
    addPrim(
        db[db_active].ot + depth,
        poly
    );


    db_nextpri =
        (char *)(poly + 1);
}


/*
 * ============================================================
 * Desenha um retângulo 3D usando dois triângulos
 * ============================================================
 *
 * A -------- B
 * |          |
 * |          |
 * D -------- C
 *
 * Triângulos:
 *
 * A-B-C
 * A-C-D
 */

void draw_wall(
    SVECTOR *a,
    SVECTOR *b,
    SVECTOR *c,
    SVECTOR *d,
    int r,
    int g,
    int bcolor
)
{
    draw_triangle(
        a,
        b,
        c,
        r,
        g,
        bcolor
    );

    draw_triangle(
        a,
        c,
        d,
        r - 15,
        g - 15,
        bcolor - 15
    );
}


/*
 * ============================================================
 * SOUTH SILENCE
 * ============================================================
 */

int main(void)
{
    MATRIX matrix;


    /*
     * ========================================================
     * ROTAÇÃO DA CÂMERA/ENQUADRAMENTO
     *
     * 0      = visão original
     * 256    = pequena rotação no eixo Y
     *
     * Estamos usando valores fixos primeiro.
     * Depois transformaremos isso em um sistema de câmera
     * propriamente dito.
     * ========================================================
     */

    SVECTOR rotation =
    {
        0,
        256,
        0,
        0
    };


    /*
     * Translação da cena em relação ao ponto de visão.
     */
    VECTOR position =
    {
        0,
        0,
        600
    };


    /*
     * ========================================================
     * CHÃO
     * ========================================================
     */

    SVECTOR floor_a =
    {
        -250,
        -120,
        300,
        0
    };

    SVECTOR floor_b =
    {
         250,
        -120,
        300,
        0
    };

    SVECTOR floor_c =
    {
         250,
        -120,
        -250,
        0
    };

    SVECTOR floor_d =
    {
        -250,
        -120,
        -250,
        0
    };


    /*
     * ========================================================
     * PAREDE DE FUNDO
     * ========================================================
     */

    SVECTOR back_a =
    {
        -250,
        -120,
        300,
        0
    };

    SVECTOR back_b =
    {
         250,
        -120,
        300,
        0
    };

    SVECTOR back_c =
    {
         250,
         180,
        300,
        0
    };

    SVECTOR back_d =
    {
        -250,
         180,
        300,
        0
    };


    /*
     * ========================================================
     * PAREDE ESQUERDA
     * ========================================================
     */

    SVECTOR left_a =
    {
        -250,
        -120,
        -250,
        0
    };

    SVECTOR left_b =
    {
        -250,
        -120,
         300,
        0
    };

    SVECTOR left_c =
    {
        -250,
         180,
         300,
        0
    };

    SVECTOR left_d =
    {
        -250,
         180,
        -250,
        0
    };


    /*
     * ========================================================
     * PAREDE DIREITA
     * ========================================================
     */

    SVECTOR right_a =
    {
         250,
        -120,
         300,
        0
    };

    SVECTOR right_b =
    {
         250,
        -120,
        -250,
        0
    };

    SVECTOR right_c =
    {
         250,
         180,
        -250,
        0
    };

    SVECTOR right_d =
    {
         250,
         180,
         300,
        0
    };


    /*
     * ========================================================
     * INICIALIZAÇÃO
     * ========================================================
     */

    init_graphics();


    /*
     * ========================================================
     * LOOP PRINCIPAL
     * ========================================================
     */

    while (1)
    {
        /*
         * Cria a matriz de rotação.
         */
        RotMatrix(
            &rotation,
            &matrix
        );


        /*
         * Adiciona a posição da cena.
         */
        TransMatrix(
            &matrix,
            &position
        );


        /*
         * Envia a matriz para o GTE.
         */
        gte_SetRotMatrix(
            &matrix
        );

        gte_SetTransMatrix(
            &matrix
        );


        /*
         * ====================================================
         * CHÃO
         * ====================================================
         */

        draw_wall(
            &floor_a,
            &floor_b,
            &floor_c,
            &floor_d,
            55,
            55,
            60
        );


        /*
         * ====================================================
         * PAREDE DE FUNDO
         * ====================================================
         */

        draw_wall(
            &back_a,
            &back_b,
            &back_c,
            &back_d,
            75,
            75,
            82
        );


        /*
         * ====================================================
         * PAREDE ESQUERDA
         * ====================================================
         */

        draw_wall(
            &left_a,
            &left_b,
            &left_c,
            &left_d,
            60,
            60,
            68
        );


        /*
         * ====================================================
         * PAREDE DIREITA
         * ====================================================
         */

        draw_wall(
            &right_a,
            &right_b,
            &right_c,
            &right_d,
            45,
            45,
            52
        );


        /*
         * ====================================================
         * MOSTRA O FRAME
         * ====================================================
         */

        display();
    }


    return 0;
}
