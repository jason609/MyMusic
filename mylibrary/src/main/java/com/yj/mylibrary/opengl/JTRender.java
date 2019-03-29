package com.yj.mylibrary.opengl;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;
import android.view.Surface;

import com.yj.mylibrary.R;

import java.nio.Buffer;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class JTRender implements GLSurfaceView.Renderer,SurfaceTexture.OnFrameAvailableListener {

    public static final int RENDER_YUV=1;
    public static final int RENDER_MEDIACODEC=2;

    private Context mContext;
    private int program;
    private int avPosition;
    private int afPosition;

    private int renderType=RENDER_YUV;

    //yuv
    private int sampler_y;
    private int sampler_u;
    private int sampler_v;
    private int[] texture_yuv;
    private int width_yuv;
    private int height_yuv;

    private Buffer y;
    private Buffer u;
    private Buffer v;


    //mediacodec
    private int program_mediacodec;
    private int avPosition_mediacodec;
    private int afPosition_mediacodec;
    private int samplerOES_mediacodec;
    private int textureId_mediacodec;
    private SurfaceTexture surfaceTexture;
    private Surface surface;
    private OnSurfaceCreateListener mOnSurfaceCreateListener;
    private OnRenderListener mOnRenderListener;

    //绘制四边形的两种方式, 顶点坐标
    private final float[]vertexData={
        -1f,-1f,
        1f,-1f,
        -1f,1f,
        1f,1f

    };

    //纹理坐标
    private final float[]textureData={
            0f,1f,
            1f,1f,
            0f,0f,
            1f,0f
    };

    private FloatBuffer verBuffer,textureBuffer;


    public JTRender(Context contxt){
        this.mContext=contxt;
        verBuffer= ByteBuffer.allocateDirect(vertexData.length*4)
                   .order(ByteOrder.nativeOrder())
                   .asFloatBuffer()
                   .put(vertexData);
        verBuffer.position(0);
        textureBuffer= ByteBuffer.allocateDirect(textureData.length*4)
                   .order(ByteOrder.nativeOrder())
                   .asFloatBuffer()
                   .put(textureData);
        textureBuffer.position(0);
    }


    public void setRenderType(int renderType) {
        this.renderType = renderType;
    }

    public void setOnSurfaceCreateListener(OnSurfaceCreateListener mOnSurfaceCreateListener) {
        this.mOnSurfaceCreateListener = mOnSurfaceCreateListener;
    }

    public void setOnRenderListener(OnRenderListener mOnRenderListener) {
        this.mOnRenderListener = mOnRenderListener;
    }

    @Override
    public void onSurfaceCreated(GL10 gl10, EGLConfig eglConfig) {

         initRenderYuv();
         initRenderMediacodec();

    }

    @Override
    public void onSurfaceChanged(GL10 gl10, int width, int height) {
        GLES20.glViewport(0,0,width,height);//设置宽高
    }

    @Override
    public void onDrawFrame(GL10 gl10) {
        GLES20.glClear(GLES20.GL_COLOR_BUFFER_BIT);//清屏
        GLES20.glClearColor(0.0f,0.0f,0.0f,1.0f);

        if(renderType==RENDER_YUV) {
            renderYUV();
        }else if(renderType==RENDER_MEDIACODEC){
            renderMediacodec();
        }
        GLES20.glDrawArrays(GLES20.GL_TRIANGLE_STRIP, 0, 4);
    }




    private void initRenderYuv(){
        String vertexSource=JTShaderUtil.readRawTxt(mContext, R.raw.vertex_shader);
        String fragmentSource=JTShaderUtil.readRawTxt(mContext,R.raw.fragment_yuv);

        program=JTShaderUtil.createProgram(vertexSource,fragmentSource);

        avPosition=GLES20.glGetAttribLocation(program,"av_Position");
        afPosition=GLES20.glGetAttribLocation(program,"af_Position");
        sampler_y=GLES20.glGetUniformLocation(program,"sampler_y");
        sampler_u=GLES20.glGetUniformLocation(program,"sampler_u");
        sampler_v=GLES20.glGetUniformLocation(program,"sampler_v");

        texture_yuv=new int[3];
        GLES20.glGenTextures(3,texture_yuv,0);

        for(int i=0;i<3;i++){
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D,texture_yuv[i]);

            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_WRAP_S,GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_WRAP_T,GLES20.GL_REPEAT);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_MIN_FILTER,GLES20.GL_LINEAR);
            GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_MAG_FILTER,GLES20.GL_LINEAR);
        }


    }

    public void setYUVRenderData(int widht,int height,byte[]y,byte[]u,byte[]v){
        this.width_yuv=widht;
        this.height_yuv=height;
        this.y=ByteBuffer.wrap(y);
        this.u=ByteBuffer.wrap(u);
        this.v=ByteBuffer.wrap(v);
    }


    private void renderYUV(){

        if(width_yuv>0&&height_yuv>0&&y!=null&&u!=null&&v!=null) {

            GLES20.glUseProgram(program);

            GLES20.glEnableVertexAttribArray(avPosition);
            GLES20.glVertexAttribPointer(avPosition, 2, GLES20.GL_FLOAT, false, 8, verBuffer);

            GLES20.glEnableVertexAttribArray(afPosition);
            GLES20.glVertexAttribPointer(afPosition, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture_yuv[0]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv, height_yuv, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, y);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE1);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture_yuv[1]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, u);

            GLES20.glActiveTexture(GLES20.GL_TEXTURE2);
            GLES20.glBindTexture(GLES20.GL_TEXTURE_2D, texture_yuv[2]);
            GLES20.glTexImage2D(GLES20.GL_TEXTURE_2D, 0, GLES20.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES20.GL_LUMINANCE, GLES20.GL_UNSIGNED_BYTE, v);


            GLES20.glUniform1i(sampler_y, 0);
            GLES20.glUniform1i(sampler_u, 1);
            GLES20.glUniform1i(sampler_v, 2);

            y.clear();
            u.clear();
            v.clear();

            y = null;
            u = null;
            v = null;

        }
    }


    private void initRenderMediacodec(){
        String vertexSource=JTShaderUtil.readRawTxt(mContext, R.raw.vertex_shader);
        String fragmentSource=JTShaderUtil.readRawTxt(mContext,R.raw.fragment_mediacodec);

        program_mediacodec=JTShaderUtil.createProgram(vertexSource,fragmentSource);

        avPosition_mediacodec=GLES20.glGetAttribLocation(program_mediacodec,"av_Position");
        afPosition_mediacodec=GLES20.glGetAttribLocation(program_mediacodec,"af_Position");
        samplerOES_mediacodec=GLES20.glGetUniformLocation(program_mediacodec,"sTexture");

        int[] textureIds=new int[1];

        GLES20.glGenTextures(1,textureIds,0);

        textureId_mediacodec=textureIds[0];

        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_WRAP_S,GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_WRAP_T,GLES20.GL_REPEAT);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_MIN_FILTER,GLES20.GL_LINEAR);
        GLES20.glTexParameteri(GLES20.GL_TEXTURE_2D,GLES20.GL_TEXTURE_MAG_FILTER,GLES20.GL_LINEAR);

        surfaceTexture=new SurfaceTexture(textureId_mediacodec);
        surface=new Surface(surfaceTexture);

        surfaceTexture.setOnFrameAvailableListener(this);

        if(mOnSurfaceCreateListener!=null){
            mOnSurfaceCreateListener.onSurfaceCreate(surface);
        }

    }

    private void renderMediacodec() {
        surfaceTexture.updateTexImage();
        GLES20.glUseProgram(program_mediacodec);

        GLES20.glEnableVertexAttribArray(avPosition_mediacodec);
        GLES20.glVertexAttribPointer(avPosition_mediacodec, 2, GLES20.GL_FLOAT, false, 8, verBuffer);

        GLES20.glEnableVertexAttribArray(afPosition_mediacodec);
        GLES20.glVertexAttribPointer(afPosition_mediacodec, 2, GLES20.GL_FLOAT, false, 8, textureBuffer);

        GLES20.glActiveTexture(GLES20.GL_TEXTURE0);
        GLES20.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId_mediacodec);
        GLES20.glUniform1i(samplerOES_mediacodec,0);
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        if(mOnRenderListener!=null){
            mOnRenderListener.onRender();
        }
    }


    public interface OnSurfaceCreateListener{
        void onSurfaceCreate(Surface surface);
    }

    public interface OnRenderListener{
        void onRender();
    }
}
