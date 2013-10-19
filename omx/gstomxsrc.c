/*
 * Copyright (C) 2011, Hewlett-Packard Development Company, L.P.
 *   Author: Sebastian Dröge <sebastian.droege@collabora.co.uk>, Collabora Ltd.
 * Copyright (C) 2013, Collabora Ltd.
 *   Author: Sebastian Dröge <sebastian.droege@collabora.co.uk>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/gst.h>

#if defined (USE_OMX_TARGET_RPI) && defined(__GNUC__)
#ifndef __VCCOREVER__
#define __VCCOREVER__ 0x04000000
#endif

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC optimize ("gnu89-inline")
#endif

#if defined (USE_OMX_TARGET_RPI) && defined(__GNUC__)
#pragma GCC reset_options
#pragma GCC diagnostic pop
#endif

#include "gstomxsrc.h"

GST_DEBUG_CATEGORY_STATIC (gst_omx_src_debug_category);
#define GST_CAT_DEFAULT gst_omx_src_debug_category

/* prototypes */

enum
{
  PROP_0
};

/* class initialization */

#define DEBUG_INIT \
  GST_DEBUG_CATEGORY_INIT (gst_omx_src_debug_category, "omxsrc", 0, \
      "debug category for gst-omx source base class");


G_DEFINE_ABSTRACT_TYPE_WITH_CODE (GstOMXSrc, gst_omx_src,
    GST_TYPE_PUSH_SRC, DEBUG_INIT);

static void
gst_omx_src_class_init (GstOMXSrcClass * klass)
{
  klass->cdata.type = GST_OMX_COMPONENT_TYPE_SOURCE;
  klass->cdata.default_src_template_caps = "ANY";
}

static void
gst_omx_src_init (GstOMXSrc * self)
{
}
