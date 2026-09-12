#include <stdint.h>
#include <psxgpu.h>
#include <psxetc.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

#define FOCAL_LENGTH 256

typedef struct
{
    long x;
    long y;
    long z;
} Vertex3D;

static int project_x(Vertex3D v)
{
    return 160 + (int)((v.x * FOCAL_LENGTH) / v.z);
}

static int project_y(Vertex3D v)
{
    return 120 - (int)((v.y * FOCAL_LENGTH) / v.z);
}

int main(void)
{
    DISPENV disp;
    DRAWENV draw;

    ResetGraph(0);

    SetDefDispEnv(&disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    setRGB0(&draw, 12, 12, 16);
    draw.isbg = 1;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    SetDispMask(1);

    /*
     * Três pontos no espaço 3D.
     *
     * Os dois primeiros estão a Z=350.
     * O terceiro está mais distante, em Z=500.
     *
     * Isso cria perspectiva.
     */
    Vertex3D vertices[3] =
    {
        { -80,  60, 350 },
        {  80,  60, 350 },
        {   0, -60, 500 }
    };

    int x0 = project_x(vertices[0]);
    int y0 = project_y(vertices[0]);

    int x1 = project_x(vertices[1]);
    int y1 = project_y(vertices[1]);

    int x2 = project_x(vertices[2]);
    int y2 = project_y(vertices[2]);

    uint32_t ot[1];

    ClearOTagR(ot, 1);

    POLY_F3 triangle;

    setPolyF3(&triangle);

    setRGB0(&triangle, 100, 100, 100);

    setXY3(
        &triangle,
        x0, y0,
        x2, y2,
        x1, y1
    );

    addPrim(&ot[0], &triangle);

    DrawOTag(&ot[0]);

    while (1)
    {
        VSync(0);
    }

    return 0;
}
