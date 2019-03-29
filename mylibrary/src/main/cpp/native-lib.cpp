#include <jni.h>
#include <string>
#include "JTCallJava.h"
#include "JTFFmpeg.h"


extern "C"{
#include <libavformat/avformat.h>
}

JavaVM *javaVM=NULL;
JTCallJava *callJava=NULL;
JTFFmpeg *jtfFmpeg=NULL;

JTPlayerStatus *playerStatus=NULL;

bool nexit=true;

pthread_t pthread_start;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm,void *reserved)
{
	jint result =-1;
	javaVM=vm;
	JNIEnv *env;

	if(javaVM->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK){
		return result;
	}

	return JNI_VERSION_1_4;
}




extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1parpared(JNIEnv *env, jobject instance, jstring source_) {
	const char *source = env->GetStringUTFChars(source_, 0);
	// TODO
	if(jtfFmpeg==NULL){
         if(callJava==NULL){
			 callJava=new JTCallJava(javaVM,env,&instance);
		 }

        callJava->onCallLoad(MAIN_THREAD, true);

        playerStatus=new JTPlayerStatus();

		jtfFmpeg=new JTFFmpeg(playerStatus,callJava,source);
		jtfFmpeg->parpared();
	}

	//env->ReleaseStringUTFChars(source_, source);
}


void *startCallBack(void * data){
    JTFFmpeg *fmpeg=(JTFFmpeg* )(data);
    fmpeg->start();
   // pthread_exit(&pthread_start);
	return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1start(JNIEnv *env, jobject instance) {

    // TODO
    if(jtfFmpeg!=NULL){
      // jtfFmpeg->start();
        pthread_create(&pthread_start,NULL,startCallBack,jtfFmpeg);
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1pause(JNIEnv *env, jobject instance) {

    // TODO
	if(jtfFmpeg!=NULL){
		jtfFmpeg->pause();
	}

}extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1resume(JNIEnv *env, jobject instance) {

	// TODO
	if(jtfFmpeg!=NULL){
		jtfFmpeg->resume();
	}
}extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1stop(JNIEnv *env, jobject instance) {

    if(!nexit){
        return;
    }

    nexit= false;

    jclass jclz=env->GetObjectClass(instance);
    jmethodID  jmid_next=env->GetMethodID(jclz,"onCallNext","()V");

    // TODO
	if(jtfFmpeg!=NULL){
		jtfFmpeg->release();

		pthread_join(pthread_start,NULL);

		delete(jtfFmpeg);
		jtfFmpeg=NULL;

		if(callJava!=NULL){
			delete(callJava);
			callJava=NULL;
		}

		if(playerStatus!=NULL){
			delete(playerStatus);
			playerStatus=NULL;
		}
	}

    nexit= true;

    env->CallVoidMethod(instance,jmid_next);

}extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1seek(JNIEnv *env, jobject instance, jint secds) {

    // TODO
    if(jtfFmpeg!=NULL){
        jtfFmpeg->seek(secds);
    }

}extern "C"
JNIEXPORT jint JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1duration(JNIEnv *env, jobject instance) {

    // TODO
    if(jtfFmpeg!=NULL){
        return jtfFmpeg->duration;
    }
    return 0;

}extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1volume(JNIEnv *env, jobject instance, jint percent) {

    // TODO
    if(jtfFmpeg!=NULL){
        jtfFmpeg->setVolume(percent);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1mute(JNIEnv *env, jobject instance, jint mute) {

    // TODO
    if(jtfFmpeg!=NULL){
        jtfFmpeg->setMute(mute);
    }
}extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1pitch(JNIEnv *env, jobject instance, jfloat pitch) {

    // TODO
	if(jtfFmpeg!=NULL){
		jtfFmpeg->setPitch(pitch);
	}

}extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1speed(JNIEnv *env, jobject instance, jfloat speed) {

	// TODO
	if(jtfFmpeg!=NULL){
		jtfFmpeg->setSpeed(speed);
	}

}extern "C"
JNIEXPORT jint JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1samplerate(JNIEnv *env, jobject instance) {

	// TODO
    if(jtfFmpeg!=NULL){
		return jtfFmpeg->getSampleRate();
	}

	return 0;
}extern "C"
JNIEXPORT void JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1startstoprecord(JNIEnv *env, jobject instance,
                                                         jboolean start) {

    // TODO
    if(jtfFmpeg!=NULL){
        jtfFmpeg->startStopRecord(start);
    }

}extern "C"
JNIEXPORT jboolean JNICALL
Java_com_yj_mylibrary_player_JTPlayer_n_1cutaudioplay(JNIEnv *env, jobject instance, jint starttime,
                                                      jint endtime, jboolean showpcm) {

    // TODO
    if(jtfFmpeg!=NULL){
        return jtfFmpeg->cutAudioPlay(starttime,endtime,showpcm);
    }

    return false;

}