//
// Created by ASUS on 2019/3/13.
//

#ifndef MYMUSIC_JTCALLJAVA_H
#define MYMUSIC_JTCALLJAVA_H

#include <string>
#include "jni.h"
#include "AndroidLog.h"

#define MAIN_THREAD 0
#define CHILD_THREAD 1

class JTCallJava {

public:

    JavaVM *javaVM=NULL;
    JNIEnv *jniEnv=NULL;
    jobject jobj=NULL;
    jmethodID jmid_parpared=NULL;
    jmethodID jmid_load=NULL;
    jmethodID jmid_time_info=NULL;
    jmethodID jmid_error=NULL;
    jmethodID jmid_complete=NULL;
    jmethodID jmid_db=NULL;
    jmethodID jmid_aac=NULL;
    jmethodID jmid_pcm_info=NULL;
    jmethodID jmid_render_yuv=NULL;
    jmethodID jmid_support_vedio=NULL;
    jmethodID jmid_init_mediacodec=NULL;
    jmethodID jmid_decode_avpacket=NULL;


public:

    JTCallJava(JavaVM *javaVM,JNIEnv *jniEnv,jobject *obj);
    ~JTCallJava();

    void onCallParpared(int type);

    void onCallLoad(int type, bool load);

    void onCallInfo(int type,int currentTime,int totalTime);

    void onCallError(int type,int code, char* msg);

    void onCallComplete(int type);

    void onCallVolumeDB(int type,int db);

    void onCallPcmToAac(int type,int size,void *buffer);

    void onCallPcmInfo(int type,void *buffer,int size);

    void onCallRenderYUV(int type,int width,int height,uint8_t *y,uint8_t *u,uint8_t *v);


    bool onCallIsSupportVideo(const char *ffcodecname);


    void onCallInitVedioMediaCodec(const char* mime,int widht,int height,int csd0_size,int csd1_size,uint8_t *csd_0,uint8_t *csd_1);

    void onCallDecodeAVPacket(int datasize,uint8_t *data);
};




#endif //MYMUSIC_JTCALLJAVA_H
