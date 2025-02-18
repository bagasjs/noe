#include "../noe.h"
#include "../noe_ext.h"

#define MICROUI_IMPLEMENTATION
#include "../vendors/microui.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendors/stb_image_write.h"

#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE "Paint"


int main(void)
{
    char title[1024] = WINDOW_TITLE;

    noe_Context *ctx = noe_init(WINDOW_TITLE, WINDOW_WIDTH, 
            WINDOW_HEIGHT, NOE_FLAG_DEFAULT);

    noe_Image canvas = noe_create_image(32, 32, 
            NOE_PIXELFORMAT_R8G8B8);
    noe_Color bg = NOE_WHITE;
    noe_Color fg = NOE_BLACK;
    noe_image_draw_rect(canvas, bg, noe_rect(0,0, canvas.w,canvas.h));

    while(noe_step(ctx, NULL)) {
        noe_Vec2 pos = noe_cursor_pos(ctx);
        int xpos = (int)(pos.x*canvas.w/
                noe_screen_width(ctx));
        xpos = NOE_CLAMP(xpos, 0, canvas.w - 1);
        int ypos = (int)(pos.y*canvas.h/
                noe_screen_height(ctx));
        ypos = NOE_CLAMP(ypos, 0, canvas.h - 1);
        snprintf(title, sizeof(title), 
                "%s - W(%d,%d) C(%d,%d)", WINDOW_TITLE, 
                noe_screen_width(ctx), 
                noe_screen_height(ctx), 
                xpos + 1, ypos + 1);
        noe_set_window_title(ctx, title);

        if(noe_key_pressed(ctx, NOE_KEY_S)) {
            printf("Saving\n");
            noe_image_save_to_pngfile(canvas, "result.png");
        }
        if(noe_key_pressed(ctx, NOE_KEY_C)) {
            printf("Clear mode\n");
            NOE_SWAP(noe_Color, bg, fg);
        }
        if(noe_key_pressed(ctx, NOE_KEY_R)) {
            noe_Rect r = noe_rect(0,0, canvas.w, canvas.h);
            noe_image_draw_rect(canvas, bg, r);
        }

        noe_clear_background(ctx, NOE_WHITE); 
        if(noe_button_down(ctx, NOE_BUTTON_LEFT)) 
            noe_image_draw_pixel(canvas, fg, xpos, ypos);
        if(noe_button_down(ctx, NOE_BUTTON_RIGHT)) 
            noe_image_draw_pixel(canvas, bg, xpos, ypos);
        noe_draw_image_scaled_to_screen(ctx, canvas);
    }

    noe_close(ctx);
}
