//
// Created by ASUS on 2019/3/13.
//

#ifndef MYMUSIC_JTAUDIO_H
#define MYMUSIC_JTAUDIO_H

#include "JTQueue.h"
#include "JTPlayerStatus.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include "JTCallJava.h"
#include "SoundTouch.h"
#include "WlPcmBean.h"
#include "WlBufferQueue.h"

using namespace soundtouch;

extern "C"{
 #include <libavcodec/avcodec.h>
 #include <libswresample/swresample.h>
#include <libavutil/time.h>
};


class JTAudio {
public:
    int streamIndex=-1;

    AVCodecParameters *codecpar=NULL;
    AVCodecContext *avCodecCtx=NULL;
    JTQueue *queuePacket=NULL;
    JTPlayerStatus *jtPlayerStatus=NULL;

    pthread_t thread_play;

    AVPacket *avPacket=NULL;
    AVFrame *avFrame=NULL;

    uint8_t *buffer=NULL;

    JTCallJava *jtCallJava=NULL;

    int data_size=0;
    int ret=-1;
    int sample_rate=0;
    int duration=0;
    double now_time=0;
    double clock=0;
    double last_time=0;
    int volumePercent=100;
    int mute=2;

    float pitch=1.0f;
    float speed=1.0f;

    bool isRecordPcm=false;
    bool readFrameFinished= true;

    bool isCut=false;
    int endtime=0;
    bool showPcm=false;

    pthread_t pcmCallBackThread;
    WlBufferQueue *bufferQueue;
    int defaultPcmSize=4096;

    AVRational time_base;



    SLObjectItf  engineObject=NULL;
    SLEngineItf  engineItf=NULL;

    SLObjectItf  outputMixObj=NULL;
    SLEnvironmentalReverbItf  opEnvironmentReverb=NULL;
    SLEnvironmentalReverbSettings reverbSettings=SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    SLObjectItf  pcmPlayerObj=NULL;

    SLPlayItf playItf=NULL;
    SLVolumeItf pcmVolumeItf=NULL;
    SLMIDIMuteSoloItf pcmMidSoloItf=NULL;

    SLAndroidSimpleBufferQueueItf pcmBufferQueue=NULL;

    //创建SoundTouch 对象
    SoundTouch *soundTouch=NULL;
    SAMPLETYPE *sampltBuffer=NULL;

    bool finished= true;
    uint8_t *out_buffer=NULL;
    int nb=0;
    int num=0;

    pthread_mutex_t codecMutex;


public:
    JTAudio(JTPlayerStatus *status,int samplerate,JTCallJava *callJava);
    ~JTAudio();

    void play();

    int resampleAudio(void **pcmbuf);

    void initOpenSLES();

    int getCurrentSampleRateForOpenSLES(int samplerate);

    void pause();

    void resume();

    void stop();

    void release();

    void setVolume(int percent);

    void setMute(int mute);

    int getSoundTouchData();

    void setPitch(float pitch);

    void setSpeed(float speed);

    int getPcmDB(char *pcmdata,size_t size);

    void startStopRecord(bool start);

};


#endif //MYMUSIC_JTAUDIO_H
