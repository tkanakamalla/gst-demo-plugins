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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __GST_WHIPSINK_H__
#define __GST_WHIPSINK_H__
#include <gst/gst.h>
#include <gst/gstbin.h>
#include <gst/sdp/sdp.h>
#include <gst/rtp/rtp.h>

#define GST_USE_UNSTABLE_API
#include <gst/webrtc/webrtc.h>

/* For signalling */
#include <libsoup/soup.h>
#include <string.h>

G_BEGIN_DECLS
#define GST_TYPE_WHIPSINK   (gst_whipsink_get_type())
#define GST_WHIPSINK(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_WHIPSINK,GstWhipsink))
#define GST_WHIPSINK_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_WHIPSINK,GstWhipsinkClass))
#define GST_IS_WHIPSINK(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_WHIPSINK))
#define GST_IS_WHIPSINK_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_WHIPSINK))
typedef struct _GstWhipsink GstWhipsink;
typedef struct _GstWhipsinkClass GstWhipsinkClass;

struct _GstWhipsink
{
  GstBin parent;
  GstPad *sinkpad;
  GstElement *webrtcbin;
  SoupSession *soup_session;
  char *resource_url;
  GMutex state_lock;
  GMutex lock;
  gchar *whip_endpoint;
  gchar *stun_server;
  gchar *turn_server;
  GstWebRTCBundlePolicy bundle_policy;
  gboolean use_link_headers;
};

struct _GstWhipsinkClass
{
  GstBinClass parent_class;
};

GType gst_whipsink_get_type (void);

G_END_DECLS
#endif /*  __GST_WHIPSINK_H__  */
