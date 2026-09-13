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
        5,
        5,
        8
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
        5,
        5,
        8
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
    uint8_t bcol
)
{
    SVECTOR v000 = { x0, y0, z0 };
    SVECTOR v100 = { x1, y0, z0 };
    SVECTOR v110 = { x1, y1, z0 };
    SVECTOR v010 = { x0, y1, z0 };

    SVECTOR v001 = { x0, y0, z1 };
    SVECTOR v101 = { x1, y0, z1 };
    SVECTOR v111 = { x1, y1, z1 };
    SVECTOR v011 = { x0, y1, z1 };

    draw_quad(
        &v000,
        &v100,
        &v110,
        &v010,
        r,
        g,
        bcol
    );

    draw_quad(
        &v101,
        &v001,
        &v011,
        &v111,
        r,
        g,
        bcol
    );

    draw_quad(
        &v001,
        &v000,
        &v010,
        &v011,
        r,
        g,
        bcol
    );

    draw_quad(
        &v100,
        &v101,
        &v111,
        &v110,
        r,
        g,
        bcol
    );

    draw_quad(
        &v010,
        &v110,
        &v111,
        &v011,
        r,
        g,
        bcol
    );

    draw_quad(
        &v001,
        &v101,
        &v100,
        &v000,
        r,
        g,
        bcol
    );
}


/*
 * Lajotas do chão
 */

void draw_floor_tiles(void)
{
    const int LEFT = -300;
    const int RIGHT = 300;
    const int FRONT = 0;
    const int BACK = 500;
    const int FLOOR = 120;

    int x;
    int z;

    for (x = LEFT; x < RIGHT; x += 75)
    {
        for (z = FRONT; z < BACK; z += 75)
        {
            draw_box(
                x + 2,
                FLOOR - 2,
                z + 2,
                x + 73,
                FLOOR,
                z + 73,
                145,
                145,
                138
            );
        }
    }

    /*
     * Juntas longitudinais
     */

    for (x = LEFT; x <= RIGHT; x += 75)
    {
        draw_box(
            x - 2,
            FLOOR - 3,
            FRONT,
            x + 2,
            FLOOR,
            BACK,
            42,
            42,
            40
        );
    }

    /*
     * Juntas transversais
     */

    for (z = FRONT; z <= BACK; z += 75)
    {
        draw_box(
            LEFT,
            FLOOR - 3,
            z - 2,
            RIGHT,
            FLOOR,
            z + 2,
            42,
            42,
            40
        );
    }
}


/*
 * Reboco da parede esquerda
 */

void draw_left_plaster(void)
{
    const int LEFT = -300;
    const int FLOOR = 120;
    const int CEILING = -120;

    int i;

    for (i = 0; i < 9; i++)
    {
        int y = -105 + i * 25;

        draw_box(
            LEFT - 2,
            y,
            250,
            LEFT + 2,
            y + 12,
            485,
            66 + (i % 3) * 5,
            60 + (i % 2) * 4,
            55
        );
    }

    /*
     * Manchas de reboco
     */

    draw_box(
        LEFT - 3,
        -20,
        300,
        LEFT + 2,
        15,
        340,
        53,
        49,
        45
    );

    draw_box(
        LEFT - 3,
        40,
        390,
        LEFT + 2,
        65,
        430,
        76,
        69,
        62
    );

    draw_box(
        LEFT - 3,
        -90,
        410,
        LEFT + 2,
        -65,
        455,
        57,
        53,
        49
    );
}


/*
 * Reboco da parede direita
 */

void draw_right_plaster(void)
{
    const int RIGHT = 300;

    draw_box(
        RIGHT - 2,
        -105,
        45,
        RIGHT + 2,
        -85,
        100,
        61,
        57,
        53
    );

    draw_box(
        RIGHT - 2,
        -55,
        160,
        RIGHT + 2,
        -30,
        210,
        75,
        68,
        61
    );

    draw_box(
        RIGHT - 2,
        20,
        75,
        RIGHT + 2,
        42,
        115,
        54,
        51,
        47
    );

    draw_box(
        RIGHT - 2,
        55,
        400,
        RIGHT + 2,
        80,
        455,
        69,
        63,
        57
    );
}


/*
 * Reboco da parede do fundo
 */

void draw_back_plaster(void)
{
    const int BACK = 500;

    draw_box(
        -285,
        -105,
        BACK - 2,
        -215,
        -80,
        BACK + 2,
        65,
        59,
        53
    );

    draw_box(
        215,
        -95,
        BACK - 2,
        285,
        -70,
        BACK + 2,
        59,
        54,
        49
    );

    draw_box(
        -80,
        65,
        BACK - 2,
        -20,
        90,
        BACK + 2,
        78,
        70,
        61
    );

    draw_box(
        100,
        70,
        BACK - 2,
        170,
        95,
        BACK + 2,
        53,
        49,
        45
    );
}


/*
 * Janela
 */

void draw_window(void)
{
    const int BACK = 500;

    /*
     * Vidro escuro
     */

    draw_box(
        -100,
        -50,
        BACK - 8,
        100,
        50,
        BACK - 2,
        32,
        55,
        72
    );

    /*
     * Moldura superior
     */

    draw_box(
        -115,
        -62,
        BACK - 14,
        115,
        -50,
        BACK + 3,
        105,
        83,
        55
    );

    /*
     * Moldura inferior
     */

    draw_box(
        -115,
        50,
        BACK - 14,
        115,
        62,
        BACK + 3,
        105,
        83,
        55
    );

    /*
     * Molduras laterais
     */

    draw_box(
        -115,
        -62,
        BACK - 14,
        -100,
        62,
        BACK + 3,
        105,
        83,
        55
    );

    draw_box(
        100,
        -62,
        BACK - 14,
        115,
        62,
        BACK + 3,
        105,
        83,
        55
    );

    /*
     * Divisória vertical
     */

    draw_box(
        -6,
        -50,
        BACK - 16,
        6,
        50,
        BACK + 4,
        105,
        83,
        55
    );

    /*
     * Divisória horizontal
     */

    draw_box(
        -100,
        -6,
        BACK - 16,
        100,
        6,
        BACK + 4,
        105,
        83,
        55
    );

    /*
     * Pequeno puxador
     */

    draw_box(
        15,
        8,
        BACK - 18,
        25,
        25,
        BACK - 8,
        125,
        105,
        70
    );
}


/*
 * Porta
 */

void draw_door(void)
{
    const int LEFT = -300;

    /*
     * Folha da porta
     */

    draw_box(
        LEFT + 5,
        -75,
        80,
        LEFT + 14,
        120,
        230,
        74,
        45,
        30
    );

    /*
     * Painel superior
     */

    draw_box(
        LEFT + 1,
        -62,
        100,
        LEFT + 6,
        -5,
        210,
        94,
        57,
        34
    );

    /*
     * Painel inferior
     */

    draw_box(
        LEFT + 1,
        15,
        100,
        LEFT + 6,
        100,
        210,
        92,
        55,
        32
    );

    /*
     * Batente frontal
     */

    draw_box(
        LEFT - 2,
        -85,
        68,
        LEFT + 15,
        125,
        82,
        105,
        72,
        40
    );

    /*
     * Batente traseiro
     */

    draw_box(
        LEFT - 2,
        -85,
        228,
        LEFT + 15,
        125,
        242,
        105,
        72,
        40
    );

    /*
     * Batente superior
     */

    draw_box(
        LEFT - 2,
        -85,
        68,
        LEFT + 15,
        -68,
        242,
        108,
        74,
        40
    );

    /*
     * Maçaneta
     */

    draw_box(
        LEFT - 1,
        5,
        202,
        LEFT + 9,
        18,
        216,
        155,
        125,
        65
    );
}


/*
 * Ventilador de teto desligado
 */

void draw_ceiling_fan(void)
{
    /*
     * Haste
     */

    draw_box(
        -8,
        -120,
        245,
        8,
        -78,
        255,
        65,
        65,
        65
    );

    /*
     * Corpo central
     */

    draw_box(
        -18,
        -85,
        235,
        18,
        -68,
        265,
        55,
        55,
        55
    );

    /*
     * Quatro pás
     */

    draw_box(
        -125,
        -78,
        243,
        -18,
        -70,
        253,
        72,
        72,
        68
    );

    draw_box(
        18,
        -78,
        235,
        125,
        -70,
        245,
        72,
        72,
        68
    );

    draw_box(
        -8,
        -78,
        130,
        8,
        -70,
        220,
        72,
        72,
        68
    );

    draw_box(
        -8,
        -78,
        270,
        8,
        -70,
        370,
        72,
        72,
        68
    );
}


/*
 * Móvel e TV ligada
 */

void draw_tv_area(void)
{
    /*
     * Rack
     */

    draw_box(
        -245,
        55,
        345,
        40,
        120,
        430,
        54,
        38,
        30
    );

    /*
     * Parte superior do rack
     */

    draw_box(
        -255,
        42,
        335,
        50,
        58,
        440,
        74,
        50,
        35
    );

    /*
     * TV
     */

    draw_box(
        -205,
        -45,
        350,
        -15,
        45,
        365,
        25,
        24,
        22
    );

    draw_box(
        -195,
        -38,
        342,
        -25,
        38,
        350,
        70,
        68,
        60
    );

    /*
     * Tela ligada
     */

    draw_box(
        -188,
        -30,
        337,
        -32,
        30,
        341,
        48,
        82,
        105
    );

    /*
     * Brilho da tela
     */

    draw_box(
        -180,
        -22,
        333,
        -42,
        20,
        337,
        63,
        101,
        125
    );

    /*
     * Base da TV
     */

    draw_box(
        -150,
        40,
        352,
        -65,
        55,
        365,
        45,
        40,
        35
    );
}


/*
 * Sofá
 */

void draw_sofa(void)
{
    /*
     * Base
     */

    draw_box(
        55,
        60,
        300,
        255,
        120,
        405,
        62,
        45,
        39
    );

    /*
     * Assento
     */

    draw_box(
        55,
        18,
        320,
        255,
        75,
        400,
        77,
        55,
        45
    );

    /*
     * Encosto
     */

    draw_box(
        55,
        -15,
        365,
        255,
        65,
        405,
        72,
        52,
        43
    );

    /*
     * Braço esquerdo
     */

    draw_box(
        45,
        0,
        305,
        78,
        105,
        410,
        68,
        49,
        40
    );

    /*
     * Braço direito
     */

    draw_box(
        235,
        0,
        305,
        270,
        105,
        410,
        68,
        49,
        40
    );

    /*
     * Almofadas
     */

    draw_box(
        88,
        -2,
        320,
        145,
        28,
        350,
        83,
        61,
        50
    );

    draw_box(
        155,
        -2,
        215,
        212,
        28,
        350,
        83,
        61,
        50
    );
}


/*
 * Mesa central
 */

void draw_coffee_table(void)
{
    draw_box(
        -80,
        70,
        235,
        70,
        88,
        325,
        70,
        47,
        30
    );

    draw_box(
        -70,
        88,
        245,
        60,
        100,
        315,
        90,
        60,
        38
    );

    /*
     * Copo abandonado
     */

    draw_box(
        -35,
        75,
        270,
        -20,
        85,
        282,
        115,
        115,
        105
    );

    /*
     * Revista
     */

    draw_box(
        5,
        82,
        250,
        45,
        86,
        290,
        75,
        65,
        48
    );
}


/*
 * Mesa lateral e luminária
 */

void draw_side_table(void)
{
    draw_box(
        220,
        65,
        145,
        275,
        115,
        190,
        62,
        42,
        30
    );

    draw_box(
        228,
        105,
        155,
        267,
        120,
        185,
        78,
        52,
        34
    );

    /*
     * Abajur
     */

    draw_box(
        235,
        5,
        155,
        260,
        75,
        175,
        48,
        38,
        28
    );

    draw_box(
        225,
        -5,
        150,
        270,
        10,
        180,
        72,
        52,
        28
    );

    draw_box(
        228,
        -5,
        152,
        268,
        8,
        178,
        125,
        88,
        42
    );
}


/*
 * Estante
 */

void draw_bookshelf(void)
{
    /*
     * Estrutura
     */

    draw_box(
        110,
        5,
        390,
        270,
        120,
        470,
        72,
        48,
        31
    );

    /*
     * Prateleiras
     */

    draw_box(
        105,
        8,
        385,
        275,
        18,
        475,
        95,
        62,
        38
    );

    draw_box(
        105,
        45,
        385,
        275,
        55,
        475,
        95,
        62,
        38
    );

    draw_box(
        105,
        82,
        385,
        275,
        92,
        475,
        95,
        62,
        38
    );

    /*
     * Livros - prateleira inferior
     */

    draw_box(
        125,
        -5,
        405,
        140,
        10,
        430,
        95,
        45,
        35
    );

    draw_box(
        143,
        -8,
        405,
        155,
        10,
        430,
        45,
        65,
        80
    );

    draw_box(
        158,
        -4,
        405,
        175,
        12,
        430,
        110,
        70,
        42
    );

    draw_box(
        182,
        -8,
        405,
        200,
        10,
        430,
        58,
        48,
        90
    );

    /*
     * Livros - prateleira do meio
     */

    draw_box(
        118,
        28,
        408,
        132,
        48,
        433,
        75,
        45,
        30
    );

    draw_box(
        138,
        25,
        408,
        151,
        50,
        433,
        50,
        70,
        65
    );

    draw_box(
        160,
        30,
        408,
        176,
        51,
        433,
        105,
        55,
        35
    );

    draw_box(
        190,
        24,
        408,
        202,
        50,
        433,
        55,
        55,
        75
    );

    draw_box(
        215,
        30,
        408,
        230,
        50,
        433,
        100,
        70,
        42
    );

    /*
     * Livros - prateleira superior
     */

    draw_box(
        120,
        65,
        410,
        136,
        88,
        432,
        50,
        60,
        85
    );

    draw_box(
        142,
        65,
        410,
        157,
        90,
        432,
        95,
        50,
        35
    );

    draw_box(
        165,
        70,
        410,
        181,
        90,
        432,
        60,
        85,
        55
    );

    draw_box(
        190,
        63,
        410,
        205,
        89,
        432,
        110,
        65,
        38
    );

    draw_box(
        215,
        68,
        410,
        231,
        90,
        432,
        52,
        58,
        90
    );

    /*
     * Livro inclinado no topo
     */

    draw_box(
        240,
        90,
        420,
        260,
        105,
        440,
        88,
        55,
        35
    );
}


/*
 * Cama
 */

void draw_bed(void)
{
    /*
     * Estrutura
     */

    draw_box(
        -265,
        45,
        80,
        -85,
        120,
        275,
        57,
        43,
        35
    );

    /*
     * Colchão
     */

    draw_box(
        -260,
        0,
        70,
        -90,
        55,
        275,
        91,
        79,
        65
    );

    /*
     * Cobertor bagunçado
     */

    draw_box(
        -245,
        -2,
        85,
        -105,
        25,
        260,
        70,
        67,
        62
    );

    draw_box(
        -180,
        20,
        130,
        -100,
        40,
        255,
        84,
        71,
        56
    );

    /*
     * Travesseiros
     */

    draw_box(
        -245,
        -5,
        85,
        -190,
        18,
        145,
        110,
        103,
        88
    );

    draw_box(
        -180,
        -2,
        85,
        -125,
        20,
        145,
        103,
        97,
        82
    );
}


/*
 * Planta
 */

void draw_plant(void)
{
    draw_box(
        -280,
        70,
        90,
        -225,
        120,
        135,
        70,
        43,
        28
    );

    /*
     * Tronco
     */

    draw_box(
        -265,
        25,
        100,
        -230,
        90,
        125,
        43,
        55,
        35
    );

    /*
     * Folhas
     */

    draw_box(
        -280,
        5,
        85,
        -235,
        55,
        110,
        35,
        65,
        35
    );

    draw_box(
        -265,
        15,
        105,
        -220,
        65,
        130,
        38,
        75,
        40
    );
    draw_box(
        -250,
        0,
        90,
        -210,
        45,
        105,
        45,
        80,
        42
    );
}


void draw_floor_mess(void)
{
    draw_box(
        -250,
        88,
        285,
        -205,
        118,
        325,
        105,
        72,
        40
    );

    draw_box(
        -150,
        90,
        300,
        -105,
        120,
        350,
        70,
        48,
        30
    );

    draw_box(
        -60,
        108,
        170,
        15,
        115,
        205,
        82,
        50,
        35
    );

    draw_box(
        -45,
        105,
        185,
        30,
        112,
        225,
        45,
        62,
        80
    );

    draw_box(
        20,
        108,
        390,
        70,
        115,
        430,
        95,
        55,
        35
    );

    draw_box(
        95,
        103,
        225,
        145,
        118,
        275,
        58,
        64,
        73
    );

    draw_box(
        130,
        100,
        225,
        175,
        117,
        270,
        72,
        48,
        75
    );

    draw_box(
        80,
        80,
        210,
        92,
        112,
        220,
        45,
        75,
        80
    );

    draw_box(
        10,
        92,
        290,
        45,
        98,
        305,
        40,
        40,
        42
    );
}


void draw_picture(void)
{
    draw_box(
        115,
        -75,
        492,
        215,
        -50,
        498,
        80,
        58,
        38
    );

    draw_box(
        123,
        -70,
        488,
        207,
        -55,
        493,
        42,
        48,
        58
    );

    draw_box(
        135,
        -67,
        485,
        195,
        -58,
        490,
        70,
        45,
        40
    );
}


void draw_small_objects(void)
{
    draw_box(
        -15,
        70,
        350,
        5,
        82,
        360,
        105,
        105,
        98
    );

    draw_box(
        15,
        55,
        375,
        60,
        95,
        415,
        112,
        76,
        38
    );

    draw_box(
        -115,
        45,
        360,
        -75,
        58,
        390,
        28,
        27,
        25
    );

    draw_box(
        -60,
        50,
        400,
        -35,
        57,
        425,
        75,
        52,
        35
    );
}


void draw_room(void)
{
    const int LEFT = -300;
    const int RIGHT = 300;

    const int FRONT = 0;
    const int BACK = 500;

    const int FLOOR = 120;
    const int CEILING = -120;

    draw_box(
        LEFT,
        FLOOR,
        FRONT,
        RIGHT,
        FLOOR + 4,
        BACK,
        125,
        125,
        118
    );

    draw_box(
        LEFT,
        CEILING,
        FRONT,
        RIGHT,
        CEILING + 5,
        BACK,
        30,
        30,
        34
    );

    draw_box(
        RIGHT,
        CEILING,
        FRONT,
        RIGHT + 4,
        FLOOR,
        BACK,
        58,
        54,
        50
    );

    draw_box(
        LEFT,
        CEILING,
        FRONT,
        LEFT + 4,
        FLOOR,
        80,
        62,
        57,
        52
    );

    draw_box(
        LEFT,
        CEILING,
        230,
        LEFT + 4,
        FLOOR,
        BACK,
        62,
        57,
        52
    );

    draw_box(
        LEFT,
        CEILING,
        80,
        LEFT + 4,
        -75,
        230,
        62,
        57,
        52
    );

    draw_box(
        LEFT,
        CEILING,
        BACK,
        -100,
        -50,
        BACK + 4,
        65,
        59,
        54
    );

    draw_box(
        100,
        CEILING,
        BACK,
        RIGHT,
        -50,
        BACK + 4,
        65,
        59,
        54
    );

    draw_box(
        -100,
        -50,
        BACK,
        100,
        50,
        BACK + 4,
        65,
        59,
        54
    );

    draw_box(
        LEFT,
        50,
        BACK,
        RIGHT,
        FLOOR,
        BACK + 4,
        65,
        59,
        54
    );

    draw_floor_tiles();
    draw_left_plaster();
    draw_right_plaster();
    draw_back_plaster();
    draw_window();
    draw_door();
    draw_ceiling_fan();
    draw_bed();
    draw_tv_area();
    draw_sofa();
    draw_coffee_table();
    draw_side_table();
    draw_bookshelf();
    draw_plant();
    draw_picture();
    draw_floor_mess();
    draw_small_objects();
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

