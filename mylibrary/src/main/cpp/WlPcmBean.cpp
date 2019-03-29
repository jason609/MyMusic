//
// Created by yangw on 2018-4-1.
//

#include "WlPcmBean.h"

WlPcmBean::WlPcmBean(SAMPLETYPE *buffer, int size) {

    this->buffer = (char *) malloc(size);
    this->buffsize = size;
    memcpy(this->buffer, buffer, size);

}

WlPcmBean::~WlPcmBean() {
    free(buffer);
    buffer = NULL;
}
