#include "noe.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

bool LoadFontFromFile(TextFont *font, const char *filePath)
{
    uint8_t *data = LoadFileData(filePath, 0);
    bool result = LoadFont(font, data, 64, 0, NULL);
    UnloadFileData(data);
    return result;
}

bool LoadFont(TextFont *font, const uint8_t *fontBuffer, int fontSize, int codepointAmount, int *codepoints)
{
    stbtt_fontinfo info;
 
    if (!stbtt_InitFont(&info, fontBuffer, 0)) {
        TRACELOG(LOG_ERROR, "Failed to initialize font (stb_truetype)");
        return false;
    }

    codepointAmount = codepointAmount != 0 ? codepointAmount : 95;
    int isCodepointGenerated = 0;
    if(!codepoints) {
        isCodepointGenerated = 1;
        codepoints = NOE_MALLOC(sizeof(int) * codepointAmount);
        for(int i = 0; i < 95; ++i)
            codepoints[i] = 32 + i;
    }

    float scale = stbtt_ScaleForPixelHeight(&info, fontSize);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    int x = 0;
    int y = 0;

    int bh = fontSize;
    int bw = 0;
    // Get the width of the atlas
    for(int i = 0; i < codepointAmount; ++i) {
        int codepoint = codepoints[i];
        int ax;
        int lsb;
        stbtt_GetCodepointHMetrics(&info, codepoint, &ax, &lsb);
        bw += roundf(ax * scale);
    }

    uint8_t *bitmap = NOE_MALLOC(bw * bh * sizeof(uint8_t));
    GlyphInfo *chars = NOE_MALLOC(sizeof(GlyphInfo) * codepointAmount);

    ascent = roundf(ascent * scale);
    descent = roundf(descent * scale);

    for(int i = 0; i < codepointAmount; ++i) {
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
        chars[i].s0 = x;
        chars[i].t0 = 0.0f;

        /* advance x */
        x += roundf(ax * scale);
        chars[i].s1 = x;
        chars[i].t1 = fontSize;
    }

    ascent = roundf(ascent * scale);
    descent = roundf(descent * scale);

    if(isCodepointGenerated)
        NOE_FREE(codepoints);

    Image atlas;
    atlas.width  = bw;
    atlas.height = bh;

#if 0
    atlas.compAmount = 1;
    atlas.data = bitmap;
#else
    // Two channels
    atlas.compAmount = 4;
    atlas.data = (uint8_t *)NOE_MALLOC(atlas.width*atlas.height*atlas.compAmount*sizeof(uint8_t));

    for (int i = 0, k = 0; i < (int)(atlas.width*atlas.height); i++, k += atlas.compAmount) {
        atlas.data[k + 0] = 255;
        atlas.data[k + 1] = 255;
        atlas.data[k + 2] = 255;
        atlas.data[k + 3] = ((uint8_t *)bitmap)[i];
    }
    NOE_FREE(bitmap);
#endif

    font->atlas = atlas;
    font->glyphs = chars;
    font->glyphsCount = codepointAmount;

    if(!LoadTexture(&font->texture, atlas.data, atlas.width, atlas.height, atlas.compAmount)) {
        UnloadImage(font->atlas);
        NOE_FREE(font->glyphs);
        return false;
    }

    return true;
}

void UnloadFont(TextFont font)
{
    UnloadTexture(font.texture);
    UnloadImage(font.atlas);
    NOE_FREE(font.glyphs);
}

GlyphInfo FindGlyphInfo(TextFont font, int codepoint)
{
    for(int i = 0; i < font.glyphsCount; ++i) {
        if(font.glyphs[i].codepoint == codepoint) 
            return font.glyphs[i];
    }
    return font.glyphs[0];
}

void DrawTextEx(TextFont font, const char *text, int fontSize, Vector2 pos, Color color)
{
    float scale = (float)fontSize/(float)font.atlas.height;
    GlyphInfo info;
    size_t textLength = strlen(text);
    for(int i = 0; i < (int)textLength; ++i) {
        info = FindGlyphInfo(font, text[i]);
        Rectangle src;
        src.x = info.s0;
        src.y = info.t0;
        src.width  = (float)info.s1 - info.s0;
        src.height = (float)info.t1 - info.t0;

        Rectangle dst;
        dst.x = pos.x;
        dst.y = pos.y;
        dst.width = (info.s1-info.s0)*scale;
        dst.height = font.atlas.height*scale;
        DrawTextureEx(font.texture, src, dst, color);
        pos.x += dst.width;
    }
}

Vector2 GetTextDimension(TextFont font, const char *text, int fontSize)
{
    float scale = (float)fontSize/(float)font.atlas.height;
    GlyphInfo info;
    size_t textLength = strlen(text);
    Vector2 result = {0};

    result.y = font.atlas.height*scale;
    for(int i = 0; i < (int)textLength; ++i) {
        info = FindGlyphInfo(font, text[i]);
        result.x += (info.s1-info.s0)*scale;
    }

    return result;
}
