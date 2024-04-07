#include "noe.h"
#include "nomath.h"

#define WIDTH 800
#define HEIGHT 600

int main(void)
{
    InitApplication();
    Shader shader = GetDefaultShader();
    Matrix matrix = MatrixOrthographic(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);

    while(!WindowShouldClose()) {
    }

    DeinitApplication();
    return 0;
}
