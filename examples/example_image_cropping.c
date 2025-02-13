#include "../noe.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../vendors/stb_image.h"

int main(void)
{
    noe_Context *c = noe_init("Test", 800, 600, 0);
    int w, h, comp;

    stbi_uc *data = stbi_load("res/example_cropping.png", &w, &h, &comp, 0);
    noe_Image img = noe_load_image(data, w, h,comp == 4 ? NOE_PIXELFORMAT_R8G8B8A8 : NOE_PIXELFORMAT_R8G8B8);
    noe_Rect src = noe_rect(0, 0, 16, 16);
    noe_Rect dst = noe_rect(0, 0, 100, 100);

    while(noe_step(c, NULL)) {
        noe_clear_background(c, NOE_WHITE);

        if(noe_key_pressed(c, NOE_KEY_SPACE)) {
            src.x += src.w;
            if(src.x + src.w > img.w) {
                src.x = 0;
                src.y += src.h;
                if(src.y + src.h > img.w) {
                    src.y = 0;
                }
            }
        }

        noe_draw_image2(c, img, src, dst);
    }
    stbi_image_free(data);
    noe_close(c);
}
