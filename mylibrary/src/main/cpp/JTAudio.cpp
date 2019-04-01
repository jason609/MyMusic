//
// Created by ASUS on 2019/3/13.
//



#include "JTAudio.h"

JTAudio::JTAudio(JTPlayerStatus *status,int samplerate,JTCallJava *callJava) {
    this->jtPlayerStatus=status;
    queuePacket=new JTQueue(jtPlayerStatus);
    this->sample_rate=samplerate;
    this->jtCallJava=callJava;

    this->isCut=false;
    this->endtime=0;
    this->showPcm=false;

    buffer= (uint8_t *)(av_malloc(sample_rate * 2 * 2));

    sampltBuffer=(SAMPLETYPE*)av_malloc(samplerate*2*2);

    soundTouch=new SoundTouch();
    soundTouch->setSampleRate(samplerate);
    soundTouch->setChannels(2);

    soundTouch->setPitch(pitch);
    soundTouch->setTempo(speed);

    pthread_mutex_init(&codecMutex,NULL);
}

JTAudio::~JTAudio() {
    pthread_mutex_destroy(&codecMutex);
}

void *decodePlay(void *data){
      JTAudio *jtAudio=(JTAudio *)(data);
      jtAudio->initOpenSLES();
    //  pthread_exit(&jtAudio->thread_play);
     return 0;
}

void *pcmCallBack(void *data){
    JTAudio *jtAudio=(JTAudio *)(data);
    jtAudio->bufferQueue=new WlBufferQueue(jtAudio->jtPlayerStatus);

    while (jtAudio->jtPlayerStatus!=NULL&&!jtAudio->jtPlayerStatus->exit){
        WlPcmBean *wlPcmBean=NULL;
        jtAudio->bufferQueue->getBuffer(&wlPcmBean);

        if(wlPcmBean==NULL){
            continue;
        }

        if(wlPcmBean->buffsize<jtAudio->defaultPcmSize){//不用分包
              if(jtAudio->isRecordPcm) {//只有录音时才回调
                  jtAudio->jtCallJava->onCallPcmToAac(CHILD_THREAD, wlPcmBean->buffsize,wlPcmBean->buffer);
                        }

            if(jtAudio->showPcm){
                jtAudio->jtCallJava->onCallPcmInfo(CHILD_THREAD,wlPcmBean->buffer,wlPcmBean->buffsize);
            }
        } else{

           int pck_num=wlPcmBean->buffsize/jtAudio->defaultPcmSize;
           int pack_sub=wlPcmBean->buffsize%jtAudio->defaultPcmSize;

            for(int i=0;i<pck_num;i++){

                char *bf=(char *)malloc(jtAudio->defaultPcmSize);
                memcpy(bf,wlPcmBean->buffer+i*jtAudio->defaultPcmSize,jtAudio->defaultPcmSize);

                if(jtAudio->isRecordPcm) {//只有录音时才回调
                    jtAudio->jtCallJava->onCallPcmToAac(CHILD_THREAD, jtAudio->defaultPcmSize,bf);
                }

                if(jtAudio->showPcm){
                    jtAudio->jtCallJava->onCallPcmInfo(CHILD_THREAD,bf,jtAudio->defaultPcmSize);
                }

                free(bf);
                bf=NULL;
            }


            if(pack_sub>0){
                char *bf=(char *)malloc(pack_sub);
                memcpy(bf,wlPcmBean->buffer+pck_num*jtAudio->defaultPcmSize,pack_sub);

                if(jtAudio->isRecordPcm) {//只有录音时才回调
                    jtAudio->jtCallJava->onCallPcmToAac(CHILD_THREAD, pack_sub,bf);
                }

                if(jtAudio->showPcm){
                    jtAudio->jtCallJava->onCallPcmInfo(CHILD_THREAD,bf,pack_sub);
                }

                free(bf);
                bf=NULL;

            }

        }

        delete(wlPcmBean);
        wlPcmBean=NULL;

    }

   // pthread_exit(&jtAudio->pcmCallBackThread);
    return 0;
}


void JTAudio::play() {
     if(jtPlayerStatus!=NULL&&!jtPlayerStatus->exit) {
         pthread_create(&thread_play, NULL, decodePlay, this);
         pthread_create(&pcmCallBackThread, NULL, pcmCallBack, this);
     }
}

//音频文件重采样
int JTAudio::resampleAudio(void **pcmbuf) {
    data_size=0;
    while (jtPlayerStatus!=NULL&&!jtPlayerStatus->exit){

        if(jtPlayerStatus->seek){
            av_usleep(1000*100);
            continue;
        }

        if(queuePacket->getQueueSize()==0){
            if(!jtPlayerStatus->load){
                jtPlayerStatus->load= true;
                jtCallJava->onCallLoad(CHILD_THREAD,true);
            }

            av_usleep(1000*100);
            continue;
        } else{
            if(jtPlayerStatus->load){
                jtPlayerStatus->load= false;
                jtCallJava->onCallLoad(CHILD_THREAD,false);
            }
        }

        if(readFrameFinished) {
            avPacket = av_packet_alloc();
            if (queuePacket->getAVPkt(avPacket) != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }


            pthread_mutex_lock(&codecMutex);

            ret = avcodec_send_packet(avCodecCtx, avPacket);

            if (ret != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;

                pthread_mutex_unlock(&codecMutex);
                continue;
            }

        }

        avFrame=av_frame_alloc();
        ret=avcodec_receive_frame(avCodecCtx,avFrame);

        if(ret==0){
            readFrameFinished=false;
            if(avFrame->channels>0&&avFrame->channel_layout ==0){
                 avFrame->channel_layout=av_get_default_channel_layout(avFrame->channels);
            } else if(avFrame->channels==0&&avFrame->channel_layout>0){
                avFrame->channels=av_get_channel_layout_nb_channels(avFrame->channel_layout);
            }

            SwrContext *swr_ctx;
            swr_ctx=swr_alloc_set_opts(
                    NULL,
                    AV_CH_LAYOUT_STEREO,
                    AV_SAMPLE_FMT_S16,
                    avFrame->sample_rate,
                    avFrame->channel_layout,
                    (AVSampleFormat)(avFrame->format),
                    avFrame->sample_rate,
                    NULL,
                    NULL
            );
            if(!swr_ctx || swr_init(swr_ctx)<0){

                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket=NULL;

                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame=NULL;

                if(swr_ctx!=NULL){
                    swr_free(&swr_ctx);
                    swr_ctx=NULL;
                }

                readFrameFinished=true;

                pthread_mutex_unlock(&codecMutex);

                continue;

            }


            nb=swr_convert(
              swr_ctx,
              &buffer,
              avFrame->nb_samples,
              (const uint8_t **)(avFrame->data),
              avFrame->nb_samples
            );


            int out_channels=av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            data_size=nb*out_channels*av_get_bytes_per_sample(AV_SAMPLE_FMT_S16);


            now_time=avFrame->pts*av_q2d(time_base);

            if(now_time<clock){
                now_time=clock;
            }

            clock=now_time;

            *pcmbuf=buffer;

            if(LOG_DEBUG){
               // LOGD("data size is %d",data_size);
            }


           /* av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket=NULL;*/

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame=NULL;

            swr_free(&swr_ctx);
            swr_ctx=NULL;

            pthread_mutex_unlock(&codecMutex);
            break;

        } else{
            readFrameFinished=true;
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket=NULL;

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame=NULL;

            pthread_mutex_unlock(&codecMutex);

            continue;
        }

    }
    return data_size;
}



int JTAudio::getSoundTouchData() {
    out_buffer=NULL;
    while (jtPlayerStatus!=NULL&&!jtPlayerStatus->exit){
        if(finished){

            finished= false;

            data_size=resampleAudio((void **)&out_buffer);

            if(data_size>0){
                for(int i=0;i<data_size/2+1;i++){
                    sampltBuffer[i]=(out_buffer[i*2] | ((out_buffer[i*2+1])<<8));
                }

                soundTouch->putSamples(sampltBuffer,nb);
                num=soundTouch->receiveSamples(sampltBuffer,data_size/4);
            } else{
                soundTouch->flush();
            }
        }

        if(num==0){
            finished=true;
            continue;
        } else{
            if(out_buffer==NULL){
                num=soundTouch->receiveSamples(sampltBuffer,data_size/4);

                if(num==0){
                    finished=true;
                    continue;
                }
            }

            return num;
        }

    }

    return 0;
}



//播放器回调
void pcmBufferCallBack(SLAndroidSimpleBufferQueueItf bfq,void *context){
     JTAudio *audio=(JTAudio *)(context);
     if(audio!=NULL){
         int bufferSize=audio->getSoundTouchData();
         if(bufferSize>0){
             audio->clock+=bufferSize/((double)audio->sample_rate*2*2);
           //  if(audio->clock>audio->duration)return;
             if(audio->clock-audio->last_time>=0.1) {
                 audio->last_time=audio->clock;
                 audio->jtCallJava->onCallInfo(CHILD_THREAD, audio->clock, audio->duration);
             }

           /*  if(audio->isRecordPcm) {//只有录音时才回调
                 audio->jtCallJava->onCallPcmToAac(CHILD_THREAD, bufferSize * 2 * 2, audio->sampltBuffer);
             }*/

             audio->bufferQueue->putBuffer(audio->sampltBuffer,bufferSize*4);
             audio->jtCallJava->onCallVolumeDB(CHILD_THREAD,
             audio->getPcmDB((char *)audio->sampltBuffer,bufferSize*4)
             );

             (*audio->pcmBufferQueue)->Enqueue(audio->pcmBufferQueue,audio->sampltBuffer,bufferSize*4);


             if(audio->isCut){
                /* if(audio->showPcm){
                       audio->jtCallJava->onCallPcmInfo(CHILD_THREAD,audio->sampltBuffer,bufferSize*4);
                 }*/

                 if(audio->clock>audio->endtime){
                     if(LOG_DEBUG){
                         LOGD("裁剪退出！");
                     }
                     audio->jtPlayerStatus->exit=true;
                 }
             }

         }
     }

};

//初始化openSLES
void JTAudio::initOpenSLES()
{

    slCreateEngine(&engineObject,0,0,0,0,0);

    (*engineObject)->Realize(engineObject,SL_BOOLEAN_FALSE);
    (*engineObject)->GetInterface(engineObject,SL_IID_ENGINE,&engineItf);

    const SLInterfaceID mids[1]={SL_IID_ENVIRONMENTALREVERB};

    const SLboolean mreq[1]={SL_BOOLEAN_FALSE};

    (*engineItf)->CreateOutputMix(engineItf,&outputMixObj,1,mids,mreq);
    (*outputMixObj)->Realize(outputMixObj,SL_BOOLEAN_FALSE);
    (*outputMixObj)->GetInterface(outputMixObj,SL_IID_ENVIRONMENTALREVERB,&opEnvironmentReverb);

    (*opEnvironmentReverb)->SetEnvironmentalReverbProperties(opEnvironmentReverb,&reverbSettings);


    SLDataLocator_AndroidBufferQueue androidBufferQueue={SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE,2};
    SLDataFormat_PCM pcm={
            SL_DATAFORMAT_PCM,
            2,
            (SLuint32)getCurrentSampleRateForOpenSLES(sample_rate),
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };

    SLDataLocator_OutputMix outputMix={SL_DATALOCATOR_OUTPUTMIX,outputMixObj};

    SLDataSource slDataSource={&androidBufferQueue,&pcm};

    SLDataSink audioSnk={&outputMix,NULL};

    const SLInterfaceID ids[4]={SL_IID_BUFFERQUEUE,SL_IID_VOLUME,SL_IID_PLAYBACKRATE,SL_IID_MUTESOLO};
    const SLboolean mreqs[4]={SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE,SL_BOOLEAN_TRUE};

    (*engineItf)->CreateAudioPlayer(engineItf,&pcmPlayerObj,&slDataSource,&audioSnk,4,ids,mreqs);
    (*pcmPlayerObj)->Realize(pcmPlayerObj,SL_BOOLEAN_FALSE);
    (*pcmPlayerObj)->GetInterface(pcmPlayerObj,SL_IID_PLAY,&playItf);

    (*pcmPlayerObj)->GetInterface(pcmPlayerObj,SL_IID_BUFFERQUEUE,&pcmBufferQueue);

    //获取声音接口
    (*pcmPlayerObj)->GetInterface(pcmPlayerObj,SL_IID_VOLUME,&pcmVolumeItf);
    //获取声道接口
    (*pcmPlayerObj)->GetInterface(pcmPlayerObj,SL_IID_MUTESOLO,&pcmMidSoloItf);

    setVolume(volumePercent);
    setMute(mute);

    (*pcmBufferQueue)->RegisterCallback(pcmBufferQueue,pcmBufferCallBack,this);

    (*playItf)->SetPlayState(playItf,SL_PLAYSTATE_PLAYING);

    pcmBufferCallBack(pcmBufferQueue,this);

}

//当前采样率转换
int JTAudio::getCurrentSampleRateForOpenSLES(int samplerate) {
    int rate=0;
    switch (samplerate){
        case 80000:
            rate=SL_SAMPLINGRATE_8;
            break;
        case 11025:
            rate=SL_SAMPLINGRATE_11_025;
            break;
        case 12000:
            rate=SL_SAMPLINGRATE_12;
            break;
        case 16000:
            rate=SL_SAMPLINGRATE_16;
            break;
        case 22050:
            rate=SL_SAMPLINGRATE_22_05;
            break;
        case 24000:
            rate=SL_SAMPLINGRATE_24;
            break;
        case 32000:
            rate=SL_SAMPLINGRATE_32;
            break;
        case 44100:
            rate=SL_SAMPLINGRATE_44_1;
            break;
        case 48000:
            rate=SL_SAMPLINGRATE_48;
            break;
        case 64000:
            rate=SL_SAMPLINGRATE_64;
            break;
        case 88200:
            rate=SL_SAMPLINGRATE_88_2;
            break;
        case 96000:
            rate=SL_SAMPLINGRATE_96;
            break;
        case 192000:
            rate=SL_SAMPLINGRATE_192;
            break;

        default:
            rate=SL_SAMPLINGRATE_44_1;
            break;

    }
    return rate;
}

//暂停播放
void JTAudio::pause() {
    if(playItf!=NULL){
        (*playItf)->SetPlayState(playItf,SL_PLAYSTATE_PAUSED);
    }
}

//播放
void JTAudio::resume() {
    if(playItf!=NULL){
        (*playItf)->SetPlayState(playItf,SL_PLAYSTATE_PLAYING);
    }
}

//停止播放
void JTAudio::stop() {
    if(playItf!=NULL){
        (*playItf)->SetPlayState(playItf,SL_PLAYSTATE_STOPPED);
    }
}

//释放资源
void JTAudio::release() {
    stop();

    if(queuePacket!=NULL) {
        queuePacket->noticeQueue();
    }

    pthread_join(thread_play,NULL);

    if(bufferQueue!=NULL){
        bufferQueue->noticeThread();
        pthread_join(pcmCallBackThread,NULL);
        bufferQueue->release();
        delete(bufferQueue);
        bufferQueue=NULL;
    }

    if(queuePacket!=NULL){
        delete(queuePacket);
        queuePacket=NULL;
    }

    if(pcmPlayerObj!=NULL){
        (*pcmPlayerObj)->Destroy(pcmPlayerObj);
        pcmPlayerObj=NULL;
        playItf=NULL;
        pcmBufferQueue=NULL;
        pcmVolumeItf=NULL;
        pcmMidSoloItf=NULL;
    }

    if(outputMixObj!=NULL){
        (*outputMixObj)->Destroy(outputMixObj);
        outputMixObj=NULL;
        opEnvironmentReverb=NULL;
    }

    if(engineObject!=NULL){
        (*engineObject)->Destroy(engineObject);
        engineObject=NULL;
        engineItf=NULL;
    }


    if(buffer!=NULL){
        free(buffer);
        buffer=NULL;
    }

    if(out_buffer!=NULL){
        out_buffer=NULL;
    }

    if(soundTouch!=NULL){
        delete soundTouch;
        soundTouch=NULL;
    }

    if(sampltBuffer!=NULL){
        free(sampltBuffer);
        sampltBuffer=NULL;
    }

    if(avCodecCtx!=NULL){
        pthread_mutex_lock(&codecMutex);
        avcodec_close(avCodecCtx);
        avcodec_free_context(&avCodecCtx);
        avCodecCtx=NULL;
        pthread_mutex_unlock(&codecMutex);
    }


    if(jtPlayerStatus!=NULL){
        jtPlayerStatus=NULL;
    }

    if(jtCallJava!=NULL){
        jtCallJava=NULL;
    }

}

//设置音量
void JTAudio::setVolume(int percent) {
    volumePercent=percent;
     if(pcmVolumeItf!=NULL){
         if(percent>30){
             (*pcmVolumeItf)->SetVolumeLevel(pcmVolumeItf,(100-percent)*-20);
         } else if(percent>25){
             (*pcmVolumeItf)->SetVolumeLevel(pcmVolumeItf,(100-percent)*-22);
         } else if(percent>20){
             (*pcmVolumeItf)->SetVolumeLevel(pcmVolumeItf,(100-percent)*-25);
         } else if(percent>15){
             (*pcmVolumeItf)->SetVolumeLevel(pcmVolumeItf,(100-percent)*-28);
         } else if(percent>10){
             (*pcmVolumeItf)->SetVolumeLevel(pcmVolumeItf,(100-percent)*-30);
         } else if(percent>5){
             (*pcmVolumeItf)->SetVolumeLevel(pcmVolumeItf,(100-percent)*-34);
         } else if(percent>3){
             (*pcmVolumeItf)->SetVolumeLevel(pcmVolumeItf,(100-percent)*-37);
         } else if(percent>0){
             (*pcmVolumeItf)->SetVolumeLevel(pcmVolumeItf,(100-percent)*-40);
         } else{
             (*pcmVolumeItf)->SetVolumeLevel(pcmVolumeItf,(100-percent)*-100);
         }

     }
}

void JTAudio::setMute(int mute) {
    this->mute=mute;
    if(pcmMidSoloItf!=NULL){
        if(mute==0)//右声道
        {
            (*pcmMidSoloItf)->SetChannelMute(pcmMidSoloItf,1,false);
            (*pcmMidSoloItf)->SetChannelMute(pcmMidSoloItf,0,true);
        }
        else if (mute==1)//左声道
        {
            (*pcmMidSoloItf)->SetChannelMute(pcmMidSoloItf,1,true);
            (*pcmMidSoloItf)->SetChannelMute(pcmMidSoloItf,0,false);
        }
        else if(mute==2)//立体声
        {
            (*pcmMidSoloItf)->SetChannelMute(pcmMidSoloItf,1,false);
            (*pcmMidSoloItf)->SetChannelMute(pcmMidSoloItf,0,false);
        }
    }

}

void JTAudio::setPitch(float pitch) {
    if(soundTouch!=NULL){
        soundTouch->setPitch(pitch);
    }

}

void JTAudio::setSpeed(float speed) {
    if(soundTouch!=NULL){
        soundTouch->setTempo(speed);
    }
}

int JTAudio::getPcmDB(char *pcmdata, size_t size) {
    int db=0;
    short int pervalue=0;
    double sum=0;

    for(int i=0;i<size;i+=2){
        memcpy(&pervalue,pcmdata+i,2);
        sum+=abs(pervalue);
    }

    sum=sum/(size/2);

    if(sum>0){
        db=(int)20.0*log10(sum);
    }

    return db;
}

void JTAudio::startStopRecord(bool start) {
     this->isRecordPcm=start;
}


