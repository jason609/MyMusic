package com.yj.mymusic;

import android.Manifest;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.widget.Button;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.tbruyelle.rxpermissions2.RxPermissions;
import com.yj.mylibrary.JTTimeInfo;
import com.yj.mylibrary.enums.MuteEnum;
import com.yj.mylibrary.listener.JTOnCompleteListener;
import com.yj.mylibrary.listener.JTOnErrorListener;
import com.yj.mylibrary.listener.JTOnLoadListener;
import com.yj.mylibrary.listener.JTOnParparedListener;
import com.yj.mylibrary.listener.JTOnPauseResumeListener;
import com.yj.mylibrary.listener.JTOnTimeInfoListener;
import com.yj.mylibrary.listener.JTOnVolumeDBListener;
import com.yj.mylibrary.log.MyLog;
import com.yj.mylibrary.player.JTPlayer;
import com.yj.mylibrary.utils.DateTimeUtil;

import java.io.File;

import io.reactivex.functions.Consumer;

public class MainActivity extends AppCompatActivity {

    private JTPlayer mPlayer;
    private TextView tv_time,tvVolume;
    private SeekBar seekBar,seekVolume;
    private int position=0;
    private boolean isSeek=false;

    private Handler mHandler=new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what){
                case 111:
                    if(!isSeek) {
                        JTTimeInfo info = (JTTimeInfo) msg.obj;
                        tv_time.setText(DateTimeUtil.formatTimeForMMss(info.getCurrentTime()) + "/" + DateTimeUtil.formatTimeForMMss(info.getTotalTime()));
                        seekBar.setProgress(info.getCurrentTime()*100/info.getTotalTime());
                    }
                    break;
            }
        }
    };


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        RxPermissions rxPermissions = new RxPermissions(this);

        rxPermissions
                .request(Manifest.permission.WRITE_EXTERNAL_STORAGE,Manifest.permission.RECORD_AUDIO)
                .subscribe(new Consumer<Boolean>() {
                    @Override
                    public void accept(Boolean aBoolean) throws Exception {
                        if(aBoolean){

                        }else {
                            Toast.makeText(MainActivity.this,"没有读写权限！",Toast.LENGTH_SHORT).show();
                            finish();
                        }
                    }
                });

        // Example of a call to a native method
        Button tv = (Button) findViewById(R.id.sample_text);
        tv_time = (TextView) findViewById(R.id.tv_time);
        tvVolume = (TextView) findViewById(R.id.tv_volume);
        seekBar=(SeekBar)findViewById(R.id.seek_bar);
        seekVolume=(SeekBar)findViewById(R.id.seek_volume);

        mPlayer=new JTPlayer();



        mPlayer.setVolume(50);
        mPlayer.setMute(MuteEnum.MUTE_LEFT);




        mPlayer.setJTOnParparedListener(new JTOnParparedListener() {
            @Override
            public void onParpared() {
                MyLog.d("准备好了，开始解码...");
                mPlayer.start();
               // mPlayer.cutAudioPlay(20,40,true);
            }
        });

        mPlayer.setJTOnLoadListener(new JTOnLoadListener() {
            @Override
            public void onLoad(boolean load) {
                if(load){
                    MyLog.d("加载中...");
                }else {
                    MyLog.d("播放中...");
                }
            }
        });

        mPlayer.setJTOnPauseResumeListener(new JTOnPauseResumeListener() {
            @Override
            public void onPause(boolean pause) {
                if(pause){
                    MyLog.d("暂停...");
                }else {
                    MyLog.d("播放...");
                }
            }
        });

        mPlayer.setJTOnTimeInfoListener(new JTOnTimeInfoListener() {
            @Override
            public void onTimeInfo(JTTimeInfo info) {
                MyLog.d("时间总长..."+info.getTotalTime()+"当前播放时间..."+info.getCurrentTime());
                Message msg=Message.obtain();
                msg.obj=info;
                msg.what=111;
                mHandler.sendMessage(msg);
            }
        });

        mPlayer.setJTOnErrorListener(new JTOnErrorListener() {
            @Override
            public void onError(int code, String msg) {
                MyLog.d("错误码："+code +"错误提示："+msg);
            }
        });

        mPlayer.setJTOnCompleteListener(new JTOnCompleteListener() {
            @Override
            public void onComplete() {
                MyLog.d("播放完成！");
            }
        });

        seekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
                   if(mPlayer.getDuration()>0&&isSeek) {
                       position = mPlayer.getDuration() * progress / 100;
                   }

            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                   isSeek=true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                   mPlayer.seek(position);
                   isSeek=false;
            }
        });

        seekVolume.setProgress(mPlayer.getVolumePercent());
        tvVolume.setText("音量："+mPlayer.getVolumePercent()+"%");

        seekVolume.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean b) {
                   mPlayer.setVolume(progress);
                   tvVolume.setText("音量："+progress+"%");
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
            }
        });


        mPlayer.setJTOnVolumeDBListener(new JTOnVolumeDBListener() {
            @Override
            public void onDBValue(int db) {
              //  MyLog.d("当前分贝："+db);
            }
        });
    }


    public void begin(View view){
        mPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
      //  String path=Environment.getExternalStorageDirectory()+File.separator+"张韶涵+传世之爱.ape";
      /*  String path=Environment.getExternalStorageDirectory()+File.separator+"[阳光电影www.ygdy8.net]寻夢環游记.HD.720p.国英双语中字.mkv";
        mPlayer.setSource(path);*/
        mPlayer.parpared();

    }
    public void pause(View view){
        mPlayer.pause();
    }
    public void play(View view){
        mPlayer.resume();
    }

    public void stop(View view){
        mPlayer.stop();
    }

    public void seek(View view){
        mPlayer.seek(214);
    }

    public void next(View view){
        mPlayer.playNext("http://ngcdn004.cnr.cn/live/dszs/index.m3u8");
    }

    public void left(View view){
        mPlayer.setMute(MuteEnum.MUTE_LEFT);
    }
    public void right(View view){
        mPlayer.setMute(MuteEnum.MUTE_RIGHT);
    }
    public void stereo(View view){
        mPlayer.setMute(MuteEnum.MUTE_CENTER);
    }


    public void speed(View view) {
        mPlayer.setSpeed(1.5f);
        mPlayer.setPitch(1.0f);
    }


    public void speedpitch(View view) {
        mPlayer.setSpeed(1.5f);
        mPlayer.setPitch(1.5f);
    }

    public void pitch(View view) {
        mPlayer.setSpeed(1.0f);
        mPlayer.setPitch(1.5f);
    }

    public void startRec(View view) {
        mPlayer.startRecord(new File(Environment.getExternalStorageDirectory()+File.separator+"testPlayer.aac"));
    }

    public void pauseRec(View view) {
        mPlayer.pauseRecord();
    }

    public void resumeRec(View view) {
        mPlayer.resumeRecord();
    }

    public void stopRec(View view) {
        mPlayer.stopRecord();
    }

    public void cutaudio(View view) {
        mPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        ///  String path=Environment.getExternalStorageDirectory()+File.separator+"张韶涵+传世之爱.ape";
        //  mPlayer.setSource(path);
        mPlayer.parpared();
    }

    public void opengl(View view) {
        startActivity(new Intent(this,OpenGLDemoActivity.class));
    }
}
