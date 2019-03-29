package com.yj.mylibrary.utils;


import android.media.MediaCodecList;

import com.yj.mylibrary.log.MyLog;

import java.util.HashMap;
import java.util.Map;

public class JTVedioSupportUtil {

    private static Map<String,String> codecMap=new HashMap<>();

    static {
        codecMap.put("h264","video/avc");
        codecMap.put("rv40","video/vnd.rn-realvideo");
    }

    public static String findVedioCodecName(String ffCodeName){
        if(codecMap.containsKey(ffCodeName)){
            return codecMap.get(ffCodeName);
        }
        return "";
    }

    public static boolean isSupportCodec(String ffCodeName){
        boolean isSupport=true;

        int count = MediaCodecList.getCodecCount();

        for(int i=0;i<count;i++){


            String[] types=MediaCodecList.getCodecInfoAt(i).getSupportedTypes();

            for(int j=0;j<types.length;j++){

                MyLog.d("支持的解码格式："+types[j]);

                if(types[j].equals(findVedioCodecName(ffCodeName))){
                    isSupport=true;
                    break;
                }
            }

            if(isSupport){
                break;
            }

        }

        return isSupport;
    }

}
