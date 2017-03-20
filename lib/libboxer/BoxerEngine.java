package org.starlo.boxer;

import java.io.*;

import android.util.*;
import android.widget.*;
import android.graphics.drawable.*;

public class BoxerEngine
{
    static public native void preload(String path);
    static public native void boxerMain();

    static ImageView mScreen = null;
    static public void initialize(ImageView view)
    {
        mScreen = view;
    }

    static public void showStage(String path)
    {
        final long startTime = System.nanoTime();
        try
        {
            File background = new File(path);
            Drawable frame =
                Drawable.createFromStream(new BufferedInputStream(new FileInputStream(background)), null);
            if(mScreen != null) mScreen.setImageDrawable(frame);
        }catch(Exception e)
        {
            Log.v("Boxer", e.toString());
        }
        Log.v("Boxer", "Frame time: "+new Long((System.nanoTime()-startTime)/1000000).toString());
    }

    static{ System.loadLibrary("boxer"); }
}
