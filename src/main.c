#include <sys/types.h>
#include <psxgpu.h>
#include <psxgte.h>
#include <psxapi.h>

int main(void)
{
    ResetGraph(0);

    while (1)
    {
        VSync(0);
    }

    return 0;
}
