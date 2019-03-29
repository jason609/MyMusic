package com.yj.mymusic;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.support.annotation.Nullable;
import android.support.v7.app.AppCompatActivity;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.animation.TranslateAnimation;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;
import android.widget.Toast;

import com.yj.mylibrary.JTTimeInfo;
import com.yj.mylibrary.listener.JTOnParparedListener;
import com.yj.mylibrary.listener.JTOnTimeInfoListener;
import com.yj.mylibrary.log.MyLog;
import com.yj.mylibrary.opengl.JTGLSurfaceView;
import com.yj.mylibrary.player.JTPlayer;
import com.yj.mylibrary.utils.DateTimeUtil;

import java.io.File;

public class OpenGLDemoActivity extends AppCompatActivity{

    private JTGLSurfaceView gl_surface;
    private JTPlayer mPlayer;
    private SeekBar seek_video;
    private int position;
    private boolean isSeek;
    private TextView tv_time;

    private LinearLayout ll_bottom;


    private Handler mHandler=new Handler(){
        @Override
        public void handleMessage(Message msg) {
            super.handleMessage(msg);
            switch (msg.what){
                case 111:
                    if(!isSeek) {
                        JTTimeInfo info = (JTTimeInfo) msg.obj;
                        tv_time.setText(DateTimeUtil.formatTimeForMMss(info.getCurrentTime()) + "/" + DateTimeUtil.formatTimeForMMss(info.getTotalTime()));
                        seek_video.setProgress(info.getCurrentTime()*100/info.getTotalTime());
                    }

                    if(System.currentTimeMillis()-time>10*1000&&ll_bottom.getVisibility()==View.VISIBLE){
                        Toast.makeText(OpenGLDemoActivity.this,"执行缩回动画",Toast.LENGTH_SHORT).show();
                        TranslateAnimation animation=new TranslateAnimation(
                                TranslateAnimation.RELATIVE_TO_SELF, 0, TranslateAnimation.RELATIVE_TO_SELF, 0,
                                TranslateAnimation.RELATIVE_TO_SELF, 0, TranslateAnimation.RELATIVE_TO_SELF, 1
                        );

                        animation.setDuration(500);
                        ll_bottom.setAnimation(animation);
                        ll_bottom.setVisibility(View.GONE);
                    }

                    break;
            }
        }
    };

    private long time;

    @Override
    protected void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Window window = getWindow();
        //隐藏标题栏
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        //隐藏状态栏
        //定义全屏参数
        int flag= WindowManager.LayoutParams.FLAG_FULLSCREEN;
        //设置当前窗体为全屏显示
        window.setFlags(flag, flag);


        setContentView(R.layout.activity_opengl_demo);
        mPlayer=new JTPlayer();
        gl_surface=(JTGLSurfaceView)findViewById(R.id.gl_surface);
        seek_video=findViewById(R.id.seek_video);
        tv_time = (TextView) findViewById(R.id.tv_time);
        ll_bottom = (LinearLayout) findViewById(R.id.ll_bottom);

        mPlayer.setJTGLSurfaceView(gl_surface);



        mPlayer.setJTOnParparedListener(new JTOnParparedListener() {
            @Override
            public void onParpared() {
                MyLog.d("准备好了，开始解码...");
                mPlayer.start();
                // mPlayer.cutAudioPlay(20,40,true);
            }
        });


        gl_surface.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {

                if(ll_bottom.getVisibility()==View.GONE) {

                    time = System.currentTimeMillis();

                    TranslateAnimation animation = new TranslateAnimation(
                            TranslateAnimation.RELATIVE_TO_SELF, 0, TranslateAnimation.RELATIVE_TO_SELF, 0,
                            TranslateAnimation.RELATIVE_TO_SELF, 1, TranslateAnimation.RELATIVE_TO_SELF, 0
                    );

                    animation.setDuration(500);
                    ll_bottom.setAnimation(animation);
                    ll_bottom.setVisibility(View.VISIBLE);
                }else {
                    TranslateAnimation animation=new TranslateAnimation(
                            TranslateAnimation.RELATIVE_TO_SELF, 0, TranslateAnimation.RELATIVE_TO_SELF, 0,
                            TranslateAnimation.RELATIVE_TO_SELF, 0, TranslateAnimation.RELATIVE_TO_SELF, 1
                    );

                    animation.setDuration(500);
                    ll_bottom.setAnimation(animation);
                    ll_bottom.setVisibility(View.GONE);
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


        seek_video.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
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

    }


    /**
     * 重写Activity的生命周期
     */
    @Override
    protected void onPause() {
        super.onPause();
        if (mPlayer!=null){
            mPlayer.pause();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if (mPlayer!=null){
            mPlayer.resume();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mPlayer!=null){
            mPlayer.stop();
        }
    }



    public void begin(View view){
        // mPlayer.setSource("http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3");
        //  String path=Environment.getExternalStorageDirectory()+File.separator+"张韶涵+传世之爱.ape";
       // String path= Environment.getExternalStorageDirectory()+ File.separator+"[阳光电影www.ygdy8.net]寻夢環游记.HD.720p.国英双语中字.mkv";
         String path= Environment.getExternalStorageDirectory()+ File.separator+"test.mp4";
        mPlayer.setSource(path);
       // mPlayer.setSource("rtmp://123.232.118.118:1935/live/T143985769_1_0?token=Hf5QetTQ1Wq4FYBH0Z0xiNOZ2Z2qZ0JmbT5auZ0JpGc25A01rogmxfBjriqQYSNFthi16mfiJ8O6J7VPyK3G4oOZ1mu3GLZ1oZ0vndZ0pibdWcesxVBcZ3");
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

    public void next(View view){

        mPlayer.playNext(Environment.getExternalStorageDirectory()+ File.separator+"[阳光电影www.ygdy8.net]寻夢環游记.HD.720p.国英双语中字.mkv");
    }

}
