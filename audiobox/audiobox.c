#include "miniaudio.h"
#include "noe.h"
#include "nomath.h"
#include "nogui.h"
#include <stdint.h>

#define WIDTH  800
#define HEIGHT 600

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
void GuiSlider(Vector2 pos, int min, int max, int *value);

#define AB_AUDIO_FORMAT ma_format_f32
// #define AB_SAMPLE_RATE 44100
#define AB_OUTPUT_CHANNELS 2
#define AB_SAMPLE_RATE 48000
#define AB_PCMF_TO_SEC(pcmf) ((pcmf)/(AB_SAMPLE_RATE))

int main(void)
{
    ma_result result;
    ma_decoder decoder;
    ma_device_config device_config;
    ma_device device;

    const char *musicFilePath = "./res/audio/miki-matsubara-stay-with-me.mp3";
    result = ma_decoder_init_file(musicFilePath, NULL, &decoder);
    if(result != MA_SUCCESS) {
        TRACELOG(LOG_FATAL, "Failed to load music `%s`", musicFilePath);
        return -1;
    }

    device_config = ma_device_config_init(ma_device_type_playback);
    device_config.playback.format = AB_AUDIO_FORMAT;
    device_config.playback.channels = AB_OUTPUT_CHANNELS;
    device_config.sampleRate = AB_SAMPLE_RATE;
    device_config.dataCallback = data_callback;
    device_config.pUserData = &decoder;

    if (ma_device_init(NULL, &device_config, &device) != MA_SUCCESS) {
        TRACELOG(LOG_FATAL, "Failed to open playback device");
        ma_decoder_uninit(&decoder);
        return -3;
    }

    SetupWindow("Audiobox", WIDTH, HEIGHT, WINDOW_SETUP_DEFAULT);
    InitApplication();

    Shader shader = GetDefaultShader();
    Matrix projection = MatrixOrthographic(0.0f, WIDTH, HEIGHT, 0.0f, -1.0f, 1.0f);
    SetProjectionMatrix(projection);

    TextFont font;
    if(!LoadFontFromFile(&font, "./res/firacode.ttf")) {
        TRACELOG(LOG_ERROR, "Failed load font");
        return -1;
    }

    Texture ikan;
    if(!LoadTextureFromFile(&ikan, "./res/ikan.jpg")) {
        TRACELOG(LOG_ERROR, "Failed load texture");
        return -1;
    }

    Rectangle ikanShape;
    ikanShape.width = ikan.width;
    ikanShape.height = ikan.height;
    ikanShape.x = 0.0f;
    ikanShape.y = 0.0f;

    GuiInit(font, 24);

    bool play_music = false;
    const char *restart = "RESTART";
    const char *play = "PLAY";
    const char *stop = "STOP";
    const char *playStop = play;
#define MAX_VOLUME 100
#define MIN_VOLUME 1
    int volume = 50;
    int prev_volume = volume;
    ma_device_set_master_volume(&device, ((float)(volume+MIN_VOLUME)/MAX_VOLUME));

    ma_uint64 maximum_pcm_frame = 0;
    ma_decoder_get_length_in_pcm_frames(&decoder, &maximum_pcm_frame);
    ma_uint64 maximum_seconds = AB_PCMF_TO_SEC(maximum_pcm_frame);

    ma_uint64 current_pcm_frame = 0;

    char volumeText[1024];
    while(!WindowShouldClose()) {
        PollInputEvents();
        if(IsKeyPressed(KEY_ESCAPE)) break;

        if(play_music) {
            if (ma_device_start(&device) != MA_SUCCESS) {
                TRACELOG(LOG_ERROR, "Failed to start playback device\n");
                ma_device_uninit(&device);
                ma_decoder_uninit(&decoder);
                return -4;
            }
        } else {
            ma_device_stop(&device);
        }

        ma_data_source_get_cursor_in_pcm_frames(&decoder, &current_pcm_frame);

        ClearBackground(HEX2COLOR(0x202020FF));
        DrawTexture(ikan, ikanShape.x, ikanShape.y, ikanShape.width, ikanShape.height);
        GuiBegin();
            ma_uint64 current_seconds = AB_PCMF_TO_SEC(current_pcm_frame);
            StringFormat(volumeText, sizeof(volumeText), "Current: %lu/%lu", current_seconds, maximum_seconds);
            GuiLabel((Vector2){.x=100.0f, .y=400.0f}, RED, volumeText);
            GuiSlider((Vector2){.x=300.0f, .y=400.0f}, 0, maximum_pcm_frame, (int*)&current_pcm_frame);

            GuiSlider((Vector2){.x=200.0f, .y=200.0f}, MIN_VOLUME, MAX_VOLUME, &volume);
            if(volume != prev_volume) {
                prev_volume = volume;
                ma_device_set_master_volume(&device, ((float)(volume+MIN_VOLUME)/MAX_VOLUME));
            }
            StringFormat(volumeText, sizeof(volumeText), "Volume: %d", volume);
            GuiLabel((Vector2){.x=400.0f, .y=300.0f}, RED, volumeText);

            if(GuiButton((Vector2){.x=10.0f, .y=10.0f}, playStop)) {
                play_music = !play_music;
                playStop = play_music ? stop : play;
            }

            if(GuiButton((Vector2){.x=100.0f, .y=10.0f}, restart)) {
                ma_decoder_seek_to_pcm_frame(&decoder, 0);
            }
        RenderFlush(shader);
        RenderPresent();
    }

    ma_device_uninit(&device);
    ma_decoder_uninit(&decoder);
    DeinitApplication();
    return 0;
}

static void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount)
{
    ma_decoder* pDecoder = (ma_decoder*)pDevice->pUserData;
    if (pDecoder == NULL) {
        return;
    }
    ma_decoder_read_pcm_frames(pDecoder, pOutput, frameCount, NULL);

    (void)pInput;
}
