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


/* ---------------------------------------------------------
   CÂMERA
   --------------------------------------------------------- */

typedef struct
{
    SVECTOR rotation;
    VECTOR position;

} Camera;


/* ---------------------------------------------------------
   INICIALIZAÇÃO GRÁFICA
   --------------------------------------------------------- */

void init_graphics(void)
{
    ResetGraph(0);

    /* Buffer 0 */

    SetDefDispEnv(&db[0].disp, 0, 0,
                  SCREEN_XRES, SCREEN_YRES);

    SetDefDrawEnv(&db[0].draw, SCREEN_XRES, 0,
                  SCREEN_XRES, SCREEN_YRES);

    db[0].draw.isbg = 1;

    setRGB0(&db[0].draw, 8, 8, 12);


    /* Buffer 1 */

    SetDefDispEnv(&db[1].disp,
                  SCREEN_XRES, 0,
                  SCREEN_XRES, SCREEN_YRES);

    SetDefDrawEnv(&db[1].draw,
                  0, 0,
                  SCREEN_XRES, SCREEN_YRES);

    db[1].draw.isbg = 1;

    setRGB0(&db[1].draw, 8, 8, 12);


    ClearOTagR(db[0].ot, OT_LEN);

    ClearOTagR(db[1].ot, OT_LEN);


    InitGeom();

    gte_SetGeomOffset(
        SCREEN_XRES / 2,
        SCREEN_YRES / 2
    );

    gte_SetGeomScreen(
        SCREEN_XRES / 2
    );


    PutDrawEnv(&db[0].draw);
    PutDispEnv(&db[0].disp);

    SetDispMask(1);

    db_active = 0;
}


/* ---------------------------------------------------------
   TROCA DE BUFFER
   --------------------------------------------------------- */

void display(void)
{
    DrawSync(0);

    VSync(0);

    db_active ^= 1;

    PutDrawEnv(&db[db_active].draw);
    PutDispEnv(&db[db_active].disp);

    ClearOTagR(
        db[db_active].ot,
        OT_LEN
    );

    db_nextpri = db[db_active].packet;

    DrawOTag(
        db[db_active ^ 1].ot + OT_LEN - 1
    );
}


/* ---------------------------------------------------------
   CÂMERA
   --------------------------------------------------------- */

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

    gte_SetRotMatrix(&matrix);

    gte_SetTransMatrix(&matrix);
}


/* ---------------------------------------------------------
   TRIÂNGULO
   --------------------------------------------------------- */

void draw_triangle(
    SVECTOR *a,
    SVECTOR *b,
    SVECTOR *c,
    uint8_t r,
    uint8_t g,
    uint8_t bl
)
{
    POLY_F3 *poly;

    int depth;


    poly = (POLY_F3 *)db_nextpri;

    setPolyF3(poly);

    setRGB0(
        poly,
        r,
        g,
        bl
    );


    gte_ldv3(
        a,
        b,
        c
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

    gte_stotz(&depth);

    depth >>= 8;


    if (depth < 0)
        depth = 0;

    if (depth >= OT_LEN)
        depth = OT_LEN - 1;


    addPrim(
        db[db_active].ot + depth,
        poly
    );


    db_nextpri += sizeof(POLY_F3);
}


/* ---------------------------------------------------------
   QUADRILÁTERO
   --------------------------------------------------------- */

void draw_quad(
    SVECTOR *a,
    SVECTOR *b,
    SVECTOR *c,
    SVECTOR *d,
    uint8_t r,
    uint8_t g,
    uint8_t bl
)
{
    draw_triangle(
        a,
        b,
        c,
        r,
        g,
        bl
    );

    draw_triangle(
        a,
        c,
        d,
        r,
        g,
        bl
    );
}


/* ---------------------------------------------------------
   MAIN
   --------------------------------------------------------- */

int main(void)
{
    Camera camera =
    {
        /*
         * Esta é a rotação que já havia
         * demonstrado o piso corretamente.
         *
         * 256 = 22,5 graus aproximadamente.
         */
        {
            256,
            0,
            0,
            0
        },

        /*
         * Mantemos os objetos à frente
         * do ponto de projeção.
         */
        {
            0,
            0,
            650
        }
    };


    /*
     * PISO
     *
     * Grande retângulo horizontal.
     */

    SVECTOR floor_a = {
        -300,
        120,
        0
    };

    SVECTOR floor_b = {
         300,
        120,
        0
    };

    SVECTOR floor_c = {
         300,
        120,
        500
    };

    SVECTOR floor_d = {
        -300,
        120,
        500
    };


    /*
     * PAREDE DO FUNDO
     */

    SVECTOR back_a = {
        -300,
        -120,
        500
    };

    SVECTOR back_b = {
         300,
        -120,
        500
    };

    SVECTOR back_c = {
         300,
         120,
         500
    };

    SVECTOR back_d = {
        -300,
         120,
         500
    };


    /*
     * PAREDE LATERAL ESQUERDA
     */

    SVECTOR side_a = {
        -300,
        -120,
        0
    };

    SVECTOR side_b = {
        -300,
        -120,
        500
    };

    SVECTOR side_c = {
        -300,
         120,
         500
    };

    SVECTOR side_d = {
        -300,
         120,
         0
    };


    init_graphics();


    while (1)
    {
        db_nextpri =
            db[db_active].packet;


        /*
         * Aplica a câmera.
         */

        set_camera(&camera);


        /*
         * PISO
         *
         * VERDE
         */

        draw_quad(
            &floor_a,
            &floor_b,
            &floor_c,
            &floor_d,
            40,
            150,
            60
        );


        /*
         * PAREDE DO FUNDO
         *
         * VERMELHA
         */

        draw_quad(
            &back_a,
            &back_b,
            &back_c,
            &back_d,
            180,
            45,
            45
        );


        /*
         * PAREDE LATERAL
         *
         * AZUL
         */

        draw_quad(
            &side_a,
            &side_b,
            &side_c,
            &side_d,
            45,
            80,
            180
        );


        display();
    }


    return 0;
}
