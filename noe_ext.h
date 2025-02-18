//
// A collection of extension of Noe functions. These codes is separated 
// into extensions to make Noe still small yet powerful
//

#ifndef NOE_EXT_STBTT_H_
#define NOE_EXT_STBTT_H_

#include "noe.h"

bool noe_image_save_to_pngfile(noe_Image image, const char *filepath);
noe_Font noe_load_font_from_ttf(const char *filepath, int fontsz);
void noe_font_to_c_header(noe_Font font, const char *filepath);

#endif // NOE_EXT_STBTT_H_
