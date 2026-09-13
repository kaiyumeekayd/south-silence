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
 * CÂMERA
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
 * TRIÂNGULO 3D
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


    /*
     * Reserva espaço para o polígono.
     */

    poly =
        (POLY_F3 *)db_nextpri;


    /*
     * Configura o triângulo.
     */

    setPolyF3(poly);


    setRGB0(
        poly,
        r,
        g,
        bcolor
    );


    /*
     * Envia os vértices para o GTE.
     */

    gte_ldv3(
        a,
        b,
        c
    );


    /*
     * Transformação 3D + perspectiva.
     */

    gte_rtpt();


    /*
     * Coordenadas na tela.
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
     * Profundidade média.
     */

    gte_avsz3();

    gte_stotz(
        &depth
    );


    /*
     * Converte para a Ordering Table.
     */

    depth >>= 8;


    if (depth < 0)
        depth = 0;


    if (depth >= OT_LEN)
        depth = OT_LEN - 1;


    /*
     * Adiciona à Ordering Table.
     */

    addPrim(
        db[db_active].ot + depth,
        poly
    );


    /*
     * Próximo polígono.
     */

    db_nextpri =
        (char *)(poly + 1);
}


/*
 * ============================================================
 * QUADRILÁTERO
 *
 * Formado por dois triângulos.
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
 * ============================================================
 */

int main(void)
{
    /*
     * ========================================================
     * CÂMERA
     * ========================================================
     */

    Camera camera =
    {
        {
            0,
            256,
            0,
            0
        },

        {
            0,
            0,
            600
        }
    };


    /*
     * ========================================================
     * CHÃO
     *
     * TESTE DE POSIÇÃO VERTICAL
     *
     * Antes: Y = 120
     * Agora: Y = 60
     * ========================================================
     */

    SVECTOR floor_a =
    {
        -250,
         60,
         300,
        0
    };


    SVECTOR floor_b =
    {
         250,
         60,
         300,
        0
    };


    SVECTOR floor_c =
    {
         250,
         60,
        -250,
        0
    };


    SVECTOR floor_d =
    {
        -250,
         60,
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
         * CÂMERA
         */

        set_camera(
            &camera
        );


        /*
         * ====================================================
         * CHÃO
         *
         * AZUL
         * ====================================================
 */

        draw_quad(
            &floor_a,
            &floor_b,
            &floor_c,
            &floor_d,

            40,
            80,
            220,

            20,
            40,
            140
        );


        /*
         * ====================================================
         * PAREDE DE FUNDO
         *
         * VERMELHA
         * ====================================================
 */

        draw_quad(
            &back_a,
            &back_b,
            &back_c,
            &back_d,

            220,
            40,
            40,

            140,
            20,
            20
        );


        /*
         * ====================================================
         * PAREDE ESQUERDA
         *
         * VERDE
         * ====================================================
 */

        draw_quad(
            &left_a,
            &left_b,
            &left_c,
            &left_d,

            40,
            200,
            70,

            20,
            110,
            35
        );


        /*
         * ====================================================
         * PAREDE DIREITA
         *
         * AMARELA
         * ====================================================
 */

        draw_quad(
            &right_a,
            &right_b,
            &right_c,
            &right_d,

            230,
            200,
            40,

            150,
            120,
            20
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
