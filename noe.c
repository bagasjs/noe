#include "noe.h"

#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif


//////////////////////////////////////////////////////
///
/// Utility APIs
///

double noe_gettime(void)
{
#ifdef _WIN32
    static LARGE_INTEGER g_frequency = {0};
    if(g_frequency.QuadPart == 0) {
        QueryPerformanceFrequency(&g_frequency);
    }
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return (double)t.QuadPart/(double)g_frequency.QuadPart;
#endif
    return 0;
}

void noe_sleep(int milis)
{
#ifdef _WIN32
    Sleep((DWORD)milis);
#endif
}

noe_Rect noe_clip_rect(noe_Rect outer, noe_Rect inner)
{
    int l, t, r, b;
    l = NOE_MAX(inner.x, outer.x);
    t = NOE_MAX(inner.y, outer.y);
    r = NOE_MIN(inner.w + inner.x, outer.w + outer.x);
    b = NOE_MIN(inner.h + inner.y, outer.h + outer.y);
    if(l > r || t > b) return noe_rect(0, 0, 0, 0);
    return noe_rect(l, t, r - l, b - t);
}

//////////////////////////////////////////////////////
///
/// Image Related APIs
///

struct noe_PixelFormatInfo {
    int channels;
};

static const struct noe_PixelFormatInfo g_pixelformatinfos[_COUNT_NOE_PIXELFORMATS] = {
    [NOE_PIXELFORMAT_R8G8B8A8] = { .channels = 4, },
    [NOE_PIXELFORMAT_R8G8B8] = { .channels = 3, },
    [NOE_PIXELFORMAT_B8G8R8A8] = { .channels = 4, },
    [NOE_PIXELFORMAT_B8G8R8] = { .channels = 3, },
};

noe_Image noe_load_image(void *data, int width, int height, int format)
{
    noe_Image res;
    res.w = width;
    res.h = height;
    res.pixels = data;
    res.texture = NULL;
    res.format = format;
    return res;
}

noe_Image noe_create_image(int width, int height, int pixelformat)
{
    uint8_t *pixels = NOE_MALLOC(width * height * g_pixelformatinfos[pixelformat].channels);
    return noe_load_image(pixels, width, height, pixelformat);
}

void noe_destroy_image(noe_Image image)
{
    NOE_FREE(image.pixels);
}

static void noe_image_put_pixel(noe_Image image, int x, int y, noe_Color color)
{
    if((0 > x || x > image.w) || (0 > y || y > image.h)) return;
    int index = (image.w * y + x) * g_pixelformatinfos[image.format].channels;
    switch(image.format) {
        case NOE_PIXELFORMAT_R8G8B8:
            {
                image.pixels[index + 0] = color.r;
                image.pixels[index + 1] = color.g;
                image.pixels[index + 2] = color.b;
            }
            break;
        case NOE_PIXELFORMAT_R8G8B8A8:
            {
                image.pixels[index + 0] = color.r;
                image.pixels[index + 1] = color.g;
                image.pixels[index + 2] = color.b;
                image.pixels[index + 3] = color.a;
            }
            break;
        case NOE_PIXELFORMAT_B8G8R8:
            {
                image.pixels[index + 0] = color.b;
                image.pixels[index + 1] = color.g;
                image.pixels[index + 2] = color.r;
            }
            break;
        case NOE_PIXELFORMAT_B8G8R8A8:
            {
                image.pixels[index + 0] = color.b;
                image.pixels[index + 1] = color.g;
                image.pixels[index + 2] = color.r;
                image.pixels[index + 3] = color.a;
            }
            break;
    }
}

static noe_Color noe_image_get_pixel(noe_Image image, int x, int y)
{
    if (x < 0 || x >= image.w || y < 0 || y >= image.h) return NOE_BLACK;
    int index = (image.w * y + x) * g_pixelformatinfos[image.format].channels;
    noe_Color color;
    switch(image.format) {
        case NOE_PIXELFORMAT_R8G8B8:
            {
                color.r = image.pixels[index + 0];
                color.g = image.pixels[index + 1];
                color.b = image.pixels[index + 2];
            }
            break;
        case NOE_PIXELFORMAT_R8G8B8A8:
            {
                color.r = image.pixels[index + 0];
                color.g = image.pixels[index + 1];
                color.b = image.pixels[index + 2];
                color.a = image.pixels[index + 3];
            }
            break;
        case NOE_PIXELFORMAT_B8G8R8:
            {
                color.b = image.pixels[index + 0];
                color.g = image.pixels[index + 1];
                color.r = image.pixels[index + 2];
            }
            break;
        case NOE_PIXELFORMAT_B8G8R8A8:
            {
                color.b = image.pixels[index + 0];
                color.g = image.pixels[index + 1];
                color.r = image.pixels[index + 2];
                color.a = image.pixels[index + 3];
            }
            break;
    }
    return color;
}

void noe_image_resize(noe_Image *dst, noe_Image src)
{
    float scale_x = (float)src.w/(float)dst->w;
    float scale_y = (float)src.h/(float)dst->h;

    for(int y = 0; y < dst->h; ++y) {
        for(int x = 0; x < dst->w; ++x) {
            // Map destination pixel (x, y) to source image
            float src_x = x * scale_x;
            float src_y = y * scale_y;

            // Get the top-left pixel indices in the source image
            int x1 = (int)src_x;
            int y1 = (int)src_y;

            // Ensure the bottom-right pixel indices don't exceed bounds
            int x2 = (x1 + 1 < src.w) ? x1 + 1 : x1;
            int y2 = (y1 + 1 < src.h) ? y1 + 1 : y1;

            // Get fractional parts for interpolation
            float frac_x = src_x - x1;
            float frac_y = src_y - y1;

            // Fetch colors from the four neighbors           
            noe_Color c11 = noe_image_get_pixel(src, x1, y1); // Top-left
            noe_Color c12 = noe_image_get_pixel(src, x1, y2); // Bottom-left
            noe_Color c21 = noe_image_get_pixel(src, x2, y1); // Top-right
            noe_Color c22 = noe_image_get_pixel(src, x2, y2); // Bottom-right

            // Interpolate along the x-axis for the top and bottom rows
            noe_Color top = {
                .r = c11.r * (1 - frac_x) + c21.r * frac_x,
                .g = c11.g * (1 - frac_x) + c21.g * frac_x,
                .b = c11.b * (1 - frac_x) + c21.b * frac_x,
                .a = c11.a * (1 - frac_x) + c21.a * frac_x,
            };

            noe_Color bottom = {
                .r = c12.r * (1 - frac_x) + c22.r * frac_x,
                .g = c12.g * (1 - frac_x) + c22.g * frac_x,
                .b = c12.b * (1 - frac_x) + c22.b * frac_x,
                .a = c12.a * (1 - frac_x) + c22.a * frac_x,
            };

            // Interpolate along the y-axis to get the final pixel
            noe_Color result = {
                .r = top.r * (1 - frac_y) + bottom.r * frac_y,
                .g = top.g * (1 - frac_y) + bottom.g * frac_y,
                .b = top.b * (1 - frac_y) + bottom.b * frac_y,
                .a = top.a * (1 - frac_y) + bottom.a * frac_y,
            };

            // Place the resulting pixel in the destination image
            noe_image_put_pixel(*dst, x, y, result);
        }
    }
}


//////////////////////////////////////////////////////
///
/// Context Related APIs
///

#define NOE_SUPPORTED_KEYS 256
#define NOE_SUPPORTED_BTNS 8

typedef struct noe_Context {
    bool initialized;

    const char *name;
    const char *title;
    noe_Image canvas;

    bool prev_key_states[NOE_SUPPORTED_KEYS];
    bool curr_key_states[NOE_SUPPORTED_KEYS];
    bool prev_btn_states[NOE_SUPPORTED_BTNS];
    bool curr_btn_states[NOE_SUPPORTED_BTNS];

    noe_Vec2 prev_cursor_pos;
    noe_Vec2 curr_cursor_pos;
    noe_Vec2 prev_wheel_mov;
    noe_Vec2 curr_wheel_mov;

    double init_time;
    double last_frame_time;
    double target_frame_time;

    struct {
        uint8_t *buf;
        uint32_t cap;
        uint32_t count;
    } tmp;

#ifdef _WIN32
    LARGE_INTEGER frequency;
    HINSTANCE inst;
    HWND wnd;
#endif
} noe_Context;

#ifdef _WIN32
typedef struct BINFO{
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[3];
} BINFO;

static int _noe_win32_scancode_mapping[0x1FF] = {
    [0x00B] = NOE_KEY_0, [0x002] = NOE_KEY_1, [0x003] = NOE_KEY_2, [0x004] = NOE_KEY_3, [0x005] = NOE_KEY_4, [0x006] = NOE_KEY_5,
    [0x007] = NOE_KEY_6, [0x008] = NOE_KEY_7, [0x009] = NOE_KEY_8, [0x00A] = NOE_KEY_9, [0x01E] = NOE_KEY_A, [0x030] = NOE_KEY_B,
    [0x02E] = NOE_KEY_C, [0x020] = NOE_KEY_D, [0x012] = NOE_KEY_E, [0x021] = NOE_KEY_F, [0x022] = NOE_KEY_G, [0x023] = NOE_KEY_H,
    [0x017] = NOE_KEY_I, [0x024] = NOE_KEY_J, [0x025] = NOE_KEY_K, [0x026] = NOE_KEY_L, [0x032] = NOE_KEY_M, [0x031] = NOE_KEY_N,
    [0x018] = NOE_KEY_O, [0x019] = NOE_KEY_P, [0x010] = NOE_KEY_Q, [0x013] = NOE_KEY_R, [0x01F] = NOE_KEY_S, [0x014] = NOE_KEY_T,
    [0x016] = NOE_KEY_U, [0x02F] = NOE_KEY_V, [0x011] = NOE_KEY_W, [0x02D] = NOE_KEY_X, [0x015] = NOE_KEY_Y, [0x02C] = NOE_KEY_Z,
    [0x028] = NOE_KEY_APOSTROPHE, [0x02B] = NOE_KEY_BACKSLASH, [0x033] = NOE_KEY_COMMA,
    [0x00D] = NOE_KEY_EQUAL, /** [0x029] = NOE_KEY_GRAVE_ACCENT, */ [0x01A] = NOE_KEY_LEFT_BRACKET,
    [0x00C] = NOE_KEY_MINUS, [0x034] = NOE_KEY_PERIOD, [0x01B] = NOE_KEY_RIGHT_BRACKET,
    [0x027] = NOE_KEY_SEMICOLON, [0x035] = NOE_KEY_SLASH, /** [0x056] = NOE_KEY_WORLD_2, */
    [0x00E] = NOE_KEY_BACKSPACE, [0x153] = NOE_KEY_DELETE, [0x14F] = NOE_KEY_END,
    [0x01C] = NOE_KEY_ENTER, [0x001] = NOE_KEY_ESCAPE, [0x147] = NOE_KEY_HOME,
    [0x152] = NOE_KEY_INSERT, /** [0x15D] = NOE_KEY_MENU, */ [0x151] = NOE_KEY_PAGE_DOWN,
    [0x149] = NOE_KEY_PAGE_UP, /** [0x045] = NOE_KEY_PAUSE, */ [0x039] = NOE_KEY_SPACE,
    [0x00F] = NOE_KEY_TAB, //[0x03A] = NOE_KEY_CAPS_LOCK, * [0x145] = NOE_KEY_NUM_LOCK, */
    /** [0x046] = NOE_KEY_SCROLL_LOCK, */ [0x03B] = NOE_KEY_F1, [0x03C] = NOE_KEY_F2,
    [0x03D] = NOE_KEY_F3, [0x03E] = NOE_KEY_F4, [0x03F] = NOE_KEY_F5,
    [0x040] = NOE_KEY_F6, [0x041] = NOE_KEY_F7, [0x042] = NOE_KEY_F8,
    [0x043] = NOE_KEY_F9, [0x044] = NOE_KEY_F10, [0x057] = NOE_KEY_F11,
    [0x058] = NOE_KEY_F12, /** [0x064] = NOE_KEY_F13, */ /** [0x065] = NOE_KEY_F14, */
    /** [0x066] = NOE_KEY_F15, */ /** [0x067] = NOE_KEY_F16, */ /** [0x068] = NOE_KEY_F17, */
    /** [0x069] = NOE_KEY_F18, */ /** [0x06A] = NOE_KEY_F19, */ /** [0x06B] = NOE_KEY_F20, */
    /** [0x06C] = NOE_KEY_F21, */ /** [0x06D] = NOE_KEY_F22, */ /** [0x06E] = NOE_KEY_F23, */
    /** [0x076] = NOE_KEY_F24, */ /** [0x038] = NOE_KEY_LEFT_ALT, */ /** [0x01D] = NOE_KEY_LEFT_CONTROL, */
    /** [0x02A] = NOE_KEY_LEFT_SHIFT, */ /** [0x15B] = NOE_KEY_LEFT_SUPER, */ /** [0x137] = NOE_KEY_PRINT_SCREEN, */
    /** [0x138] = NOE_KEY_RIGHT_ALT, */ /** [0x11D] = NOE_KEY_RIGHT_CONTROL, */ /** [0x036] = NOE_KEY_RIGHT_SHIFT, */
    /** [0x15C] = NOE_KEY_RIGHT_SUPER, */ [0x150] = NOE_KEY_DOWN, [0x14B] = NOE_KEY_LEFT,
    [0x14D] = NOE_KEY_RIGHT, [0x148] = NOE_KEY_UP, /** [0x052] = NOE_KEY_KP_0, */
    /** [0x04F] = NOE_KEY_KP_1, */ /** [0x050] = NOE_KEY_KP_2, */ /** [0x051] = NOE_KEY_KP_3, */
    /** [0x04B] = NOE_KEY_KP_4, */ /** [0x04C] = NOE_KEY_KP_5, */ /** [0x04D] = NOE_KEY_KP_6, */
    /** [0x047] = NOE_KEY_KP_7, */ /** [0x048] = NOE_KEY_KP_8, */ /** [0x049] = NOE_KEY_KP_9, */
    /** [0x04E] = NOE_KEY_KP_ADD, */ /** [0x053] = NOE_KEY_KP_DECIMAL, */ /** [0x135] = NOE_KEY_KP_DIVIDE, */
    /** [0x11C] = NOE_KEY_KP_ENTER, */ /** [0x059] = NOE_KEY_KP_EQUAL, */ /** [0x037] = NOE_KEY_KP_MULTIPLY, */
    /** [0x04A] = NOE_KEY_KP_SUBTRACT, */
};

static LRESULT CALLBACK _noe_win32_window_proc(
    _In_ HWND wnd, _In_ UINT msg, _In_ WPARAM wp, _In_ LPARAM lp)
{
    noe_Context *ctx = (noe_Context *)GetWindowLongPtr(wnd, GWLP_USERDATA);

    switch(msg) {
        case WM_SIZE:
            {
                if(ctx->initialized) {
                    RECT r;
                    GetClientRect(wnd, &r);
                    int new_w = r.right - r.left;
                    int new_h = r.bottom - r.top;
                    noe_Image new_canvas = noe_create_image(new_w,new_h,ctx->canvas.format);
                    noe_image_resize(&new_canvas, ctx->canvas);
                    noe_destroy_image(ctx->canvas);
                    ctx->canvas = new_canvas;
                }
            }
            break;
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(wnd, &ps);
                BITMAPINFO bmi = {
                    .bmiHeader.biSize = sizeof(BITMAPINFOHEADER),
                    .bmiHeader.biBitCount = 32,
                    .bmiHeader.biCompression = BI_RGB,
                    .bmiHeader.biPlanes = 1,
                    .bmiHeader.biWidth = ctx->canvas.w,
                    .bmiHeader.biHeight = -ctx->canvas.h,
                };
                StretchDIBits(hdc, 0, 0, ctx->canvas.w, ctx->canvas.h,
                        0, 0, ctx->canvas.w, ctx->canvas.h,
                        ctx->canvas.pixels, &bmi, DIB_RGB_COLORS, SRCCOPY);
                EndPaint(wnd, &ps);
            }
            break;
        case WM_MOUSEMOVE:
            {
                ctx->prev_cursor_pos = ctx->curr_cursor_pos;
                ctx->curr_cursor_pos.x = GET_X_LPARAM(lp);
                ctx->curr_cursor_pos.y = GET_Y_LPARAM(lp);
                // ctx->curr_cursor_pos.x = NOE_MIN(NOE_MAX(GET_X_LPARAM(lp), 0), ctx->canvas.w);
                // ctx->curr_cursor_pos.y = NOE_MIN(NOE_MAX(GET_Y_LPARAM(lp), 0), ctx->canvas.h);
            }
            break;
        case WM_LBUTTONDOWN: 
        case WM_LBUTTONUP:
        case WM_RBUTTONDOWN: 
        case WM_RBUTTONUP:
        case WM_MBUTTONDOWN: 
        case WM_MBUTTONUP:
            {
                int button = (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP) ? 1 :
                    (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP) ? 2 : 3;
                if (msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN) {
                    SetCapture(wnd);
                    ctx->curr_btn_states[button] = 1;
                } else {
                    ReleaseCapture();
                    ctx->curr_btn_states[button] = 0;
                }
            } break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            {
                int key, scancode;
                bool key_state = !(HIWORD(lp) & KF_UP) ? 1 : 0;

                // If it's pressed then set the modifier bits
                if(key_state == 1) {
                    if(GetKeyState(VK_SHIFT) & 0x8000) key_state |= NOE_KEYMOD_SHIFT;
                    if(GetKeyState(VK_CONTROL) & 0x8000) key_state |= NOE_KEYMOD_CONTROL;
                    if(GetKeyState(VK_MENU) & 0x8000) key_state |= NOE_KEYMOD_ALT;
                    if((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000) key_state |= NOE_KEYMOD_SHIFT;
                    if(GetKeyState(VK_CAPITAL) & 1) key_state |= NOE_KEYMOD_CAPSLOCK;
                    if(GetKeyState(VK_NUMLOCK) & 1) key_state |= NOE_KEYMOD_NUMLOCK;
                }

                scancode = (HIWORD(lp) & (KF_EXTENDED | 0xff));
                if(!scancode) scancode = MapVirtualKeyA((UINT)wp, MAPVK_VK_TO_VSC);

                // HACK: Alt+PrtSc has a different scancode than just PrtSc
                if (scancode == 0x54) scancode = 0x137;

                // HACK: Ctrl+Pause has a different scancode than just Pause
                if (scancode == 0x146) scancode = 0x45;

                // HACK: CJK IME sets the extended bit for right Shift
                if (scancode == 0x136) scancode = 0x36;
                key = _noe_win32_scancode_mapping[scancode];

                if(0 <= key && key < NOE_SUPPORTED_KEYS) 
                    ctx->curr_key_states[key] = key_state;
            }
            break;
        case WM_CLOSE:
            DestroyWindow(wnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(wnd, msg, wp, lp);
    }
    return 0;
}
#endif

noe_Context *noe_init(const char *name, int w, int h, uint8_t flags)
{
    noe_Context *ctx = NOE_MALLOC(sizeof(*ctx));
    memset(ctx, 0, sizeof(noe_Context));
    if(!ctx) return NULL;

    ctx->name = name;
    ctx->title = name;
    ctx->tmp.buf = NULL;
    ctx->tmp.count = 0;
    ctx->tmp.cap = 0;
    memset(ctx->prev_key_states, 0, sizeof(ctx->prev_key_states));
    memset(ctx->curr_key_states, 0, sizeof(ctx->curr_key_states));
    memset(ctx->prev_btn_states, 0, sizeof(ctx->prev_btn_states));
    memset(ctx->curr_btn_states, 0, sizeof(ctx->curr_btn_states));

#ifdef _WIN32
    ctx->canvas = noe_create_image(w, h, NOE_PIXELFORMAT_B8G8R8A8);
    QueryPerformanceFrequency(&ctx->frequency);
    ctx->inst = GetModuleHandle(NULL);
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_VREDRAW | CS_HREDRAW;
    wc.hInstance = ctx->inst;
    wc.lpfnWndProc = _noe_win32_window_proc;
    wc.lpszClassName = ctx->name;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx(&wc);
    RECT wr = {0};
    wr.right = ctx->canvas.w;
    wr.bottom = ctx->canvas.h;
    DWORD ws = WS_OVERLAPPEDWINDOW;
    AdjustWindowRect(&wr,ws,0);
    ctx->wnd = CreateWindowEx(WS_EX_APPWINDOW | WS_EX_ACCEPTFILES, ctx->name,ctx->title,
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
            wr.right - wr.left, wr.bottom - wr.top, NULL, NULL, ctx->inst, NULL);

    if(ctx->wnd == NULL) {
        NOE_FREE(ctx);
        return NULL;
    }
    SetWindowLongPtr(ctx->wnd, GWLP_USERDATA, (LONG_PTR)ctx);
    ShowWindow(ctx->wnd, SW_NORMAL);
    UpdateWindow(ctx->wnd);
#endif

    ctx->target_frame_time = 1.0/60.0;
    ctx->init_time = noe_gettime();
    ctx->last_frame_time = ctx->init_time;
    ctx->initialized = true;
    return ctx;
}

static void *noe_tmp_alloc(noe_Context *ctx, uint32_t size)
{
    if(ctx->tmp.count + size <= ctx->tmp.cap) {
        ctx->tmp.cap = ctx->tmp.cap * 2 + size;
        uint8_t *newbuf = NOE_MALLOC(ctx->tmp.cap);
        if(!newbuf) 
            // TODO: Error: Buy More RAM
            return NULL;
        memcpy(newbuf,ctx->tmp.buf,ctx->tmp.count);
        NOE_FREE(ctx->tmp.buf);
        ctx->tmp.buf = newbuf;
    }
    uint8_t *ptr = &ctx->tmp.buf[ctx->tmp.count];
    memset(ptr, 0, size);
    ctx->tmp.count += size;
    return ptr;
}

static void noe_tmp_reset(noe_Context *ctx)
{
    ctx->tmp.count = 0;
}

void noe_close(noe_Context *ctx)
{
    if(!ctx) return;
    noe_destroy_image(ctx->canvas);
    NOE_FREE(ctx->tmp.buf);
    NOE_FREE(ctx);
}

void noe_set_window_title(noe_Context *ctx, const char *title)
{
#ifdef _WIN32
    SetWindowText(ctx->wnd, title);
#endif
}

bool noe_step(noe_Context *ctx, double *dt)
{
    /// Draw to window
#ifdef _WIN32
    RedrawWindow(ctx->wnd, 0, 0, RDW_INVALIDATE | RDW_UPDATENOW);
#endif

    // Handle delta time
    double now = noe_gettime();
    double prev = ctx->last_frame_time;
    double wait = (prev + ctx->target_frame_time) - now;
    if(wait > 0) {
        noe_sleep(wait * 1000);
        ctx->last_frame_time += ctx->target_frame_time;
    } else {
        ctx->last_frame_time = now;
    }
    if(dt) *dt = ctx->last_frame_time - prev;

#ifdef _WIN32
    MSG msg;
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if(msg.message == WM_QUIT) return false;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
#endif

    return true;
}

void noe_clear_background(noe_Context *ctx, noe_Color color)
{
    noe_draw_rect(ctx, color, noe_rect(0,0,ctx->canvas.w, ctx->canvas.h));
}

void noe_draw_rect(noe_Context *ctx, noe_Color color, noe_Rect r)
{
    r = noe_clip_rect(noe_rect(0,0, ctx->canvas.w, ctx->canvas.h), r);
    int xs = r.x, xd = r.x + r.w;
    int ys = r.y, yd = r.y + r.h;
    for(int dy = ys; dy < yd; ++dy) {
        for(int dx = xs; dx < xd; ++dx) {
            noe_image_put_pixel(ctx->canvas, dx, dy, color);
        }
    }
}

bool noe_key_pressed(noe_Context *ctx, int key)
{
    if(key < 0 && key > NOE_SUPPORTED_KEYS) return false;
    return ctx->prev_key_states[key] == 0 && ctx->curr_key_states[key] == 1;
}

bool noe_key_released(noe_Context *ctx, int key)
{
    if(key < 0 && key > NOE_SUPPORTED_KEYS) return false;
    return ctx->prev_key_states[key] == 1 && ctx->curr_key_states[key] == 0;
}

bool noe_key_down(noe_Context *ctx, int key)
{
    if(key < 0 && key > NOE_SUPPORTED_KEYS) return false;
    return ctx->curr_key_states[key] == 1;
}

bool noe_key_up(noe_Context *ctx, int key)
{
    if(key < 0 && key > NOE_SUPPORTED_KEYS) return false;
    return ctx->curr_key_states[key] == 0;
}

noe_Vec2 noe_cursor_pos(noe_Context *ctx)
{
    return ctx->curr_cursor_pos;
}

noe_Vec2 noe_cursor_delta(noe_Context *ctx)
{
    return noe_vec2(ctx->curr_cursor_pos.x - ctx->prev_cursor_pos.x, 
            ctx->curr_cursor_pos.y - ctx->prev_cursor_pos.y);
}

void noe_draw_image(noe_Context *ctx, noe_Image image, int x, int y)
{
    if((0 > x || x > image.w) || (0 > y || y > image.h)) return;

    noe_Rect r = noe_rect(x,y,image.w,image.h);
    r = noe_clip_rect(noe_rect(0,0,ctx->canvas.w,ctx->canvas.h), r);

    for(int dy = r.y; dy < r.y + r.h; ++dy) {
        for(int dx = r.x; dx < r.x + r.w; ++dx) {
            noe_image_put_pixel(ctx->canvas, dx, dy, noe_image_get_pixel(image, dx, dy));
        }
    }
}

void noe_draw_image2(noe_Context *ctx, noe_Image image, noe_Rect src, noe_Rect dst)
{
    noe_tmp_reset(ctx);
    const int pixelformat = image.format;
    const int channels = g_pixelformatinfos[pixelformat].channels;
    uint8_t *dstbuf = noe_tmp_alloc(ctx, dst.w * dst.h * channels);
    uint8_t *srcbuf = noe_tmp_alloc(ctx, src.w * src.h * channels);

    noe_Image srci = noe_load_image(srcbuf,src.w,src.h,pixelformat);
    noe_Image dsti = noe_load_image(dstbuf,dst.w,dst.h,pixelformat);
    for(int y = 0; y < src.h; ++y) {
        for(int x = 0; x < src.w; ++x) {
            noe_image_put_pixel(srci,x,y, noe_image_get_pixel(image, src.x + x, src.y + y));
        }
    }

    noe_image_resize(&dsti, srci);
    noe_draw_image(ctx,dsti,dst.x,dst.y);
}
