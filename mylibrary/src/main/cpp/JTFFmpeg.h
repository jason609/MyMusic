//
// Created by ASUS on 2019/3/13.
//

#ifndef MYMUSIC_JTFFMPEG_H
#define MYMUSIC_JTFFMPEG_H

#include "JTCallJava.h"
#include "pthread.h"
#include "JTAudio.h"
#include "JTVedio.h"

extern "C"{
#include <libavformat/avformat.h>
#include <libavutil/time.h>
}

class JTFFmpeg {

public:
    JTCallJava *callJava=NULL;
    const char *url=NULL;
    pthread_t decodeThread;
    AVFormatContext *pFormatCtx=NULL;
    JTAudio *jtAudio=NULL;
    JTVedio *jtVedio=NULL;
    JTPlayerStatus *playerStatus=NULL;
    pthread_mutex_t init_mutex;
    pthread_mutex_t seek_mutex;
    const AVBitStreamFilter *bsFilter=NULL;

    bool exit=false;
    int duration=0;

    bool supportMedicCode=false;

public:
    JTFFmpeg(JTPlayerStatus *status,JTCallJava *callJava, const char* url);

    ~JTFFmpeg();

    void parpared();

    void decodeFFmpegThread();

    void start();

    void pause();

    void resume();

    void release();

    void seek(int64_t secds);

    void setVolume(int percent);

    void setMute(int mute);

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getSampleRate();

    void startStopRecord(bool start);

    bool cutAudioPlay(int starttime,int endtime, bool showpcm);

    int getCodecContext(AVCodecParameters *par,AVCodecContext **avCodecCtx);

};


#endif //MYMUSIC_JTFFMPEG_H
