//
// Created by ywl on 2017-12-3.
//

#ifndef WLPLAYER_BUFFERQUEUE_H
#define WLPLAYER_BUFFERQUEUE_H

#include "deque"
#include "JTPlayerStatus.h"
#include "WlPcmBean.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include "pthread.h"
};

class WlBufferQueue {

public:
    std::deque<WlPcmBean *> queueBuffer;
    pthread_mutex_t mutexBuffer;
    pthread_cond_t condBuffer;
    JTPlayerStatus *wlPlayStatus = NULL;

public:
    WlBufferQueue(JTPlayerStatus *playStatus);
    ~WlBufferQueue();
    int putBuffer(SAMPLETYPE *buffer, int size);
    int getBuffer(WlPcmBean **pcmBean);
    int clearBuffer();

    void release();
    int getBufferSize();

    int noticeThread();
};


#endif //WLPLAYER_BUFFERQUEUE_H
