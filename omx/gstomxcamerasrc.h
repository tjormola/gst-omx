/*
 * Copyright (C) <2013> Tuomas Jormola <tj@solitudo.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 */

#ifndef __GST_OMX_CAMERA_SRC_H__
#define __GST_OMX_CAMERA_SRC_H__

#include <gst/gst.h>

#include "gstomxsrc.h"

G_BEGIN_DECLS

#define GST_TYPE_OMX_CAMERA_SRC \
  (gst_omx_camera_src_get_type())
#define GST_OMX_CAMERA_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_OMX_CAMERA_SRC,GstOMXCameraSrc))
#define GST_OMX_CAMERA_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_OMX_CAMERA_SRC,GstOMXCameraSrcClass))
#define GST_OMX_CAMERA_SRC_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj),GST_TYPE_OMX_CAMERA_SRC,GstOMXCameraSrcClass))
#define GST_IS_OMX_CAMERA_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_OMX_CAMERA_SRC))
#define GST_IS_OMX_CAMERA_SRC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_OMX_CAMERA_SRC))

// TODO: Stolen from GstBaseSrc. The source says subclasses can access
// GstBaseSrc->live_lock and GstBaseSrc->live_cond, but would it be better to
// use our own mutex and cond var?
#define GST_LIVE_GET_LOCK(elem)               (&GST_BASE_SRC_CAST(elem)->live_lock)
#define GST_LIVE_LOCK(elem)                   g_mutex_lock(GST_LIVE_GET_LOCK(elem))
#define GST_LIVE_TRYLOCK(elem)                g_mutex_trylock(GST_LIVE_GET_LOCK(elem))
#define GST_LIVE_UNLOCK(elem)                 g_mutex_unlock(GST_LIVE_GET_LOCK(elem))
#define GST_LIVE_GET_COND(elem)               (&GST_BASE_SRC_CAST(elem)->live_cond)
#define GST_LIVE_WAIT(elem)                   g_cond_wait (GST_LIVE_GET_COND (elem), GST_LIVE_GET_LOCK (elem))
#define GST_LIVE_WAIT_UNTIL(elem, end_time)   g_cond_timed_wait (GST_LIVE_GET_COND (elem), GST_LIVE_GET_LOCK (elem), end_time)
#define GST_LIVE_SIGNAL(elem)                 g_cond_signal (GST_LIVE_GET_COND (elem));
#define GST_LIVE_BROADCAST(elem)              g_cond_broadcast (GST_LIVE_GET_COND (elem));

typedef enum _GstOMXCameraSrcCompIndices GstOMXCameraSrcCompIndices;
typedef enum _GstOMXCameraSrcPortIndices GstOMXCameraSrcPortIndices;

typedef struct _GstOMXCameraSrc GstOMXCameraSrc;
typedef struct _GstOMXCameraSrcClass GstOMXCameraSrcClass;
typedef struct _GstOMXCameraSrcConfig GstOMXCameraSrcConfig;

enum _GstOMXCameraSrcCompIndices
{
  CAMERA = 0
#ifdef USE_OMX_TARGET_RPI
  ,NULL_SINK = 1
#endif
};

enum _GstOMXCameraSrcPortIndices
{
  CAMERA_IN = 0,
  CAMERA_VIDEO_OUT = 1
#ifdef USE_OMX_TARGET_RPI
  ,CAMERA_PREVIEW_OUT = 2,
  NULL_SINK_IN = 3
#endif
};

struct _GstOMXCameraSrcConfig
{
  OMX_PARAM_U32TYPE device;
#ifdef USE_OMX_TARGET_RPI
  OMX_CONFIG_SHARPNESSTYPE sharpness;
#endif
  OMX_CONFIG_GAMMATYPE gamma;
  OMX_CONFIG_CONTRASTTYPE contrast;
  OMX_CONFIG_BRIGHTNESSTYPE brightness;
  OMX_CONFIG_SATURATIONTYPE saturation;
  OMX_CONFIG_IMAGEFILTERTYPE image_filter;
  OMX_CONFIG_COLORENHANCEMENTTYPE color_enhancement;
  OMX_CONFIG_WHITEBALCONTROLTYPE white_balance;
  OMX_CONFIG_EXPOSURECONTROLTYPE exposure_control;
  OMX_CONFIG_EXPOSUREVALUETYPE exposure_value;
  OMX_CONFIG_FRAMESTABTYPE frame_stabilisation;
  OMX_CONFIG_MIRRORTYPE mirror;
  gint horizontal_flip;
  gint vertical_flip;
};

struct _GstOMXCameraSrc
{
  GstOMXSrc parent;

  // OMX resources
  /* Indexed by GstOMXCameraSrcCompIndices */
  GstOMXComponent **comp;
  /* Indexed by GstOMXCameraSrcPortIndices */
  GstOMXPort **port;

  // Configuration stuff
  GstOMXCameraSrcConfig config;

  // These are for syncing
  gboolean camera_configured;
  gboolean video_configured;

  // This describes the current video capability
  // TODO: On RPi, we need something more if encoder is being used
  GstVideoInfo * info;

  // TODO: This describes the format of a single OMX buffer
  // in OMX_COLOR_FormatYUV420PackedPlanar format.
  // Currently that's the only thing we support and this was a quick way to
  // implement it. Something else needs to be done when we support both raw and
  // compressed video with RPi.
  GstVideoInfo * omx_buf_info;

  // For the buffer fields
  /* running time and frames for current caps */
  GstClockTime running_time;
  gint64 n_frames;

  // TODO: This are not needed or used at the moment as we're living with the
  // same caps during our whole lifetime
  /* total caps running time and frames */
  GstClockTime accum_rtime;
  gint64 accum_frames;
};

struct _GstOMXCameraSrcClass
{
  GstOMXSrcClass parent_class;
};

GType gst_omx_camera_src_get_type (void);

G_END_DECLS

#endif /* __GST_OMX_CAMERA_SRC_H__ */
