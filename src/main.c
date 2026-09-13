#include <stdint.h>

#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>


#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define OT_LEN 256
#define PACKET_LEN 2048

#define CENTERX (SCREEN_XRES >> 1)
#define CENTERY (SCREEN_YRES >> 1)


/*
 * ============================================================
 * DOUBLE BUFFER
 * ============================================================
 */

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
 * CUBO
 *
 * Mesma convenção básica do exemplo oficial do GTE.
 * ============================================================
 */

SVECTOR cube_verts[] =
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
 * ============================================================
 * FACES
 * ============================================================
 */

typedef struct
{
    short v0;
    short v1;
    short v2;
    short v3;

} Face;


Face cube_faces[] =
{
    { 0, 1, 2, 3 },
    { 4, 5, 6, 7 },
    { 5, 4, 0, 1 },
    { 6, 7, 3, 2 },
    { 0, 2, 5, 7 },
    { 3, 1, 6, 4 }
};


#define CUBE_FACES 6


/*
 * ============================================================
 * INICIALIZAÇÃO
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
        CENTERX,
        CENTERY
    );


    gte_SetGeomScreen(
        CENTERX
    );


    /*
     * DISPLAY
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
 * DISPLAY
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
 * MAIN
 * ============================================================
 */

int main(void)
{
    /*
     * Rotação inicial.
     *
     * O cubo começa sem rotação.
     */

    SVECTOR rotation =
    {
        0,
        0,
        0,
        0
    };


    /*
     * Posição do cubo.
     *
     * O exemplo oficial usa Z = 400.
     */

    VECTOR position =
    {
        0,
        0,
        400
    };


    /*
     * Matriz de transformação.
     */

    MATRIX matrix;


    /*
     * Inicialização.
     */

    init_graphics();


    /*
     * ========================================================
     * LOOP
     * ========================================================
     */

    while (1)
    {
        POLY_F4 *poly;

        long depth;


        /*
         * ----------------------------------------------------
         * MATRIZ
         * ----------------------------------------------------
         */

        RotMatrix(
            &rotation,
            &matrix
        );


        TransMatrix(
            &matrix,
            &position
        );


        /*
         * ----------------------------------------------------
         * GTE
         * ----------------------------------------------------
         */

        gte_SetRotMatrix(
            &matrix
        );


        gte_SetTransMatrix(
            &matrix
        );


        /*
         * ----------------------------------------------------
         * RESERVA PRIMITIVA
         * ----------------------------------------------------
         */

        poly =
            (POLY_F4 *)db_nextpri;


        /*
         * ----------------------------------------------------
         * CADA FACE DO CUBO
         * ----------------------------------------------------
         */

        for (int i = 0; i < CUBE_FACES; i++)
        {
            Face *face;

            face =
                &cube_faces[i];


            /*
             * Carrega três vértices.
             */

            gte_ldv3(
                &cube_verts[face->v0],
                &cube_verts[face->v1],
                &cube_verts[face->v2]
            );


            /*
             * Rotação + translação + perspectiva.
             */

            gte_rtpt();


            /*
             * Backface culling.
             */

            gte_nclip();


            gte_stopz(
                &depth
            );


            /*
             * Se estiver virada para trás,
             * não desenha.
             */

            if (depth < 0)
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
                continue;


            if (depth >= OT_LEN)
                continue;


            /*
             * Configura quadrilátero.
             */

            setPolyF4(
                poly
            );


            /*
             * Primeiro triângulo.
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
             * Quarto vértice.
             */

            gte_ldv0(
                &cube_verts[face->v3]
            );


            gte_rtps();


            gte_stsxy(
                &poly->x3
            );


            /*
             * Cores diferentes por face.
             */

            if (i == 0)
            {
                setRGB0(
                    poly,
                    220,
                    40,
                    40
                );
            }
            else if (i == 1)
            {
                setRGB0(
                    poly,
                    40,
                    220,
                    40
                );
            }
            else if (i == 2)
            {
                setRGB0(
                    poly,
                    40,
                    80,
                    220
                );
            }
            else if (i == 3)
            {
                setRGB0(
                    poly,
                    220,
                    220,
                    40
                );
            }
            else if (i == 4)
            {
                setRGB0(
                    poly,
                    220,
                    40,
                    220
                );
            }
            else
            {
                setRGB0(
                    poly,
                    40,
                    220,
                    220
                );
            }


            /*
             * Ordering Table.
             */

            addPrim(
                db[db_active].ot + depth,
                poly
            );


            /*
             * Próxima primitiva.
             */

            poly++;
        }


        /*
         * Atualiza ponteiro.
         */

        db_nextpri =
            (char *)poly;


        /*
         * Mostra frame.
         */

        display();
    }


    return 0;
}
