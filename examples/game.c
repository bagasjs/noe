#include "../noe.h"
#include <stdio.h>

#define GAME_TITLE "The Follower"

typedef struct Game {
    noe_Context *ctx;
    noe_Rect enemy;
    double dt;
    char title[1024];
    float threshold;
    int speed;
    double starttime;
    double playtime;
    bool play;
} Game;

void game_init(Game *game)
{
    game->enemy = noe_rect(0, 0, 20, 20);
    game->dt = 0;
    game->speed = 100.0;
    game->threshold = 1.0;
    game->starttime = noe_gettime();
    game->playtime = 0;
    game->play = true;
}

void game_update(Game *game)
{
    if(noe_key_pressed(game->ctx, NOE_KEY_ESCAPE)) {
        noe_set_should_close(game->ctx, true);
    }

    if(game->play) {
        snprintf(game->title, sizeof(game->title), 
                "%s - %dx%d", GAME_TITLE, 
                noe_screen_width(game->ctx), 
                noe_screen_height(game->ctx));
        noe_set_window_title(game->ctx, game->title);

        noe_Vec2 cursor = noe_cursor_pos(game->ctx);
        noe_Vec2 dir = noe_vec2_sub(cursor, 
                noe_vec2(game->enemy.x, game->enemy.y));
        float t2 = game->threshold*game->threshold;
        if(noe_vec2_distance_sqr(dir) > t2) {
            dir = noe_vec2_normalize(dir);
            game->enemy.x += dir.x * game->speed * game->dt;
            game->enemy.y += dir.y * game->speed * game->dt;
        } else {
            game->play = false;
            double now = noe_gettime();
            game->playtime = now - game->starttime;
            game->enemy = noe_rect(0, 0, 20, 20);
        }
    } else {
        snprintf(game->title, sizeof(game->title), 
            "Game over, Score: %.2f. Press 'R' to replay", 
            game->playtime);
        noe_set_window_title(game->ctx, game->title);
        if(noe_key_pressed(game->ctx, NOE_KEY_R)) {
            game->play = true;
            game->playtime = 0;
            game->starttime = noe_gettime();
        }
    }

    noe_clear_background(game->ctx, NOE_WHITE);
    noe_draw_rect(game->ctx, NOE_RED, game->enemy);
}

int main(void)
{
    Game game;
    game.ctx = noe_init(GAME_TITLE, 800, 600, NOE_FLAG_DEFAULT);

    game_init(&game);
    while(noe_step(game.ctx, &game.dt)) {
        game_update(&game);
    }

    noe_close(game.ctx);
}
