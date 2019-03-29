//
// Created by ASUS on 2019/3/20.
//



#include "JTVedio.h"

JTVedio::JTVedio(JTPlayerStatus *status, JTCallJava *call) {
        this->playerStatus=status;
        this->callJava=call;

        queue=new JTQueue(playerStatus);
        pthread_mutex_init(&codecMutex,NULL);
}

JTVedio::~JTVedio() {
        pthread_mutex_destroy(&codecMutex);
}


void *playVedio(void * data){
    JTVedio *jtVedio=(JTVedio *)data;

    while(jtVedio->playerStatus!=NULL&&!jtVedio->playerStatus->exit) {

        if (jtVedio->playerStatus->seek) {
            av_usleep(1000 * 100);
            continue;
        }


        if (jtVedio->playerStatus->pause) {
            av_usleep(1000 * 100);
            continue;
        }

        if (jtVedio->queue->getQueueSize() == 0) {
            if (!jtVedio->playerStatus->load) {
                jtVedio->playerStatus->load = true;
                jtVedio->callJava->onCallLoad(CHILD_THREAD, true);
            }
            av_usleep(1000 * 100);
            continue;
        } else {
            if (jtVedio->playerStatus->load) {

                jtVedio->playerStatus->load = false;
                jtVedio->callJava->onCallLoad(CHILD_THREAD, false);

            }
        }


        AVPacket *avPacket = av_packet_alloc();

        if (jtVedio->queue->getAVPkt(avPacket) != 0) {
            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;
            continue;
        }


        if (jtVedio->codectype == CODEC_MEDIACODEC) {

            if (LOG_DEBUG) {
                LOGD("硬解码")
            }

            if(av_bsf_send_packet(jtVedio->absCtx,avPacket)!=0){
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                continue;
            }

            while (av_bsf_receive_packet(jtVedio->absCtx,avPacket)==0){

                LOGD("开始硬解码")

                double diff = jtVedio->getFrameDiffTime(NULL,avPacket);

                av_usleep(jtVedio->getDelayTime(diff) * 1000 * 1000);

                jtVedio->callJava->onCallDecodeAVPacket(avPacket->size,avPacket->data);

                av_packet_free(&avPacket);
                av_free(avPacket);
                continue;
            }

            avPacket = NULL;

        } else if (jtVedio->codectype==CODEC_YUV) {

            pthread_mutex_lock(&jtVedio->codecMutex);

            if (avcodec_send_packet(jtVedio->avCodecCtx, avPacket) != 0) {
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&jtVedio->codecMutex);
                continue;
            };

            AVFrame *avFrame = av_frame_alloc();


            if (avcodec_receive_frame(jtVedio->avCodecCtx, avFrame) != 0) {
                av_frame_free(&avFrame);
                av_free(avFrame);
                avFrame = NULL;
                av_packet_free(&avPacket);
                av_free(avPacket);
                avPacket = NULL;
                pthread_mutex_unlock(&jtVedio->codecMutex);
                continue;

            }

            if (LOG_DEBUG) {
                LOGD("子线程解码一个avframe成功")
            }

            if (avFrame->format == AV_PIX_FMT_YUV420P) {


                double diff = jtVedio->getFrameDiffTime(avFrame,NULL);

                av_usleep(jtVedio->getDelayTime(diff) * 1000 * 1000);


                //直接渲染

                if (LOG_DEBUG) {
                    LOGD("当前视频格式YUV420Pa")
                }

                jtVedio->callJava->onCallRenderYUV(CHILD_THREAD, jtVedio->avCodecCtx->width,
                                                   jtVedio->avCodecCtx->height,
                                                   avFrame->data[0],
                                                   avFrame->data[1],
                                                   avFrame->data[2]
                );

            } else {
                AVFrame *avFrameYUV420P = av_frame_alloc();
                int num = av_image_get_buffer_size(
                        AV_PIX_FMT_YUV420P,
                        jtVedio->avCodecCtx->width,
                        jtVedio->avCodecCtx->height,
                        1);

                uint8_t *buffer = (uint8_t *) av_malloc(num * sizeof(uint8_t));

                av_image_fill_arrays(
                        avFrameYUV420P->data,
                        avFrameYUV420P->linesize,
                        buffer,
                        AV_PIX_FMT_YUV420P,
                        jtVedio->avCodecCtx->width,
                        jtVedio->avCodecCtx->height,
                        1
                );


                SwsContext *sws_ctx = sws_getContext(
                        jtVedio->avCodecCtx->width,
                        jtVedio->avCodecCtx->height,
                        jtVedio->avCodecCtx->pix_fmt,
                        jtVedio->avCodecCtx->width,
                        jtVedio->avCodecCtx->height,
                        AV_PIX_FMT_YUV420P,
                        SWS_BICUBIC,
                        NULL,
                        NULL,
                        NULL
                );

                if (!sws_ctx) {
                    av_frame_free(&avFrameYUV420P);
                    av_free(avFrameYUV420P);
                    av_free(buffer);

                    pthread_mutex_unlock(&jtVedio->codecMutex);
                    continue;
                }


                sws_scale(
                        sws_ctx,
                        (const uint8_t *const *) (avFrame->data),
                        avFrame->linesize,
                        0,
                        avFrame->height,
                        avFrameYUV420P->data,
                        avFrameYUV420P->linesize
                );

                //渲染

                double diff = jtVedio->getFrameDiffTime(avFrameYUV420P,NULL);

                av_usleep(jtVedio->getDelayTime(diff) * 1000 * 1000);

                jtVedio->callJava->onCallRenderYUV(CHILD_THREAD, jtVedio->avCodecCtx->width,
                                                   jtVedio->avCodecCtx->height,
                                                   avFrameYUV420P->data[0],
                                                   avFrameYUV420P->data[1],
                                                   avFrameYUV420P->data[2]
                );


                av_frame_free(&avFrameYUV420P);
                av_free(avFrameYUV420P);
                av_free(buffer);
                sws_freeContext(sws_ctx);

            }

            av_frame_free(&avFrame);
            av_free(avFrame);
            avFrame = NULL;

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket = NULL;

            pthread_mutex_unlock(&jtVedio->codecMutex);

        }
    }

   // pthread_exit(&jtVedio->thread_play);

    return 0;
}

void JTVedio::play() {

    if(playerStatus!=NULL&&!playerStatus->exit) {
        pthread_create(&thread_play, NULL, playVedio, this);
    }

}

void JTVedio::release() {

    if(queue!=NULL) {
        queue->noticeQueue();
    }

    pthread_join(thread_play,NULL);


    if(queue!=NULL){
        delete(queue);
        queue=NULL;
    }

    if(absCtx!=NULL){
        av_bsf_free(&absCtx);
        absCtx=NULL;
    }

    if(avCodecCtx!=NULL){
        pthread_mutex_lock(&codecMutex);
        avcodec_close(avCodecCtx);
        avcodec_free_context(&avCodecCtx);
        avCodecCtx=NULL;
        pthread_mutex_unlock(&codecMutex);
    }


    if(playerStatus!=NULL){
        playerStatus=NULL;
    }

    if(callJava!=NULL){
        callJava=NULL;
    }
}

double JTVedio::getFrameDiffTime(AVFrame *avFrame,AVPacket *avPacket) {

    double pts=0;

    if(avFrame!=NULL){
        pts=av_frame_get_best_effort_timestamp(avFrame);
    }


    if(avPacket!=NULL){
        pts=avPacket->pts;
    }

    if(pts==AV_NOPTS_VALUE){
        pts=0;
    }

    pts*= av_q2d(time_base);

    if(pts>0){
         colock=pts;
    }

    double diff=audio->clock-colock;

    return diff;
}

double JTVedio::getDelayTime(double diff) {

    if(diff>0.003){

        delayTime=delayTime*2/3;

        if(delayTime<defaultDelayTime/2){
            delayTime=defaultDelayTime*2/3;
        } else if(delayTime>defaultDelayTime*2){
            delayTime=defaultDelayTime*2;
        }

    } else if(diff<-0.003){

        delayTime=delayTime*3/2;

        if(delayTime<defaultDelayTime/2){
            delayTime=defaultDelayTime*2/3;
        } else if(delayTime>defaultDelayTime*2){
            delayTime=defaultDelayTime*2;
        }

    } else if(diff==0.003){

    }


    if(diff>=0.5){
        delayTime=0;
    } else if(diff<=-0.5){
        delayTime=defaultDelayTime*2;
    }


    if(fabs(diff)>=10){
        delayTime=defaultDelayTime;
    }

    return delayTime;
}
