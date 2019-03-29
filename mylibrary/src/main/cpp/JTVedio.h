//
// Created by ASUS on 2019/3/20.
//

#ifndef MYMUSIC_JTVEDIO_H
#define MYMUSIC_JTVEDIO_H

#define CODEC_YUV  0
#define CODEC_MEDIACODEC  1


#include "JTQueue.h"
#include "JTCallJava.h"
#include "JTAudio.h"

extern "C"{
#include <libavcodec/avcodec.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
};


class JTVedio {

public:
    int streamIndex=-1;
    AVCodecContext *avCodecCtx=NULL;
    AVCodecParameters *codecPar=NULL;
    JTQueue *queue=NULL;
    JTPlayerStatus *playerStatus=NULL;
    JTCallJava *callJava=NULL;
    AVRational time_base;

    pthread_t thread_play;
    JTAudio *audio=NULL;

    AVBSFContext *absCtx=NULL;

    double colock=0;
    double delayTime=0;
    double defaultDelayTime=0.04;

    pthread_mutex_t codecMutex;

    int codectype=CODEC_YUV;


public:
    JTVedio(JTPlayerStatus *status,JTCallJava *call);
    ~JTVedio();

    void play();

    void release();

    double getFrameDiffTime(AVFrame *avFrame,AVPacket *avPacket);

    double getDelayTime(double diff);
};


#endif //MYMUSIC_JTVEDIO_H
