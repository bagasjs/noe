#include "noe.h"
#include <stdio.h>

#define GAME_TITLE "The Follower"
#define SPEED 100

int main(void)
{
    noe_Context *ctx = noe_init(GAME_TITLE, 800, 600, NOE_FLAG_DEFAULT);

    noe_Rect enemy = noe_rect(0, 0, 20, 20);

    double dt = 0;
    char title[1024] = {0};

    float threshold = 0.1f;

    double startime = noe_gettime();
    double playtime = 0;
    bool play = true;
    while(noe_step(ctx, &dt)) {
        if(noe_key_pressed(ctx, NOE_KEY_ESCAPE)) break;

        if(play) {
            snprintf(title, sizeof(title), GAME_TITLE);
            noe_set_window_title(ctx, title);

            noe_Vec2 cursor = noe_cursor_pos(ctx);
            noe_Vec2 direction = noe_vec2_sub(cursor, noe_vec2(enemy.x,enemy.y));
            if(noe_vec2_distance_sqr(direction) > threshold*threshold) {
                direction = noe_vec2_normalize(direction);
                enemy.x += direction.x * SPEED * dt;
                enemy.y += direction.y * SPEED * dt;
            } else {
                play = false;
                double now = noe_gettime();
                playtime = now - startime;
                enemy = noe_rect(0, 0, 20, 20);
            }
        } else {
            snprintf(title, sizeof(title), "Game over, Current score: %.2f. Press 'R' to replay", playtime);
            noe_set_window_title(ctx, title);
            if(noe_key_pressed(ctx, NOE_KEY_R)) {
                play = true;
                playtime = 0;
                startime = noe_gettime();
            }
        }

        noe_clear_background(ctx, NOE_WHITE);
        noe_draw_rect(ctx, NOE_RED, enemy);
    }

    noe_close(ctx);
}
