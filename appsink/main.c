#include <gst/gst.h>
#ifdef HAVE_GTK
#include <gtk/gtk.h>
#endif

#include <stdlib.h>

#define CAPS "video/x-raw,format=RGB,width=160,pixel-aspect-ratio=1/1"

int
main (int argc, char *argv[])
{
  GstElement *pipeline, *sink;
  gint width, height;
  GstSample *sample;
  gchar *descr;
  GError *error = NULL;
  gint64 duration, position;
  GstStateChangeReturn ret;
  gboolean res;
  GstMapInfo map;

  gst_init (&argc, &argv);

  if (argc != 2) {
    g_print ("usage: %s <uri>\n Writes snapshot.png in the current directory\n",
        argv[0]);
    exit (-1);
  }

  /* create a new pipeline */
  descr =
      g_strdup_printf ("uridecodebin uri=%s ! videoconvert ! videoscale ! "
      " appsink name=sink caps=\"" CAPS "\"", argv[1]);
  pipeline = gst_parse_launch (descr, &error);

  if (error != NULL) {
    g_print ("could not construct pipeline: %s\n", error->message);
    g_error_free (error);
    exit (-1);
  }

  /* get sink */
  sink = gst_bin_get_by_name (GST_BIN (pipeline), "sink");

  /* set to PAUSED to make the first frame arrive in the sink */
  ret = gst_element_set_state (pipeline, GST_STATE_PAUSED);
  switch (ret) {
    case GST_STATE_CHANGE_FAILURE:
      g_print ("failed to play the file\n");
      exit (-1);
    case GST_STATE_CHANGE_NO_PREROLL:
      /* for live sources, we need to set the pipeline to PLAYING before we can
       * receive a buffer. We don't do that yet */
      g_print ("live sources not supported yet\n");
      exit (-1);
    default:
      break;
  }
  /* This can block for up to 5 seconds. If your machine is really overloaded,
   * it might time out before the pipeline prerolled and we generate an error. A
   * better way is to run a mainloop and catch errors there. */
  ret = gst_element_get_state (pipeline, NULL, NULL, 5 * GST_SECOND);
  if (ret == GST_STATE_CHANGE_FAILURE) {
    g_print ("failed to play the file\n");
    exit (-1);
  }

  /* get the duration */
  gst_element_query_duration (pipeline, GST_FORMAT_TIME, &duration);

  if (duration != -1)
    /* we have a duration, seek to 5% */
    position = duration * 5 / 100;
  else
    /* no duration, seek to 1 second, this could EOS */
    position = 1 * GST_SECOND;

  /* seek to the a position in the file. Most files have a black first frame so
   * by seeking to somewhere else we have a bigger chance of getting something
   * more interesting. An optimisation would be to detect black images and then
   * seek a little more */
  gst_element_seek_simple (pipeline, GST_FORMAT_TIME,
      GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_FLUSH, position);

  /* get the preroll buffer from appsink, this block untils appsink really
   * prerolls */
  g_signal_emit_by_name (sink, "pull-preroll", &sample, NULL);

  /* if we have a buffer now, convert it to a pixbuf. It's possible that we
   * don't have a buffer because we went EOS right away or had an error. */
  if (sample) {
    GstBuffer *buffer;
    GstCaps *caps;
    GstStructure *s;

    /* get the snapshot buffer format now. We set the caps on the appsink so
     * that it can only be an rgb buffer. The only thing we have not specified
     * on the caps is the height, which is dependant on the pixel-aspect-ratio
     * of the source material */
    caps = gst_sample_get_caps (sample);
    if (!caps) {
      g_print ("could not get snapshot format\n");
      exit (-1);
    }
    s = gst_caps_get_structure (caps, 0);

    /* we need to get the final caps on the buffer to get the size */
    res = gst_structure_get_int (s, "width", &width);
    res |= gst_structure_get_int (s, "height", &height);
    if (!res) {
      g_print ("could not get snapshot dimension\n");
      exit (-1);
    }

    /* create pixmap from buffer and save, gstreamer video buffers have a stride
     * that is rounded up to the nearest multiple of 4 */
    buffer = gst_sample_get_buffer (sample);
    gst_buffer_map (buffer, &map, GST_MAP_READ);
#ifdef HAVE_GTK
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_data (map.data,
        GDK_COLORSPACE_RGB, FALSE, 8, width, height,
        GST_ROUND_UP_4 (width * 3), NULL, NULL);

    /* save the pixbuf */
    gdk_pixbuf_save (pixbuf, "snapshot.png", "png", &error, NULL);
#endif
    gst_buffer_unmap (buffer, &map);
    gst_sample_unref (sample);
  } else {
    g_print ("could not make snapshot\n");
  }

  /* cleanup and exit */
  gst_element_set_state (pipeline, GST_STATE_NULL);
  gst_object_unref (pipeline);

  exit (0);
}

