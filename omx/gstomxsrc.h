/*
 * Copyright (C) 2011, Hewlett-Packard Development Company, L.P.
 *   Author: Sebastian Dröge <sebastian.droege@collabora.co.uk>, Collabora Ltd.
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

#ifndef __GST_OMX_SRC_H__
#define __GST_OMX_SRC_H__

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>
//#include <gst/base/gstbasesrc.h>

#include "gstomx.h"

G_BEGIN_DECLS

#define GST_TYPE_OMX_SRC \
  (gst_omx_src_get_type())
#define GST_OMX_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_OMX_SRC,GstOMXSrc))
#define GST_OMX_SRC_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_OMX_SRC,GstOMXSrcClass))
#define GST_OMX_SRC_GET_CLASS(obj) \
  (G_TYPE_INSTANCE_GET_CLASS((obj),GST_TYPE_OMX_SRC,GstOMXSrcClass))
#define GST_IS_OMX_SRC(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_OMX_SRC))
#define GST_IS_OMX_SRC_CLASS(obj) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_OMX_SRC))

typedef struct _GstOMXSrc GstOMXSrc;
typedef struct _GstOMXSrcClass GstOMXSrcClass;

struct _GstOMXSrc
{
  //GstBaseSrc parent;
  GstPushSrc parent;
};

struct _GstOMXSrcClass
{
  //GstBaseSrcClass parent_class;
  GstPushSrcClass parent_class;

  GstOMXClassData cdata;
};

GType gst_omx_src_get_type (void);

G_END_DECLS

#endif /* __GST_OMX_SRC_H__ */
