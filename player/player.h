#pragma once 
#include <iostream>
#include "im2d.h"
#include "rga.h"
#include "RgaUtils.h"
#include "opencv2/core/core.hpp"
#include "yolo/rkyolov5.h"
#include "utils/mpp_decoder.h"
#include "mk_mediakit.h"

#define LOGD 
#define step 5
/*-------------------------------------------
                  Main Functions
-------------------------------------------*/
typedef struct {
    RKYOLOV5* rkyolov5;
    MppDecoder* decoder;
} app_context_t;

static void API_CALL on_track_frame_out(void *user_data, mk_frame frame) {
    static int frame_index = 1;
    if (frame_index %  step == 0){
        app_context_t * ctx = (app_context_t *) user_data;
        LOGD("on_track_frame_out ctx=%p\n", user_data);
        const char* data = mk_frame_get_data(frame);
        size_t size = mk_frame_get_data_size(frame);
        LOGD("data: %s, size: %d", data, size);
        LOGD("decoder=%p\n", ctx->decoder);
        ctx->decoder->Decode((uint8_t *)data, size, 0);
        frame_index = 1;
    }
    else{
        frame_index++;
    }
}

static void API_CALL on_mk_play_event_func(void *user_data, int err_code, const char *err_msg, mk_track tracks[],
                                    int track_count) {
    if (err_code == 0) {
        //success
        printf("Play success!\n");
        int i;
        printf("Track count : %d ", track_count);
        for (i = 0; i < track_count; ++i)
        {
            LOGD("Track i : %d \n", tracks[i]);
            if (mk_track_is_video(tracks[i]))
            {
                log_info("Got video track: %s", mk_track_codec_name(tracks[i]));
                //监听track数据回调
                mk_track_add_delegate(tracks[i], on_track_frame_out, user_data);
            }
        }
    } else {
        LOGD("play failed: %d %s", err_code, err_msg);
    }
}

static void API_CALL on_mk_shutdown_func(void *user_data, int err_code, const char *err_msg, mk_track tracks[], int track_count) {
    printf("play interrupted: %d %s", err_code, err_msg);
}

static int process_video_rtsp(void* userdata, const char* url)
{
    mk_config config;
    memset(&config, 0, sizeof(mk_config));
    config.log_mask = LOG_CONSOLE;
    mk_env_init(&config);
    mk_player player = mk_player_create();
    mk_player_set_on_result(player, on_mk_play_event_func, userdata);
    mk_player_set_on_shutdown(player, on_mk_shutdown_func, userdata);
    mk_player_play(player, url);
    printf("Enter any key to exit\n");
    getchar();

    if (player) {
        mk_player_release(player);
    }
    return 0;
}

void _frame_callback(void *userdata, int width_stride, int height_stride, int width, int height, int format, int fd, void *data);