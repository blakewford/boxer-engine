package org.starlo.boxer;

import java.io.*;

import android.util.*;
import android.media.*;
import android.widget.*;
import android.content.*;
import android.graphics.*;
import android.graphics.drawable.*;

public class BoxerEngine
{
    public enum Control
    {
        UNKNOWN,
        UP,
        LEFT,
        DOWN,
        RIGHT,
        AUX1
    }

    static public native void preload(String path);
    static public native void boxerMain();
    static public native void audioResourceThread();
    static public native void triggerControl(int control);

    static ImageView mScreen = null;
    static AudioTrack mPlayer = null;
    static public void initialize(ImageView view)
    {
        mScreen = view;
        AudioAttributes attributes = new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_GAME)
                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                .build();
        AudioFormat format = new AudioFormat.Builder()
                .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                .setSampleRate(44100)
                .setChannelMask(AudioFormat.CHANNEL_OUT_STEREO)
                .build();
        int id = ((AudioManager)mScreen.getContext().getSystemService(Context.AUDIO_SERVICE)).generateAudioSessionId();
        mPlayer = new AudioTrack(attributes, format, 1024, AudioTrack.MODE_STREAM, id);
        mPlayer.play();
    }

    static public void showStage(final byte[] data)
    {
        final long startTime = System.nanoTime();
        mScreen.post(new Runnable()
        {
            public void run()
            {
                try
                {
                    Drawable frame = new BitmapDrawable(mScreen.getResources(), BitmapFactory.decodeByteArray(data, 0, data.length));
                    if(mScreen != null)
                    {
                        mScreen.setImageDrawable(frame);
                    }
                }catch(Exception e)
                {
                    Log.v("Boxer", e.toString());
                }
            }
        });
        //Log.v("Boxer", "Frame time: "+new Long((System.nanoTime()-startTime)/1000000).toString());
    }

    static public void audioWrite(short[] data)
    {
        mPlayer.write(data, 0, data.length);
    }

    static public void triggerControl(Control control)
    {
        triggerControl(control.ordinal());
    }

    static{ System.loadLibrary("boxer"); }
}
