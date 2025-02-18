#include <stdio.h>
#include "noe_ext.h"
#include "noe.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "vendors/stb_truetype.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "vendors/stb_image_write.h"

bool noe_image_save_to_pngfile(noe_Image image, const char *filepath)
{
    int comp = noe_pixelformat_channel_amount(image.format);
    if(comp < 0) return false;
    stbi_write_png(filepath, image.w, image.h, comp, image.pixels, image.w * comp);
    return true;
}

noe_Font noe_load_font_from_ttf(const char *filepath, int fontsz)
{
    size_t size = 0;
    unsigned char* font_data = 0;
    FILE* font_file = fopen(filepath, "rb");
    fseek(font_file, 0, SEEK_END);
    size = ftell(font_file); /* how long is the file ? */
    fseek(font_file, 0, SEEK_SET); /* reset */ 
    font_data = malloc(size); 
    fread(font_data, size, 1, font_file);
    fclose(font_file);

    int codepoint_amount = 0;
    int *codepoints = 0;
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
    noe_Glyph *chars = malloc(sizeof(noe_Glyph) * codepoint_amount);

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
        chars[i].l = x;
        chars[i].t = 0.0f;

        /* advance x */
        x += roundf(ax * scale);
        chars[i].r = x;
        chars[i].b = fontsz;
    }

    if(codepoint_generated)
        free(codepoints);
    noe_Image image = noe_load_image(bitmap, bw, bh, NOE_PIXELFORMAT_GRAYSCALE);
    return noe_load_font(image, chars, codepoint_amount);
}

void noe_font_to_c_header(noe_Font font, const char *filepath)
{
    FILE *f = fopen(filepath, "w");
    if(!f) {
        fprintf(stderr, "Failed to open file %s\n", filepath);
        return;
    }

    fprintf(f, "#ifndef NOE_FONT_DATA_H_\n");
    fprintf(f, "#define NOE_FONT_DATA_H_\n");
    fprintf(f, "#include \"noe.h\"\n");
    fprintf(f, "static uint8_t FONT_RAW_IMAGE_DATA[] = {\n");
    int chan = noe_pixelformat_channel_amount(font.atlas.format);
    NOE_ASSERT(chan == 1 && "A font currently only support 1 channel / grayscale format");
    for(int y = 0; y < font.atlas.h; ++y) {
        fprintf(f, "   ");
        for(int x = 0; x < font.atlas.w; ++x) {
            fprintf(f, "0x%x, ", font.atlas.pixels[y*font.atlas.w + x]);
        }
        fprintf(f, "\n");
    }
    fprintf(f, "};\n");
    fprintf(f, "static noe_Glyph FONT_CODEPOINTS[] = {\n");
    for(int i = 0; i < font.codepoints_count; ++i) {
        noe_Glyph cp = font.codepoints[i];
        fprintf(f, "    { .codepoint = %d, .l = %d, .t = %d, .r = %d, .b = %d, },\n", 
                cp.codepoint, cp.l, cp.t, cp.r, cp.b);
    }
    fprintf(f, "};\n");
    fprintf(f, "static noe_Font FONT = {\n");
    fprintf(f, "     .atlas.w      = %u,\n", font.atlas.w);
    fprintf(f, "     .atlas.h      = %u,\n", font.atlas.h);
    fprintf(f, "     .atlas.format = NOE_PIXELFORMAT_GRAYSCALE,\n");
    fprintf(f, "     .atlas.pixels = FONT_RAW_IMAGE_DATA,\n");
    fprintf(f, "     .codepoints_count = %u,\n", font.codepoints_count);
    fprintf(f, "     .codepoints = FONT_CODEPOINTS,\n");
    fprintf(f, "};\n");
    fprintf(f, "#endif // NOE_FONT_DATA_H_\n");
}
