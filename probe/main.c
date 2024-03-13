#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include "stb_image_write.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <raylib.h>

struct glyph {
    int codepoint;
    float s1, t1, s2, t2;
};

struct font_atlas {
    const uint8_t *data;
    int width, height;
    struct glyph *chars;
};

struct font_atlas load_font_atlas(const uint8_t *font_data, int fontsz, int codepoint_amount, int *codepoints)
{
    struct font_atlas result;
    stbtt_fontinfo info;
 
    if (!stbtt_InitFont(&info, font_data, 0))
    {
        printf("failed\n");
    }

    codepoint_amount = codepoint_amount != 0 ? codepoint_amount : 95;
    int codepoint_generated = 0;
    if(!codepoints) {
        codepoint_generated = 1;
        codepoints = calloc(sizeof(int), codepoint_amount);
        for(int i = 0; i < 95; ++i)
            codepoints[i] = 32 + i;
    }
    
    float scale = stbtt_ScaleForPixelHeight(&info, fontsz);
    
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
    
    int x = 0;
    int y = 0;

    int bh = fontsz;
    int bw = 0;
    // Get the width of the atlas
    for(int i = 0; i < codepoint_amount; ++i) {
        int codepoint = codepoints[i];
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&info, codepoint, &ax, &lsb);
        bw += roundf(ax * scale);
    }

    uint8_t* bitmap = malloc(bw * bh * sizeof(uint8_t));
    struct glyph *chars = malloc(sizeof(struct glyph) * codepoint_amount);

    ascent = roundf(ascent * scale);
    descent = roundf(descent * scale);

    int i;
    for(i = 0; i < codepoint_amount; ++i) {
        /* how wide is this character */
        int codepoint = codepoints[i];
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&info, codepoint, &ax, &lsb);
        /* (Note that each Codepoint call has an alternative Glyph version which caches the work required to lookup the character codepoint.) */

        /* get bounding box for character (may be offset to account for chars that dip above or below the line) */
        int c_x1, c_y1, c_x2, c_y2;
        stbtt_GetCodepointBitmapBox(&info, codepoint, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
        
        /* compute y (different characters have different heights) */
        y = ascent + c_y1;
        
        /* render character (stride and offset is important here) */
        int byteOffset = x + roundf(lsb * scale) + (y * bw);
        stbtt_MakeCodepointBitmap(&info, bitmap + byteOffset, c_x2 - c_x1, c_y2 - c_y1, bw, scale, scale, codepoint);

        chars[i].codepoint = codepoint;
        chars[i].s1 = x;
        chars[i].t1 = 0.0f;

        /* advance x */
        x += roundf(ax * scale);
        chars[i].s2 = x;
        chars[i].t2 = fontsz;
    }

    if(codepoint_generated)
        free(codepoints);

    result.width = bw;
    result.data = bitmap;
    result.height = bh;
    result.chars = chars;
    return result;
}

int main(void)
{
    /* load font file */
    long size;
    unsigned char* fontBuffer;
    
    FILE* fontFile = fopen("./firacode.ttf", "rb");
    fseek(fontFile, 0, SEEK_END);
    size = ftell(fontFile); /* how long is the file ? */
    fseek(fontFile, 0, SEEK_SET); /* reset */
    
    fontBuffer = malloc(size);
    
    fread(fontBuffer, size, 1, fontFile);
    fclose(fontFile);

    struct font_atlas atlas = load_font_atlas(fontBuffer, 64, 0, NULL);
    stbi_write_png("out.png", atlas.width, atlas.height, 1, atlas.data, atlas.width);

    SetTraceLogLevel(LOG_ALL);
    InitWindow(800, 600, "Hello World");

    Texture tex = LoadTexture("out.png");

    float newFontSize = 24.0f;
    float scale = newFontSize/atlas.height;
    const char *text = "Hello, World";
    size_t textLength = strlen(text);
    Vector2 pos = {0.0f, 0.0f};
    while(!WindowShouldClose()) {
        BeginDrawing();

        for(int i = 0; i < (int)textLength; ++i) {
            struct glyph cp = atlas.chars[text[i] - 32];
            TraceLog(LOG_DEBUG, "cp(%c) s1=%f,t1=%f,s2=%f,t2=%f", cp.codepoint, cp.s1, cp.t1, cp.s2, cp.t2);

            Rectangle src;
            src.x = cp.s1;
            src.y = cp.t1;
            src.width  = (float)cp.s2 - cp.s1;
            src.height = (float)cp.t2 - cp.t1;

            Rectangle dst;
            dst.x = pos.x;
            dst.y = 0;
            dst.width = (cp.s2-cp.s1)*scale;
            dst.height = tex.height*scale;
            DrawTexturePro(tex, src, dst, (Vector2){0.0f, 0.0f}, 0.0f, RED);
            pos.x += dst.width;
        }

        pos.x = 0.0f;
        EndDrawing();
    }

    CloseWindow();
}

