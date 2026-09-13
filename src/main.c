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
   INICIALIZAÇÃO GRÁFICA
   ========================================================= */

void init_graphics(void)
{
    ResetGraph(0);


    /* -----------------------------------------------------
       BUFFER 0
       ----------------------------------------------------- */

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


    /* -----------------------------------------------------
       BUFFER 1
       ----------------------------------------------------- */

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


    /* -----------------------------------------------------
       ORDERING TABLE
       ----------------------------------------------------- */

    ClearOTagR(
        db[0].ot,
        OT_LEN
    );

    ClearOTagR(
        db[1].ot,
        OT_LEN
    );


    /* -----------------------------------------------------
       GTE
       ----------------------------------------------------- */

    InitGeom();

    gte_SetGeomOffset(
        SCREEN_XRES / 2,
        SCREEN_YRES / 2
    );

    gte_SetGeomScreen(
        SCREEN_XRES / 2
    );


    PutDrawEnv(
        &db[0].draw
    );

    PutDispEnv(
        &db[0].disp
    );

    SetDispMask(1);

    db_active = 0;

    db_nextpri =
        db[0].packet;
}


/* =========================================================
   DISPLAY / DOUBLE BUFFER
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


    gte_SetRotMatrix(
        &matrix
    );

    gte_SetTransMatrix(
        &matrix
    );
}


/* =========================================================
   TRIÂNGULO 3D
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


    /* -----------------------------------------------------
       TRANSFORMAÇÃO GTE
       ----------------------------------------------------- */

    gte_ldv3(
        a,
        b,
        c
    );

    gte_rtpt();


    /* -----------------------------------------------------
       COORDENADAS DE TELA
       ----------------------------------------------------- */

    gte_stsxy0(
        &poly->x0
    );

    gte_stsxy1(
        &poly->x1
    );

    gte_stsxy2(
        &poly->x2
    );


    /* -----------------------------------------------------
       PROFUNDIDADE
       ----------------------------------------------------- */

    gte_avsz3();

    gte_stotz(
        &depth
    );


    depth >>= 8;


    if (depth < 0)
        depth = 0;

    if (depth >= OT_LEN)
        depth = OT_LEN - 1;


    /* -----------------------------------------------------
       ORDERING TABLE
       ----------------------------------------------------- */

    addPrim(
        db[db_active].ot + depth,
        poly
    );


    db_nextpri +=
        sizeof(POLY_F3);
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
    /* =====================================================
       CÂMERA VALIDADA
       ===================================================== */

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


    /* =====================================================
       DIMENSÕES DA SALA
       ===================================================== */

    const int ROOM_LEFT  = -300;
    const int ROOM_RIGHT =  300;

    const int ROOM_FRONT = 0;
    const int ROOM_BACK  = 500;

    const int FLOOR_Y = 120;
    const int CEIL_Y  = -120;


    /* =====================================================
       DIMENSÕES DA JANELA
       ===================================================== */

    const int WINDOW_LEFT  = -100;
    const int WINDOW_RIGHT =  100;

    const int WINDOW_TOP    = -50;
    const int WINDOW_BOTTOM = 50;


    /* =====================================================
       DIMENSÕES DA PORTA
       ===================================================== */

    const int DOOR_FRONT = 80;
    const int DOOR_BACK  = 230;

    const int DOOR_TOP    = -75;
    const int DOOR_BOTTOM = 120;


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
       PAREDE ESQUERDA
       ===================================================== */

    SVECTOR left_front_a =
    {
        ROOM_LEFT,
        CEIL_Y,
        ROOM_FRONT
    };

    SVECTOR left_front_b =
    {
        ROOM_LEFT,
        CEIL_Y,
        DOOR_FRONT
    };

    SVECTOR left_front_c =
    {
        ROOM_LEFT,
        DOOR_TOP,
        DOOR_FRONT
    };

    SVECTOR left_front_d =
    {
        ROOM_LEFT,
        DOOR_TOP,
        ROOM_FRONT
    };


    SVECTOR left_top_a =
    {
        ROOM_LEFT,
        CEIL_Y,
        DOOR_FRONT
    };

    SVECTOR left_top_b =
    {
        ROOM_LEFT,
        CEIL_Y,
        DOOR_BACK
    };

    SVECTOR left_top_c =
    {
        ROOM_LEFT,
        DOOR_TOP,
        DOOR_BACK
    };

    SVECTOR left_top_d =
    {
        ROOM_LEFT,
        DOOR_TOP,
        DOOR_FRONT
    };


    SVECTOR left_back_a =
    {
        ROOM_LEFT,
        CEIL_Y,
        DOOR_BACK
    };

    SVECTOR left_back_b =
    {
        ROOM_LEFT,
        CEIL_Y,
        ROOM_BACK
    };

    SVECTOR left_back_c =
    {
        ROOM_LEFT,
        DOOR_TOP,
        ROOM_BACK
    };

    SVECTOR left_back_d =
    {
        ROOM_LEFT,
        DOOR_TOP,
        DOOR_BACK
    };


    SVECTOR left_bottom_a =
    {
        ROOM_LEFT,
        DOOR_BOTTOM,
        DOOR_FRONT
    };

    SVECTOR left_bottom_b =
    {
        ROOM_LEFT,
        DOOR_BOTTOM,
        DOOR_BACK
    };

    SVECTOR left_bottom_c =
    {
        ROOM_LEFT,
        FLOOR_Y,
        DOOR_BACK
    };

    SVECTOR left_bottom_d =
    {
        ROOM_LEFT,
        FLOOR_Y,
        DOOR_FRONT
    };


    /* =====================================================
       PAREDE DO FUNDO
       ===================================================== */

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

    const int WINDOW_FRAME = 12;


    SVECTOR frame_left_a =
    {
        WINDOW_LEFT - WINDOW_FRAME,
        WINDOW_TOP - WINDOW_FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_left_b =
    {
        WINDOW_LEFT,
        WINDOW_TOP - WINDOW_FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_left_c =
    {
        WINDOW_LEFT,
        WINDOW_BOTTOM + WINDOW_FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_left_d =
    {
        WINDOW_LEFT - WINDOW_FRAME,
        WINDOW_BOTTOM + WINDOW_FRAME,
        ROOM_BACK - 8
    };


    SVECTOR frame_right_a =
    {
        WINDOW_RIGHT,
        WINDOW_TOP - WINDOW_FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_right_b =
    {
        WINDOW_RIGHT + WINDOW_FRAME,
        WINDOW_TOP - WINDOW_FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_right_c =
    {
        WINDOW_RIGHT + WINDOW_FRAME,
        WINDOW_BOTTOM + WINDOW_FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_right_d =
    {
        WINDOW_RIGHT,
        WINDOW_BOTTOM + WINDOW_FRAME,
        ROOM_BACK - 8
    };


    SVECTOR frame_top_a =
    {
        WINDOW_LEFT - WINDOW_FRAME,
        WINDOW_TOP - WINDOW_FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_top_b =
    {
        WINDOW_RIGHT + WINDOW_FRAME,
        WINDOW_TOP - WINDOW_FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_top_c =
    {
        WINDOW_RIGHT + WINDOW_FRAME,
        WINDOW_TOP,
        ROOM_BACK - 8
    };

    SVECTOR frame_top_d =
    {
        WINDOW_LEFT - WINDOW_FRAME,
        WINDOW_TOP,
        ROOM_BACK - 8
    };


    SVECTOR frame_bottom_a =
    {
        WINDOW_LEFT - WINDOW_FRAME,
        WINDOW_BOTTOM,
        ROOM_BACK - 8
    };

    SVECTOR frame_bottom_b =
    {
        WINDOW_RIGHT + WINDOW_FRAME,
        WINDOW_BOTTOM,
        ROOM_BACK - 8
    };

    SVECTOR frame_bottom_c =
    {
        WINDOW_RIGHT + WINDOW_FRAME,
        WINDOW_BOTTOM + WINDOW_FRAME,
        ROOM_BACK - 8
    };

    SVECTOR frame_bottom_d =
    {
        WINDOW_LEFT - WINDOW_FRAME,
        WINDOW_BOTTOM + WINDOW_FRAME,
        ROOM_BACK - 8
    };


    /* =====================================================
       PORTA
       ===================================================== */

    SVECTOR door_a =
    {
        ROOM_LEFT + 4,
        DOOR_TOP,
        DOOR_FRONT
    };

    SVECTOR door_b =
    {
        ROOM_LEFT + 4,
        DOOR_TOP,
        DOOR_BACK
    };

    SVECTOR door_c =
    {
        ROOM_LEFT + 4,
        DOOR_BOTTOM,
        DOOR_BACK
    };

    SVECTOR door_d =
    {
        ROOM_LEFT + 4,
        DOOR_BOTTOM,
        DOOR_FRONT
    };


    /* =====================================================
       MOLDURA DA PORTA
       ===================================================== */

    const int DOOR_FRAME = 10;


    SVECTOR door_frame_front_a =
    {
        ROOM_LEFT + 2,
        DOOR_TOP - DOOR_FRAME,
        DOOR_FRONT - DOOR_FRAME
    };

    SVECTOR door_frame_front_b =
    {
        ROOM_LEFT + 2,
        DOOR_TOP,
        DOOR_FRONT - DOOR_FRAME
    };

    SVECTOR door_frame_front_c =
    {
        ROOM_LEFT + 2,
        DOOR_BOTTOM,
        DOOR_FRONT - DOOR_FRAME
    };

    SVECTOR door_frame_front_d =
    {
        ROOM_LEFT + 2,
        DOOR_BOTTOM + DOOR_FRAME,
        DOOR_FRONT - DOOR_FRAME
    };


    SVECTOR door_frame_back_a =
    {
        ROOM_LEFT + 2,
        DOOR_TOP - DOOR_FRAME,
        DOOR_BACK + DOOR_FRAME
    };

    SVECTOR door_frame_back_b =
    {
        ROOM_LEFT + 2,
        DOOR_TOP,
        DOOR_BACK + DOOR_FRAME
    };

    SVECTOR door_frame_back_c =
    {
        ROOM_LEFT + 2,
        DOOR_BOTTOM,
        DOOR_BACK + DOOR_FRAME
    };

    SVECTOR door_frame_back_d =
    {
        ROOM_LEFT + 2,
        DOOR_BOTTOM + DOOR_FRAME,
        DOOR_BACK + DOOR_FRAME
    };


    SVECTOR door_frame_top_a =
    {
        ROOM_LEFT + 2,
        DOOR_TOP - DOOR_FRAME,
        DOOR_FRONT - DOOR_FRAME
    };

    SVECTOR door_frame_top_b =
    {
        ROOM_LEFT + 2,
        DOOR_TOP - DOOR_FRAME,
        DOOR_BACK + DOOR_FRAME
    };

    SVECTOR door_frame_top_c =
    {
        ROOM_LEFT + 2,
        DOOR_TOP,
        DOOR_BACK + DOOR_FRAME
    };

    SVECTOR door_frame_top_d =
    {
        ROOM_LEFT + 2,
        DOOR_TOP,
        DOOR_FRONT - DOOR_FRAME
    };


    /* =====================================================
       INICIALIZAÇÃO
       ===================================================== */

    init_graphics();


    /* =====================================================
       LOOP PRINCIPAL
       ===================================================== */

    while (1)
    {
        set_camera(&camera);


        /* -------------------------------------------------
           PISO
           ------------------------------------------------- */

        draw_quad(
            &floor_a,
            &floor_b,
            &floor_c,
            &floor_d,
            55,
            55,
            60
        );


        /* -------------------------------------------------
           TETO
           ------------------------------------------------- */

        draw_quad(
            &ceiling_a,
            &ceiling_b,
            &ceiling_c,
            &ceiling_d,
            35,
            35,
            40
        );


        /* -------------------------------------------------
           PAREDE DIREITA
           ------------------------------------------------- */

        draw_quad(
            &right_a,
            &right_b,
            &right_c,
            &right_d,
            75,
            70,
            65
        );


        /* -------------------------------------------------
           PAREDE ESQUERDA
           ------------------------------------------------- */

        draw_quad(
            &left_front_a,
            &left_front_b,
            &left_front_c,
            &left_front_d,
            70,
            65,
            60
        );

        draw_quad(
            &left_top_a,
            &left_top_b,
            &left_top_c,
            &left_top_d,
            70,
            65,
            60
        );

        draw_quad(
            &left_back_a,
            &left_back_b,
            &left_back_c,
            &left_back_d,
            70,
            65,
            60
        );

        draw_quad(
            &left_bottom_a,
            &left_bottom_b,
            &left_bottom_c,
            &left_bottom_d,
            70,
            65,
            60
        );


        /* -------------------------------------------------
           PAREDE DO FUNDO
           ------------------------------------------------- */

        draw_quad(
            &back_top_a,
            &back_top_b,
            &back_top_c,
            &back_top_d,
            72,
            67,
            62
        );

        draw_quad(
            &back_bottom_a,
            &back_bottom_b,
            &back_bottom_c,
            &back_bottom_d,
            72,
            67,
            62
        );

        draw_quad(
            &back_left_a,
            &back_left_b,
            &back_left_c,
            &back_left_d,
            72,
            67,
            62
        );

        draw_quad(
            &back_right_a,
            &back_right_b,
            &back_right_c,
            &back_right_d,
            72,
            67,
            62
        );


        /* 
