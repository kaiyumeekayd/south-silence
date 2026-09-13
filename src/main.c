#include <stdint.h>

#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define OT_LEN      256
#define PACKET_LEN  16384

#define CENTER_X (SCREEN_XRES / 2)
#define CENTER_Y (SCREEN_YRES / 2)

/* ---------------------------------------------------------
   Render buffer
   --------------------------------------------------------- */

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
   Camera
   --------------------------------------------------------- */

typedef struct
{
    SVECTOR rotation;
    VECTOR position;

} Camera;


/* ---------------------------------------------------------
   Inicialização gráfica
   --------------------------------------------------------- */

void init_graphics(void)
{
    ResetGraph(0);

    /* Buffer 0 */

    SetDefDispEnv(&db[0].disp, 0, 0,
                  SCREEN_XRES,
                  SCREEN_YRES);

    SetDefDrawEnv(&db[0].draw,
                  SCREEN_XRES, 0,
                  SCREEN_XRES,
                  SCREEN_YRES);

    /* Buffer 1 */

    SetDefDispEnv(&db[1].disp,
                  SCREEN_XRES, 0,
                  SCREEN_XRES,
                  SCREEN_YRES);

    SetDefDrawEnv(&db[1].draw,
                  0, 0,
                  SCREEN_XRES,
                  SCREEN_YRES);

    /* Background */

    setRGB0(&db[0].draw, 20, 20, 24);
    setRGB0(&db[1].draw, 20, 20, 24);

    db[0].draw.isbg = 1;
    db[1].draw.isbg = 1;

    ClearOTagR(db[0].ot, OT_LEN);
    ClearOTagR(db[1].ot, OT_LEN);

    db_active = 0;

    db_nextpri = db[0].packet;

    /* GTE */

    InitGeom();

    gte_SetGeomOffset(CENTER_X, CENTER_Y);
    gte_SetGeomScreen(CENTER_X);

    PutDrawEnv(&db[0].draw);
    PutDispEnv(&db[0].disp);

    SetDispMask(1);
}


/* ---------------------------------------------------------
   Troca de buffer
   --------------------------------------------------------- */

void display(void)
{
    DrawSync(0);
    VSync(0);

    int old_buffer = db_active;

    db_active ^= 1;

    ClearOTagR(db[db_active].ot, OT_LEN);

    PutDrawEnv(&db[db_active].draw);
    PutDispEnv(&db[db_active].disp);

    DrawOTag(&db[old_buffer].ot[OT_LEN - 1]);

    db_nextpri = db[db_active].packet;
}


/* ---------------------------------------------------------
   Câmera
   --------------------------------------------------------- */

void set_camera(Camera *camera)
{
    MATRIX matrix;

    RotMatrix(&camera->rotation, &matrix);
    TransMatrix(&matrix, &camera->position);

    gte_SetRotMatrix(&matrix);
    gte_SetTransMatrix(&matrix);
}


/* ---------------------------------------------------------
   Triângulo 3D
   --------------------------------------------------------- */

void draw_triangle(
    SVECTOR *a,
    SVECTOR *b,
    SVECTOR *c,
    uint8_t r,
    uint8_t g,
    uint8_t bcolor)
{
    POLY_F3 *poly;

    long depth;

    poly = (POLY_F3 *)db_nextpri;

    setPolyF3(poly);

    setRGB0(poly, r, g, bcolor);

    gte_ldv3(a, b, c);

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


/* ---------------------------------------------------------
   Quadrado 3D
   --------------------------------------------------------- */

void draw_quad(
    SVECTOR *a,
    SVECTOR *b,
    SVECTOR *c,
    SVECTOR *d,
    uint8_t r,
    uint8_t g,
    uint8_t bcolor)
{
    draw_triangle(
        a, b, c,
        r, g, bcolor
    );

    draw_triangle(
        a, c, d,
        r, g, bcolor
    );
}


/* ---------------------------------------------------------
   Piso quadriculado
   --------------------------------------------------------- */

void draw_floor(void)
{
    int x;
    int z;

    for (z = -500; z < 300; z += 100)
    {
        for (x = -400; x < 400; x += 100)
        {
            SVECTOR a = { x,       150, z };
            SVECTOR b = { x + 100, 150, z };
            SVECTOR c = { x + 100, 150, z + 100 };
            SVECTOR d = { x,       150, z + 100 };

            if (((x / 100) + (z / 100)) & 1)
            {
                draw_quad(
                    &a, &b, &c, &d,
                    95, 70, 48
                );
            }
            else
            {
                draw_quad(
                    &a, &b, &c, &d,
                    125, 92, 58
                );
            }
        }
    }
}


/* ---------------------------------------------------------
   Teto
   --------------------------------------------------------- */

void draw_ceiling(void)
{
    SVECTOR a = { -400, -150, -500 };
    SVECTOR b = {  400, -150, -500 };
    SVECTOR c = {  400, -150,  300 };
    SVECTOR d = { -400, -150,  300 };

    draw_quad(
        &a, &b, &c, &d,
        70, 70, 78
    );
}


/* ---------------------------------------------------------
   Parede esquerda
   --------------------------------------------------------- */

void draw_left_wall(void)
{
    SVECTOR a = { -400, -150, -500 };
    SVECTOR b = { -400, -150,  300 };
    SVECTOR c = { -400,  150,  300 };
    SVECTOR d = { -400,  150, -500 };

    draw_quad(
        &a, &b, &c, &d,
        55, 100, 65
    );
}


/* ---------------------------------------------------------
   Parede direita
   --------------------------------------------------------- */

void draw_right_wall(void)
{
    SVECTOR a = { 400, -150,  300 };
    SVECTOR b = { 400, -150, -500 };
    SVECTOR c = { 400,  150, -500 };
    SVECTOR d = { 400,  150,  300 };

    draw_quad(
        &a, &b, &c, &d,
        65, 80, 125
    );
}


/* ---------------------------------------------------------
   Parede traseira
   --------------------------------------------------------- */

void draw_back_wall(void)
{
    /* Parede esquerda da janela */

    SVECTOR a1 = { -400, -150, -500 };
    SVECTOR b1 = { -120, -150, -500 };
    SVECTOR c1 = { -120,  150, -500 };
    SVECTOR d1 = { -400,  150, -500 };

    draw_quad(
        &a1, &b1, &c1, &d1,
        130, 65, 65
    );


    /* Parede direita da janela */

    SVECTOR a2 = { 120, -150, -500 };
    SVECTOR b2 = { 400, -150, -500 };
    SVECTOR c2 = { 400,  150, -500 };
    SVECTOR d2 = { 120,  150, -500 };

    draw_quad(
        &a2, &b2, &c2, &d2,
        130, 65, 65
    );


    /* Parede acima da janela */

    SVECTOR a3 = { -120, -150, -500 };
    SVECTOR b3 = { 120, -150, -500 };
    SVECTOR c3 = { 120, -60, -500 };
    SVECTOR d3 = { -120, -60, -500 };

    draw_quad(
        &a3, &b3, &c3, &d3,
        130, 65, 65
    );


    /* Parede abaixo da janela */

    SVECTOR a4 = { -120, 60, -500 };
    SVECTOR b4 = { 120, 60, -500 };
    SVECTOR c4 = { 120, 150, -500 };
    SVECTOR d4 = { -120, 150, -500 };

    draw_quad(
        &a4, &b4, &c4, &d4,
        130, 65, 65
    );
}


/* ---------------------------------------------------------
   Janela
   --------------------------------------------------------- */

void draw_window(void)
{
    /* Vidro */

    SVECTOR a = { -110, -50, -490 };
    SVECTOR b = {  110, -50, -490 };
    SVECTOR c = {  110,  50, -490 };
    SVECTOR d = { -110,  50, -490 };

    draw_quad(
        &a, &b, &c, &d,
        40, 100, 150
    );


    /* Moldura esquerda */

    SVECTOR l1 = { -125, -65, -480 };
    SVECTOR l2 = { -110, -65, -480 };
    SVECTOR l3 = { -110,  65, -480 };
    SVECTOR l4 = { -125,  65, -480 };

    draw_quad(
        &l1, &l2, &l3, &l4,
        90, 60, 35
    );


    /* Moldura direita */

    SVECTOR r1 = { 110, -65, -480 };
    SVECTOR r2 = { 125, -65, -480 };
    SVECTOR r3 = { 125,  65, -480 };
    SVECTOR r4 = { 110,  65, -480 };

    draw_quad(
        &r1, &r2, &r3, &r4,
        90, 60, 35
    );


    /* Moldura superior */

    SVECTOR t1 = { -125, -65, -480 };
    SVECTOR t2 = { 125, -65, -480 };
    SVECTOR t3 = { 125, -50, -480 };
    SVECTOR t4 = { -125, -50, -480 };

    draw_quad(
        &t1, &t2, &t3, &t4,
        90, 60, 35
    );


    /* Moldura inferior */

    SVECTOR b1 = { -125, 50, -480 };
    SVECTOR b2 = { 125, 50, -480 };
    SVECTOR b3 = { 125, 65, -480 };
    SVECTOR b4 = { -125, 65, -480 };

    draw_quad(
        &b1, &b2, &b3, &b4,
        90, 60, 35
    );
}


/* ---------------------------------------------------------
   Cama
   --------------------------------------------------------- */

void draw_bed(void)
{
    /* Colchão */

    SVECTOR a = { -260, 80, -100 };
    SVECTOR b = { -20,  80, -100 };
    SVECTOR c = { -20,  80, 100 };
    SVECTOR d = { -260, 80, 100 };

    draw_quad(
        &a, &b, &c, &d,
        175, 175, 180
    );

    /* Base */

    SVECTOR a2 = { -260, 80, -100 };
    SVECTOR b2 = { -20,  80, -100 };
    SVECTOR c2 = { -20, 140, 100 };
    SVECTOR d2 = { -260, 140, 100 };

    draw_quad(
        &a2, &b2, &c2, &d2,
        90, 45, 30
    );
}


/* ---------------------------------------------------------
   Mesa
   --------------------------------------------------------- */

void draw_table(void)
{
    /* Tampo */

    SVECTOR a = { 120, 70, 80 };
    SVECTOR b = { 280, 70, 80 };
    SVECTOR c = { 280, 70, 200 };
    SVECTOR d = { 120, 70, 200 };

    draw_quad(
        &a, &b, &c, &d,
        160, 105, 55
    );

    /* Perna 1 */

    SVECTOR p1 = { 130, 70, 90 };
    SVECTOR p2 = { 145, 70, 90 };
    SVECTOR p3 = { 145, 150, 90 };
    SVECTOR p4 = { 130, 150, 90 };

    draw_quad(
        &p1, &p2, &p3, &p4,
        100, 60, 30
    );


    /* Perna 2 */

    SVECTOR q1 = { 255, 70, 90 };
    SVECTOR q2 = { 270, 70, 90 };
    SVECTOR q3 = { 270, 150, 90 };
    SVECTOR q4 = { 255, 150, 90 };

    draw_quad(
        &q1, &q2, &q3, &q4,
        100, 60, 30
    );
}


/* ---------------------------------------------------------
   Caixa
   --------------------------------------------------------- */

void draw_box(void)
{
    SVECTOR a = { 20, 100, 230 };
    SVECTOR b = { 100, 100, 230 };
    SVECTOR c = { 100, 150, 230 };
    SVECTOR d = { 20, 150, 230 };

    draw_quad(
        &a, &b, &c, &d,
        180, 150, 35
    );
}


/* ---------------------------------------------------------
   MAIN
   --------------------------------------------------------- */

int main(void)
{
    init_graphics();

    /*
       A câmera é elevada e inclinada.

       768 = rotação aproximada de 67,5 graus
       no eixo X.

       A posição Z coloca a sala dentro
       do volume visível do GTE.
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


    for (;;)
    {
        /* Câmera */

        set_camera(&camera);


        /* Sala */

        draw_floor();

        draw_ceiling();

        draw_left_wall();

        draw_right_wall();

        draw_back_wall();

        draw_window();


        /* Móveis */

        draw_bed();

        draw_table();

        draw_box();


        /* Próximo frame */

        display();
    }

    return 0;
}
