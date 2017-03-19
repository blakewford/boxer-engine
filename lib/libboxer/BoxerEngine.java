package org.starlo.boxer;

public class BoxerEngine
{
    static public native preload(String path);

    static{ System.loadLibrary("boxer"); }
}
