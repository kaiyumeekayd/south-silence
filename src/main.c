#include <sys/types.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxapi.h>

#define OT_LEN 8

static DISPENV disp;
static DRAWENV draw;

static u_long ot[OT_LEN];
static POLY_F3 poly;

/* Vértices do triângulo em 3D */
static SVECTOR vertices[3] = {
    {-60,  50, 400, 0},
    { 60,  50, 400, 0},
    {  0, -60, 400, 0}
};

/* Coordenadas resultantes na tela */
static long screen_x[3];
static long screen_y[3];

int main(void)
{
    MATRIX matrix;
    VECTOR position;
    long depth;
    long flag;

    ResetGraph(0);

    SetDefDispEnv(&disp, 0, 0, 320, 240);
    SetDefDrawEnv(&draw, 0, 0, 320, 240);

    draw.isbg = 1;
    draw.r0 = 15;
    draw.g0 = 15;
    draw.b0 = 15;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);

    /* Inicializa o GTE */
    InitGeom();

    /* Centro da tela */
    SetGeomOffset(160, 120);

    /* Distância focal */
    SetGeomScreen(256);

    /* Câmera sem rotação */
    RotMatrix(&((SVECTOR){0, 0, 0, 0}), &matrix);

    /* Posição da câmera */
    position.vx = 0;
    position.vy = 0;
    position.vz = 0;

    TransMatrix(&matrix, &position);

    SetRotMatrix(&matrix);
    SetTransMatrix(&matrix);

    while (1)
    {
        DrawSync(0);
        VSync(0);

        ClearOTagR(ot, OT_LEN);

        /*
         * Transforma os três vértices 3D
         * em coordenadas 2D.
         */
        for (int i = 0; i < 3; i++)
        {
            SVECTOR *v = &vertices[i];

            flag = RotTransPers(
                v,
                (long *)&screen_x[i],
                (long *)&screen_y[i],
                &depth,
                &flag
            );
        }

        setPolyF3(&poly);

        setRGB0(&poly, 180, 180, 180);

        setXY3(
            &poly,
            screen_x[0], screen_y[0],
            screen_x[1], screen_y[1],
            screen_x[2], screen_y[2]
        );

        addPrim(&ot[0], &poly);

        DrawOTag(&ot[0]);
    }

    return 0;
}
