#include "../noe.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendors/stb_image_write.h"

#define WINDOW_WIDTH 480
#define WINDOW_HEIGHT 480
#define WINDOW_TITLE "Paint"

typedef struct App {
    noe_Context *ctx;
    noe_Image canvas;
    noe_Color bg;
    noe_Color fg;
    char title[1024];
} App;

void app_init(App *app)
{
    app->canvas = noe_create_image(32, 32, 
            NOE_PIXELFORMAT_R8G8B8);
    app->bg = NOE_WHITE;
    app->fg = NOE_BLACK;

    noe_image_draw_rect(app->canvas, app->bg, noe_rect(0,0,
                app->canvas.w,app->canvas.h));

}

void app_update(App *app)
{
    noe_Vec2 pos = noe_cursor_pos(app->ctx);
    int xpos = (int)(pos.x*app->canvas.w/
            noe_screen_width(app->ctx));
    xpos = NOE_CLAMP(xpos, 0, app->canvas.w - 1);
    int ypos = (int)(pos.y*app->canvas.h/
            noe_screen_height(app->ctx));
    ypos = NOE_CLAMP(ypos, 0, app->canvas.h - 1);
    snprintf(app->title, sizeof(app->title), 
            "%s - W(%d,%d) C(%d,%d)", WINDOW_TITLE, 
            noe_screen_width(app->ctx), 
            noe_screen_height(app->ctx), 
            xpos, ypos);
    noe_set_window_title(app->ctx, app->title);

    if(noe_key_pressed(app->ctx, NOE_KEY_S)) {
        printf("Saving\n");
        stbi_write_png("result.png", 
                app->canvas.w, 
                app->canvas.h, 3, 
                app->canvas.pixels, 
                app->canvas.w*3);
    }
    if(noe_key_pressed(app->ctx, NOE_KEY_C)) {
        printf("Clear mode\n");
        NOE_SWAP(noe_Color, app->bg, app->fg);
    }
    if(noe_key_pressed(app->ctx, NOE_KEY_R)) {
        noe_Rect r = noe_rect(0,0, app->canvas.w, app->canvas.h);
        noe_image_draw_rect(app->canvas, app->bg, r);
    }

    noe_clear_background(app->ctx, NOE_WHITE); 
    if(noe_button_down(app->ctx, NOE_BUTTON_LEFT)) 
        noe_image_draw_pixel(app->canvas, app->fg, xpos, ypos);
    noe_draw_image_scaled_to_screen(app->ctx, app->canvas);
}

int main(void)
{
    App app;
    app.ctx = noe_init(WINDOW_TITLE, WINDOW_WIDTH, 
            WINDOW_HEIGHT, NOE_FLAG_DEFAULT);
    app_init(&app);
    while(noe_step(app.ctx, NULL)) {
        app_update(&app);
    }

    noe_close(app.ctx);
}
