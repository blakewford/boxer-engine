#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>

#include "boxer.h"
#include "boxer_internal.h"

#include <cairo.h>
#include <gtk/gtk.h>

#include <pulse/simple.h>

namespace boxer
{

int32_t getDefaultFrameDelay()
{
    return 1000;
}

struct cairoDisplay
{
  int32_t width;
  int32_t height;
  int32_t stride;
  unsigned char* data;
  int32_t bytesPerPixel;
  cairo_surface_t* surface;
};

uint8_t* gData = NULL;
cairoDisplay gDisplay;
GtkWidget* gWindow = NULL;

gboolean Draw(GtkWidget* widget, cairo_t* cairo, gpointer)
{
    cairo_surface_flush (gDisplay.surface);

    if(gData != NULL)
    {
        int l = gDisplay.height;
        assert(gDisplay.bytesPerPixel == 4);
        const boxer::bbfPixel* data = (const boxer::bbfPixel*)gData;
        for(int i = 0; i < gDisplay.height; i++)
        {
            l--;
            int k = 0;
            for(int j = 0; j < gDisplay.stride; j+=gDisplay.bytesPerPixel)
            {
                const int32_t cairoOffset = i*gDisplay.stride;
                const int32_t boxerOffset = (l*gDisplay.width) + k;
                gDisplay.data[cairoOffset + j +0] = data[boxerOffset].d.R*8;
                gDisplay.data[cairoOffset + j +1] = data[boxerOffset].d.G*4;
                gDisplay.data[cairoOffset + j +2] = data[boxerOffset].d.B*8;
                gDisplay.data[cairoOffset + j +3] = 0xFF;
                k++;
            }
        }
        cairo_surface_mark_dirty (gDisplay.surface);
        cairo_set_source_surface(cairo, gDisplay.surface, 0, 0);
        cairo_paint(cairo);
    }

    return false;
}

void quitGTK()
{
    gtk_main_quit();
    gKeepGoing = false;
}

void* runGTK(void* param)
{
    gDisplay.surface = cairo_image_surface_create_from_png("debug.png");
    gDisplay.data    = cairo_image_surface_get_data(gDisplay.surface);
    gDisplay.width   = cairo_image_surface_get_width(gDisplay.surface);
    gDisplay.height  = cairo_image_surface_get_height(gDisplay.surface);
    gDisplay.stride  = cairo_image_surface_get_stride(gDisplay.surface);

    gDisplay.bytesPerPixel = gDisplay.stride / gDisplay.width;

    GtkWidget* drawingArea;

    gtk_init(NULL, NULL);

    gWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    drawingArea = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(gWindow), drawingArea);

    g_signal_connect(G_OBJECT(drawingArea), "draw", G_CALLBACK(Draw), NULL);
    g_signal_connect(gWindow, "destroy", G_CALLBACK (quitGTK), NULL);

    gtk_window_set_position(GTK_WINDOW(gWindow), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(gWindow), gDisplay.width, gDisplay.height);
    gtk_window_set_title(GTK_WINDOW(gWindow), ""); // TODO: Add game title to api

    gtk_widget_show_all(gWindow);

    gtk_main();

    cairo_surface_destroy(gDisplay.surface);

    return NULL;
}

void initializeDisplay()
{
    pthread_t gtkThread;
    pthread_create(&gtkThread, NULL, runGTK, NULL);
}

const char* getDebugImagePath()
{
    return "";
}

void writeDisplay(uint8_t* data)
{
    gData = data;
    gtk_widget_queue_draw(gWindow);
}

void writeAudioResource(audioParam* param)
{
    const boxer::wavStat* stat = (const wavStat*)getResource(param->id);
    const uint8_t* data = (const uint8_t*)(getResource(param->id) + WAV_HEADER_SIZE);
    static const pa_sample_spec spec = { .format = PA_SAMPLE_S16LE, .rate = 44100, .channels = 2 };
    pa_simple* stream = pa_simple_new(NULL, NULL, PA_STREAM_PLAYBACK, NULL, "boxer_track", &spec, NULL, NULL, NULL);
    if(stream != NULL)
    {
        int32_t i = 0;
        while((i+AUDIO_BUFFER_SIZE < stat->size) && param->keepGoing)
        {
            if(pa_simple_write(stream, &data[i], AUDIO_BUFFER_SIZE, NULL) < 0)
            {
                break;
            }
            i+=AUDIO_BUFFER_SIZE;
        }
        pa_simple_drain(stream, NULL);
        pa_simple_free(stream);
    }
}

void shutdownAudio(int32_t id)
{
}

control getControlInput()
{
    char c = getch();
    control mapped = UNKNOWN;
    switch(c)
    {
        case 'w':
        case 'W':
            mapped = UP;
            break;
        case 'a':
        case 'A':
            mapped = LEFT;
            break;
        case 's':
        case 'S':
            mapped = DOWN;
            break;
        case 'd':
        case 'D':
            mapped = RIGHT;
            break;
        case ' ':
            mapped = AUX1;
            break;
    }

    return mapped;
}

void initializeInput()
{
    WINDOW* w = initscr();
    noecho();
    scrollok(w, true);
}

void shutdownInput()
{
    endwin();
}

}
