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
 * CÂMERA FIXA
 * ============================================================
 */

typedef struct
{
    SVECTOR rotation;
    VECTOR position;

} Camera;


/*
 * ============================================================
 * GRÁFICOS
 * ============================================================
 */

void init_graphics(void)
{
    ResetGraph(0);


    /*
     * FRAMEBUFFER 0
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
     * FRAMEBUFFER 1
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
     * ORDERING TABLE
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


    /*
     * TELA
     */

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
 * TROCA DE FRAMEBUFFER
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
 * CÂMERA
 * ============================================================
 */

void set_camera(Camera *camera)
{
    MATRIX matrix;


    RotMatrix(
        &camera->rotation,
        &matrix
    );


    TransMatrix(
        &matrix,
        &camera->position
    );


    gte_SetRotMatrix(
        &matrix
    );


    gte_SetTransMatrix(
        &matrix
    );
}


/*
 * ============================================================
 * TRIÂNGULO
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
     * GTE
     */

    gte_ldv3(
        a,
        b,
        c
    );


    gte_rtpt();


    /*
     * Coordenadas de tela
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
     * Profundidade
     */

    gte_avsz3();

    gte_stotz(
        &depth
    );


    depth >>= 8;


    if (depth < 0)
        depth = 0;


    if (depth >= OT_LEN)
        depth = OT_LEN - 1;


    /*
     * Ordering Table
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
 * QUADRILÁTERO
 * ============================================================
 */

void draw_quad(
    SVECTOR *a,
    SVECTOR *b,
    SVECTOR *c,
    SVECTOR *d,
    int r1,
    int g1,
    int b1,
    int r2,
    int g2,
    int b2
)
{
    draw_triangle(
        a,
        b,
        c,
        r1,
        g1,
        b1
    );


    draw_triangle(
        a,
        c,
        d,
        r2,
        g2,
        b2
    );
}


/*
 * ============================================================
 * SOUTH SILENCE
 *
 * PRIMEIRA SALA
 * ============================================================
 */

int main(void)
{
    /*
     * ========================================================
     * CÂMERA FIXA
     *
     * Câmera elevada olhando para o interior.
     *
     * 768 = inclinação aproximada para baixo.
     * ========================================================
     */

    Camera camera =
    {
        {
            768,
            0,
            0,
            0
        },

        {
            0,
            0,
            650
        }
    };


    /*
     * ========================================================
     * DIMENSÕES DA SALA
     * ========================================================
     *
     * X = largura
     * Y = altura
     * Z = profundidade
     *
     * Chão:
     *     Y = 150
     *
     * Teto:
     *     Y = -150
     *
     * Fundo:
     *     Z = 500
     *
     * Frente:
     *     Z = 0
     *
     * ========================================================
     */


    /*
     * ========================================================
     * CHÃO
     * ========================================================
     */

    SVECTOR floor_a =
    {
        -300,
         150,
           0,
        0
    };

    SVECTOR floor_b =
    {
         300,
         150,
           0,
        0
    };

    SVECTOR floor_c =
    {
         300,
         150,
         500,
        0
    };

    SVECTOR floor_d =
    {
        -300,
         150,
         500,
        0
    };


    /*
     * ========================================================
     * TETO
     * ========================================================
     */

    SVECTOR ceiling_a =
    {
        -300,
        -150,
           0,
        0
    };

    SVECTOR ceiling_b =
    {
         300,
        -150,
           0,
        0
    };

    SVECTOR ceiling_c =
    {
         300,
        -150,
         500,
        0
    };

    SVECTOR ceiling_d =
    {
        -300,
        -150,
         500,
        0
    };


    /*
     * ========================================================
     * PAREDE ESQUERDA
     * ========================================================
     */

    SVECTOR left_a =
    {
        -300,
         150,
           0,
        0
    };

    SVECTOR left_b =
    {
        -300,
         150,
         500,
        0
    };

    SVECTOR left_c =
    {
        -300,
        -150,
         500,
        0
    };

    SVECTOR left_d =
    {
        -300,
        -150,
           0,
        0
    };


    /*
     * ========================================================
     * PAREDE DIREITA
     * ========================================================
     */

    SVECTOR right_a =
    {
         300,
         150,
         500,
        0
    };

    SVECTOR right_b =
    {
         300,
         150,
           0,
        0
    };

    SVECTOR right_c =
    {
         300,
        -150,
           0,
        0
    };

    SVECTOR right_d =
    {
         300,
        -150,
         500,
        0
    };


    /*
     * ========================================================
     * PAREDE DO FUNDO
     *
     * A parede será dividida para criar a janela.
     * ========================================================
     */


    /*
     * Parte inferior da parede
     */

    SVECTOR back_bottom_a =
    {
        -300,
         150,
         500,
        0
    };

    SVECTOR back_bottom_b =
    {
         300,
         150,
         500,
        0
    };

    SVECTOR back_bottom_c =
    {
         300,
          50,
         500,
        0
    };

    SVECTOR back_bottom_d =
    {
        -300,
          50,
         500,
        0
    };


    /*
     * Parte superior
     */

    SVECTOR back_top_a =
    {
        -300,
         -50,
         500,
        0
    };

    SVECTOR back_top_b =
    {
         300,
         -50,
         500,
        0
    };

    SVECTOR back_top_c =
    {
         300,
        -150,
         500,
        0
    };

    SVECTOR back_top_d =
    {
        -300,
        -150,
         500,
        0
    };


    /*
     * Lado esquerdo da janela
     */

    SVECTOR back_window_left_a =
    {
        -300,
          50,
         500,
        0
    };

    SVECTOR back_window_left_b =
    {
        -100,
          50,
         500,
        0
    };

    SVECTOR back_window_left_c =
    {
        -100,
         -50,
         500,
        0
    };

    SVECTOR back_window_left_d =
    {
        -300,
         -50,
         500,
        0
    };


    /*
     * Lado direito da janela
     */

    SVECTOR back_window_right_a =
    {
         100,
          50,
         500,
        0
    };

    SVECTOR back_window_right_b =
    {
         300,
          50,
         500,
        0
    };

    SVECTOR back_window_right_c =
    {
         300,
         -50,
         500,
        0
    };

    SVECTOR back_window_right_d =
    {
         100,
         -50,
         500,
        0
    };


    /*
     * ========================================================
     * JANELA
     *
     * Colocada ligeiramente à frente da parede.
     * ========================================================
     */

    SVECTOR window_a =
    {
        -100,
          50,
         495,
        0
    };

    SVECTOR window_b =
    {
         100,
          50,
         495,
        0
    };

    SVECTOR window_c =
    {
         100,
         -50,
         495,
        0
    };

    SVECTOR window_d =
    {
        -100,
         -50,
         495,
        0
    };


    /*
     * ========================================================
     * MOLDURA ESQUERDA
     * ========================================================
     */

    SVECTOR frame_left_a =
    {
        -110,
          60,
         490,
        0
    };

    SVECTOR frame_left_b =
    {
        -100,
          60,
         490,
        0
    };

    SVECTOR frame_left_c =
    {
        -100,
         -60,
         490,
        0
    };

    SVECTOR frame_left_d =
    {
        -110,
         -60,
         490,
        0
    };


    /*
     * ========================================================
     * MOLDURA DIREITA
     * ========================================================
     */

    SVECTOR frame_right_a =
    {
         100,
          60,
         490,
        0
    };

    SVECTOR frame_right_b =
    {
         110,
          60,
         490,
        0
    };

    SVECTOR frame_right_c =
    {
         110,
         -60,
         490,
        0
    };

    SVECTOR frame_right_d =
    {
         100,
         -60,
         490,
        0
    };


    /*
     * ========================================================
     * MOLDURA SUPERIOR
     * ========================================================
     */

    SVECTOR frame_top_a =
    {
        -110,
          60,
         490,
        0
    };

    SVECTOR frame_top_b =
    {
         110,
          60,
         490,
        0
    };

    SVECTOR frame_top_c =
    {
         100,
          50,
         490,
        0
    };

    SVECTOR frame_top_d =
    {
        -100,
          50,
         490,
        0
    };


    /*
     * ========================================================
     * MOLDURA INFERIOR
     * ========================================================
     */

    SVECTOR frame_bottom_a =
    {
        -110,
         -60,
         490,
        0
    };

    SVECTOR frame_bottom_b =
    {
         110,
         -60,
         490,
        0
    };

    SVECTOR frame_bottom_c =
    {
         100,
         -50,
         490,
        0
    };

    SVECTOR frame_bottom_d =
    {
        -100,
         -50,
         490,
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
         * CÂMERA
         */

        set_camera(
            &camera
        );


        /*
         * ====================================================
         * CHÃO
         *
         * Marrom/cinza
         * ====================================================
 */

        draw_quad(
            &floor_a,
            &floor_b,
            &floor_c,
            &floor_d,

            75,
            65,
            55,

            55,
            48,
            42
        );


        /*
         * ====================================================
         * TETO
         * ====================================================
 */

        draw_quad(
            &ceiling_a,
            &ceiling_b,
            &ceiling_c,
            &ceiling_d,

            45,
            45,
            50,

            30,
            30,
            35
        );


        /*
         * ====================================================
         * PAREDE ESQUERDA
         * ====================================================
 */

        draw_quad(
            &left_a,
            &left_b,
            &left_c,
            &left_d,

            95,
            90,
            82,

            70,
            65,
            60
        );


        /*
         * ====================================================
         * PAREDE DIREITA
         * ====================================================
 */

        draw_quad(
            &right_a,
            &right_b,
            &right_c,
            &right_d,

            105,
            100,
            92,

            75,
            70,
            65
        );


        /*
         * ====================================================
         * PAREDE DO FUNDO
         *
         * PARTE INFERIOR
         * ====================================================
 */

        draw_quad(
            &back_bottom_a,
            &back_bottom_b,
            &back_bottom_c,
            &back_bottom_d,

            85,
            80,
            75,

            65,
            60,
            55
        );


        /*
         * ====================================================
         * PAREDE DO FUNDO
         *
         * PARTE SUPERIOR
         * ====================================================
 */

        draw_quad(
            &back_top_a,
            &back_top_b,
            &back_top_c,
            &back_top_d,

            85,
            80,
            75,

            65,
            60,
            55
        );


        /*
         * ====================================================
         * PAREDE DO FUNDO
         *
         * LADO ESQUERDO DA JANELA
         * ====================================================
 */

        draw_quad(
            &back_window_left_a,
            &back_window_left_b,
            &back_window_left_c,
            &back_window_left_d,

            85,
            80,
            75,

            65,
            60,
            55
        );


        /*
         * ====================================================
         * PAREDE DO FUNDO
         *
         * LADO DIREITO DA JANELA
         * ====================================================
 */

        draw_quad(
            &back_window_right_a,
            &back_window_right_b,
            &back_window_right_c,
            &back_window_right_d,

            85,
            80,
            75,

            65,
            60,
            55
        );


        /*
         * ====================================================
         * VIDRO DA JANELA
         * ====================================================
 */

        draw_quad(
            &window_a,
            &window_b,
            &window_c,
            &window_d,

            35,
            70,
            95,

            20,
            40,
            65
        );


        /*
         * ====================================================
         * MOLDURA ESQUERDA
         * ====================================================
 */

        draw_quad(
            &frame_left_a,
            &frame_left_b,
            &frame_left_c,
            &frame_left_d,

            35,
            30,
            28,

            25,
            22,
            20
        );


        /*
         * ====================================================
         * MOLDURA DIREITA
         * ====================================================
 */

        draw_quad(
            &frame_right_a,
            &frame_right_b,
            &frame_right_c,
            &frame_right_d,

            35,
            30,
            28,

            25,
            22,
            20
        );


        /*
         * ====================================================
         * MOLDURA SUPERIOR
         * ====================================================
 */

        draw_quad(
            &frame_top_a,
            &frame_top_b,
            &frame_top_c,
            &frame_top_d,

            35,
            30,
            28,

            25,
            22,
            20
        );


        /*
         * ====================================================
         * MOLDURA INFERIOR
         * ====================================================
 */

        draw_quad(
            &frame_bottom_a,
            &frame_bottom_b,
            &frame_bottom_c,
            &frame_bottom_d,

            35,
            30,
            28,

            25,
            22,
            20
        );


        /*
         * ====================================================
         * MOSTRA FRAME
         * ====================================================
 */

        display();
    }


    return 0;
}
