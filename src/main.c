#include <stdint.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define OT_LEN 256
#define PACKET_LEN 16384

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

typedef struct
{
    SVECTOR rotation;
    VECTOR position;
} Camera;

typedef struct
{
    int x0;
    int y0;
    int z0;

    int x1;
    int y1;
    int z1;
} Box;

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

    ClearOTagR(
        db[0].ot,
        OT_LEN
    );

    ClearOTagR(
        db[1].ot,
        OT_LEN
    );

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
    db_nextpri = db[0].packet;
}

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

    db_nextpri = db[db_active].packet;

    DrawOTag(
        db[db_active ^ 1].ot + OT_LEN - 1
    );
}

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

    poly = (POLY_F3 *)db_nextpri;

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

void draw_box(
    int x0,
    int y0,
    int z0,
    int x1,
    int y1,
    int z1,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    SVECTOR v000 =
    {
        x0,
        y0,
        z0
    };

    SVECTOR v100 =
    {
        x1,
        y0,
        z0
    };

    SVECTOR v110 =
    {
        x1,
        y1,
        z0
    };

    SVECTOR v010 =
    {
        x0,
        y1,
        z0
    };

    SVECTOR v001 =
    {
        x0,
        y0,
        z1
    };

    SVECTOR v101 =
    {
        x1,
        y0,
        z1
    };

    SVECTOR v111 =
    {
        x1,
        y1,
        z1
    };

    SVECTOR v011 =
    {
        x0,
        y1,
        z1
    };

    draw_quad(
        &v000,
        &v100,
        &v110,
        &v010,
        r,
        g,
        b
    );

    draw_quad(
        &v101,
        &v001,
        &v011,
        &v111,
        r,
        g,
        b
    );

    draw_quad(
        &v001,
        &v000,
        &v010,
        &v011,
        r,
        g,
        b
    );

    draw_quad(
        &v100,
        &v101,
        &v111,
        &v110,
        r,
        g,
        b
    );

    draw_quad(
        &v010,
        &v110,
        &v111,
        &v011,
        r,
        g,
        b
    );

    draw_quad(
        &v001,
        &v101,
        &v100,
        &v000,
        r,
        g,
        b
    );
}

void draw_wall_quad(
    SVECTOR *a,
    SVECTOR *b,
    SVECTOR *c,
    SVECTOR *d,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    draw_quad(
        a,
        b,
        c,
        d,
        r,
        g,
        b
    );
}

void draw_rug(void)
{
    draw_box(
        -180,
        118,
        170,
        150,
        124,
        370,
        48,
        38,
        34
    );

    draw_box(
        -170,
        117,
        180,
        140,
        121,
        360,
        70,
        55,
        42
    );
}

void draw_sofa(void)
{
    draw_box(
        40,
        55,
        330,
        250,
        120,
        410,
        70,
        48,
        38
    );

    draw_box(
        40,
        0,
        350,
        250,
        60,
        405,
        75,
        52,
        40
    );

    draw_box(
        50,
        5,
        340,
        90,
        125,
        400,
        82,
        56,
        42
    );

    draw_box(
        200,
        5,
        240,
        125,
        125,
        400,
        82,
        56,
        42
    );

    draw_box(
        65,
        115,
        225,
        235,
        155,
        395,
        78,
        54,
        42
    );

    draw_box(
        55,
        115,
        225,
        235,
        145,
        390,
        65,
        45,
        36
    );

    draw_box(
        55,
        120,
        345,
        75,
        165,
        395,
        90,
        65,
        48
    );
}

void draw_coffee_table(void)
{
    draw_box(
        -80,
        72,
        270,
        70,
        88,
        360,
        70,
        45,
        30
    );

    draw_box(
        -70,
        88,
        280,
        60,
        100,
        350,
        90,
        58,
        38
    );

    draw_box(
        -65,
        100,
        285,
        -50,
        120,
        300,
        55,
        38,
        27
    );

    draw_box(
        45,
        100,
        285,
        60,
        120,
        300,
        55,
        38,
        27
    );
}

void draw_tv_unit(void)
{
    draw_box(
        -250,
        65,
        365,
        -70,
        120,
        410,
        60,
        43,
        32
    );

    draw_box(
        -235,
        105,
        360,
        -85,
        125,
        405,
        72,
        50,
        36
    );

    draw_box(
        -220,
        20,
        375,
        -100,
        100,
        400,
        50,
        34,
        27
    );
}

void draw_tv(void)
{
    draw_box(
        -205,
        -55,
        350,
        -115,
        25,
        370,
        18,
        18,
        20
    );

    draw_box(
        -195,
        -48,
        343,
        -125,
        18,
        360,
        28,
        30,
        32
    );

    draw_box(
        -170,
        -15,
        338,
        -150,
        -5,
        342,
        80,
        75,
        65
    );

    draw_box(
        -165,
        -12,
        337,
        -155,
        -7,
        341,
        35,
        50,
        60
    );
}

void draw_side_table(void)
{
    draw_box(
        220,
        65,
        150,
        275,
        115,
        190,
        65,
        45,
        32
    );

    draw_box(
        228,
        105,
        155,
        267,
        120,
        185,
        80,
        55,
        38
    );

    draw_box(
        230,
        0,
        155,
        240,
        70,
        170,
        55,
        38,
        28
    );
}

void draw_lamp(void)
{
    draw_box(
        242,
        115,
        165,
        255,
        155,
        172,
        120,
        90,
        45
    );

    draw_box(
        246,
        25,
        166,
        251,
        120,
        170,
        80,
        65,
        42
    );

    draw_box(
        232,
        0,
        158,
        265,
        20,
        178,
        70,
        50,
        30
    );
}

void draw_plant(void)
{
    draw_box(
        -270,
        80,
        90,
        -225,
        120,
        135,
        65,
        42,
        28
    );

    draw_box(
        -265,
        30,
        100,
        -230,
        90,
        125,
        42,
        65,
        38
    );

    draw_box(
        -280,
        5,
        85,
        -235,
        55,
        110,
        35,
        60,
        32
    );

    draw_box(
        -265,
        20,
        105,
        -220,
        65,
        130,
        38,
        70,
        35
    );

    draw_box(
        -250,
        0,
        90,
        -210,
        45,
        105,
        40,
        75,
        40
    );
}

void draw_picture(void)
{
    draw_box(
        -40,
        -55,
        493,
        70,
        5,
        500,
        80,
        55,
        35
    );

    draw_box(
        -30,
        -45,
        488,
        60,
        -5,
        495,
        40,
        55,
        70
    );

    draw_box(
        -20,
        -35,
        485,
        50,
        -15,
        490,
        70,
        50,
        35
    );
}

void draw_curtains(void)
{
    draw_box(
        -125,
        -55,
        480,
        -100,
        70,
        492,
        75,
        48,
        38
    );

    draw_box(
        100,
        -55,
        125,
        145,
        70,
        492,
        75,
        48,
        38
    );

    draw_box(
        -135,
        -65,
        475,
        155,
        -53,
        490,
        85,
        58,
        42
    );
}

void draw_room(void)
{
    const int ROOM_LEFT = -300;
    const int ROOM_RIGHT = 300;
    const int ROOM_FRONT = 0;
    const int ROOM_BACK = 500;

    const int FLOOR_Y = 120;
    const int CEIL_Y = -120;

    const int WINDOW_LEFT = -100;
    const int WINDOW_RIGHT = 100;
    const int WINDOW_TOP = -50;
    const int WINDOW_BOTTOM = 50;

    const int DOOR_FRONT = 80;
    const int DOOR_BACK = 230;
    const int DOOR_TOP = -75;
    const int DOOR_BOTTOM = 120;

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

    SVECTOR window_a =
    {
        WINDOW_LEFT,
        WINDOW_TOP,
        ROOM_BACK - 5
    };

    SVECTOR window_b =
    {
        WINDOW_RIGHT,
        WINDOW_TOP,
        ROOM_BACK - 5
    };

    SVECTOR window_c =
    {
        WINDOW_RIGHT,
        WINDOW_BOTTOM,
        ROOM_BACK - 5
    };

    SVECTOR window_d =
    {
        WINDOW_LEFT,
        WINDOW_BOTTOM,
        ROOM_BACK - 5
    };

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

    draw_quad(
        &floor_a,
        &floor_b,
        &floor_c,
        &floor_d,
        58,
        55,
        50
    );

    draw_quad(
        &ceiling_a,
        &ceiling_b,
        &ceiling_c,
        &ceiling_d,
        34,
        34,
        38
    );

    draw_quad(
        &right_a,
        &right_b,
        &right_c,
        &right_d,
        72,
        66,
        60
    );

    draw_quad(
        &left_a,
        &left_b,
        &left_c,
        &left_d,
        68,
        62,
        57
    );

    draw_quad(
        &back_top_a,
        &back_top_b,
        &back_top_c,
        &back_top_d,
        70,
        65,
        59
    );

    draw_quad(
        &back_bottom_a,
        &back_bottom_b,
        &back_bottom_c,
        &back_bottom_d,
        70,
        65,
        59
    );

    draw_quad(
        &back_left_a,
        &back_left_b,
        &back_left_c,
        &back_left_d,
        70,
        65,
        59
    );

    draw_quad(
        &back_right_a,
        &back_right_b,
        &back_right_c,
        &back_right_d,
        70,
        65,
        59
    );

    draw_box(
        -300,
        105,
        0,
        300,
        120,
        500,
        82,
        63,
        45
    );

    draw_box(
        -300,
        105,
        490,
        300,
        120,
        500,
        82,
        63,
        45
    );

    draw_quad(
        &window_a,
        &window_b,
        &window_c,
        &window_d,
        25,
        55,
        82
    );

    draw_box(
        -115,
        -62,
        490,
        115,
        -50,
        502,
        92,
        65,
        38
    );

    draw_box(
        -115,
        50,
        490,
        115,
        62,
        502,
        92,
        65,
        38
    );

    draw_box(
        -112,
        -62,
        490,
        -100,
        62,
        502,
        92,
        65,
        38
    );

    draw_box(
        100,
        -62,
        490,
        112,
        62,
        502,
        92,
        65,
        38
    );

    draw_box(
        -6,
        -50,
        490,
        6,
        50,
        502,
        92,
        65,
        38
    );

    draw_box(
        -305,
        DOOR_TOP - 10,
        DOOR_FRONT - 10,
        -294,
        DOOR_BOTTOM + 10,
        DOOR_FRONT,
        95,
        65,
        38
    );

    draw_box(
        -305,
        DOOR_TOP - 10,
        DOOR_BACK,
        -294,
        DOOR_BOTTOM + 10,
        DOOR_BACK + 10,
        95,
        65,
        38
    );

    draw_box(
        -305,
        DOOR_TOP - 10,
        DOOR_FRONT - 10,
        -294,
        DOOR_TOP,
        DOOR_BACK + 10,
        105,
        72,
        40
    );

    draw_quad(
        &door_a,
        &door_b,
        &door_c,
        &door_d,
        78,
        48,
        30
    );

    draw_box(
        -298,
        10,
        145,
        -292,
        30,
        165,
        120,
        88,
        48
    );

    draw_box(
        -297,
        15,
        150,
        -290,
        24,
        158,
        35,
        25,
        18
    );

    draw_rug();

    draw_tv_unit();

    draw_tv();

    draw_sofa();

    draw_coffee_table();

    draw_side_table();

    draw_lamp();

    draw_plant();

    draw_picture();

    draw_curtains();
}

int main(void)
{
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

    init_graphics();

    while (1)
    {
        set_camera(&camera);

        draw_room();

        display();
    }

    return 0;
}
