//
// Created by ASUS on 2019/3/13.
//

#include "JTCallJava.h"

JTCallJava::JTCallJava(JavaVM *javaVM, JNIEnv *env, jobject *obj) {
    this->javaVM=javaVM;
    this->jniEnv=env;
    this->jobj=env->NewGlobalRef(*obj);

    jclass jlz=jniEnv->GetObjectClass(jobj);

    if(!jlz){

        if(LOG_DEBUG){
            LOGE("get jclass worng!")
        }
        return;
    }

    jmid_parpared =jniEnv->GetMethodID(jlz,"onCallParpared","()V");
    jmid_load =jniEnv->GetMethodID(jlz,"onCallLoad","(Z)V");
    jmid_time_info =jniEnv->GetMethodID(jlz,"onCallTimeInfo","(II)V");
    jmid_error =jniEnv->GetMethodID(jlz,"onCallError","(ILjava/lang/String;)V");
    jmid_complete =jniEnv->GetMethodID(jlz,"onCallComplete","()V");
    jmid_db =jniEnv->GetMethodID(jlz,"onCallVolumeDB","(I)V");
    jmid_aac =jniEnv->GetMethodID(jlz,"encodecPcmToAAC","(I[B)V");
    jmid_pcm_info =jniEnv->GetMethodID(jlz,"onCallPcmInfo","([BI)V");
    jmid_render_yuv =jniEnv->GetMethodID(jlz,"onCallRenderYUV","(II[B[B[B)V");
    jmid_support_vedio =jniEnv->GetMethodID(jlz,"onCallIsSuppoerMediaCode","(Ljava/lang/String;)Z");
    jmid_init_mediacodec =jniEnv->GetMethodID(jlz,"initVedioMediaCodec","(Ljava/lang/String;II[B[B)V");
    jmid_decode_avpacket =jniEnv->GetMethodID(jlz,"decodeAVpacket","(I[B)V");

}

JTCallJava::~JTCallJava() {

}

void JTCallJava::onCallParpared(int type) {

    if(type==MAIN_THREAD){

        jniEnv->CallVoidMethod(jobj,jmid_parpared);

    } else if(type==CHILD_THREAD){

        JNIEnv *env;

        if(javaVM->AttachCurrentThread(&env,0)!=JNI_OK){
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng!")
            }
            return;
        }

        env->CallVoidMethod(jobj,jmid_parpared);

        javaVM->DetachCurrentThread();

    }

}

void JTCallJava::onCallLoad(int type, bool load) {
    if(type==MAIN_THREAD){

        jniEnv->CallVoidMethod(jobj,jmid_load,load);

    } else if(type==CHILD_THREAD){

        JNIEnv *env;

        if(javaVM->AttachCurrentThread(&env,0)!=JNI_OK){
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng!")
            }
            return;
        }

        env->CallVoidMethod(jobj,jmid_load,load);

        javaVM->DetachCurrentThread();

    }
}

void JTCallJava::onCallInfo(int type, int currentTime, int totalTime) {
    if(type==MAIN_THREAD){

        jniEnv->CallVoidMethod(jobj,jmid_time_info,currentTime,totalTime);

    } else if(type==CHILD_THREAD){

        JNIEnv *env;

        if(javaVM->AttachCurrentThread(&env,0)!=JNI_OK){
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng!")
            }
            return;
        }

        env->CallVoidMethod(jobj,jmid_time_info,currentTime,totalTime);

        javaVM->DetachCurrentThread();

    }
}

void JTCallJava::onCallError(int type, int code, char *msg) {
    if(type==MAIN_THREAD){
        jstring jmsg=jniEnv->NewStringUTF(msg);
        jniEnv->CallVoidMethod(jobj,jmid_error,code,jmsg);

        jniEnv->DeleteLocalRef(jmsg);

    } else if(type==CHILD_THREAD){

        JNIEnv *env;

        if(javaVM->AttachCurrentThread(&env,0)!=JNI_OK){
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng!")
            }
            return;
        }

        jstring jmsg=env->NewStringUTF(msg);

        env->CallVoidMethod(jobj,jmid_error,code,jmsg);

        env->DeleteLocalRef(jmsg);
        javaVM->DetachCurrentThread();



    }

}

void JTCallJava::onCallComplete(int type) {
    if(type==MAIN_THREAD){

        jniEnv->CallVoidMethod(jobj,jmid_complete);

    } else if(type==CHILD_THREAD){

        JNIEnv *env;

        if(javaVM->AttachCurrentThread(&env,0)!=JNI_OK){
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng!")
            }
            return;
        }

        env->CallVoidMethod(jobj,jmid_complete);

        javaVM->DetachCurrentThread();

    }
}

void JTCallJava::onCallVolumeDB(int type, int db) {
    if(type==MAIN_THREAD){

        jniEnv->CallVoidMethod(jobj,jmid_db,db);

    } else if(type==CHILD_THREAD){

        JNIEnv *env;

        if(javaVM->AttachCurrentThread(&env,0)!=JNI_OK){
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng!")
            }
            return;
        }

        env->CallVoidMethod(jobj,jmid_db,db);

        javaVM->DetachCurrentThread();

    }
}

void JTCallJava::onCallPcmToAac(int type, int size, void *buffer) {
    if(type==MAIN_THREAD){
        jbyteArray jBuffer=jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jBuffer,0,size,(jbyte*)buffer);
        jniEnv->CallVoidMethod(jobj,jmid_aac,size,jBuffer);
        jniEnv->DeleteLocalRef(jBuffer);

    } else if(type==CHILD_THREAD){

        JNIEnv *jniEnv;

        if(javaVM->AttachCurrentThread(&jniEnv,0)!=JNI_OK){
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng!")
            }
            return;
        }

        jbyteArray jBuffer=jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jBuffer,0,size,(jbyte*)buffer);

        jniEnv->CallVoidMethod(jobj,jmid_aac,size,jBuffer);

        jniEnv->DeleteLocalRef(jBuffer);

        javaVM->DetachCurrentThread();

    }
}

void JTCallJava::onCallPcmInfo(int type, void *buffer, int size) {
    if(type==MAIN_THREAD){
        jbyteArray jBuffer=jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jBuffer,0,size,(jbyte*)buffer);
        jniEnv->CallVoidMethod(jobj,jmid_pcm_info,jBuffer,size);
        jniEnv->DeleteLocalRef(jBuffer);

    } else if(type==CHILD_THREAD){

        JNIEnv *jniEnv;

        if(javaVM->AttachCurrentThread(&jniEnv,0)!=JNI_OK){
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng!")
            }
            return;
        }

        jbyteArray jBuffer=jniEnv->NewByteArray(size);
        jniEnv->SetByteArrayRegion(jBuffer,0,size,(jbyte*)buffer);

        jniEnv->CallVoidMethod(jobj,jmid_pcm_info,jBuffer,size);

        jniEnv->DeleteLocalRef(jBuffer);

        javaVM->DetachCurrentThread();

    }
}

void
JTCallJava::onCallRenderYUV(int type, int width, int height, uint8_t *y, uint8_t *u, uint8_t *v) {
    if(type==CHILD_THREAD){

        JNIEnv *jniEnv;

        if(javaVM->AttachCurrentThread(&jniEnv,0)!=JNI_OK){
            if(LOG_DEBUG){
                LOGE("get child thread jnienv worng!")
            }
            return;
        }

        jbyteArray yb=jniEnv->NewByteArray(width*height);
        jniEnv->SetByteArrayRegion(yb,0,width*height,(jbyte*)y);

        jbyteArray ub=jniEnv->NewByteArray(width*height/4);
        jniEnv->SetByteArrayRegion(ub,0,width*height/4,(jbyte*)u);

        jbyteArray vb=jniEnv->NewByteArray(width*height/4);
        jniEnv->SetByteArrayRegion(vb,0,width*height/4,(jbyte*)v);

        jniEnv->CallVoidMethod(jobj,jmid_render_yuv,width,height,yb,ub,vb);

        jniEnv->DeleteLocalRef(yb);
        jniEnv->DeleteLocalRef(ub);
        jniEnv->DeleteLocalRef(vb);

        javaVM->DetachCurrentThread();

    }
}


void JTCallJava::onCallInitVedioMediaCodec(const char *mime, int widht, int height,int csd0_size,int csd1_size, uint8_t *csd_0, uint8_t *csd_1) {

    JNIEnv *jniEnv;

    if(javaVM->AttachCurrentThread(&jniEnv,0)!=JNI_OK){
        if(LOG_DEBUG){
            LOGE("get child thread jnienv worng!")
        }
        return;
    }

    jstring type = jniEnv->NewStringUTF(mime);

    jbyteArray csd0=jniEnv->NewByteArray(csd0_size);
    jniEnv->SetByteArrayRegion(csd0,0,csd0_size,(jbyte*)csd_0);

    jbyteArray csd1=jniEnv->NewByteArray(csd1_size);
    jniEnv->SetByteArrayRegion(csd1,0,csd1_size,(jbyte*)csd_1);


    jniEnv->CallVoidMethod(jobj,jmid_init_mediacodec,type,widht,height,csd0,csd1);

    jniEnv->DeleteLocalRef(csd0);
    jniEnv->DeleteLocalRef(csd1);

    javaVM->DetachCurrentThread();

}

void JTCallJava::onCallDecodeAVPacket(int datasize, uint8_t *data) {

    JNIEnv *jniEnv;

    if(javaVM->AttachCurrentThread(&jniEnv,0)!=JNI_OK){
        if(LOG_DEBUG){
            LOGE("get child thread jnienv worng!")
        }
        return;
    }

    jbyteArray packetdata=jniEnv->NewByteArray(datasize);
    jniEnv->SetByteArrayRegion(packetdata,0,datasize,(jbyte*)data);

    jniEnv->CallVoidMethod(jobj,jmid_decode_avpacket,datasize,packetdata);


    jniEnv->DeleteLocalRef(packetdata);
    javaVM->DetachCurrentThread();
}

bool JTCallJava::onCallIsSupportVideo(const char *ffcodecname) {

    bool support = false;

    JNIEnv *jniEnv;

    bool isAttached = false;

    if(javaVM->AttachCurrentThread(&jniEnv, 0) != JNI_OK)
    {
        if(LOG_DEBUG)
        {
            LOGE("call onCallComplete worng");
        }
        return support;
    } else{
        isAttached=true;
    }

    jstring type = jniEnv->NewStringUTF(ffcodecname);
    support = jniEnv->CallBooleanMethod(jobj, jmid_support_vedio, type);
    jniEnv->DeleteLocalRef(type);
    if(isAttached) {
        javaVM->DetachCurrentThread();
    }
    return support;
}


