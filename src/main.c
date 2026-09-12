#include <psxgpu.h>
#include <psxetc.h>

#define SCREEN_WIDTH  320
#define SCREEN_HEIGHT 240

int main(void)
{
    DISPENV disp;
    DRAWENV draw;

    /* Inicializa o GPU */
    ResetGraph(0);

    /* Configura tela */
    SetDefDispEnv(&disp, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    SetDefDrawEnv(&draw, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    /* Fundo vermelho */
    setRGB0(&draw, 100, 0, 0);
    draw.isbg = 1;

    PutDispEnv(&disp);
    PutDrawEnv(&draw);

    /* Liga a saída de vídeo */
    SetDispMask(1);

    while (1)
    {
        VSync(0);
    }

    return 0;
}
