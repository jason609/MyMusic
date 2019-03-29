//
// Created by ASUS on 2019/3/13.
//

#include "JTQueue.h"

JTQueue::JTQueue(JTPlayerStatus *status) {

    this->jtPlayerStatus=status;
    pthread_mutex_init(&mutexPkt,NULL);
    pthread_cond_init(&condPkt,NULL);
}

JTQueue::~JTQueue() {
    clearAvpacket();
    pthread_mutex_destroy(&mutexPkt);
    pthread_cond_destroy(&condPkt);
}

int JTQueue::putAVPkt(AVPacket *avPacket) {
    pthread_mutex_lock(&mutexPkt);

    queueAVpkt.push(avPacket);

    if(LOG_DEBUG){
       // LOGD("放入一个AVPacket,数量为  %d",queueAVpkt.size());
    }

    pthread_cond_signal(&condPkt);

    pthread_mutex_unlock(&mutexPkt);
    return 0;
}

int JTQueue::getAVPkt(AVPacket *packet) {
    pthread_mutex_lock(&mutexPkt);

    while (jtPlayerStatus!=NULL&&!jtPlayerStatus->exit){
        if(queueAVpkt.size()>0){

            AVPacket *avPacket=queueAVpkt.front();

            if(av_packet_ref(packet,avPacket)==0){
                queueAVpkt.pop();
            }

            av_packet_free(&avPacket);
            av_free(avPacket);
            avPacket=NULL;

            if(LOG_DEBUG){
              //  LOGD("从队列里面取出一个AVPacket,还剩下 %d 个",queueAVpkt.size());
            }

            break;

        } else{
            pthread_cond_wait(&condPkt,&mutexPkt);
        }
    }


    pthread_mutex_unlock(&mutexPkt);
    return 0;
}

int JTQueue::getQueueSize() {
    int size=0;
    pthread_mutex_lock(&mutexPkt);
    size=queueAVpkt.size();
    pthread_mutex_unlock(&mutexPkt);
    return size;
}


void JTQueue::clearAvpacket() {
    pthread_cond_signal(&condPkt);
    pthread_mutex_lock(&mutexPkt);

    while (!queueAVpkt.empty()){
        AVPacket *avPacket=queueAVpkt.front();
        queueAVpkt.pop();
        av_packet_free(&avPacket);
        av_free(avPacket);
        avPacket=NULL;
    }

    pthread_mutex_unlock(&mutexPkt);
}

void JTQueue::noticeQueue() {
    pthread_cond_signal(&condPkt);
}
