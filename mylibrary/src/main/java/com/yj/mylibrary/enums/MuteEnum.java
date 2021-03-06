package com.yj.mylibrary.enums;

public enum  MuteEnum {

    MUTE_LEFT("LEFT",1),
    MUTE_RIGHT("RIGHT",0),
    MUTE_CENTER("CENTER",2);

    private String name;
    private int value;

    MuteEnum(String name, int value) {
        this.name = name;
        this.value = value;
    }

    public int getValue() {
        return value;
    }
}
