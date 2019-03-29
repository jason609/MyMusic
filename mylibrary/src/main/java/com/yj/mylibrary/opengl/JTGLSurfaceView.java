package com.yj.mylibrary.opengl;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

public class JTGLSurfaceView extends GLSurfaceView{

    private JTRender render;

    public JTGLSurfaceView(Context context) {
        this(context,null);
    }

    public JTGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(2);//设置版本
        render=new JTRender(context);
        setRenderer(render);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);

        render.setOnRenderListener(new JTRender.OnRenderListener() {
            @Override
            public void onRender() {
                requestRender();
            }
        });

    }


    public void setYUVData(int widht,int height,byte[]y,byte[]u,byte[]v){
        if(render!=null) {
            render.setYUVRenderData(widht, height, y, u, v);
            requestRender();
        }
    }

    public JTRender getRender() {
        return render;
    }
}
