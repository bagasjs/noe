#include "noe.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <glad/glad.h>

bool LoadFontGlyphs(TextFont *font, const uint8_t *fontData, uint32_t fontSize, int codepointAmount, int *codepoints)
{
    stbtt_fontinfo info;
    if(!stbtt_InitFont(&info, fontData, stbtt_GetFontOffsetForIndex(fontData, 0))) {
        TRACELOG(LOG_ERROR, "Failed to load font glyphs");
        return false;
    }

    float scale = stbtt_ScaleForPixelHeight(&info, (float)fontSize);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);

    codepointAmount = (codepointAmount > 0)? codepointAmount : 95;
    bool genFontChars = false;
    if (codepoints == NULL) {
        codepoints = (int *)MemoryAlloc(codepointAmount*sizeof(int));
        for (int i = 0; i < codepointAmount; i++) codepoints[i] = i + 32;
        genFontChars = true;
    }

    GlyphInfo *chars = (GlyphInfo *)MemoryAlloc(codepointAmount * sizeof(GlyphInfo));
    if(!chars) {
        TRACELOG(LOG_ERROR, "Failed to allocate memory for the glyphs");
        return false;
    }

    for (int i = 0; i < codepointAmount; i++) {
        int chw = 0, chh = 0;   // Character width and height (on generation)
        int ch = codepoints[i];  // Character value to get info for
        chars[i].codepoint = ch;

        int index = stbtt_FindGlyphIndex(&info, ch);
        if (index == 0) {
            TRACELOG(LOG_WARNING, "Failed to load glyph for `%c`", (char)ch);
            continue;
        }

        chars[i].image.data = stbtt_GetCodepointBitmap(&info, scale, scale, ch, &chw, &chh, 
                &chars[i].offsetX, &chars[i].offsetY);

        if (chars[i].image.data != NULL)    // Glyph data has been found in the font
        {
            stbtt_GetCodepointHMetrics(&info, ch, &chars[i].advanceX, NULL);
            chars[i].advanceX = (int)((float)chars[i].advanceX*scale);

            // Load characters images
            chars[i].image.width = chw;
            chars[i].image.height = chh;
            chars[i].image.compAmount = 1;

            chars[i].offsetY += (int)((float)ascent*scale);
        }

        if (ch == 32) {
            stbtt_GetCodepointHMetrics(&info, ch, &chars[i].advanceX, NULL);
            chars[i].advanceX = (int)((float)chars[i].advanceX*scale);

            Image imSpace = {
                .data = MemoryAlloc(chars[i].advanceX*fontSize*2),
                .width = chars[i].advanceX,
                .height = fontSize,
                .compAmount = 1,
            };

            chars[i].image = imSpace;
        }

    }

    font->baseSize = fontSize;
    font->glyphsInfo = chars;
    font->glyphsCount = codepointAmount;

    if(genFontChars) MemoryFree(codepoints);
    TRACELOG(LOG_INFO, "Font glyphs loaded successfully");
    return true;
}

Image GenImageFontAtlas(const GlyphInfo *glyphsInfo, Rectangle **glyphRecs, 
        uint32_t glyphsCount, uint32_t fontSize, int padding)
{
    TRACELOG(LOG_INFO, "Generating font atlas");
    if(!glyphsInfo || !glyphRecs) {
        TRACELOG(LOG_ERROR, "Failed to generate font atlas due to invalid arguments");
        return (Image){0};
    }

    Image atlas = {0};

    // Calculate image size based on total glyph width and glyph row count
    uint32_t totalWidth = 0;
    uint32_t maxGlyphWidth = 0;

    Rectangle *recs = (Rectangle *)MemoryAlloc(glyphsCount*sizeof(Rectangle));
    if(!recs) {
        TRACELOG(LOG_ERROR, "Failed to allocate memory for glyph rectangles");
        return (Image){0};
    }

    for (uint32_t i = 0; i < glyphsCount; i++) {
        if (glyphsInfo[i].image.width > maxGlyphWidth) maxGlyphWidth = glyphsInfo[i].image.width;
        totalWidth += glyphsInfo[i].image.width + 2*padding;
    }

#if defined(SUPPORT_FONT_ATLAS_SIZE_CONSERVATIVE)
    int rowCount = 0;
    int imageSize = 64;  // Define minimum starting value to avoid unnecessary calculation steps for very small images

    // NOTE: maxGlyphWidth is maximum possible space left at the end of row
    while (totalWidth > (imageSize - maxGlyphWidth)*rowCount) {
        imageSize *= 2;                                 // Double the size of image (to keep POT)
        rowCount = imageSize/(fontSize + 2*padding);    // Calculate new row count for the new image size
    }

    atlas.width = imageSize;   // Atlas bitmap width
    atlas.height = imageSize;  // Atlas bitmap height
#else
    int paddedFontSize = fontSize + 2*padding;
    // No need for a so-conservative atlas generation
    float totalArea = totalWidth*paddedFontSize*1.2f;
    float imageMinSize = sqrtf(totalArea);
    int imageSize = (int)powf(2, ceilf(logf(imageMinSize)/logf(2)));

    if (totalArea < ((float)(imageSize*imageSize)/2)) {
        atlas.width = imageSize;    // Atlas bitmap width
        atlas.height = imageSize/2; // Atlas bitmap height
    } else {
        atlas.width = imageSize;   // Atlas bitmap width
        atlas.height = imageSize;  // Atlas bitmap height
    }
#endif

    atlas.compAmount = 1;
    atlas.data = (uint8_t *)MemoryAlloc(atlas.compAmount*atlas.width*atlas.height);

    uint32_t offsetX = padding;
    uint32_t offsetY = padding;

    // NOTE: Using simple packaging, one char after another
    for (uint32_t i = 0; i < glyphsCount; i++) {
        // Check remaining space for glyph
        if (offsetX >= (atlas.width - glyphsInfo[i].image.width - 2*padding)) {
            offsetX = padding;

            offsetY += (fontSize + 2*padding);

            if (offsetY > (atlas.height - fontSize - padding)) {
                for(uint32_t j = i + 1; j < glyphsCount; j++)
                {
                    TRACELOG(LOG_WARNING, "FONT: Failed to package character (%i)", j);
                    // Make sure remaining recs contain valid data
                    recs[j].x = 0;
                    recs[j].y = 0;
                    recs[j].width = 0;
                    recs[j].height = 0;
                }
                break;
            }
        }

        // Copy pixel data from glyph image to atlas
        for (uint32_t y = 0; y < glyphsInfo[i].image.height; y++) {
            for (uint32_t x = 0; x < glyphsInfo[i].image.width; x++) {
                ((unsigned char *)atlas.data)[(offsetY + y)*atlas.width + (offsetX + x)] = 
                    ((unsigned char *)glyphsInfo[i].image.data)[y*glyphsInfo[i].image.width + x];
            }
        }

        // Fill chars rectangles in atlas info
        recs[i].x = (float)offsetX;
        recs[i].y = (float)offsetY;
        recs[i].width = (float)glyphsInfo[i].image.width;
        recs[i].height = (float)glyphsInfo[i].image.height;

        // Move atlas position X for next character drawing
        offsetX += (glyphsInfo[i].image.width + 2*padding);
    }

    for (int i = 0, k = atlas.width*atlas.height - 1; i < 3; i++) {
        ((unsigned char *)atlas.data)[k - 0] = 255;
        ((unsigned char *)atlas.data)[k - 1] = 255;
        ((unsigned char *)atlas.data)[k - 2] = 255;
        k -= atlas.width;
    }

    *glyphRecs = recs;

    return atlas;
}


bool LoadFontEx(TextFont *font, const uint8_t *fontData, size_t dataSize, uint32_t fontSize, uint32_t bw, uint32_t bh, int codepointCount, int *codepoints)
{
    (void)dataSize;
    (void)bw;
    (void)bh;

    if(!LoadFontGlyphs(font, fontData, fontSize, codepointCount, codepoints)) {
        TRACELOG(LOG_ERROR, "Failed to load font's glyphs");
        return false;
    }

    font->glyphPadding = 4;
    Image atlas = GenImageFontAtlas(font->glyphsInfo, &font->recs, font->glyphsCount, font->baseSize, font->glyphPadding);
    font->atlas = atlas;

    return LoadTextureFromImage(&font->texture, font->atlas);
}

bool LoadFontFromFile(TextFont *font, const char *filePath)
{
    size_t fileSize = 0;
    uint8_t *data = LoadFileData(filePath, &fileSize);
    if(!data) {
        TRACELOG(LOG_WARNING, "Failed to load font from file %s", filePath);
        return false;
    }

    bool result = false;
    result = LoadFontEx(font, data, fileSize, 32, 512, 128, 0, NULL);
    UnloadFileData(data);
    return result;
}

int GetGlyphIndex(TextFont font, int codepoint)
{
    for(int i = 0; i < (int)font.glyphsCount; ++i) {
        if(font.glyphsInfo[i].codepoint == codepoint)
            return i;
    }
    return -1;
}

void DrawTextCodepoint(TextFont font, int codepoint, Vector2 position, float fontSize, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scaleFactor = fontSize/font.baseSize;     // Character quad scaling factor

    // Character destination rectangle on screen
    // NOTE: We consider glyphPadding on drawing
    Rectangle dstRec = { position.x + font.glyphsInfo[index].offsetX*scaleFactor - (float)font.glyphPadding*scaleFactor,
                      position.y + font.glyphsInfo[index].offsetY*scaleFactor - (float)font.glyphPadding*scaleFactor,
                      (font.recs[index].width + 2.0f*font.glyphPadding)*scaleFactor,
                      (font.recs[index].height + 2.0f*font.glyphPadding)*scaleFactor };

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f*font.glyphPadding, font.recs[index].height + 2.0f*font.glyphPadding };

    // Draw the character texture on the screen
    DrawTextureEx(font.texture, srcRec, dstRec);
}

int GetCodepointNext(const char *text, int *codepointSize)
{
    const char *ptr = text;
    int codepoint = 0x3f;       // Codepoint (defaults to '?')
    *codepointSize = 1;

    // Get current codepoint and bytes processed
    if (0xf0 == (0xf8 & ptr[0]))
    {
        // 4 byte UTF-8 codepoint
        if(((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80) || ((ptr[3] & 0xC0) ^ 0x80)) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x07 & ptr[0]) << 18) | ((0x3f & ptr[1]) << 12) | ((0x3f & ptr[2]) << 6) | (0x3f & ptr[3]);
        *codepointSize = 4;
    }
    else if (0xe0 == (0xf0 & ptr[0]))
    {
        // 3 byte UTF-8 codepoint */
        if(((ptr[1] & 0xC0) ^ 0x80) || ((ptr[2] & 0xC0) ^ 0x80)) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x0f & ptr[0]) << 12) | ((0x3f & ptr[1]) << 6) | (0x3f & ptr[2]);
        *codepointSize = 3;
    }
    else if (0xc0 == (0xe0 & ptr[0]))
    {
        // 2 byte UTF-8 codepoint
        if((ptr[1] & 0xC0) ^ 0x80) { return codepoint; } // 10xxxxxx checks
        codepoint = ((0x1f & ptr[0]) << 6) | (0x3f & ptr[1]);
        *codepointSize = 2;
    }
    else if (0x00 == (0x80 & ptr[0]))
    {
        // 1 byte UTF-8 codepoint
        codepoint = ptr[0];
        *codepointSize = 1;
    }

    return codepoint;
}

void DrawText(TextFont font, const char *text, int posX, int posY, int fontSize, Color color)
{
    Vector2 position = { .x=(float)posX, .y=(float)posY };

    int defaultFontSize = 10;   // Default Font chars height in pixel
    if (fontSize < defaultFontSize) fontSize = defaultFontSize;
    int spacing = fontSize/defaultFontSize;
    DrawTextEx(font, text, position, (float)fontSize, (float)spacing, color, 15);
}

void DrawTextEx(TextFont font, const char *text, Vector2 position, float fontSize, float spacing, Color tint, int lineSpacing)
{
    if (font.texture.ID == 0) return; // Security check in case of not valid font

    int size = StringLength(text);    // Total size in bytes of the text, scanned by codepoints in loop

    int textOffsetY = 0;            // Offset between lines (on linebreak '\n')
    float textOffsetX = 0.0f;       // Offset X to next character to draw

    float scaleFactor = fontSize/font.baseSize;         // Character quad scaling factor

    for (int i = 0; i < size;) {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepointNext(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n') {
            // NOTE: Line spacing is a global variable, use SetTextLineSpacing() to setup
            textOffsetY += lineSpacing;
            textOffsetX = 0.0f;
        } else {
            if ((codepoint != ' ') && (codepoint != '\t')) {
                DrawTextCodepoint(font, codepoint, 
                        (Vector2){ .x=position.x + textOffsetX, .y=position.y + textOffsetY }, 
                        fontSize, tint);
            }


            if (font.glyphsInfo[index].advanceX == 0) 
                textOffsetX += ((float)font.recs[index].width*scaleFactor + spacing);
            else 
                textOffsetX += ((float)font.glyphsInfo[index].advanceX*scaleFactor + spacing);

        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}

void UnloadFont(TextFont font)
{
    UnloadTexture(font.texture);
    UnloadImage(font.atlas);
    MemoryFree(font.glyphsInfo);
    MemoryFree(font.recs);
}
