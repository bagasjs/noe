# Noe
A small game engine written in C. I took some inspiration from this two library
- [github.com/raysan5/raylib](https://github.com/raysan5/raylib)
- [github.com/rxi/kit](https://github.com/rxi/kit)

## Example
```c
#include "noe.h"
int main(void)
{
    noe_Context *ctx = noe_create("Application Name", 800, 600, NOE_FLAG_DEFAULT);
    while(noe_step(ctx, NULL)) {
        noe_clear_background(NOE_WHITE);
        noe_draw_text(ctx, "Hello, World", 10, 10, 16);
    }
    noe_destroy(ctx);
}
```
