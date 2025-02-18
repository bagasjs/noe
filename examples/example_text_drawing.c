#include "../noe.h"
#include "../noe_ext.h"

int main(void)
{
    noe_Context *c = noe_init("Drawing font example", 800, 600, 0);
    noe_Font font = noe_load_font_from_ttf("./res/firacode.ttf", 64);
    noe_font_to_c_header(font, "firacode.h");
    while(noe_step(c, NULL)) {
        noe_clear_background(c, NOE_BLACK);
        noe_draw_text(c, font, "Hello, World", 100, 100, 24);
        noe_draw_text(c, font, "Hello, World", 100, 200, 24);
    }
    noe_unload_font(font);
    noe_close(c);
}
