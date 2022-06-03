/* GStreamer
 * Copyright (C) 2022 Taruntej Kanakamalla <taruntejk@live.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstneovideoconv
 *
 * The neovideoconv element does basic video editing stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v videotestsrc  !  neovideoconv  !  video/x-raw,width=1920,height=1440,framerate=30/1 ! videoconvert ! autovideosink 
 * ]|
 * Converts the colorscale video to grayscale with GRAY8 format.
 * </refsect2>
 */

#include "gst/base/gstbasetransform.h"
#include "gst/gstcaps.h"
#include "gst/gstinfo.h"
#include "gst/gstpad.h"
#include "gst/video/video-format.h"
#include "gst/video/video-frame.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/video/video.h>
#include <gst/video/gstvideofilter.h>
#include "gstneovideoconv.h"

GST_DEBUG_CATEGORY_STATIC (gst_neovideoconv_debug_category);
#define GST_CAT_DEFAULT gst_neovideoconv_debug_category

/* prototypes */


static void gst_neovideoconv_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_neovideoconv_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_neovideoconv_dispose (GObject * object);
static void gst_neovideoconv_finalize (GObject * object);

static gboolean gst_neovideoconv_start (GstBaseTransform * trans);
static gboolean gst_neovideoconv_stop (GstBaseTransform * trans);
static gboolean gst_neovideoconv_set_info (GstVideoFilter * filter,
    GstCaps * incaps, GstVideoInfo * in_info, GstCaps * outcaps,
    GstVideoInfo * out_info);
static GstFlowReturn gst_neovideoconv_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * inframe, GstVideoFrame * outframe);
static GstFlowReturn gst_neovideoconv_transform_frame_ip (GstVideoFilter *
    filter, GstVideoFrame * frame);
static GstCaps *gst_neovideoconv_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter);

enum
{
  PROP_0
};

/* pad templates */

#define VIDEO_SRC_CAPS \
    GST_VIDEO_CAPS_MAKE("{ GRAY8, RGB }")
#define VIDEO_SINK_CAPS \
    GST_VIDEO_CAPS_MAKE("RGB")

/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstNeovideoconv, gst_neovideoconv,
    GST_TYPE_VIDEO_FILTER,
    GST_DEBUG_CATEGORY_INIT (gst_neovideoconv_debug_category, "neovideoconv", 0,
        "debug category for neovideoconv element"));

static void
gst_neovideoconv_class_init (GstNeovideoconvClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseTransformClass *base_transform_class =
      GST_BASE_TRANSFORM_CLASS (klass);
  GstVideoFilterClass *video_filter_class = GST_VIDEO_FILTER_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
      gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
          gst_caps_from_string (VIDEO_SRC_CAPS)));
  gst_element_class_add_pad_template (GST_ELEMENT_CLASS (klass),
      gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
          gst_caps_from_string (VIDEO_SINK_CAPS)));

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS (klass),
      "Simple element to convert RGB frame to GRAY8", "Generic",
      "Demostration of transform element to convert RGB to GRAY8",
      "taruntejk@live.com");

  gobject_class->set_property = gst_neovideoconv_set_property;
  gobject_class->get_property = gst_neovideoconv_get_property;
  gobject_class->dispose = gst_neovideoconv_dispose;
  gobject_class->finalize = gst_neovideoconv_finalize;
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_neovideoconv_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_neovideoconv_stop);
  base_transform_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_neovideoconv_transform_caps);
  base_transform_class->transform_ip_on_passthrough = FALSE;
  video_filter_class->set_info = GST_DEBUG_FUNCPTR (gst_neovideoconv_set_info);
  video_filter_class->transform_frame =
      GST_DEBUG_FUNCPTR (gst_neovideoconv_transform_frame);
  video_filter_class->transform_frame_ip =
      GST_DEBUG_FUNCPTR (gst_neovideoconv_transform_frame_ip);

}

static void
gst_neovideoconv_init (GstNeovideoconv * neovideoconv)
{
}

void
gst_neovideoconv_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (object);

  GST_DEBUG_OBJECT (neovideoconv, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_neovideoconv_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (object);

  GST_DEBUG_OBJECT (neovideoconv, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_neovideoconv_dispose (GObject * object)
{
  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (object);

  GST_DEBUG_OBJECT (neovideoconv, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_neovideoconv_parent_class)->dispose (object);
}

void
gst_neovideoconv_finalize (GObject * object)
{
  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (object);

  GST_DEBUG_OBJECT (neovideoconv, "finalize");

  /* clean up object here */

  G_OBJECT_CLASS (gst_neovideoconv_parent_class)->finalize (object);
}

static gboolean
gst_neovideoconv_start (GstBaseTransform * trans)
{
  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (trans);

  GST_DEBUG_OBJECT (neovideoconv, "start");

  return TRUE;
}

static gboolean
gst_neovideoconv_stop (GstBaseTransform * trans)
{
  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (trans);

  GST_DEBUG_OBJECT (neovideoconv, "stop");

  return TRUE;
}

static gboolean
gst_neovideoconv_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (filter);

  GST_DEBUG_OBJECT(neovideoconv, "in caps : %" GST_PTR_FORMAT, incaps);
  GST_DEBUG_OBJECT(neovideoconv, "out caps : %" GST_PTR_FORMAT, outcaps);

  if (GST_VIDEO_FORMAT_INFO_FORMAT(in_info->finfo) == GST_VIDEO_FORMAT_INFO_FORMAT(out_info->finfo)) {
    //set as passthrough
    gst_base_transform_set_passthrough (GST_BASE_TRANSFORM(filter), TRUE);
  }


  return TRUE;
}

/* transform */
static GstFlowReturn
gst_neovideoconv_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * inframe, GstVideoFrame * outframe)
{
  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (filter);

  GST_INFO_OBJECT (neovideoconv, "transform_frame %p %p", inframe, outframe);
  gint row, col;
  gint pixel_stride, row_stride, row_wrap;
  gint d_pixel_stride, d_row_stride, d_row_wrap;
  guint8 r, g, b;
  guint8 y;
  gint s_offsets[3], d_offsets[3];
  guint8 *src, *dest;

  src = GST_VIDEO_FRAME_PLANE_DATA (inframe, 0);
  dest = GST_VIDEO_FRAME_PLANE_DATA (outframe, 0);
  s_offsets[GST_VIDEO_COMP_R] =
      GST_VIDEO_FRAME_COMP_POFFSET (inframe, GST_VIDEO_COMP_R);
  s_offsets[GST_VIDEO_COMP_G] =
      GST_VIDEO_FRAME_COMP_POFFSET (inframe, GST_VIDEO_COMP_G);
  s_offsets[GST_VIDEO_COMP_B] =
      GST_VIDEO_FRAME_COMP_POFFSET (inframe, GST_VIDEO_COMP_B);

  d_offsets[GST_VIDEO_COMP_Y] =
      GST_VIDEO_FRAME_COMP_POFFSET (outframe, GST_VIDEO_COMP_Y);

  row_stride = GST_VIDEO_FRAME_PLANE_STRIDE (inframe, 0);
  pixel_stride = GST_VIDEO_FRAME_COMP_PSTRIDE (inframe, 0);
  row_wrap = row_stride - pixel_stride * inframe->info.width;

  d_row_stride = GST_VIDEO_FRAME_PLANE_STRIDE (outframe, 0);
  d_pixel_stride = GST_VIDEO_FRAME_COMP_PSTRIDE (outframe, 0);
  d_row_wrap = d_row_stride - d_pixel_stride * outframe->info.width;

  if (outframe->info.finfo->format == GST_VIDEO_FORMAT_GRAY8) {
    for (int row = 0; row < inframe->info.height; row++) {
      for (int col = 0; col < inframe->info.width; col++) {
        r = src[s_offsets[GST_VIDEO_COMP_R]];
        g = src[s_offsets[GST_VIDEO_COMP_G]];
        b = src[s_offsets[GST_VIDEO_COMP_B]];
        /*Lightness method*/
        //y = (MIN(r, MIN(g, b)) + MAX(r, MAX(g,b)))/2;
        /*Average method*/
        //y = (r + g + b) / 3;
        /*Luminosity Method*/
        y = 0.3*r + 0.59*g + 0.11*b;
        dest[d_offsets[GST_VIDEO_COMP_Y]] = y;
        src += pixel_stride;
        dest += d_pixel_stride;
      }
    }
  }
  return GST_FLOW_OK;
}

static GstFlowReturn
gst_neovideoconv_transform_frame_ip (GstVideoFilter * filter,
    GstVideoFrame * inframe)
{
  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (filter);

  GST_DEBUG_OBJECT (neovideoconv, "transform_frame_ip");
  return GST_FLOW_OK;
}

static GstCaps *
gst_neovideoconv_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{

  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (trans);
  GValue v_formats = G_VALUE_INIT;
  GST_DEBUG_OBJECT (neovideoconv, "%s", __func__);
  GstCaps *ret_caps = NULL, *temp_caps;
  GST_DEBUG_OBJECT (neovideoconv, "received caps %p : %" GST_PTR_FORMAT, caps,
      caps);
  //check the pad direction SRC or SINK
  if (direction == GST_PAD_SINK) {
    //return SRC caps;
    temp_caps = gst_caps_copy (caps);
    GValue item = G_VALUE_INIT;
    gst_value_list_init (&v_formats, 2);
    g_value_init (&item, G_TYPE_STRING);
    g_value_set_string (&item, gst_video_format_to_string (GST_VIDEO_FORMAT_RGB));
    gst_value_list_append_value (&v_formats, &item);
    g_value_unset (&item);
    g_value_init (&item, G_TYPE_STRING);
    g_value_set_string (&item, gst_video_format_to_string (GST_VIDEO_FORMAT_GRAY8));
    gst_value_list_append_value (&v_formats, &item);
    g_value_unset (&item);
    gst_caps_set_value (temp_caps, "format", &v_formats);
    g_value_unset (&v_formats);
    GST_DEBUG_OBJECT (neovideoconv, "%s temp src caps are %" GST_PTR_FORMAT,
        __func__, temp_caps);
  } else {
    //return SINK caps
    temp_caps = gst_caps_copy (caps);
    gst_caps_set_simple (temp_caps, "format", G_TYPE_STRING,
        gst_video_format_to_string (GST_VIDEO_FORMAT_RGB), NULL);
    GST_DEBUG_OBJECT (neovideoconv, "%s temp sink caps are %" GST_PTR_FORMAT,
        __func__, temp_caps);
  }

  if (filter) {
    GST_ERROR_OBJECT (neovideoconv, "filter caps : %s",
        gst_caps_to_string (filter));
    ret_caps =
        gst_caps_intersect_full (temp_caps, filter, GST_CAPS_INTERSECT_FIRST);
    gst_caps_unref (temp_caps);
  } else {
    ret_caps = temp_caps;
  }

  GST_DEBUG_OBJECT (neovideoconv, "caps %p : %" GST_PTR_FORMAT, ret_caps,
      ret_caps);

  return ret_caps;
}
