//
// Created by ASUS on 2019/3/13.
//



#include "JTFFmpeg.h"



JTFFmpeg::JTFFmpeg(JTPlayerStatus *status, JTCallJava *callJava, const char *url) {
    this->playerStatus=status;
    this->callJava=callJava;
    this->url=url;
    pthread_mutex_init(&init_mutex,NULL);
    pthread_mutex_init(&seek_mutex,NULL);
}


void *decodeFfmpeg(void *data){
    JTFFmpeg *jtfFmpeg= (JTFFmpeg *)(data);
    jtfFmpeg->decodeFFmpegThread();
   // pthread_exit(&jtfFmpeg->decodeThread);
    return 0;
}


void JTFFmpeg::parpared() {

    pthread_create(&decodeThread,NULL,decodeFfmpeg,this);

}

int avformat_callback(void *ctx){
    JTFFmpeg *jtfFmpeg= (JTFFmpeg *)(ctx);
    if(jtfFmpeg->playerStatus->exit){
        return AVERROR_EOF;
    }

    return 0;

}

void JTFFmpeg::decodeFFmpegThread() {

    pthread_mutex_lock(&init_mutex);

    av_register_all();
    avformat_network_init();
    pFormatCtx=avformat_alloc_context();


    pFormatCtx->interrupt_callback.callback=avformat_callback;
    pFormatCtx->interrupt_callback.opaque=this;

    if(avformat_open_input(&pFormatCtx,url,NULL,NULL)!=0){
        if(LOG_DEBUG){
            LOGE("can not open url: s%",url);
        }
        callJava->onCallError(CHILD_THREAD,1001,"can not open url");
        exit=true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    if(avformat_find_stream_info(pFormatCtx,NULL)<0){
        if(LOG_DEBUG){
            LOGE("can not find stream form url: s%",url);
        }
        callJava->onCallError(CHILD_THREAD,1002,"can not find stream form url");
        exit=true;
        pthread_mutex_unlock(&init_mutex);
        return;
    }

    for(int i=0;i<pFormatCtx->nb_streams;i++){
        if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO){//得到音频流
               if(jtAudio==NULL){
                   jtAudio=new JTAudio(playerStatus,pFormatCtx->streams[i]->codecpar->sample_rate,callJava);
                   jtAudio->streamIndex=i;
                   jtAudio->codecpar=pFormatCtx->streams[i]->codecpar;
                   jtAudio->duration=pFormatCtx->duration/AV_TIME_BASE;
                   jtAudio->time_base=pFormatCtx->streams[i]->time_base;

                   duration=jtAudio->duration;
               }
        } else if(pFormatCtx->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_VIDEO){//得到视频流
               if(jtAudio==NULL){
                   jtVedio=new JTVedio(playerStatus,callJava);
                   jtVedio->streamIndex=i;
                   jtVedio->codecPar=pFormatCtx->streams[i]->codecpar;
                   jtVedio->time_base=pFormatCtx->streams[i]->time_base;


                   int num=pFormatCtx->streams[i]->avg_frame_rate.num;
                   int den=pFormatCtx->streams[i]->avg_frame_rate.den;

                   if(num!=0&&den!=0){
                       int fps=num/den;
                       jtVedio->defaultDelayTime=1.0/fps;
                   }

               }
        }
    }


    if(jtAudio!=NULL){
        getCodecContext(jtAudio->codecpar,&jtAudio->avCodecCtx);
    }

    if(jtVedio!=NULL){
        getCodecContext(jtVedio->codecPar,&jtVedio->avCodecCtx);
    }

    if(callJava!=NULL) {
        if (playerStatus != NULL && !playerStatus->exit) {
        callJava->onCallParpared(CHILD_THREAD);
       } else{
            exit=true;
        }
    }



    pthread_mutex_unlock(&init_mutex);

}

void JTFFmpeg::start() {

    if(jtAudio==NULL){
        if(LOG_DEBUG){
            LOGE("audio is null!");
        }
        callJava->onCallError(CHILD_THREAD,1007,"audio is null!");
        return;
    }


    if(jtVedio==NULL){
        return;
    }

    supportMedicCode= false;

    jtVedio->audio=jtAudio;


    const char *codecname=((const AVCodec *)jtVedio->avCodecCtx->codec)->name;

    supportMedicCode=callJava->onCallIsSupportVideo(codecname);

    if(supportMedicCode){
        if(LOG_DEBUG){
            LOGE("当前设备支持硬解码当前视频！");
        }



        if(strcasecmp(codecname,"h264")==0){

            bsFilter=av_bsf_get_by_name("h264_mp4toannexb");
        } else if(strcasecmp(codecname,"h265")==0){
            bsFilter=av_bsf_get_by_name("hevc_mp4toannexb");

        }

        if(bsFilter==NULL){
            supportMedicCode=false;
            goto end;
        }


        if(av_bsf_alloc(bsFilter,&jtVedio->absCtx)!=0){

            supportMedicCode=false;
            goto end;
        }

        if(avcodec_parameters_copy(jtVedio->absCtx->par_in,jtVedio->codecPar)<0){
            av_bsf_free(&jtVedio->absCtx);
            jtVedio->absCtx=NULL;
            supportMedicCode=false;
            goto end;
        }

        if(av_bsf_init(jtVedio->absCtx)!=0){
            av_bsf_free(&jtVedio->absCtx);
            jtVedio->absCtx=NULL;
            supportMedicCode=false;
            goto end;
        }

        jtVedio->absCtx->time_base_in=jtVedio->time_base;

    }

    end:

    if(supportMedicCode){
        jtVedio->codectype=CODEC_MEDIACODEC;
        jtVedio->callJava->onCallInitVedioMediaCodec(
                codecname,
                jtVedio->avCodecCtx->width,
                jtVedio->avCodecCtx->height,
                jtVedio->avCodecCtx->extradata_size,
                jtVedio->avCodecCtx->extradata_size,
                jtVedio->avCodecCtx->extradata,
                jtVedio->avCodecCtx->extradata
        );
    }

    jtAudio->play();
    jtVedio->play();


    while(playerStatus!=NULL&&!playerStatus->exit){

        if(playerStatus->seek){
            av_usleep(1000*100);
            continue;
        }

        if(jtAudio->queuePacket->getQueueSize()>40){
            av_usleep(1000*100);
            continue;
        }

        AVPacket *avPacket=av_packet_alloc();

        pthread_mutex_lock(&seek_mutex);
        int ret=av_read_frame(pFormatCtx,avPacket);
        pthread_mutex_unlock(&seek_mutex);
        if(ret==0){

            if(avPacket->stream_index==jtAudio->streamIndex){
                if(LOG_DEBUG){
                  //  LOGE("获取到音频avpacket");
                }

                jtAudio->queuePacket->putAVPkt(avPacket);

            }
            else if(avPacket->stream_index==jtVedio->streamIndex){
                jtVedio->queue->putAVPkt(avPacket);
                if(LOG_DEBUG){
                   //   LOGE("获取到视频avpacket");
                }
            }
            else {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
            }

        } else{
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket=NULL;

            while(playerStatus!=NULL&&!playerStatus->exit){
                if(jtAudio->queuePacket->getQueueSize()>0){
                    av_usleep(1000*100);
                    continue;
                } else{

                    if(!playerStatus->seek){
                        av_usleep(1000*500);
                        playerStatus->exit=true;
                    }

                    break;
                }
            }

        }

    }

    if(callJava!=NULL){
        callJava->onCallComplete(CHILD_THREAD);
    }

    exit= true;
    if(LOG_DEBUG){
        LOGE("解码完成！");
    }

}

void JTFFmpeg::pause() {

    if(playerStatus !=NULL){
        playerStatus->pause= true;
    }

    if(jtAudio!=NULL){
        jtAudio->pause();
    }


}

void JTFFmpeg::resume() {

    if(playerStatus !=NULL){
        playerStatus->pause= false;
    }

    if(jtAudio!=NULL){
        jtAudio->resume();
    }


}

void JTFFmpeg::release() {

   /* if(playerStatus->exit){
        return;
    }*/

    playerStatus->exit=true;

    pthread_join(decodeThread,NULL);

    pthread_mutex_lock(&init_mutex);

    int sleepCount=0;
    while (!exit){

        if(sleepCount>1000){
            exit=true;
        }

        if(LOG_DEBUG){
            LOGE("wait ffmpeg exit %d",sleepCount);
        }
        sleepCount++;
        av_usleep(1000*10);
    }

    if(jtAudio!=NULL){
        jtAudio->release();
        delete(jtAudio);
        jtAudio=NULL;
    }

    if(jtVedio!=NULL){
        jtVedio->release();
        delete(jtVedio);
        jtVedio=NULL;
    }

    if(pFormatCtx!=NULL){
        avformat_close_input(&pFormatCtx);
        avformat_free_context(pFormatCtx);
        pFormatCtx=NULL;
    }

    if(playerStatus!=NULL){
        playerStatus=NULL;
    }

    if(callJava!=NULL){
        callJava=NULL;
    }

    pthread_mutex_unlock(&init_mutex);
}

JTFFmpeg::~JTFFmpeg() {
    pthread_mutex_destroy(&init_mutex);
    pthread_mutex_destroy(&seek_mutex);
}

void JTFFmpeg::seek(int64_t secds) {
     if(duration<=0){
         return;
     }

     if(secds>=0&&secds<=duration){


         playerStatus->seek=true;
         pthread_mutex_lock(&seek_mutex);

         int64_t rel=secds*AV_TIME_BASE;
         avformat_seek_file(pFormatCtx,-1,INT64_MIN,rel,INT64_MAX,0);

         if(jtAudio!=NULL){

             jtAudio->queuePacket->clearAvpacket();
             jtAudio->clock=0;
             jtAudio->last_time=0;
             pthread_mutex_lock(&jtAudio->codecMutex);
             avcodec_flush_buffers(jtAudio->avCodecCtx);
             pthread_mutex_unlock(&jtAudio->codecMutex);
         }

       if(jtVedio!=NULL){

           jtVedio->queue->clearAvpacket();
           jtVedio->colock=0;

           pthread_mutex_lock(&jtVedio->codecMutex);
           avcodec_flush_buffers(jtVedio->avCodecCtx);
           pthread_mutex_unlock(&jtVedio->codecMutex);
       }

         pthread_mutex_unlock(&seek_mutex);
         playerStatus->seek= false;

     }
}

void JTFFmpeg::setVolume(int percent) {
     if(jtAudio!=NULL){
         jtAudio->setVolume(percent);
     }
}

void JTFFmpeg::setMute(int mute) {
    if(jtAudio!=NULL){
        jtAudio->setMute(mute);
    }
}

void JTFFmpeg::setPitch(float pitch) {
     if(jtAudio!=NULL){
         jtAudio->setPitch(pitch);
     }
}

void JTFFmpeg::setSpeed(float speed) {
    if(jtAudio!=NULL){
        jtAudio->setSpeed(speed);
    }
}

int JTFFmpeg::getSampleRate() {
    if(jtAudio!=NULL){
        return jtAudio->avCodecCtx->sample_rate;
    }
    return 0;
}

void JTFFmpeg::startStopRecord(bool start) {
    if(jtAudio!=NULL){
        jtAudio->startStopRecord(start);
    }
}

bool JTFFmpeg::cutAudioPlay(int starttime, int endtime, bool showpcm) {
    if(starttime>=0&&endtime<=duration&&endtime>starttime){
        jtAudio->isCut=true;
        jtAudio->endtime=endtime;
        jtAudio->showPcm=showpcm;

        seek(starttime);

        return true;
    }
    return false;
}

int JTFFmpeg::getCodecContext(AVCodecParameters *par, AVCodecContext **avCodecCtx) {

    //得到解码器
    AVCodec *dec=avcodec_find_decoder(par->codec_id);

    if(!dec){
        if(LOG_DEBUG){
            LOGE("can not find decoder");
        }
        callJava->onCallError(CHILD_THREAD,1003,"can not find decoder");
        exit=true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    *avCodecCtx=avcodec_alloc_context3(dec);

    if(!*avCodecCtx){
        if(LOG_DEBUG){
            LOGE("can not alloc avCodecCtx");
        }
        callJava->onCallError(CHILD_THREAD,1004,"can not alloc avCodecCtx");
        exit=true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    if(avcodec_parameters_to_context(*avCodecCtx,par)<0){
        if(LOG_DEBUG){
            LOGE("can not fill avCodecCtx");
        }
        callJava->onCallError(CHILD_THREAD,1005,"can not fill avCodecCtx");
        exit=true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    };

    if(avcodec_open2(*avCodecCtx,dec,0)!=0){
        if(LOG_DEBUG){
            LOGE("can not open audio stream");
        }
        callJava->onCallError(CHILD_THREAD,1006,"can not open audio stream");
        exit=true;
        pthread_mutex_unlock(&init_mutex);
        return -1;
    }

    return 0;
}


