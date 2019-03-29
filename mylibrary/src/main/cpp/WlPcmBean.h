//
// Created by yangw on 2018-4-1.
//

#ifndef WLMUSIC_PCMBEAN_H
#define WLMUSIC_PCMBEAN_H

#include <SoundTouch.h>

using namespace soundtouch;

class WlPcmBean {

public:
    char *buffer;
    int buffsize;

public:
    WlPcmBean(SAMPLETYPE *buffer, int size);
    ~WlPcmBean();


};


#endif //WLMUSIC_PCMBEAN_H
