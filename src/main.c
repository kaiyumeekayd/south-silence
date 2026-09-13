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
 * Índices dos vértices de cada face.
 *
 * Cada face é formada por dois triângulos.
 */
typedef struct
{
    short v0;
    short v1;
    short v2;
    short v3;
} Face;


/*
 * 8 vértices do cubo.
 */
SVECTOR cube_vertices[] =
{
    { -100, -100, -100, 0 },
    {  100, -100, -100, 0 },
    { -100,  100, -100, 0 },
    {  100,  100, -100, 0 },

    {  100, -100,  100, 0 },
    { -100, -100,  100, 0 },
    {  100,  100,  100, 0 },
    { -100,  100,  100, 0 }
};


/*
 * 6 faces.
 */
Face cube_faces[] =
{
    { 0, 1, 2, 3 },
    { 4, 5, 6, 7 },
    { 5, 4, 0, 1 },
    { 6, 7, 3, 2 },
    { 0, 2, 5, 7 },
    { 3, 1, 6, 4 }
};


void init_graphics(void)
{
    ResetGraph(0);

    /*
     * Buffer 0
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
        12,
        12,
        16
    );

    db[0].draw.isbg = 1;


    /*
     * Buffer 1
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
        12,
        12,
        16
    );

    db[1].draw.isbg = 1;


    /*
     * Ordering Tables.
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

    PutDrawEnv(
        &db[0].draw
    );


    /*
     * Inicializa GTE.
     */
    InitGeom();

    /*
     * Centro da tela.
     */
    gte_SetGeomOffset(
        SCREEN_XRES >> 1,
        SCREEN_YRES >> 1
    );

    /*
     * FOV / distância de projeção.
     */
    gte_SetGeomScreen(
        SCREEN_XRES >> 1
    );

    SetDispMask(1);
}


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
        db[1 - db_active].ot +
        (OT_LEN - 1)
    );
}


int main(void)
{
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

    MATRIX matrix;

    init_graphics();


    while (1)
    {
        POLY_F3 *poly;

        int i;

        long depth;
        long clip;


        /*
         * Matriz de transformação.
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
         * Começa a escrever os polígonos
         * no buffer atual.
         */
        poly =
            (POLY_F3 *)db_nextpri;


        /*
         * 6 faces.
         */
        for (i = 0; i < 6; i++)
        {
            /*
             * Primeiro triângulo
             * da face.
             */
            gte_ldv3(
                &cube_vertices[cube_faces[i].v0],
                &cube_vertices[cube_faces[i].v1],
                &cube_vertices[cube_faces[i].v2]
            );

            /*
             * Transformação 3D
             * + perspectiva.
             */
            gte_rtpt();

            /*
             * Backface culling.
             */
            gte_nclip();

            gte_stopz(
                &clip
            );

            /*
             * Se estiver virado para
             * trás, não desenha.
             */
            if (clip < 0)
                continue;


            /*
             * Profundidade média.
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
             * Primeiro triângulo.
             */
            setPolyF3(poly);

            setRGB0(
                poly,
                150,
                150,
                150
            );

            gte_stsxy0(
                &poly->x0
            );

            gte_stsxy1(
                &poly->x1
            );

            gte_stsxy2(
                &poly->x2
            );

            addPrim(
                db[db_active].ot + depth,
                poly
            );

            poly++;


            /*
             * Segundo triângulo da face.
             *
             * Precisamos transformar os três
             * vértices correspondentes.
             */
            gte_ldv3(
                &cube_vertices[cube_faces[i].v2],
                &cube_vertices[cube_faces[i].v1],
                &cube_vertices[cube_faces[i].v3]
            );

            gte_rtpt();

            gte_nclip();

            gte_stopz(
                &clip
            );

            if (clip < 0)
                continue;

            gte_avsz3();

            gte_stotz(
                &depth
            );

            depth >>= 2;


            if (depth < 0)
                depth = 0;

            if (depth >= OT_LEN)
                depth = OT_LEN - 1;


            setPolyF3(poly);

            setRGB0(
                poly,
                100,
                100,
                100
            );

            gte_stsxy0(
                &poly->x0
            );

            gte_stsxy1(
                &poly->x1
            );

            gte_stsxy2(
                &poly->x2
            );

            addPrim(
                db[db_active].ot + depth,
                poly
            );

            poly++;
        }


        /*
         * Atualiza ponteiro para o próximo
         * pacote.
         */
        db_nextpri =
            (char *)poly;


        /*
         * Gira o cubo.
         */
        rotation.vx += 8;
        rotation.vy += 12;


        /*
         * Troca o framebuffer.
         */
        display();
    }

    return 0;
}
