#include <stdint.h>

#include <psxgpu.h>
#include <psxgte.h>
#include <inline_c.h>
#include <psxetc.h>

#define SCREEN_XRES 320
#define SCREEN_YRES 240

#define OT_LEN 64
#define PACKET_LEN 256

typedef struct
{
    DISPENV disp;
    DRAWENV draw;

    uint32_t ot[OT_LEN];

    char packet[PACKET_LEN];

} DB;

DB db;

int main(void)
{
    SVECTOR v0;
    SVECTOR v1;
    SVECTOR v2;

    MATRIX mtx;

    POLY_F3 *poly;

    long depth;


    /*
     * =========================
     * GPU
     * =========================
     */

    ResetGraph(0);

    SetDefDispEnv(
        &db.disp,
        0,
        0,
        SCREEN_XRES,
        SCREEN_YRES
    );

    SetDefDrawEnv(
        &db.draw,
        0,
        0,
        SCREEN_XRES,
        SCREEN_YRES
    );

    setRGB0(
        &db.draw,
        10,
        10,
        15
    );

    db.draw.isbg = 1;

    PutDispEnv(&db.disp);
    PutDrawEnv(&db.draw);

    SetDispMask(1);


    /*
     * =========================
     * GTE
     * =========================
     */

    InitGeom();

    /*
     * Centro da tela.
     */
    gte_SetGeomOffset(
        SCREEN_XRES / 2,
        SCREEN_YRES / 2
    );

    /*
     * Distância focal.
     */
    gte_SetGeomScreen(
        SCREEN_XRES / 2
    );


    /*
     * Matriz de transformação.
     *
     * Rotação = 0
     * Translação Z = 600
     */

    SVECTOR rotation = {
        0,
        0,
        0,
        0
    };

    VECTOR position = {
        0,
        0,
        600
    };

    RotMatrix(
        &rotation,
        &mtx
    );

    TransMatrix(
        &mtx,
        &position
    );

    gte_SetRotMatrix(&mtx);
    gte_SetTransMatrix(&mtx);


    /*
     * =========================
     * TRIÂNGULO 3D
     * =========================
     *
     * Os três pontos estão
     * em coordenadas 3D.
     */

    v0.vx = -120;
    v0.vy = -80;
    v0.vz = 0;
    v0.pad = 0;

    v1.vx = 120;
    v1.vy = -80;
    v1.vz = 0;
    v1.pad = 0;

    v2.vx = 0;
    v2.vy = 120;
    v2.vz = 0;
    v2.pad = 0;


    /*
     * =========================
     * LOOP
     * =========================
     */

    while (1)
    {
        /*
         * Limpa Ordering Table.
         */
        ClearOTagR(
            db.ot,
            OT_LEN
        );


        /*
         * Cria o pacote do triângulo.
         */
        poly = (POLY_F3 *)db.packet;

        setPolyF3(poly);

        setRGB0(
            poly,
            180,
            180,
            180
        );


        /*
         * Carrega os três vértices
         * no GTE.
         */
        gte_ldv3(
            &v0,
            &v1,
            &v2
        );


        /*
         * Rotação + translação +
         * perspectiva.
         */
        gte_rtpt();


        /*
         * Recupera as coordenadas
         * projetadas pelo GTE.
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
         * Calcula profundidade média.
         */
        gte_avsz3();

        gte_stotz(
            &depth
        );


        /*
         * Converte profundidade para
         * o tamanho da Ordering Table.
         */
        depth >>= 3;


        if (depth < 0)
            depth = 0;

        if (depth >= OT_LEN)
            depth = OT_LEN - 1;


        /*
         * Coloca o triângulo na OT.
         */
        addPrim(
            &db.ot[depth],
            poly
        );


        /*
         * Envia para a GPU.
         */
        DrawOTag(
            &db.ot[OT_LEN - 1]
        );


        /*
         * Espera o frame.
         */
        VSync(0);
    }

    return 0;
}
