#include <stdint.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define OT_LEN 256
#define PACKET_LEN 16384


/* =========================================================
   RENDER BUFFER
   ========================================================= */

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


/* =========================================================
   CÂMERA
   ========================================================= */

typedef struct
{
    SVECTOR rotation;
    VECTOR position;

} Camera;


/* =========================================================
   INICIALIZAÇÃO
   ========================================================= */

void init_graphics(void)
{
    ResetGraph(0);


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

    db[0].draw.isbg = 1;

    setRGB0(
        &db[0].draw,
        8,
        8,
        12
    );


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

    db[1].draw.isbg = 1;

    setRGB0(
        &db[1].draw,
        8,
        8,
        12
    );


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


/* =========================================================
   DISPLAY
   ========================================================= */

void display(void)
{
    DrawSync(0);

    VSync(0);

    db_active ^= 1;


    PutDrawEnv(
        &db[db_active].draw
    );

    PutDispEnv(
        &db[db_active].disp
    );


    ClearOTagR(
        db[db_active].ot,
        OT_LEN
    );


    db_nextpri =
        db[db_active].packet;


    DrawOTag(
        db[db_active ^ 1].ot + OT_LEN - 1
    );
}


/* =========================================================
   CÂMERA
   ========================================================= */

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


/* =========================================================
   TRIÂNGULO
   ========================================================= */

void draw_triangle(
    SVECTOR *a,
    SVECTOR *b,
    SVECTOR *c,
    uint8_t r,
    uint8_t g,
    uint8_t bcol
)
{
    POLY_F3 *poly;

    int depth;


    poly =
        (POLY_F3 *)db_nextpri;


    setPolyF3(poly);

    setRGB0(
        poly,
        r,
        g,
        bcol
    );


    gte_ldv3(
        a,
        b,
        c
    );

    gte_rtpt();


    gte_stsxy0(&poly->x0);
    gte_stsxy1(&poly->x1);
    gte_stsxy2(&poly->x2);


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


/* =========================================================
   QUADRILÁTERO
   ========================================================= */

void draw_quad(
    SVECTOR *a,
    SVECTOR *b,
    SVECTOR *c,
    SVECTOR *d,
    uint8_t r,
    uint8_t g,
    uint8_t bcol
)
{
    draw_triangle(
        a,
        b,
        c,
        r,
        g,
        bcol
    );

    draw_triangle(
        a,
        c,
        d,
        r,
        g,
        bcol
    );
}


/* =========================================================
   MAIN
   ========================================================= */

int main(void)
{
    /* -----------------------------------------------------
       CÂMERA VALIDADA
       ----------------------------------------------------- */

    Camera camera =
    {
        {
            256,
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


    /* -----------------------------------------------------
       DIMENSÕES DA SALA
       ----------------------------------------------------- */

    const int ROOM_LEFT  = -300;
    const int ROOM_RIGHT =  300;

    const int ROOM_FRONT = 0;
    const int ROOM_BACK  = 500;

    const int FLOOR_Y = 120;
    const int CEIL_Y  = -120;


    /* -----------------------------------------------------
       JANELA
       ----------------------------------------------------- */

    /*
     * A janela fica centralizada na parede do fundo.
     */

    const int WINDOW_LEFT  = -100;
    const int WINDOW_RIGHT =  100;

    const int WINDOW_TOP    = -50;
    const int WINDOW_BOTTOM =  50;


    /* =====================================================
       PISO
       ===================================================== */

    SVECTOR floor_a =
    {
        ROOM_LEFT,
        FLOOR_Y,
        ROOM_FRONT
    };

    SVECTOR floor_b =
    {
        ROOM_RIGHT,
        FLOOR_Y,
        ROOM_FRONT
    };

    SVECTOR floor_c =
    {
        ROOM_RIGHT,
        FLOOR_Y,
        ROOM_BACK
    };

    SVECTOR floor_d =
    {
        ROOM_LEFT,
        FLOOR_Y,
        ROOM_BACK
    };


    /* =====================================================
       TETO
       ===================================================== */

    SVECTOR ceiling_a =
    {
        ROOM_LEFT,
        CEIL_Y,
        ROOM_FRONT
    };

    SVECTOR ceiling_b =
    {
        ROOM_RIGHT,
        CEIL_Y,
        ROOM_FRONT
    };

    SVECTOR ceiling_c =
    {
        ROOM_RIGHT,
        CEIL_Y,
        ROOM_BACK
    };

    SVECTOR ceiling_d =
    {
        ROOM_LEFT,
        CEIL_Y,
        ROOM_BACK
    };


    /* =====================================================
       PAREDE ESQUERDA
       ===================================================== */

    SVECTOR left_a =
    {
        ROOM_LEFT,
        CEIL_Y,
        ROOM_FRONT
    };

    SVECTOR left_b =
    {
        ROOM_LEFT,
        CEIL_Y,
        ROOM_BACK
    };

    SVECTOR left_c =
    {
        ROOM_LEFT,
        FLOOR_Y,
        ROOM_BACK
    };

    SVECTOR left_d =
    {
        ROOM_LEFT,
        FLOOR_Y,
        ROOM_FRONT
    };


    /* =====================================================
       PAREDE DIREITA
       ===================================================== */

    SVECTOR right_a =
    {
        ROOM_RIGHT,
        CEIL_Y,
        ROOM_BACK
    };

    SVECTOR right_b =
    {
        ROOM_RIGHT,
        CEIL_Y,
        ROOM_FRONT
    };

    SVECTOR right_c =
    {
        ROOM_RIGHT,
        FLOOR_Y,
        ROOM_FRONT
    };

    SVECTOR right_d =
    {
        ROOM_RIGHT,
        FLOOR_Y,
        ROOM_BACK
    };


    /* =====================================================
       PAREDE DO FUNDO
       ===================================================== */

    /*
     * Em vez de uma parede única,
     * criamos quatro partes ao redor da janela.
     *
     *      TOPO
     *
     * ESQ   JANELA   DIR
     *
     *     PARTE INFERIOR
     */


    /* -------------------------
       PARTE SUPERIOR
       ------------------------- */

    SVECTOR back_top_a =
    {
        ROOM_LEFT,
        CEIL_Y,
        ROOM_BACK
    };

    SVECTOR back_top_b =
    {
        ROOM_RIGHT,
        CEIL_Y,
        ROOM_BACK
    };

    SVECTOR back_top_c =
    {
        ROOM_RIGHT,
        WINDOW_TOP,
        ROOM_BACK
    };

    SVECTOR back_top_d =
    {
        ROOM_LEFT,
        WINDOW_TOP,
        ROOM_BACK
    };


    /* -------------------------
       PARTE INFERIOR
       ------------------------- */

    SVECTOR back_bottom_a =
    {
        ROOM_LEFT,
        WINDOW_BOTTOM,
        ROOM_BACK
    };

    SVECTOR back_bottom_b =
    {
        ROOM_RIGHT,
        WINDOW_BOTTOM,
        ROOM_BACK
    };

    SVECTOR back_bottom_c =
    {
        ROOM_RIGHT,
        FLOOR_Y,
        ROOM_BACK
    };

    SVECTOR back_bottom_d =
    {
        ROOM_LEFT,
        FLOOR_Y,
        ROOM_BACK
    };


    /* -------------------------
       PARTE ESQUERDA DA JANELA
       ------------------------- */

    SVECTOR back_left_a =
    {
        ROOM_LEFT,
        WINDOW_TOP,
        ROOM_BACK
    };

    SVECTOR back_left_b =
    {
        WINDOW_LEFT,
        WINDOW_TOP,
        ROOM_BACK
    };

    SVECTOR back_left_c =
    {
        WINDOW_LEFT,
        WINDOW_BOTTOM,
        ROOM_BACK
    };

    SVECTOR back_left_d =
    {
        ROOM_LEFT,
        WINDOW_BOTTOM,
        ROOM_BACK
    };


    /* -------------------------
       PARTE DIREITA DA JANELA
       ------------------------- */

    SVECTOR back_right_a =
    {
        WINDOW_RIGHT,
        WINDOW_TOP,
        ROOM_BACK
    };

    SVECTOR back_right_b =
    {
        ROOM_RIGHT,
        WINDOW_TOP,
        ROOM_BACK
    };

    SVECTOR back_right_c =
    {
        ROOM_RIGHT,
        WINDOW_BOTTOM,
        ROOM_BACK
    };

    SVECTOR back_right_d =
    {
        WINDOW_RIGHT,
        WINDOW_BOTTOM,
        ROOM_BACK
    };


    /* =====================================================
       VIDRO DA JANELA
       ===================================================== */

    /*
     * O vidro fica ligeiramente à frente da abertura.
     */

    SVECTOR window_a =
    {
        WINDOW_LEFT,
        WINDOW_TOP,
        ROOM_BACK - 4
    };

    SVECTOR window_b =
    {
        WINDOW_RIGHT,
        WINDOW_TOP,
        ROOM_BACK - 4
    };

    SVECTOR window_c =
    {
        WINDOW_RIGHT,
        WINDOW_BOTTOM,
        ROOM_BACK - 4
    };

    SVECTOR window_d =
    {
        WINDOW_LEFT,
        WINDOW_BOTTOM,
        ROOM_BACK - 4
    };


    /* =====================================================
       MOLDURA DA JANELA
       ===================================================== */

    const int FRAME = 12;


    /* Moldura esquerda */

    SVECTOR frame_left_a =
    {
        WINDOW_LEFT - FRAME,
        WINDOW_TOP - FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_left_b =
    {
        WINDOW_LEFT,
        WINDOW_TOP - FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_left_c =
    {
        WINDOW_LEFT,
        WINDOW_BOTTOM + FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_left_d =
    {
        WINDOW_LEFT - FRAME,
        WINDOW_BOTTOM + FRAME,
        ROOM_BACK - 8
    };


    /* Moldura direita */

    SVECTOR frame_right_a =
    {
        WINDOW_RIGHT,
        WINDOW_TOP - FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_right_b =
    {
        WINDOW_RIGHT + FRAME,
        WINDOW_TOP - FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_right_c =
    {
        WINDOW_RIGHT + FRAME,
        WINDOW_BOTTOM + FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_right_d =
    {
        WINDOW_RIGHT,
        WINDOW_BOTTOM + FRAME,
        ROOM_BACK - 8
    };


    /* Moldura superior */

    SVECTOR frame_top_a =
    {
        WINDOW_LEFT - FRAME,
        WINDOW_TOP - FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_top_b =
    {
        WINDOW_RIGHT + FRAME,
        WINDOW_TOP - FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_top_c =
    {
        WINDOW_RIGHT + FRAME,
        WINDOW_TOP,
        ROOM_BACK - 8
    };

    SVECTOR frame_top_d =
    {
        WINDOW_LEFT - FRAME,
        WINDOW_TOP,
        ROOM_BACK - 8
    };


    /* Moldura inferior */

    SVECTOR frame_bottom_a =
    {
        WINDOW_LEFT - FRAME,
        WINDOW_BOTTOM,
        ROOM_BACK - 8
    };

    SVECTOR frame_bottom_b =
    {
        WINDOW_RIGHT + FRAME,
        WINDOW_BOTTOM,
        ROOM_BACK - 8
    };

    SVECTOR frame_bottom_c =
    {
        WINDOW_RIGHT + FRAME,
        WINDOW_BOTTOM + FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_bottom_d =
    {
        WINDOW_LEFT - FRAME,
        WINDOW_BOTTOM + FRAME,
        ROOM_BACK - 8
    };


    /* =====================================================
       INICIALIZAÇÃO
       ===================================================== */

    init_graphics();


    /* =====================================================
       LOOP
       ===================================================== */

    while (1)
    {
        db_nextpri =
            db[db_active].packet;


        set_camera(&camera);


        /* =================================================
           PISO
           ================================================= */

        draw_quad(
            &floor_a,
            &floor_b,
            &floor_c,
            &floor_d,
            75,
            65,
            55
        );


        /* =================================================
           TETO
           ================================================= */

        draw_quad(
            &ceiling_a,
            &ceiling_b,
            &ceiling_c,
            &ceiling_d,
            65,
            65,
            70
        );


        /* =================================================
           PAREDE ESQUERDA
           ================================================= */

        draw_quad(
            &left_a,
            &left_b,
            &left_c,
            &left_d,
            55,
            95,
            70
        );


        /* =================================================
           PAREDE DIREITA
           ================================================= */

        draw_quad(
            &right_a,
            &right_b,
            &right_c,
            &right_d,
            65,
            80,
            120
        );


        /* =================================================
           PAREDE DO FUNDO
           ================================================= */

        draw_quad(
            &back_top_a,
            &back_top_b,
            &back_top_c,
            &back_top_d,
            125,
            65,
            65
        );


        draw_quad(
            &back_bottom_a,
            &back_bottom_b,
            &back_bottom_c,
            &back_bottom_d,
            125,
            65,
            65
        );


        draw_quad(
            &back_left_a,
            &back_left_b,
            &back_left_c,
            &back_left_d,
            125,
            65,
            65
        );


        draw_quad(
            &back_right_a,
            &back_right_b,
            &back_right_c,
            &back_right_d,
            125,
            65,
            65
        );


        /* =================================================
           VIDRO
           ================================================= */

        draw_quad(
            &window_a,
            &window_b,
            &window_c,
            &window_d,
            45,
            90,
            130
        );


        /* =================================================
           MOLDURA
           ================================================= */

        draw_quad(
            &frame_left_a,
            &frame_left_b,
            &frame_left_c,
            &frame_left_d,
            90,
            60,
            35
        );

        draw_quad(
            &frame_right_a,
            &frame_right_b,
            &frame_right_c,
            &frame_right_d,
            90,
            60,
            35
        );

        draw_quad(
            &frame_top_a,
            &frame_top_b,
            &frame_top_c,
            &frame_top_d,
            90,
            60,
            35
        );

        draw_quad(
            &frame_bottom_a,
            &frame_bottom_b,
            &frame_bottom_c,
            &frame_bottom_d,
            90,
            60,
            35
        );


        display();
    }


    return 0;
}
