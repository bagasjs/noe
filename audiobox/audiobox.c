#include "miniaudio.h"
#include "noe.h"

int main(void)
{
    InitApplication();

    while(!WindowShouldClose()) {
        PollInputEvents();
        RenderFlush(GetDefaultShader());
        RenderPresent();
    }
    DeinitApplication();
    return 0;
}
