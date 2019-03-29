package com.yj.mylibrary.player;

import android.media.MediaCodec;
import android.media.MediaCodecInfo;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.view.Surface;

import com.yj.mylibrary.JTTimeInfo;
import com.yj.mylibrary.enums.MuteEnum;
import com.yj.mylibrary.listener.JTOnCompleteListener;
import com.yj.mylibrary.listener.JTOnErrorListener;
import com.yj.mylibrary.listener.JTOnLoadListener;
import com.yj.mylibrary.listener.JTOnParparedListener;
import com.yj.mylibrary.listener.JTOnPauseResumeListener;
import com.yj.mylibrary.listener.JTOnPcmInfoListener;
import com.yj.mylibrary.listener.JTOnRecordTimeListener;
import com.yj.mylibrary.listener.JTOnTimeInfoListener;
import com.yj.mylibrary.listener.JTOnVolumeDBListener;
import com.yj.mylibrary.log.MyLog;
import com.yj.mylibrary.opengl.JTGLSurfaceView;
import com.yj.mylibrary.opengl.JTRender;
import com.yj.mylibrary.utils.JTVedioSupportUtil;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

public class JTPlayer {

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("avcodec-57");
        System.loadLibrary("avdevice-57");
        System.loadLibrary("avfilter-6");
        System.loadLibrary("avformat-57");
        System.loadLibrary("avutil-55");
        System.loadLibrary("postproc-54");
        System.loadLibrary("swresample-2");
        System.loadLibrary("swscale-4");
    }


    private String source;
    private static boolean playNext=false;
    private static int duration=-1;
    private static int volumePercent=100;
    private static boolean initMeidacodec=false;
    private static MuteEnum muteEnum=MuteEnum.MUTE_CENTER;
    private JTOnParparedListener mJTOnParparedListener;
    private JTOnLoadListener mJTOnLoadListener;
    private JTOnPauseResumeListener mJTOnPauseResumeListener;
    private JTOnTimeInfoListener mJTOnTimeInfoListener;
    private JTOnErrorListener mJTOnErrorListener;
    private JTOnCompleteListener mJTOnCompleteListener;
    private JTOnVolumeDBListener mJTOnVolumeDBListener;
    private JTOnRecordTimeListener mJTOnRecordTimeListener;
    private JTOnPcmInfoListener mJTOnPcmInfoListener;
    private static JTTimeInfo jtTimeInfo;
    private JTGLSurfaceView mGLSurfaceView;
    private Surface surface;

    public void setSource(String source) {
        this.source = source;
    }


    public void setJTOnTimeInfoListener(JTOnTimeInfoListener mJTOnTimeInfoListener) {
        this.mJTOnTimeInfoListener = mJTOnTimeInfoListener;
    }

    public void setJTOnLoadListener(JTOnLoadListener listener) {
        this.mJTOnLoadListener = listener;
    }

    public void setJTOnParparedListener(JTOnParparedListener listener) {
        this.mJTOnParparedListener = listener;
    }

    public void setJTOnPauseResumeListener(JTOnPauseResumeListener listener) {
        this.mJTOnPauseResumeListener = listener;
    }

    public void setJTOnErrorListener(JTOnErrorListener mJTOnErrorListener) {
        this.mJTOnErrorListener = mJTOnErrorListener;
    }


    public void setJTOnCompleteListener(JTOnCompleteListener mJTOnCompleteListener) {
        this.mJTOnCompleteListener = mJTOnCompleteListener;
    }


    public void setJTOnVolumeDBListener(JTOnVolumeDBListener mJTOnVolumeDBListener) {
        this.mJTOnVolumeDBListener = mJTOnVolumeDBListener;
    }

    public void setJTOnRecordTimeListener(JTOnRecordTimeListener mJTOnRecordTimeListener) {
        this.mJTOnRecordTimeListener = mJTOnRecordTimeListener;
    }

    public void setJTOnPcmInfoListener(JTOnPcmInfoListener mJTOnPcmInfoListener) {
        this.mJTOnPcmInfoListener = mJTOnPcmInfoListener;
    }


    public void setJTGLSurfaceView(JTGLSurfaceView mGLSurfaceView) {
        this.mGLSurfaceView = mGLSurfaceView;
        mGLSurfaceView.getRender().setOnSurfaceCreateListener(new JTRender.OnSurfaceCreateListener() {
            @Override
            public void onSurfaceCreate(Surface s) {
                surface=s;
                MyLog.d("onSurfaceCreate!");
            }
        });
    }

    //播放准备
    public void parpared(){
          if(TextUtils.isEmpty(source)){
              MyLog.d("source not be empty!");
              return;
          }

          new Thread(new Runnable() {
              @Override
              public void run() {
                   n_parpared(source);
              }
          }).start();

    }

    //开始播放
    public void start(){
          if(TextUtils.isEmpty(source)){
              MyLog.d("source not be empty!");
              return;
          }

          new Thread(new Runnable() {
              @Override
              public void run() {
                  setVolume(volumePercent);
                  setMute(muteEnum);
                  n_start();
              }
          }).start();

    }


    //停止播放
    public void stop(){
        jtTimeInfo=null;
        duration=-1;
        stopRecord();
        new Thread(new Runnable() {
            @Override
            public void run() {
                n_stop();
                releaseMediacodec();
            }
        }).start();
    }


    //暂停播放
    public void pause(){
        n_pause();
        if(mJTOnPauseResumeListener!=null){
            mJTOnPauseResumeListener.onPause(true);
        }
        MyLog.d("暂停播放");
    }

    //播放进度
    public void seek(int secds){
        n_seek(secds);
    }

    //设置音量
    public void setVolume(int percent){
        if(percent>=0&&percent<=100) {
            volumePercent=percent;
            n_volume(percent);
        }
    }

    //播放
    public void resume(){
        n_resume();
        if(mJTOnPauseResumeListener!=null){
            mJTOnPauseResumeListener.onPause(false);
        }

        MyLog.d("开始播放");
    }

    //切换播放曲目
    public void playNext(String url){
        stop();
        source=url;
        playNext=true;
    }

    //设置声道
    public void setMute(MuteEnum mute){
        muteEnum=mute;
        n_mute(mute.getValue());
    }

    //设置声调
    public void setPitch(float pitch){
        n_pitch(pitch);
    }

    //设置声速
    public void setSpeed(float speed){
        n_speed(speed);
    }

    //获取播放时间总长
    public int getDuration(){
        if(duration<0){
            duration=n_duration();
        }
        return duration;
    }

    public int getVolumePercent() {
        return volumePercent;
    }


    //开始录音
    public void startRecord(File outFile){
        if(!initMeidacodec) {
            audioSamplerate=n_samplerate();
            if (audioSamplerate > 0) {
                initMeidacodec=true;
                initMediaCodec(audioSamplerate, outFile);
                n_startstoprecord(true);

            }
        }
    }

    //停止录音
    public void stopRecord(){
        if(initMeidacodec) {
            n_startstoprecord(false);
            releaseMediacodec();
        }
    }

    //暂停录音
    public void pauseRecord(){
        n_startstoprecord(false);
    }

    //继续录音
    public void resumeRecord(){
        n_startstoprecord(true);
    }


    //裁剪音频
    public void cutAudioPlay(int startTime,int endTime,boolean showPcm){
        if( n_cutaudioplay(startTime,endTime,showPcm)){
                start();
        }else {
            stop();
            onCallError(2001,"cut audio param is wrong!");
        }
    }


    public void onCallParpared(){
        if(mJTOnParparedListener!=null){
            mJTOnParparedListener.onParpared();
        }
    }

    public void onCallLoad(boolean load){
        if(mJTOnLoadListener!=null){
            mJTOnLoadListener.onLoad(load);
        }
    }
    public void onCallError(int code,String msg){
        stop();
        if(mJTOnErrorListener!=null){
            mJTOnErrorListener.onError(code,msg);
        }
    }

    public void onCallComplete(){
        stop();
        if(mJTOnCompleteListener!=null){
            mJTOnCompleteListener.onComplete();
        }
    }


    public void onCallTimeInfo(int currentTime,int totalTime){
        if(mJTOnTimeInfoListener!=null){
            if(jtTimeInfo==null){
                jtTimeInfo=new JTTimeInfo();
            }

            jtTimeInfo.setCurrentTime(currentTime);
            jtTimeInfo.setTotalTime(totalTime);
            mJTOnTimeInfoListener.onTimeInfo(jtTimeInfo);
        }
    }



    public void onCallVolumeDB(int db){
        if(mJTOnVolumeDBListener!=null){
            mJTOnVolumeDBListener.onDBValue(db);
        }
    }


    public void onCallNext(){
           if (playNext){
                  playNext=false;
                  parpared();
           }
    }

    public void onCallPcmInfo(byte[]buffer,int buffersize){
        if(mJTOnPcmInfoListener!=null){
            mJTOnPcmInfoListener.onPcmInfo(buffer,buffersize);
        }
    }


    public void onCallRenderYUV(int widht,int height,byte[]y,byte[]u,byte[]v){
          MyLog.d("获取到视频的yuv数据");
          if(mGLSurfaceView!=null){
              mGLSurfaceView.getRender().setRenderType(JTRender.RENDER_YUV);
              mGLSurfaceView.setYUVData(widht,height,y,u,v);
          }
    }


    public boolean onCallIsSuppoerMediaCode(String ffCodecName){
         return JTVedioSupportUtil.isSupportCodec(ffCodecName);
    }



    public void initVedioMediaCodec(String codecname,int width,int height,byte[]csd_0,byte[]csd_1){

        MyLog.d("硬解码初始化开始！");

        if(surface!=null) {
            try {

                mGLSurfaceView.getRender().setRenderType(JTRender.RENDER_MEDIACODEC);

                String mime = JTVedioSupportUtil.findVedioCodecName(codecname);
                encoderFormat = MediaFormat.createVideoFormat(mime, width, height);
                encoderFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                encoderFormat.setByteBuffer("csd_0", ByteBuffer.wrap(csd_0));
                encoderFormat.setByteBuffer("csd_1", ByteBuffer.wrap(csd_1));



                enCodec = MediaCodec.createDecoderByType(mime);


                bufferInfo=new MediaCodec.BufferInfo();
                enCodec.configure(encoderFormat, surface, null, 0);

                MyLog.d("硬解码初始化成功！");

                enCodec.start();

            } catch (Exception e) {
                e.printStackTrace();
            }
        }else {
            if(mJTOnErrorListener!=null){
                mJTOnErrorListener.onError(2002,"surface is null!");
            }
        }
    }


    public void decodeAVpacket(int datasize,byte[]data){

        try {
            MyLog.d("获取到硬解码数据");

            if(surface!=null&&datasize>0&&data!=null){
                int inputBufferIndex=enCodec.dequeueInputBuffer(10);
                if(inputBufferIndex>=0){
                    ByteBuffer byteBuffer=enCodec.getInputBuffers()[inputBufferIndex];
                    byteBuffer.clear();
                    byteBuffer.put(data);
                    enCodec.queueInputBuffer(inputBufferIndex,0,datasize,0,0);
                }

                int outputBufferIndex=enCodec.dequeueOutputBuffer(bufferInfo,10);

                while (outputBufferIndex>=0){
                    enCodec.releaseOutputBuffer(outputBufferIndex,true);
                    outputBufferIndex=enCodec.dequeueOutputBuffer(bufferInfo,10);
                }
            }

        }catch (Exception e){
            e.printStackTrace();
        }
    }

    private native void n_parpared(String source);

    private native void n_start();

    private native void n_pause();

    private native void n_resume();

    private native void n_stop();

    private native void n_seek(int secds);

    private native int n_duration();

    private native void n_volume(int percent);

    private native void n_mute(int mute);

    private native void n_pitch(float pitch);

    private native void n_speed(float speed);

    private native int n_samplerate();

    private native void n_startstoprecord(boolean start);

    private native boolean n_cutaudioplay(int starttime,int endtime,boolean showpcm);


    //mediacodec

    private MediaFormat encoderFormat=null;
    private MediaCodec enCodec=null;
    private FileOutputStream outputStream=null;
    private MediaCodec.BufferInfo bufferInfo=null;
    private int perpcmsize=0;
    private byte[]outByteBuffer=null;
    private int aacSamplerate=4;
    private double recordTime=0;
    private int audioSamplerate=0;



    private void initMediaCodec(int samplerate, File outFile){

        try {

            recordTime=0;

            aacSamplerate=getADTSsamplerate(samplerate);
            encoderFormat=MediaFormat.createAudioFormat(MediaFormat.MIMETYPE_AUDIO_AAC,samplerate,2);
            encoderFormat.setInteger(MediaFormat.KEY_BIT_RATE,96000);
            encoderFormat.setInteger(MediaFormat.KEY_AAC_PROFILE, MediaCodecInfo.CodecProfileLevel.AACObjectLC);
            encoderFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, 4096);

            enCodec=MediaCodec.createEncoderByType(MediaFormat.MIMETYPE_AUDIO_AAC);
            bufferInfo=new MediaCodec.BufferInfo();

            if(enCodec==null){
                MyLog.d("create encodec wrong!");
                return;
            }


            enCodec.configure(encoderFormat,null,null,MediaCodec.CONFIGURE_FLAG_ENCODE);

            outputStream=new FileOutputStream(outFile);

            enCodec.start();

        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    private void encodecPcmToAAC(int size, byte[]buffer){

        if(buffer!=null&&enCodec!=null){
            try{
            recordTime+=size*1.0/audioSamplerate*2*2;
            if(mJTOnRecordTimeListener!=null){
                MyLog.d("录音时间："+recordTime);
                mJTOnRecordTimeListener.onRecordTime((int) recordTime);
            }

            int inputBufferIndex = enCodec.dequeueInputBuffer(0);
            if(inputBufferIndex>=0){
                ByteBuffer byteBuffer=enCodec.getInputBuffers()[inputBufferIndex];
                byteBuffer.clear();
                byteBuffer.put(buffer);
                enCodec.queueInputBuffer(inputBufferIndex,0,size,0,0);
            }

            int index=enCodec.dequeueOutputBuffer(bufferInfo,0);

            while (index>0){

                    perpcmsize=bufferInfo.size+7;
                    outByteBuffer=new byte[perpcmsize];

                    ByteBuffer byteBuffer=enCodec.getOutputBuffers()[index];
                    byteBuffer.position(bufferInfo.offset);
                    byteBuffer.limit(bufferInfo.offset+bufferInfo.size);

                    addADTSHeader(outByteBuffer,perpcmsize,aacSamplerate);

                    byteBuffer.get(outByteBuffer,7,bufferInfo.size);
                    byteBuffer.position(bufferInfo.offset);
                    outputStream.write(outByteBuffer,0,perpcmsize);

                    enCodec.releaseOutputBuffer(index,false);

                    index=enCodec.dequeueOutputBuffer(bufferInfo,0);

                    outByteBuffer=null;

                    MyLog.d("编码。。。");

            }

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }



    private int getADTSsamplerate(int samplerate)
    {
        int rate = 4;
        switch (samplerate)
        {
            case 96000:
                rate = 0;
                break;
            case 88200:
                rate = 1;
                break;
            case 64000:
                rate = 2;
                break;
            case 48000:
                rate = 3;
                break;
            case 44100:
                rate = 4;
                break;
            case 32000:
                rate = 5;
                break;
            case 24000:
                rate = 6;
                break;
            case 22050:
                rate = 7;
                break;
            case 16000:
                rate = 8;
                break;
            case 12000:
                rate = 9;
                break;
            case 11025:
                rate = 10;
                break;
            case 8000:
                rate = 11;
                break;
            case 7350:
                rate = 12;
                break;
        }
        return rate;
    }



    private void addADTSHeader(byte[]packet,int packetLen,int samplerate){
        int profile = 2;  //AAC LC
        //39=MediaCodecInfo.CodecProfileLevel.AACObjectELD;
        int freqIdx = samplerate;  //44.1KHz
        int chanCfg = 2;  //CPE

        // fill in ADTS data
        packet[0] = (byte)0xFF;
        packet[1] = (byte)0xF9;
        packet[2] = (byte)(((profile-1)<<6) + (freqIdx<<2) +(chanCfg>>2));
        packet[3] = (byte)(((chanCfg&3)<<6) + (packetLen>>11));
        packet[4] = (byte)((packetLen&0x7FF) >> 3);
        packet[5] = (byte)(((packetLen&7)<<5) + 0x1F);
        packet[6] = (byte)0xFC;
    }


    private void releaseMediacodec(){
        if(enCodec==null)return;
        try {
            if(outputStream!=null) {
                outputStream.close();
            }
            enCodec.flush();
            enCodec.stop();
            enCodec.release();
            enCodec=null;
            encoderFormat=null;
            bufferInfo=null;
            initMeidacodec=false;
            outputStream= null;

            recordTime=0;

            MyLog.d("录制完成！");

        } catch (Exception e) {
            e.printStackTrace();
        }finally {
            if(outputStream!=null){
                try {
                    outputStream.close();
                } catch (Exception e) {
                    e.printStackTrace();
                }
                outputStream=null;
            }
        }

    }

}
