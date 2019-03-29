//
// Created by ASUS on 2019/3/13.
//

#ifndef MYMUSIC_JTPLAYERSTATUS_H
#define MYMUSIC_JTPLAYERSTATUS_H


class JTPlayerStatus {

public:

    bool exit;
    bool load= true;
    bool seek= false;
    bool pause= false;

public:

    JTPlayerStatus();
    ~JTPlayerStatus();

};


#endif //MYMUSIC_JTPLAYERSTATUS_H
