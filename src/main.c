#include <psx.h>

int main(void)
{
    ResetGraph(0);

    FntLoad(960, 256);

    while (1)
    {
        FntPrint(-1, "SOUTH SILENCE");
        FntPrint(-1, "\nPS1 PROTOTYPE");
        FntPrint(-1, "\n\nSYSTEM ONLINE");

        VSync(0);
    }

    return 0;
}
