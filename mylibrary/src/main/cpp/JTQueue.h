//
// Created by ASUS on 2019/3/13.
//

#ifndef MYMUSIC_JTQUEUE_H
#define MYMUSIC_JTQUEUE_H


#include <queue>
#include "pthread.h"
#include "AndroidLog.h"
#include "JTPlayerStatus.h"

extern "C"{
#include <libavcodec/avcodec.h>
};


class JTQueue {

public:
    std::queue<AVPacket *>queueAVpkt;
    pthread_mutex_t mutexPkt;
    pthread_cond_t condPkt;
    JTPlayerStatus *jtPlayerStatus=NULL;


public:
    JTQueue(JTPlayerStatus *playerStatus);
    ~JTQueue();

    int putAVPkt(AVPacket *avPacket);
    int getAVPkt(AVPacket *avPacket);

    int getQueueSize();

    void clearAvpacket();

    void noticeQueue();

};


#endif //MYMUSIC_JTQUEUE_H
