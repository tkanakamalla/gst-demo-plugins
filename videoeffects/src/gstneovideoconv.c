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
    GST_VIDEO_CAPS_MAKE("GRAY8")
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
      "Simple element to convert RGB frame to GRAY8", "Generic", "Demostration of transform element to convert RGB to GRAY8",
      "taruntejk@live.com");

  gobject_class->set_property = gst_neovideoconv_set_property;
  gobject_class->get_property = gst_neovideoconv_get_property;
  gobject_class->dispose = gst_neovideoconv_dispose;
  gobject_class->finalize = gst_neovideoconv_finalize;
  base_transform_class->start = GST_DEBUG_FUNCPTR (gst_neovideoconv_start);
  base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_neovideoconv_stop);
  base_transform_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_neovideoconv_transform_caps);
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

  GST_ERROR_OBJECT (neovideoconv, "set_info");
  GST_ERROR_OBJECT (neovideoconv, " in info %d x %d", in_info->width,
      in_info->height);
  GST_ERROR_OBJECT (neovideoconv, " out info %d x %d", out_info->width,
      out_info->height);


  return TRUE;
}

/* transform */
static GstFlowReturn
gst_neovideoconv_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * inframe, GstVideoFrame * outframe)
{
  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (filter);

  // GST_INFO_OBJECT (neovideoconv, "transform_frame %p %p", inframe, outframe);
  // GST_INFO_OBJECT (neovideoconv, "No of planes in inframe: %d",
  //     GST_VIDEO_FRAME_N_PLANES (inframe));
  // GST_INFO_OBJECT (neovideoconv, "No of Components in inframe: %d",
  //     GST_VIDEO_FRAME_N_COMPONENTS (inframe));
  // GST_INFO_OBJECT (neovideoconv, "Inframe %d x %d, %lu",
  //     GST_VIDEO_FRAME_WIDTH (inframe), GST_VIDEO_FRAME_HEIGHT (inframe),
  //     GST_VIDEO_FRAME_SIZE (inframe));
  // GST_INFO_OBJECT (neovideoconv, "No of planes in outframe: %d",
  //     GST_VIDEO_FRAME_N_PLANES (outframe));
  // GST_INFO_OBJECT (neovideoconv, "No of Components in outframe: %d",
  //     GST_VIDEO_FRAME_N_COMPONENTS (outframe));
  // GST_INFO_OBJECT (neovideoconv, "Outframe %d x %d, %lu",
  //     GST_VIDEO_FRAME_WIDTH (outframe), GST_VIDEO_FRAME_HEIGHT (outframe),
  //     GST_VIDEO_FRAME_SIZE (outframe));

  gint i, j;
  gint pixel_stride, row_stride, row_wrap;
  gint d_pixel_stride, d_row_stride, d_row_wrap;
  guint32 r, g, b;
  guint32 y;
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

  // GST_ERROR_OBJECT (neovideoconv,
  //     "src: %p, s_offsets[GST_VIDEO_COMP_R]: %d, s_offsets[GST_VIDEO_COMP_G]: %d,s_offsets[GST_VIDEO_COMP_B]: %d",
  //     src, s_offsets[GST_VIDEO_COMP_R], s_offsets[GST_VIDEO_COMP_G],
  //     s_offsets[GST_VIDEO_COMP_B]);
  // GST_ERROR_OBJECT (neovideoconv, "dest: %p, d_offsets[GST_VIDEO_COMP_Y]: %d",
  //     dest, d_offsets[GST_VIDEO_COMP_Y]);
  row_stride = GST_VIDEO_FRAME_PLANE_STRIDE (inframe, 0);
  pixel_stride = GST_VIDEO_FRAME_COMP_PSTRIDE (inframe, 0);
  row_wrap = row_stride - pixel_stride * inframe->info.width;

  // GST_ERROR_OBJECT (neovideoconv,
  //     "src row_stride %d, pixel_stride %d, row_wrap %d ", row_stride,
  //     pixel_stride, row_wrap);


  d_row_stride = GST_VIDEO_FRAME_PLANE_STRIDE (outframe, 0);
  d_pixel_stride = GST_VIDEO_FRAME_COMP_PSTRIDE (outframe, 0);
  d_row_wrap = d_row_stride - d_pixel_stride * outframe->info.width;
  // GST_ERROR_OBJECT (neovideoconv,
  //     "dest d_row_stride %d, d_pixel_stride %d, d_row_wrap %d ", d_row_stride,
  //     d_pixel_stride, d_row_wrap);

  if (outframe->info.finfo->format == GST_VIDEO_FORMAT_GRAY8) {
    for (int i = 0; i < inframe->info.height; i++) {
      for (int j = 0; j < inframe->info.width; j++) {
        r = src[s_offsets[GST_VIDEO_COMP_R]];
        g = src[s_offsets[GST_VIDEO_COMP_G]];
        b = src[s_offsets[GST_VIDEO_COMP_B]];
        y = (r + g + b) / 3;
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
  GST_INFO_OBJECT (neovideoconv, "No of planes in inframe: %d",
      GST_VIDEO_FRAME_N_PLANES (inframe));
  GST_INFO_OBJECT (neovideoconv, "No of Components in inframe: %d",
      GST_VIDEO_FRAME_N_COMPONENTS (inframe));
  GST_INFO_OBJECT (neovideoconv, "Inframe %d x %d, %lu",
      GST_VIDEO_FRAME_WIDTH (inframe), GST_VIDEO_FRAME_HEIGHT (inframe),
      GST_VIDEO_FRAME_SIZE (inframe));
  gint i, j;
  gint pixel_stride, row_stride, row_wrap;
  guint32 r, g, b;
  guint32 grey;
  gint s_offsets[3];
  guint8 *src;

  src = GST_VIDEO_FRAME_PLANE_DATA (inframe, 0);

  s_offsets[0] = GST_VIDEO_FRAME_COMP_POFFSET (inframe, 0);
  s_offsets[1] = GST_VIDEO_FRAME_COMP_POFFSET (inframe, 1);
  s_offsets[2] = GST_VIDEO_FRAME_COMP_POFFSET (inframe, 2);


  row_stride = GST_VIDEO_FRAME_PLANE_STRIDE (inframe, 0);
  pixel_stride = GST_VIDEO_FRAME_COMP_PSTRIDE (inframe, 0);
  row_wrap = row_stride - pixel_stride * inframe->info.width;


  //if (outframe->info.finfo->format == GST_VIDEO_FORMAT_GRAY8) {
  {
    for (int i = 0; i < inframe->info.height; i++) {
      for (int j = 0; j < inframe->info.width; j++) {
        //GST_INFO_OBJECT(neovideoconv, "iter %d %d",i,j);
        r = src[s_offsets[0]];
        g = src[s_offsets[1]];
        b = src[s_offsets[2]];
        grey = (r + g + b) / 3;
        src[s_offsets[0]] = grey;
        src[s_offsets[0]] = grey;
        src[s_offsets[0]] = grey;
        src += pixel_stride;

      }
      src += row_wrap;
    }
  }


  return GST_FLOW_OK;
}

static GstCaps *
gst_neovideoconv_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{

  GstNeovideoconv *neovideoconv = GST_NEOVIDEOCONV (trans);
  GstVideoFormat src_format = GST_VIDEO_FORMAT_GRAY8, sink_format = GST_VIDEO_FORMAT_RGB;

  GST_DEBUG_OBJECT (neovideoconv, "%s", __func__);
  GstCaps *ret_caps = NULL, *temp_caps;
  GST_ERROR_OBJECT (neovideoconv, "received caps : %s",
      gst_caps_to_string (caps));
  //check the pad direction SRC or SINK
  if (direction == GST_PAD_SINK) {
    //return SRC caps;
    temp_caps = gst_caps_copy(caps);
    gst_caps_set_simple (temp_caps, "format", G_TYPE_STRING,
      gst_video_format_to_string (src_format), NULL);
    GST_ERROR_OBJECT (neovideoconv, "%s temp src caps are %" GST_PTR_FORMAT , __func__, temp_caps);
  } else {
    //return SINK caps
    temp_caps = gst_caps_copy(caps);
    gst_caps_set_simple (temp_caps, "format", G_TYPE_STRING,
      gst_video_format_to_string (sink_format), NULL);
    GST_ERROR_OBJECT (neovideoconv, "%s temp sink caps are %" GST_PTR_FORMAT , __func__, temp_caps);
  }

  if (filter) {
    GST_ERROR_OBJECT (neovideoconv, "filter caps : %s",
        gst_caps_to_string (filter));
    ret_caps =
        gst_caps_intersect_full (temp_caps, filter, GST_CAPS_INTERSECT_FIRST);
  } else {
    ret_caps = temp_caps;
  }

  GST_ERROR_OBJECT (neovideoconv, "caps: %" GST_PTR_FORMAT, ret_caps);

  return ret_caps;
}

