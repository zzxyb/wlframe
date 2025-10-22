#ifndef VA_WLF_RECORDER_H
#define VA_WLF_RECORDER_H

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>

struct wlf_queue;

struct wlf_recorder {
	struct wlf_queue *queue;
};

#endif // VA_WLF_RECORDER_H
