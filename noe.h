/*
** Copyright (c) 2025 bagasjs
**
** This library is free software; you can redistribute it and/or modify it
** under the terms of the MIT license. See the bottom of this file for details.
*/

#ifndef NOE_H_
#define NOE_H_

#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#ifndef NOE_ASSERT
#include <assert.h>
#define NOE_ASSERT(cond) assert(cond)
#endif

enum noe_key {
    NOE_KEY_NONE = 0,

    NOE_KEY_ESCAPE = 27,
    NOE_KEY_A = 65,
    NOE_KEY_B = 66,
    NOE_KEY_C = 67,
    NOE_KEY_D = 68,
    NOE_KEY_E = 69,
    NOE_KEY_F = 70,
    NOE_KEY_G = 71,
    NOE_KEY_H = 72,
    NOE_KEY_I = 73,
    NOE_KEY_J = 74,
    NOE_KEY_K = 75,
    NOE_KEY_L = 76,
    NOE_KEY_M = 77,
    NOE_KEY_N = 78,
    NOE_KEY_O = 79,
    NOE_KEY_P = 80,
    NOE_KEY_Q = 81,
    NOE_KEY_R = 82,
    NOE_KEY_S = 83,
    NOE_KEY_T = 84,
    NOE_KEY_U = 85,
    NOE_KEY_V = 86,
    NOE_KEY_W = 87,
    NOE_KEY_X = 88,
    NOE_KEY_Y = 89,
    NOE_KEY_Z = 90,

    // Numbers
    NOE_KEY_0 = 48,
    NOE_KEY_1 = 49,
    NOE_KEY_2 = 50,
    NOE_KEY_3 = 51,
    NOE_KEY_4 = 52,
    NOE_KEY_5 = 53,
    NOE_KEY_6 = 54,
    NOE_KEY_7 = 55,
    NOE_KEY_8 = 56,
    NOE_KEY_9 = 57,

    // Punctuation and symbols
    NOE_KEY_SPACE = 32,
    NOE_KEY_COMMA = 44,
    NOE_KEY_PERIOD = 46,
    NOE_KEY_SEMICOLON = 59,
    NOE_KEY_COLON = 58,
    NOE_KEY_APOSTROPHE = 39,
    NOE_KEY_QUOTE = 34,
    NOE_KEY_SLASH = 47,
    NOE_KEY_BACKSLASH = 92,
    NOE_KEY_MINUS = 45,
    NOE_KEY_EQUAL = 61,
    NOE_KEY_LEFT_BRACKET = 91,
    NOE_KEY_RIGHT_BRACKET = 93,
    NOE_KEY_GRAVE = 96,
    NOE_KEY_TILDE = 126,

    // Control keys
    NOE_KEY_BACKSPACE = 8,
    NOE_KEY_TAB = 9,
    NOE_KEY_ENTER = 13,
    NOE_KEY_SHIFT = 16,
    NOE_KEY_CONTROL = 17,
    NOE_KEY_ALT = 18,
    NOE_KEY_CAPSLOCK = 20,

    // Function keys
    NOE_KEY_F1 = 112,
    NOE_KEY_F2 = 113,
    NOE_KEY_F3 = 114,
    NOE_KEY_F4 = 115,
    NOE_KEY_F5 = 116,
    NOE_KEY_F6 = 117,
    NOE_KEY_F7 = 118,
    NOE_KEY_F8 = 119,
    NOE_KEY_F9 = 120,
    NOE_KEY_F10 = 121,
    NOE_KEY_F11 = 122,
    NOE_KEY_F12 = 123,

    // Arrow keys
    NOE_KEY_UP = 38,
    NOE_KEY_DOWN = 40,
    NOE_KEY_LEFT = 37,
    NOE_KEY_RIGHT = 39,

    // Additional keys
    NOE_KEY_INSERT = 45,
    NOE_KEY_DELETE = 46,
    NOE_KEY_HOME = 36,
    NOE_KEY_END = 35,
    NOE_KEY_PAGE_UP = 33,
    NOE_KEY_PAGE_DOWN = 34,
};

enum noe_pixelformat {
    NOE_PIXELFORMAT_R8G8B8A8 = 0,
    NOE_PIXELFORMAT_R8G8B8,
    NOE_PIXELFORMAT_B8G8R8A8,
    NOE_PIXELFORMAT_B8G8R8,
    NOE_PIXELFORMAT_GRAYSCALE,

    _COUNT_NOE_PIXELFORMATS,
};

enum noe_keymod {
    NOE_KEYMOD_SHIFT    = (1 << 1),
    NOE_KEYMOD_CONTROL  = (1 << 2),
    NOE_KEYMOD_ALT      = (1 << 3),
    NOE_KEYMOD_SUPER    = (1 << 4),
    NOE_KEYMOD_CAPSLOCK = (1 << 5),
    NOE_KEYMOD_NUMLOCK  = (1 << 6),
};

enum noe_button {
    NOE_BUTTON_PRIMARY = 0,
    NOE_BUTTON_LEFT  = NOE_BUTTON_PRIMARY,
    NOE_BUTTON_RIGHT,
    NOE_BUTTON_MIDDLE,
};

enum noe_flag {
    NOE_FLAG_VISIBLE    = (1 << 0),
    NOE_FLAG_RESIZABLE  = (1 << 0),
    NOE_FLAG_FULLSCREEN = (1 << 1),
    NOE_FLAG_USE_OPENGL = (1 << 2),
};

enum noe_resize_strategy {
    NOE_RESIZE_NEAREST = 0,
    NOE_RESIZE_LINEAR = 1,
};

#define NOE_FLAG_DEFAULT (NOE_FLAG_VISIBLE | NOE_FLAG_RESIZABLE)

#ifndef NOE_CLITERAL
#ifdef __cplusplus
#define NOE_CLITERAL(T) T
#else
#define NOE_CLITERAL(T) (T)
#endif
#endif

#define NOE_MIN(a, b) ((a) < (b) ? (a) : (b))
#define NOE_MAX(a, b) ((a) > (b) ? (a) : (b))
#define NOE_CLAMP(v, min, max) NOE_MIN(NOE_MAX((v), (min)), (max))
#define NOE_SWAP(T, a, b) do { T t = (a); (a) = (b); (b) = t; } while(0)

#if !defined(NOE_MALLOC) && !defined(NOE_FREE)
#include <stdlib.h>
#define NOE_MALLOC malloc
#define NOE_FREE free
#endif

#if !defined(NOE_MALLOC) || !defined(NOE_FREE)
#error "Please define both the NOE_MALLOC and NOE_FREE macros"
#endif

typedef struct { float x, y; } noe_Vec2;
typedef struct { float x, y, z; } noe_Vec3;
typedef struct { float x, y, z, w; } noe_Vec4;
typedef union { noe_Vec4 rows[4]; float es[4*4]; } noe_Mat4;
typedef struct { int x, y, w, h; } noe_Rect;
typedef struct { uint8_t r, g, b, a; } noe_Color;

typedef struct { 
    void *texture;
    uint8_t *pixels; 
    int format;
    int w, h; 
} noe_Image;

typedef struct noe_Glyph {
    int codepoint;
    float l, t;
    float r, b;
} noe_Glyph;

typedef struct noe_Font {
    noe_Image atlas;
    noe_Glyph *codepoints;
    uint32_t codepoints_count;
} noe_Font;

#define noe_rgb(R, G, B) noe_rgba(R, G, B, 0xFF)
#define noe_rgba(R, G, B, A) NOE_CLITERAL(noe_Color){ .r = (R), .g = (G), .b = (B), .a = (A) }
#define noe_rect(X, Y, W, H) NOE_CLITERAL(noe_Rect){ .x = (X), .y = (Y), .w = (W), .h = (H) }
#define noe_vec2(X, Y) NOE_CLITERAL(noe_Vec2){.x=(X), .y=(Y)}
#define noe_vec3(X, Y, Z) NOE_CLITERAL(noe_Vec3){.x=(X), .y=(Y), .z=(Z)}
#define noe_vec4(X, Y, Z, W) NOE_CLITERAL(noe_Vec4){.x=(X), .y=(Y), .z=(Z), .w=(W)}

#define NOE_BLACK noe_rgba(0x00, 0x00, 0x00, 0xFF)
#define NOE_WHITE noe_rgba(0xFF, 0xFF, 0xFF, 0xFF)
#define NOE_RED   noe_rgba(0xFF, 0x00, 0x00, 0xFF)
#define NOE_GREEN noe_rgba(0x00, 0xFF, 0x00, 0xFF)
#define NOE_BLUE  noe_rgba(0x00, 0x00, 0xFF, 0xFF)

typedef struct noe_Context noe_Context;

void noe_sleep(int milis);
double noe_gettime(void);

noe_Rect noe_clip_rect(noe_Rect outer, noe_Rect inner);

noe_Context *noe_init(const char *name, int w, int h, uint8_t flags);
void noe_close(noe_Context *ctx);
void noe_set_should_close(noe_Context *ctx, bool should_close);
void noe_set_window_title(noe_Context *ctx, const char *title);
bool noe_step(noe_Context *ctx, double *deltaTime);
bool noe_key_pressed(noe_Context *ctx, int key);
bool noe_key_released(noe_Context *ctx, int key);
bool noe_key_down(noe_Context *ctx, int key);
bool noe_key_up(noe_Context *ctx, int key);
bool noe_button_pressed(noe_Context *ctx, int button);
bool noe_button_down(noe_Context *ctx, int button);
bool noe_button_released(noe_Context *ctx, int button);
noe_Vec2 noe_cursor_pos(noe_Context *ctx);
noe_Vec2 noe_cursor_delta(noe_Context *ctx);
int noe_screen_width(noe_Context *ctx);
int noe_screen_height(noe_Context *ctx);
bool noe_screen_resized(noe_Context *ctx);

noe_Image noe_load_image(void *data, int width, int height, int format);
noe_Image noe_create_image(int width, int height, int format);
void noe_destroy_image(noe_Image image);
void noe_image_resize(noe_Image *dst, noe_Image src, noe_Rect r, int min, int mag);
void noe_image_draw_pixel(noe_Image image, noe_Color color, int x, int y);
noe_Color noe_image_get_pixel(noe_Image image, int x, int y);
void noe_image_draw_rect(noe_Image image, noe_Color c, noe_Rect r);

void noe_clear_background(noe_Context *ctx, noe_Color color);
void noe_draw_rect(noe_Context *ctx, noe_Color color, noe_Rect r);
void noe_draw_image(noe_Context *ctx, noe_Image image, int x, int y);
void noe_draw_image2(noe_Context *ctx, noe_Image image, noe_Rect src, noe_Rect dst);
void noe_draw_image_scaled_to_screen(noe_Context *ctx, noe_Image image);
void noe_draw_pixel(noe_Context *ctx, noe_Color color, int x, int y);

noe_Font noe_create_font(noe_Image atlas, int codepoint_count);
void noe_destroy_font(noe_Font);

/////////////////////////////
///
/// Vector operations
///

static inline noe_Vec2 noe_vec2_add(noe_Vec2 a, noe_Vec2 b) {
    return noe_vec2(a.x + b.x, a.y + b.y);
}

static inline noe_Vec2 noe_vec2_sub(noe_Vec2 a, noe_Vec2 b) {
    return noe_vec2(a.x - b.x, a.y - b.y);
}

static inline float noe_vec2_dot(noe_Vec2 a, noe_Vec2 b) {
    return a.x*b.x + a.y*b.y;
}

static inline float noe_vec2_distance_sqr(noe_Vec2 a) {
    return noe_vec2_dot(a,a);
}

static inline float noe_vec2_distance(noe_Vec2 a) {
    return sqrt(noe_vec2_distance_sqr(a));
}

static inline noe_Vec2 noe_vec2_normalize(noe_Vec2 a) {
    float dist = noe_vec2_distance(a);
    a.x /= dist;
    a.y /= dist;
    return a;
}

#endif // NOE

///////////////////////////////////////////////////
///
/// TODOs
/// 1. Adding text rendering support
/// 2. Adding Linux Platform
/// 3. Building a game
/// 4. Hardware rendering (OpenGL 3.3)
///

/* 
** Copyright (c) 2024 bagasjs
** 
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
** 
** The above copyright notice and this permission notice shall be included in all
** copies or substantial portions of the Software.
** 
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
*/
