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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>

#include <glib/gprintf.h>
#include <gst/gst.h>
#include <gst/video/videoorientation.h>

#include "gstomxcamerasrc.h"

GST_DEBUG_CATEGORY_STATIC (gst_omx_camera_src_debug_category);
#define GST_CAT_DEFAULT gst_omx_camera_src_debug_category

#define GST_OMX_CAMERA_SRC_DEVICE_NUMBER_DEFAULT               0
#define GST_OMX_CAMERA_SRC_WIDTH_DEFAULT                       480
#define GST_OMX_CAMERA_SRC_HEIGHT_DEFAULT                      270
#define GST_OMX_CAMERA_SRC_FRAMERATE_DEFAULT                   25
#ifdef USE_OMX_TARGET_RPI
#define GST_OMX_CAMERA_SRC_SHARPNESS_DEFAULT                   0
#endif
#define GST_OMX_CAMERA_SRC_GAMMA_DEFAULT                       2
#define GST_OMX_CAMERA_SRC_CONTRAST_DEFAULT                    0
#define GST_OMX_CAMERA_SRC_BRIGHTNESS_DEFAULT                  50
#define GST_OMX_CAMERA_SRC_SATURATION_DEFAULT                  0
#define GST_OMX_CAMERA_SRC_IMAGE_FILTER_DEFAULT                OMX_ImageFilterNone
#define GST_OMX_CAMERA_SRC_COLOR_ENHANCEMENT_DEFAULT           NULL
#define GST_OMX_CAMERA_SRC_WHITE_BALANCE_MODE_DEFAULT          OMX_WhiteBalControlAuto
#define GST_OMX_CAMERA_SRC_EXPOSURE_CONTROL_MODE_DEFAULT       OMX_ExposureControlAuto
#define GST_OMX_CAMERA_SRC_EXPOSURE_METERING_MODE_DEFAULT      OMX_MeteringModeSpot
#define GST_OMX_CAMERA_SRC_EXPOSURE_VALUE_COMPENSATION_DEFAULT 0
#define GST_OMX_CAMERA_SRC_EXPOSURE_ISO_SENSITIVITY_DEFAULT    100
#define GST_OMX_CAMERA_SRC_APERTURE_DEFAULT                    2
#define GST_OMX_CAMERA_SRC_FRAME_STABILISATION_DEFAULT         TRUE
#define GST_OMX_CAMERA_SRC_HORIZONTAL_FLIP_DEFAULT             FALSE
#define GST_OMX_CAMERA_SRC_VERTICAL_FLIP_DEFAULT               FALSE

enum
{
  PROP_0,
  PROP_DEVICE_NUMBER,
#ifdef USE_OMX_TARGET_RPI
  PROP_SHARPNESS,
#endif
  PROP_GAMMA,
  PROP_CONTRAST,
  PROP_BRIGHTNESS,
  PROP_SATURATION,
  PROP_IMAGE_FILTER,
  PROP_COLOR_ENHANCEMENT,
  PROP_WHITE_BALANCE_MODE,
  PROP_EXPOSURE_CONTROL_MODE,
  PROP_EXPOSURE_METERING_MODE,
  PROP_EXPOSURE_VALUE_COMPENSATION,
  PROP_EXPOSURE_ISO_SENSITIVITY,
  PROP_APERTURE,
  PROP_FRAME_STABILISATION,
  PROP_HORIZONTAL_FLIP,
  PROP_VERTICAL_FLIP,
};

static void gst_omx_camera_src_video_orientation_interface_init (GstVideoOrientationInterface * iface);

#define GST_TYPE_OMX_CAMERA_SRC_IMAGE_FILTER (gst_omx_camera_src_image_filter_get_type ())
static GType
gst_omx_camera_src_image_filter_get_type (void)
{
  static GType omx_camera_src_image_filter_type = 0;
  static const GEnumValue image_filter_types[] = {
    {OMX_ImageFilterNone,          "Disable image filtering",
                                   "none"                              },
    {OMX_ImageFilterNoise,         "Remove noise from the image",
                                   "denoise"                           },
    {OMX_ImageFilterEmboss,        "Embossed effect",
                                   "emboss"                            },
    {OMX_ImageFilterNegative,      "Negate image colors",
                                   "negative"                          },
    {OMX_ImageFilterSketch,        "Sketching effect",
                                   "sketch"                            },
    {OMX_ImageFilterOilPaint,      "Oil painting effect",
                                   "oilpaint"                          },
    {OMX_ImageFilterHatch,         "Grainy material effect",
                                   "hatch"                             },
    {OMX_ImageFilterGpen,          "Graphite pen drawing style effect",
                                   "gpen"                              },
    {OMX_ImageFilterAntialias,     "Anti-alias pixels in the image",
                                   "antialias"                         },
    {OMX_ImageFilterDeRing,        "Remove digital image processing "
                                       "artifacts in the image",
                                    "dering"                           },
    {OMX_ImageFilterSolarize,      "Solarization effect",
                                   "solarize"                          },
#if defined(USE_OMX_TARGET_RPI)
    {OMX_ImageFilterWatercolor,    "Water color painting effect",
                                    "watercolor"                       },
    {OMX_ImageFilterPastel,        "Create a pastel effect",
                                   "pastel"                            },
    {OMX_ImageFilterSharpen,       "Sharpen the image",
                                   "sharpen"                           },
    {OMX_ImageFilterFilm,          "Grainy old film effect",
                                   "film"                              },
    {OMX_ImageFilterBlur,          "Blur the image",
                                   "blur"                              },
    {OMX_ImageFilterSaturation,    "Color-saturate the image",
                                   "color-saturate"                    },
    {OMX_ImageFilterColourSwap,    "Swap colors on the image",
                                   "color-swap"                        },
    {OMX_ImageFilterWashedOut,     "Washed-out effect",
                                   "washedout"                         },
    {OMX_ImageFilterColourPoint,   "Color-point effect",
                                   "color-point"                       },
    {OMX_ImageFilterPosterise,     "Poster effect",
                                   "poster"                            },
    {OMX_ImageFilterColourBalance, "Color-balance effect",
                                   "color-balance"                     },
    {OMX_ImageFilterCartoon,       "Cartoon effect",
                                   "cartoon"                           },
#endif
    {0, NULL, NULL}
  };

  if (!omx_camera_src_image_filter_type) {
    omx_camera_src_image_filter_type =
        g_enum_register_static ("GstOMXCameraSrcImageEffect", image_filter_types);
  }
  return omx_camera_src_image_filter_type;
}

#define GST_TYPE_OMX_CAMERA_SRC_WHITE_BALANCE_MODE (gst_omx_camera_src_white_balance_mode_get_type ())
static GType
gst_omx_camera_src_white_balance_mode_get_type (void)
{
  static GType omx_camera_src_white_balance_mode_type = 0;
  static const GEnumValue white_balance_mode_types[] = {
    {OMX_WhiteBalControlOff,          "Disables white balance calucation",
                                      "off"                                  },
    {OMX_WhiteBalControlAuto,         "Automatic white balance control",
                                      "auto"                                 },
    {OMX_WhiteBalControlSunLight,     "Manual white balance control with "
                                          "clear sun light",
                                      "sun"                                  },
    {OMX_WhiteBalControlCloudy,       "Manual white balance control with "
                                          "sun light through clouds",
                                      "cloudy"                               },
    {OMX_WhiteBalControlShade,        "Manual white balance control with "
                                          "sun light in shade",
                                      "shade"                                },
    {OMX_WhiteBalControlTungsten,     "Manual white balance control with "
                                          "tungsten light",
                                      "tungsten"                             },
    {OMX_WhiteBalControlFluorescent,  "Manual white balance control with "
                                          "fluorescent light",
                                      "fluorescent"                          },
    {OMX_WhiteBalControlIncandescent, "Manual white balance control with "
                                          "incandescent light",
                                      "incandescent"                         },
    {OMX_WhiteBalControlFlash,        "Manual white balance control when "
                                          "the light source is a flash",
                                      "flash"                                },
    {OMX_WhiteBalControlHorizon,      "Manual white balance control with "
                                          "sun light when the sun is "
                                          "on the horizon",
                                      "horizon"                              },
    {0, NULL, NULL}
  };

  if (!omx_camera_src_white_balance_mode_type) {
    omx_camera_src_white_balance_mode_type =
        g_enum_register_static ("GstOMXCameraSrcWhiteBalanceMode", white_balance_mode_types);
  }
  return omx_camera_src_white_balance_mode_type;
}

#define GST_TYPE_OMX_CAMERA_SRC_EXPOSURE_CONTROL_MODE (gst_omx_camera_src_exposure_control_mode_get_type ())
static GType
gst_omx_camera_src_exposure_control_mode_get_type (void)
{
  static GType omx_camera_src_exposure_control_mode_type = 0;
  static const GEnumValue exposure_control_mode_types[] = {
    {OMX_ExposureControlOff,           "Disables exposure control",
                                           "off"                            },
    {OMX_ExposureControlAuto,          "Automatic exposure",
                                           "auto"                           },
    {OMX_ExposureControlNight,         "Exposure at night",
                                           "night"                          },
    {OMX_ExposureControlBackLight,     "Exposure with backlight "
                                                "illuminating the subject",
                                           "backlight"                      },
    {OMX_ExposureControlSpotLight,     "Exposure with a spotlight "
                                               "illuminating the subject",
                                           "spotlight"                      },
    {OMX_ExposureControlSports,        "Exposure for sports or "
                                               "other fast movement",
                                           "sports"                         },
    {OMX_ExposureControlSnow,          "Exposure for the subject in snow",
                                           "snow"                           },
    {OMX_ExposureControlBeach,         "Exposure for the subject at a beach",
                                           "beach"                          },
    {OMX_ExposureControlLargeAperture, "Exposure when using a large "
                                               "aperture on the camera",
                                           "large-aperture"                 },
    {OMX_ExposureControlSmallAperture, "Exposure when using a small "
                                               "aperture on the camera",
                                           "small-aperture"                 },
#if defined(USE_OMX_TARGET_RPI)
    {OMX_ExposureControlFireworks,     "Exposure for fireworks",
                                       "fireworks"                          },
    {OMX_ExposureControlVeryLong,      "Very long exposure",
                                       "long"                               },
    {OMX_ExposureControlFixedFps,      "Constrain frames-per-scond"
                                               "to a fixed value",
                                       "fixedfps"                           },
    {OMX_ExposureControlAntishake,     "Anti-shake mode",
                                       "antishake"                          },
#endif
    {0, NULL, NULL}
  };

  if (!omx_camera_src_exposure_control_mode_type) {
    omx_camera_src_exposure_control_mode_type =
        g_enum_register_static ("GstOMXCameraSrcExposureControlMode", exposure_control_mode_types);
  }
  return omx_camera_src_exposure_control_mode_type;
}

#define GST_TYPE_OMX_CAMERA_SRC_EXPOSURE_METERING_MODE (gst_omx_camera_src_exposure_metering_mode_get_type ())
static GType
gst_omx_camera_src_exposure_metering_mode_get_type (void)
{
  static GType omx_camera_src_exposure_metering_mode_type = 0;
  static const GEnumValue exposure_metering_mode_types[] = {
    {OMX_MeteringModeAverage, "Center weight average metering", "average"},
    {OMX_MeteringModeSpot,    "Spot (partial) metering",        "spot"   },
    {OMX_MeteringModeMatrix,  "Matrix or evaluative metering",  "matrix" },
    {0, NULL, NULL}
  };

  if (!omx_camera_src_exposure_metering_mode_type) {
    omx_camera_src_exposure_metering_mode_type =
        g_enum_register_static ("GstOMXCameraSrcExposureMeteringMode",
            exposure_metering_mode_types);
  }
  return omx_camera_src_exposure_metering_mode_type;
}

/* Virtual method declarations */
static GstFlowReturn gst_omx_camera_src_fill (GstPushSrc * push_src,
    GstBuffer * buf);

static gboolean gst_omx_camera_src_start (GstBaseSrc * base_src);
static gboolean gst_omx_camera_src_stop (GstBaseSrc * base_src);
static GstCaps * gst_omx_camera_src_get_caps (GstBaseSrc * base_src,
    GstCaps * filter);
static GstCaps * gst_omx_camera_src_fixate (GstBaseSrc * base_src,
    GstCaps * caps);
static gboolean gst_omx_camera_src_set_caps (GstBaseSrc * base_src,
    GstCaps * caps);
static gboolean gst_omx_camera_src_decide_allocation (GstBaseSrc *base_src,
    GstQuery *query);

static GstStateChangeReturn
gst_omx_camera_src_change_state (GstElement * element,
    GstStateChange transition);

static void gst_omx_camera_src_finalize (GObject * object);
static void gst_omx_camera_src_set_property (GObject * object,
    guint prop_id, const GValue * value, GParamSpec * pspec);
static void gst_omx_camera_src_get_property (GObject * object,
    guint prop_id, GValue * value, GParamSpec * pspec);

/* VideoOrientation */
static gboolean gst_omx_camera_src_vo_get_hflip (GstVideoOrientation * vo,
    gboolean * flip);
static gboolean gst_omx_camera_src_vo_get_vflip (GstVideoOrientation * vo,
    gboolean * flip);
static gboolean gst_omx_camera_src_vo_set_hflip (GstVideoOrientation * vo,
    gboolean flip);
static gboolean gst_omx_camera_src_vo_set_vflip (GstVideoOrientation * vo,
    gboolean flip);

#define parent_class gst_omx_camera_src_parent_class

#define DEBUG_INIT \
  GST_DEBUG_CATEGORY_INIT (gst_omx_camera_src_debug_category, "omxcamera_src", 0, \
      "debug category for gst-omx camera class");

G_DEFINE_TYPE_WITH_CODE (GstOMXCameraSrc, gst_omx_camera_src,
    GST_TYPE_OMX_SRC,
    G_IMPLEMENT_INTERFACE (GST_TYPE_VIDEO_ORIENTATION,
        gst_omx_camera_src_video_orientation_interface_init)
    DEBUG_INIT);

static void
gst_omx_camera_src_class_init (GstOMXCameraSrcClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstElementClass *element_class = GST_ELEMENT_CLASS (klass);
  GstBaseSrcClass *basesrc_class = GST_BASE_SRC_CLASS (klass);
  GstPushSrcClass *pushsrc_class = GST_PUSH_SRC_CLASS (klass);
  GstOMXSrcClass *omxsrc_class = GST_OMX_SRC_CLASS (klass);

  gobject_class->finalize = gst_omx_camera_src_finalize;
  gobject_class->set_property = gst_omx_camera_src_set_property;
  gobject_class->get_property = gst_omx_camera_src_get_property;

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_DEVICE_NUMBER,
      g_param_spec_uint ("device-number", "Device number",
          "Device number of the camera device",
          0, G_MAXINT, GST_OMX_CAMERA_SRC_DEVICE_NUMBER_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
#ifdef USE_OMX_TARGET_RPI
  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_SHARPNESS,
      g_param_spec_int ("sharpness", "Sharpness", "Image sharpness",
          -100, 100, GST_OMX_CAMERA_SRC_SHARPNESS_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
#endif
  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_GAMMA,
      g_param_spec_int ("gamma", "Gamma",
          "Gamma value used for gamma correction, "
              "setting not applied if not specified",
          0, 4, GST_OMX_CAMERA_SRC_GAMMA_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_CONTRAST,
      g_param_spec_int ("contrast", "Contrast",
          "Picture contrast or luma gain",
          -100, 100, GST_OMX_CAMERA_SRC_CONTRAST_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_BRIGHTNESS,
      g_param_spec_uint ("brightness", "Brightness",
          "Picture brightness, or more precisely, the black level",
          0, 100, GST_OMX_CAMERA_SRC_BRIGHTNESS_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_SATURATION,
      g_param_spec_int ("saturation", "Saturation",
          "Picture color saturation or chroma gain",
          -100, 100, GST_OMX_CAMERA_SRC_SATURATION_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_WHITE_BALANCE_MODE,
      g_param_spec_enum ("white-balance-mode", "White balance mode",
          "White balance mode to be used",
          GST_TYPE_OMX_CAMERA_SRC_WHITE_BALANCE_MODE,
          GST_OMX_CAMERA_SRC_WHITE_BALANCE_MODE_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_IMAGE_FILTER,
      g_param_spec_enum ("image-filter", "Image filter",
          "Image filter to be applied to the video",
          GST_TYPE_OMX_CAMERA_SRC_IMAGE_FILTER,
          GST_OMX_CAMERA_SRC_IMAGE_FILTER_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_COLOR_ENHANCEMENT,
      g_param_spec_string ("color-enhancement", "Color enhancement effect",
          "Specify color enhancement effect in format \"U:V\" where U and V "
              "are integers in the range 0 - 255  that are applied as "
              "constants to the U and V channels of the image, "
              "e.g. 128:128 results in a monochrome image",
          GST_OMX_CAMERA_SRC_COLOR_ENHANCEMENT_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_EXPOSURE_CONTROL_MODE,
      g_param_spec_enum ("exposure-control-mode", "Exposure control mode",
          "Exposure control mode to be used",
          GST_TYPE_OMX_CAMERA_SRC_EXPOSURE_CONTROL_MODE,
          GST_OMX_CAMERA_SRC_EXPOSURE_CONTROL_MODE_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_EXPOSURE_METERING_MODE,
      g_param_spec_enum ("exposure-metering-mode", "Exposure metering mode",
          "Exposure metering mode to be used",
          GST_TYPE_OMX_CAMERA_SRC_EXPOSURE_METERING_MODE,
          GST_OMX_CAMERA_SRC_EXPOSURE_METERING_MODE_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_EXPOSURE_VALUE_COMPENSATION,
      g_param_spec_int ("exposure-value-compensation",
          "Exposure value compensation",
          "Exposure value compensation of the image",
          -10, 10, GST_OMX_CAMERA_SRC_EXPOSURE_VALUE_COMPENSATION_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_EXPOSURE_ISO_SENSITIVITY,
      g_param_spec_uint ("exposure-iso-sensitivity",
          "Exposure ISO sensitivity", "Exposure ISO sensitivty or the ISO "
              "level to use, automatic setting used if not specified",
          100, 800, GST_OMX_CAMERA_SRC_EXPOSURE_ISO_SENSITIVITY_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_APERTURE,
      g_param_spec_uint ("aperture", "Aperture",
          "F-number for the aperture of the camera, e.g. value of 2 would "
          "mean f/2, setting not applied if not specified",
          1, 16, GST_OMX_CAMERA_SRC_APERTURE_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_FRAME_STABILISATION,
      g_param_spec_boolean ("frame-stabilisation", "Frame stabilisation",
          "Use frame stabilisation",
          GST_OMX_CAMERA_SRC_FRAME_STABILISATION_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_HORIZONTAL_FLIP,
      g_param_spec_boolean ("horizontal-flip", "Horizontal flip",
          "Flip video image horizontally",
          GST_OMX_CAMERA_SRC_HORIZONTAL_FLIP_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_VERTICAL_FLIP,
      g_param_spec_boolean ("vertical-flip", "Vertical flip",
          "Flip video image vertically",
          GST_OMX_CAMERA_SRC_VERTICAL_FLIP_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  element_class->change_state = gst_omx_camera_src_change_state;

  basesrc_class->start = GST_DEBUG_FUNCPTR (gst_omx_camera_src_start);
  basesrc_class->stop = GST_DEBUG_FUNCPTR (gst_omx_camera_src_stop);
  basesrc_class->get_caps = GST_DEBUG_FUNCPTR (gst_omx_camera_src_get_caps);
  basesrc_class->fixate = GST_DEBUG_FUNCPTR (gst_omx_camera_src_fixate);
  basesrc_class->set_caps = GST_DEBUG_FUNCPTR (gst_omx_camera_src_set_caps);
  basesrc_class->decide_allocation = GST_DEBUG_FUNCPTR (gst_omx_camera_src_decide_allocation);

  pushsrc_class->fill = GST_DEBUG_FUNCPTR (gst_omx_camera_src_fill);

  omxsrc_class->cdata.default_src_template_caps = "video/x-raw, "
#ifdef USE_OMX_TARGET_RPI
      "width=(int) [ 64, 1920 ], height=(int) [ 64, 1080 ], "
      "framerate=(fraction) [ 2/1, 30/1 ], format=(string) I420";
#else
      "width = " GST_VIDEO_SIZE_RANGE ", "
      "height = " GST_VIDEO_SIZE_RANGE ", " "framerate = " GST_VIDEO_FPS_RANGE;
#endif

  gst_element_class_set_static_metadata (element_class,
      "OpenMAX Camera video source",
      "Source/Video",
      "Source for raw video data from OpenMAX camera component",
      "Tuomas Jormola <tj@solitudo.net>");
}

static void
gst_omx_camera_src_init (GstOMXCameraSrc * self)
{
  self->camera_configured = FALSE;
  self->video_configured = FALSE;
  GST_OMX_INIT_STRUCT (&self->config.device);
#ifdef USE_OMX_TARGET_RPI
  GST_OMX_INIT_STRUCT (&self->config.sharpness);
#endif
  GST_OMX_INIT_STRUCT (&self->config.gamma);
  GST_OMX_INIT_STRUCT (&self->config.contrast);
  GST_OMX_INIT_STRUCT (&self->config.brightness);
  GST_OMX_INIT_STRUCT (&self->config.saturation);
  GST_OMX_INIT_STRUCT (&self->config.image_filter);
  GST_OMX_INIT_STRUCT (&self->config.color_enhancement);
  GST_OMX_INIT_STRUCT (&self->config.white_balance);
  GST_OMX_INIT_STRUCT (&self->config.exposure_control);
  GST_OMX_INIT_STRUCT (&self->config.exposure_value);
  GST_OMX_INIT_STRUCT (&self->config.frame_stabilisation);
  GST_OMX_INIT_STRUCT (&self->config.mirror);

  self->config.device.nPortIndex = OMX_ALL;
  self->config.device.nU32 = GST_OMX_CAMERA_SRC_DEVICE_NUMBER_DEFAULT;
  self->info = NULL;
  self->omx_buf_info = NULL;
#ifdef USE_OMX_TARGET_RPI
  self->config.sharpness.nPortIndex = OMX_ALL;
  self->config.sharpness.nSharpness = GST_OMX_CAMERA_SRC_SHARPNESS_DEFAULT;
#endif
  self->config.gamma.nPortIndex = OMX_ALL;
  self->config.contrast.nPortIndex = OMX_ALL;
  self->config.contrast.nContrast = GST_OMX_CAMERA_SRC_CONTRAST_DEFAULT;
  self->config.brightness.nPortIndex = OMX_ALL;
  self->config.brightness.nBrightness = GST_OMX_CAMERA_SRC_BRIGHTNESS_DEFAULT;
  self->config.saturation.nPortIndex = OMX_ALL;
  self->config.saturation.nSaturation = GST_OMX_CAMERA_SRC_SATURATION_DEFAULT;
  self->config.image_filter.nPortIndex = OMX_ALL;
  self->config.image_filter.eImageFilter =
    GST_OMX_CAMERA_SRC_IMAGE_FILTER_DEFAULT;
  self->config.color_enhancement.nPortIndex = OMX_ALL;
  self->config.color_enhancement.bColorEnhancement = FALSE;
  self->config.white_balance.nPortIndex = OMX_ALL;
  self->config.white_balance.eWhiteBalControl =
    GST_OMX_CAMERA_SRC_WHITE_BALANCE_MODE_DEFAULT;
  self->config.exposure_control.nPortIndex = OMX_ALL;
  self->config.exposure_control.eExposureControl =
    GST_OMX_CAMERA_SRC_EXPOSURE_CONTROL_MODE_DEFAULT;
  self->config.exposure_value.nPortIndex = OMX_ALL;
  self->config.exposure_value.eMetering =
    GST_OMX_CAMERA_SRC_EXPOSURE_METERING_MODE_DEFAULT;
  self->config.exposure_value.xEVCompensation =
    GST_OMX_CAMERA_SRC_EXPOSURE_VALUE_COMPENSATION_DEFAULT;
  self->config.exposure_value.bAutoSensitivity = TRUE;
  self->config.exposure_value.bAutoAperture = TRUE;
  self->config.frame_stabilisation.nPortIndex = OMX_ALL;
  self->config.frame_stabilisation.bStab =
    GST_OMX_CAMERA_SRC_FRAME_STABILISATION_DEFAULT;
  gst_omx_camera_src_vo_set_hflip (GST_VIDEO_ORIENTATION (self),
      GST_OMX_CAMERA_SRC_HORIZONTAL_FLIP_DEFAULT);
  gst_omx_camera_src_vo_set_vflip (GST_VIDEO_ORIENTATION (self),
      GST_OMX_CAMERA_SRC_VERTICAL_FLIP_DEFAULT);

  gst_base_src_set_format (GST_BASE_SRC (self), GST_FORMAT_TIME);
  gst_base_src_set_live (GST_BASE_SRC (self), TRUE);
}

/* Camera operations */

// TODO: This and the next function are not needed after we dynamically get the
// caps from the hardware itself and not hard code it to I420 here
static GstCaps * gst_omx_camera_src_format_caps (GstOMXCameraSrc * self,
    gint width, gint height, gint fps)
{
  GstCaps *caps;
  gchar *template;
  gint par_n, par_d;

  gst_util_double_to_fraction (((gdouble)width / (gdouble)height),
      &par_n, &par_d);

  template = g_strdup_printf ("video/x-raw, "
    "width=(int) %d, height=(int) %d, "
    "framerate=(fraction) %d/1, pixel-aspect-ratio=(fraction) %d/%d, "
    "format=(string) I420"
    , width, height, fps, par_n, par_d);

  caps = gst_caps_from_string (template);

  g_free (template);

  return caps;
}

static GstCaps * gst_omx_camera_src_default_caps (GstOMXCameraSrc * self)
{
  return gst_omx_camera_src_format_caps (self,
    GST_OMX_CAMERA_SRC_WIDTH_DEFAULT,
    GST_OMX_CAMERA_SRC_HEIGHT_DEFAULT,
    GST_OMX_CAMERA_SRC_FRAMERATE_DEFAULT);
}

static gboolean gst_omx_camera_src_config_color_enhancement_set_from_string (OMX_CONFIG_COLORENHANCEMENTTYPE * ce,
    const gchar * string)
{
  gint u, v;

  if (sscanf (string, "%d:%d", &u, &v) != 2)
    return FALSE;

  if (u < 0 || u > 255 || v < 0 || v > 255)
    return FALSE;

  ce->nCustomizedU = u;
  ce->nCustomizedV = v;

  return TRUE;
}

static gboolean
gst_omx_camera_src_is_capture_active (GstOMXCameraSrc * self)
{
  OMX_CONFIG_PORTBOOLEANTYPE capture;
  OMX_ERRORTYPE err;

  // Capture on
  GST_OMX_INIT_STRUCT (&capture);
  capture.nPortIndex = self->camera_out_port->index;
  err = gst_omx_component_get_parameter (self->camera,
      OMX_IndexConfigPortCapturing, &capture);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while checking video capture state on "
        "camera output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    return FALSE;
  }

  return capture.bEnabled;
}

/* LIVE_LOCK needs to be hold */
static gboolean
gst_omx_camera_src_open_camera_unlocked (GstOMXCameraSrc * self)
{
  GstOMXSrcClass *klass = GST_OMX_SRC_GET_CLASS (self);
  gboolean res;
  gint in_port_index, out_port_index;

  GST_DEBUG_OBJECT (self, "Opening camera");

  self->camera =
      gst_omx_component_new (GST_OBJECT_CAST (self), klass->cdata.core_name,
        klass->cdata.component_name, klass->cdata.component_role,
        klass->cdata.hacks);

  if (!self->camera) {
    GST_ERROR_OBJECT (self, "Error while creating camera component");
    res = FALSE;
    goto done;
  }

  if (gst_omx_component_get_state (self->camera, GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_ERROR_OBJECT (self, "Camera state is not loaded");
    res = FALSE;
    goto done;
  }

  if (!gst_omx_component_add_all_ports (self->camera)) {
    GST_ERROR_OBJECT (self, "Error while adding ports to camera");
    res = FALSE;
    goto done;
  }

  if (!gst_omx_component_all_ports_set_enabled (self->camera, FALSE)) {
    GST_ERROR_OBJECT (self, "Error while disabling camera ports");
    res = FALSE;
    goto done;
  }

  in_port_index = klass->cdata.in_port_index;
  out_port_index = klass->cdata.out_port_index;

  if (in_port_index == -1 || out_port_index == -1) {
    OMX_PORT_PARAM_TYPE param;
    OMX_ERRORTYPE err;

    GST_OMX_INIT_STRUCT (&param);

    err =
        gst_omx_component_get_parameter (self->camera, OMX_IndexParamVideoInit,
        &param);
    if (err != OMX_ErrorNone) {
      GST_WARNING_OBJECT (self, "Couldn't get port information: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      in_port_index = 0;
      out_port_index = 1;
    } else {
      GST_DEBUG_OBJECT (self, "Detected %u ports, starting at %u",
          (guint) param.nPorts, (guint) param.nStartPortNumber);
      in_port_index = param.nStartPortNumber + 0;
      out_port_index = param.nStartPortNumber + 1;
    }
  }

  self->camera_in_port = gst_omx_component_get_port (self->camera,
      in_port_index);
  self->camera_out_port = gst_omx_component_get_port (self->camera,
      out_port_index);

  if (!self->camera_in_port || !self->camera_out_port) {
    GST_ERROR_OBJECT (self, "Error while detecting camera ports");
    res = FALSE;
    goto done;
  }

  // TODO: On RPi, open the camera preview port

  // TODO: On RPi, open encoder and null sink components and their ports

  res = TRUE;

done:
  GST_DEBUG_OBJECT (self, "Opened camera, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  return res;
}

/* LIVE_LOCK needs to be hold */
static gboolean gst_omx_camera_src_configure_camera_unlocked (GstOMXCameraSrc
    * self)
{
  gboolean res;
  OMX_ERRORTYPE err;

  GST_DEBUG_OBJECT (self, "Configuring camera");

  while (!self->camera) {
    GST_DEBUG_OBJECT (self, "Camera not opened, waiting");
    GST_LIVE_WAIT (self);
  }

  if (gst_omx_component_get_state (self->camera, GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_ERROR_OBJECT (self, "Camera not in loaded state");
    res = FALSE;
    goto done;
  }

  /*
  // TODO: On RPi, do something like this
  // Request a callback to be made when OMX_IndexParamCameraDeviceNumber is
  // changed signaling that the camera device is ready for use.
  // Reference: http://home.nouwen.name/RaspberryPi/documentation/ilcomponents/camera.html
  OMX_CONFIG_REQUESTCALLBACKTYPE cbtype;
  GST_OMX_INIT_STRUCT (&cbtype);
  cbtype.nPortIndex = OMX_ALL;
  cbtype.nIndex = OMX_IndexParamCameraDeviceNumber;
  cbtype.bEnable = OMX_TRUE;
  err = gst_omx_component_set_config (self->camera,
      OMX_IndexConfigRequestCallback, &self->cbtype);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error setting device change callback: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  */

  // TODO: On RPi setting this parameter will trigger
  // an event (eEvent == OMX_EventParamOrConfigChanged,
  // nData2 == OMX_IndexParamCameraDeviceNumber which
  // the GstOMX event handler should trap and inform us about it
  // somehow. This should probably be implemented in gtsomx.c.
  err = gst_omx_component_set_parameter (self->camera,
      OMX_IndexParamCameraDeviceNumber, &self->config.device);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error setting parameter device number %u: %s (0x%08x)",
        self->config.device.nU32,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

#ifdef USE_OMX_TARGET_RPI
  err = gst_omx_component_set_config (self->camera,
      OMX_IndexConfigCommonSharpness, &self->config.sharpness);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config sharpness %d: %s (0x%08x)",
        self->config.sharpness.nSharpness,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#endif

  if (self->config.gamma.nGamma != 0) {
    err = gst_omx_component_set_config (self->camera,
        OMX_IndexConfigCommonGamma, &self->config.gamma);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while setting config gamma %d: %s (0x%08x)",
          self->config.gamma.nGamma,
          gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
    }
  }

  err = gst_omx_component_set_config (self->camera,
      OMX_IndexConfigCommonContrast, &self->config.contrast);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config contrast %d: %s (0x%08x)",
        self->config.contrast.nContrast,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->camera,
      OMX_IndexConfigCommonBrightness, &self->config.brightness);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config brightness %u: %s (0x%08x)",
        self->config.brightness.nBrightness,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->camera,
      OMX_IndexConfigCommonSaturation, &self->config.saturation);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config saturation %d: %s (0x%08x)",
        self->config.saturation.nSaturation,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->camera,
      OMX_IndexConfigCommonImageFilter, &self->config.image_filter);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config image filter %d: %s (0x%08x)",
        self->config.image_filter.eImageFilter,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->camera,
      OMX_IndexConfigCommonColorEnhancement, &self->config.color_enhancement);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config color enhancement %c:%c: %s (0x%08x)",
        self->config.color_enhancement.nCustomizedU,
        self->config.color_enhancement.nCustomizedV,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->camera,
      OMX_IndexConfigCommonWhiteBalance, &self->config.white_balance);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config white balance %d: %s (0x%08x)",
        self->config.white_balance.eWhiteBalControl,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->camera,
      OMX_IndexConfigCommonExposure, &self->config.exposure_control);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config exposure control %d: %s (0x%08x)",
        self->config.exposure_control.eExposureControl,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->camera,
        OMX_IndexConfigCommonExposure, &self->config.exposure_value);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config exposure value, "
            "metering mode %d, exposure value compensation %d, "
            "exposure ISO sensitivity auto %d value %u, "
            "aperture auto %d value %u: %s (0x%08x)",
        self->config.exposure_value.eMetering,
        self->config.exposure_value.xEVCompensation,
        self->config.exposure_value.bAutoSensitivity,
        self->config.exposure_value.nSensitivity,
        self->config.exposure_value.bAutoAperture,
        self->config.exposure_value.nApertureFNumber,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->camera,
        OMX_IndexConfigCommonFrameStabilisation,
        &self->config.frame_stabilisation);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config frame stabilistaion %d: %s (0x%08x)",
        self->config.frame_stabilisation.bStab,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  self->config.mirror.nPortIndex = self->camera_out_port->index;
  err = gst_omx_component_set_config (self->camera,
      OMX_IndexConfigCommonMirror, &self->config.mirror);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config mirror %d: %s (0x%08x)",
        self->config.mirror.eMirror,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  // TODO: On RPi, configure the camera preview output port (70)
  // and encoder output port port definitions to match the settings of the
  // camera video output port (71)

  self->camera_configured = TRUE;
  res = TRUE;

done:
  GST_DEBUG_OBJECT (self, "Camera configured, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  return res;
}

/* LIVE_LOCK needs to be hold */
static gboolean gst_omx_camera_src_configure_video_unlocked (GstOMXCameraSrc
    * self)
{

  gboolean res;
  gdouble fps;
  OMX_PARAM_PORTDEFINITIONTYPE port_def;
  OMX_CONFIG_FRAMERATETYPE framerate;
  OMX_ERRORTYPE err;

  while (!self->camera) {
    GST_DEBUG_OBJECT (self, "Camera not opened, waiting");
    GST_LIVE_WAIT (self);
  }

  while (!self->info) {
    GST_DEBUG_OBJECT (self, "Video capabilities not negotiated, waiting");
    GST_LIVE_WAIT (self);
  }

  // TODO: Should we support setting new caps in the fly? That would mean
  // stopping the capture, configure and then start capture if we're playing.
  // But for now, just flat-out refuse that.

  if (gst_omx_component_get_state (self->camera, GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_ERROR_OBJECT (self, "Camera not in loaded state");
    res = FALSE;
    goto done;
  }

  gst_util_fraction_to_double (GST_VIDEO_INFO_FPS_N (self->info),
      GST_VIDEO_INFO_FPS_D (self->info), &fps);

  GST_DEBUG_OBJECT (self, "Configuring video %dx%d (%d/%d) @ %.2f",
      GST_VIDEO_INFO_WIDTH (self->info),
      GST_VIDEO_INFO_HEIGHT (self->info),
      GST_VIDEO_INFO_PAR_N (self->info),
      GST_VIDEO_INFO_PAR_D (self->info),
      fps);

  err = gst_omx_port_update_port_definition (self->camera_out_port, NULL);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while getting port definition for "
        "camera output port %u: %s (0x%08x)",
        self->camera_out_port->index,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  port_def = self->camera_out_port->port_def;
  port_def.format.video.nFrameWidth = GST_VIDEO_INFO_WIDTH (self->info);
  port_def.format.video.nFrameHeight = GST_VIDEO_INFO_HEIGHT (self->info);
  port_def.format.video.xFramerate = ((OMX_U32)fps) << 16;
  if (port_def.nBufferAlignment)
    port_def.format.video.nStride =
        (port_def.format.video.nFrameWidth + port_def.nBufferAlignment - 1) &
        (~(port_def.nBufferAlignment - 1));
  else
    /* safe (?) default */
    port_def.format.video.nStride =
        GST_ROUND_UP_4 (port_def.format.video.nFrameWidth);

  err = gst_omx_port_update_port_definition (self->camera_out_port,
      &port_def);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error setting parameter port definition for camera video "
        "output port %u: %s (0x%08x)",
        port_def.nPortIndex,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  GST_OMX_INIT_STRUCT (&framerate);
  framerate.nPortIndex = port_def.nPortIndex;
  err = gst_omx_component_get_config (self->camera,
        OMX_IndexConfigVideoFramerate, &framerate);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error getting config framerate for camera video "
        "output port %u: %s (0x%08x)",
        self->camera_out_port->index,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  framerate.xEncodeFramerate = port_def.format.video.xFramerate;
  err =
      gst_omx_component_set_config (self->camera,
      OMX_IndexConfigVideoFramerate, &framerate);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config framerate %d: %s (0x%08x)",
        framerate.xEncodeFramerate >> 16,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  // TODO: On RPi, configure camera preview port defintion with the settings
  // from camera video output port definition. Also configure the encoder
  // output with the selected codec if the downstream caps requires us to
  // pass the camera video data through the encoder.

  // TODO: On RPi, probably here is the spot where we would tunnel the camera
  // preview output to null sink input and camera video output to encoder
  // input if that's needed, see just above. See this for justification
  // why preview port should be opened and directed to null sink:
  // The preview display is optional, but can be used full screen or directed
  // to a specific rectangular area on the display. If preview is disabled,
  // the null_sink component is used to 'absorb' the preview frames. It is
  // necessary for the camera to produce preview frames even if not required
  // for display, as they are used for calculating exposure and white balance
  // settings. http://www.raspberrypi.org/wp-content/uploads/2013/07/RaspiCam-Documentation.pdf

  self->video_configured = TRUE;
  res = TRUE;

done:
  GST_DEBUG_OBJECT (self, "Video configured, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  return res;
}

/* LIVE_LOCK needs to be hold */
static gboolean
gst_omx_camera_src_enable_capturing_unlocked (GstOMXCameraSrc * self)
{
  gboolean res;
  OMX_ERRORTYPE err;

  GST_DEBUG_OBJECT (self, "Enabling video capture");

  while (!self->camera) {
    GST_DEBUG_OBJECT (self, "Camera not opened, waiting");
    GST_LIVE_WAIT (self);
  }

  while (!self->video_configured) {
    GST_DEBUG_OBJECT (self, "Video not configured, waiting");
    GST_LIVE_WAIT (self);
  }

  if (gst_omx_component_get_state (self->camera, GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_ERROR_OBJECT (self, "Camera not in loaded state");
    res = FALSE;
    goto done;
  }

  // TODO: Implement proper error handling i.e. roll back all the steps so far
  // done if an error is encoutered so that the state we're going to return is
  // the same as when entering this function.

  // State to idle
  err = gst_omx_component_set_state (self->camera, OMX_StateIdle);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting camera state "
        "to idle: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->camera,
      OMX_StateIdle, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Camera didn't switch to idle state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // TODO: On RPi, handle encoder and null sink

  // Enable ports
  err = gst_omx_port_set_enabled (self->camera_in_port, TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while enabling "
        "camera input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_enabled (self->camera_out_port, TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while enabling "
        "camera output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_flushing (self->camera_in_port, 1 * GST_SECOND, FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while switching flush off "
        "on camera input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_flushing (self->camera_out_port, 1 * GST_SECOND, FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while switching flush off "
        "on camera output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // TODO: On RPi, handle camera preview output, null sink input and encoder
  // input and output ports.

  // Allocate buffers
  err = gst_omx_port_allocate_buffers (self->camera_in_port);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while allocating buffers for "
        "camera input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_allocate_buffers (self->camera_out_port);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while allocating buffers for "
        "camera output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // TODO: On RPi, don't allocate buffers on camera video output port but on
  // encoder output port if encoder is being used.

  // State to executing
  err = gst_omx_component_set_state (self->camera, OMX_StateExecuting);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting camera state to "
        "executing: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->camera,
      OMX_StateExecuting, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Camera didn't switch to executing "
        "state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // TODO: On RPi, handle encoder and null sink

  res = TRUE;

done:
  GST_DEBUG_OBJECT (self, "Video capturing enabled, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  return res;
}

static gboolean
gst_omx_camera_src_enable_capturing (GstOMXCameraSrc * self)
{
  gboolean res;

  GST_LIVE_LOCK (self);

  res = gst_omx_camera_src_enable_capturing_unlocked (self);

  GST_LIVE_UNLOCK (self);

  return res;
}

/* LIVE_LOCK needs to be hold */
static gboolean
gst_omx_camera_src_start_capturing_unlocked (GstOMXCameraSrc * self)
{
  gboolean res;
  OMX_CONFIG_PORTBOOLEANTYPE capture;
  OMX_ERRORTYPE err;

  // TODO: Implement proper error handling i.e. roll back all the steps so far
  // done if an error is encoutered so that the state we're going to return is
  // the same as when entering this function.

  GST_DEBUG_OBJECT (self, "Starting video capture");

  while (!self->camera) {
    GST_DEBUG_OBJECT (self, "Camera not opened, waiting");
    GST_LIVE_WAIT (self);
  }

  while (gst_omx_component_get_state (self->camera, GST_CLOCK_TIME_NONE) !=
      OMX_StateExecuting) {
    GST_DEBUG_OBJECT (self, "Camera not in executing state, waiting");
    GST_LIVE_WAIT (self);
  }

  // Capture on
  GST_OMX_INIT_STRUCT (&capture);
  capture.nPortIndex = self->camera_out_port->index;
  capture.bEnabled = OMX_TRUE;
  err = gst_omx_component_set_parameter (self->camera,
      OMX_IndexConfigPortCapturing, &capture);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while enabling video capture on "
        "camera output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_port_populate (self->camera_out_port);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while populating camera output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // TODO: On RPi, populate encoder output instead if encoder is being used

  res = TRUE;

done:
  GST_DEBUG_OBJECT (self, "Video capturing started, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  return res;
}

static gboolean
gst_omx_camera_src_start_capturing (GstOMXCameraSrc * self)
{
  gboolean res;

  GST_LIVE_LOCK (self);

  res = gst_omx_camera_src_start_capturing_unlocked (self);

  GST_LIVE_UNLOCK (self);

  return res;
}

/* LIVE_LOCK needs to be hold */
static gboolean
gst_omx_camera_src_stop_capturing_unlocked (GstOMXCameraSrc * self)
{
  gboolean res;
  OMX_CONFIG_PORTBOOLEANTYPE capture;
  OMX_ERRORTYPE err;

  // TODO: Implement proper error handling i.e. roll back all the steps so far
  // done if an error is encoutered so that the state we're going to return is
  // the same as when entering this function.

  GST_DEBUG_OBJECT (self, "Stopping video capture");

  if (!self->camera) {
    GST_DEBUG_OBJECT (self, "Camera not opened, return");
    res = TRUE;
    goto done;
  }

  if (gst_omx_component_get_state (self->camera, GST_CLOCK_TIME_NONE) !=
      OMX_StateExecuting) {
    GST_DEBUG_OBJECT (self, "Camera not in executing state, return");
    res = TRUE;
    goto done;
  }

  // Capture off
  GST_OMX_INIT_STRUCT (&capture);
  capture.nPortIndex = self->camera_out_port->index;
  capture.bEnabled = OMX_FALSE;
  err = gst_omx_component_set_parameter (self->camera,
      OMX_IndexConfigPortCapturing, &capture);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while disabling video capture on "
        "camera output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  // Flush buffers
  err = gst_omx_port_set_flushing (self->camera_in_port,
      1 * GST_SECOND, TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while flushing camera input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_flushing (self->camera_out_port,
      1 * GST_SECOND, TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while flushing camera output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // TODO: On RPi, handle other ports also

done:
  GST_DEBUG_OBJECT (self, "Video capturing stopped, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  return res;
}

static gboolean
gst_omx_camera_src_stop_capturing (GstOMXCameraSrc * self)
{
  gboolean res;

  GST_LIVE_LOCK (self);

  res = gst_omx_camera_src_stop_capturing_unlocked (self);

  GST_LIVE_UNLOCK (self);

  return res;
}

/* LIVE_LOCK needs to be hold */
static gboolean
gst_omx_camera_src_disable_capturing_unlocked (GstOMXCameraSrc * self)
{
  gboolean res;
  OMX_ERRORTYPE err;

  GST_DEBUG_OBJECT (self, "Disabling video capture");

  if (!self->camera) {
    GST_DEBUG_OBJECT (self, "Camera not opened, return");
    res = TRUE;
    goto done;
  }

  if (gst_omx_component_get_state (self->camera, GST_CLOCK_TIME_NONE) !=
      OMX_StateExecuting) {
    GST_DEBUG_OBJECT (self, "Camera not in executing state, return");
    res = TRUE;
    goto done;
  }

  // Disable ports
  err = gst_omx_port_set_enabled (self->camera_in_port, FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while disabling "
        "camera input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_enabled (self->camera_out_port, FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while disabling "
        "camera output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // TODO: On RPi, handle other ports also

  // Free buffers
  err = gst_omx_port_deallocate_buffers (self->camera_in_port);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while dellocating "
        "camera input port buffers: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_deallocate_buffers (self->camera_out_port);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while dellocating "
        "camera output port buffers: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // TODO: On RPi, use encoder output port if encoder is being used

  // State to idle
  err = gst_omx_component_set_state (self->camera, OMX_StateIdle);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting camera state "
        "to idle: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->camera,
      OMX_StateIdle, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Camera didn't switch to idle state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // TODO: On RPi, handle the other components also

  // State to loaded
  err = gst_omx_component_set_state (self->camera, OMX_StateLoaded);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting camera state "
        "to loaded: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->camera,
      OMX_StateLoaded, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Camera didn't switch "
        "to loaded state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // TODO: On RPi, handle the other components also

done:
  GST_DEBUG_OBJECT (self, "Video capturing disabled, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  return res;
}

static gboolean
gst_omx_camera_src_disable_capturing (GstOMXCameraSrc * self)
{
  gboolean res;

  GST_LIVE_LOCK (self);

  res = gst_omx_camera_src_disable_capturing_unlocked (self);

  GST_LIVE_UNLOCK (self);

  return res;
}

/* LIVE_LOCK needs to be hold */
static gboolean
gst_omx_camera_src_close_camera_unlocked (GstOMXCameraSrc *self)
{
  gboolean res = TRUE;

  GST_DEBUG_OBJECT (self, "Closing camera");

  if (!self->camera) {
    GST_DEBUG_OBJECT (self, "Camera not opened, return");
    goto done;
  }

  while (gst_omx_component_get_state (self->camera, GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_DEBUG_OBJECT (self, "Camera not in loaded state, waiting");
    GST_LIVE_WAIT (self);
  }

  self->camera_in_port = NULL;
  self->camera_out_port = NULL;
  if (self->camera)
    gst_omx_component_free (self->camera);
  self->camera = NULL;
  // TODO: On RPi, handle the other components and ports also

  self->camera_configured = FALSE;
  self->video_configured = FALSE;

done:
  GST_DEBUG_OBJECT (self, "Closed camera, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  return res;
}

static gboolean
gst_omx_camera_src_close_camera (GstOMXCameraSrc * self)
{
  gboolean res;

  GST_LIVE_LOCK (self);

  res = gst_omx_camera_src_close_camera_unlocked (self);

  GST_LIVE_UNLOCK (self);

  return res;
}

static
gboolean gst_omx_camera_src_poll_frame (GstOMXCameraSrc *self,
    GstVideoFrame *frame)
{
  GstOMXBuffer *buf;
  GstOMXAcquireBufferReturn acq_return;
  gint max_spans_y = GST_VIDEO_INFO_HEIGHT (self->omx_buf_info);
  gint max_spans_uv = max_spans_y / 2;
  gint valid_spans_y, valid_spans_uv, max_spans, valid_spans;
  gint dst_offset, src_offset;
  gint buf_bytes_copied, span_size_bytes = 0, frame_bytes_copied = 0;
  gint buf_slice_height, buf_discard_spans, buf_num = 0;
  gint frame_height = GST_VIDEO_INFO_HEIGHT (self->info);
  gint frame_size = GST_VIDEO_INFO_SIZE (self->info);
  gint i;
  gboolean eof = FALSE;
  OMX_ERRORTYPE err;

  // TODO: Implement proper error handling

  // TODO: On RPi, this needs to be considered differently as we might have
  // encoded frames coming from the encoder and not raw frames from the camera.
  // Maybe different fill functions for uncompressed/compressed cases?
  // A union type for data exchange of GstVideoFrame for uncompressed and
  // GstVideoCodecFrame for compressed maybe? Or maybe something else, we'll
  // figure it out later.

  // We're constructing an I420 frame here. The frame content is fragmented
  // into several OMX buffers, each port_def.format.video.nStride wide and
  // port_def.format.video.nSliceHeight tall. See description of
  // OMX_COLOR_FormatYUV420PackedPlanar in OpenMAX IL 1.1.2 spec. p. 198-201.
  // We shall loop until we get enough buffers so that the whole frame
  // has been fully received, hopefully.
  buf_slice_height = self->camera_out_port->port_def.format.video.nSliceHeight;
  buf_discard_spans =
               (buf_slice_height && (frame_height % buf_slice_height))
             ? (buf_slice_height - (frame_height % buf_slice_height))
             : 0;

  while (!eof) {

    acq_return = gst_omx_port_acquire_buffer (self->camera_out_port, &buf);

    if (acq_return == GST_OMX_ACQUIRE_BUFFER_OK && 
        buf->omx_buf->nFlags & OMX_BUFFERFLAG_DATACORRUPT) {
      acq_return = GST_OMX_ACQUIRE_BUFFER_ERROR;
    }

    eof = buf->omx_buf->nFlags & OMX_BUFFERFLAG_ENDOFFRAME;

    switch (acq_return) {
      case GST_OMX_ACQUIRE_BUFFER_FLUSHING:
      case GST_OMX_ACQUIRE_BUFFER_RECONFIGURE:
      case GST_OMX_ACQUIRE_BUFFER_EOS:
      case GST_OMX_ACQUIRE_BUFFER_ERROR:
        goto done;
      case GST_OMX_ACQUIRE_BUFFER_OK:
        break;
    }

    valid_spans_y = max_spans_y - (eof ? buf_discard_spans : 0);
    valid_spans_uv = valid_spans_y / 2;

    if (buf->omx_buf->nFilledLen != GST_VIDEO_INFO_SIZE (self->omx_buf_info)) {
      GST_ERROR_OBJECT (self, "Received an unexpected amount of data "
          "in the OMX buffer, %d vs. %d bytes",
          buf->omx_buf->nFilledLen, GST_VIDEO_INFO_SIZE (self->omx_buf_info));
      return FALSE;
    }

    buf_bytes_copied = 0;
    for (i = 0; i < GST_VIDEO_FRAME_N_COMPONENTS (frame); i++) {
      max_spans   = (i == 0 ? max_spans_y   : max_spans_uv);
      valid_spans = (i == 0 ? valid_spans_y : valid_spans_uv);
      dst_offset = buf_num * max_spans *
        GST_VIDEO_FRAME_PLANE_STRIDE (frame, i);
      src_offset = GST_VIDEO_INFO_PLANE_OFFSET (self->omx_buf_info, i);
      span_size_bytes = GST_VIDEO_INFO_PLANE_STRIDE (self->omx_buf_info, i) * valid_spans;
      buf_bytes_copied += span_size_bytes;
      if (buf_bytes_copied > GST_VIDEO_INFO_SIZE (self->omx_buf_info)) {
        GST_ERROR_OBJECT (self, "Wanted to copy too much from the OMX buffer, "
          "%d vs. %d bytes!",
          buf_bytes_copied, GST_VIDEO_INFO_SIZE (self->omx_buf_info));
        return FALSE;
      }
      frame_bytes_copied += span_size_bytes;
      if (frame_bytes_copied > frame_size) {
        GST_ERROR_OBJECT (self, "Buffers received from camera won't "
          "fit in the frame, %d vs. %d bytes!", frame_bytes_copied, frame_size);
        return FALSE;
      }
      memcpy ((gchar *)frame->data[i] + dst_offset,
        buf->omx_buf->pBuffer + buf->omx_buf->nOffset + src_offset,
        span_size_bytes);
    }

done:
    buf_num++;

    GST_LOG_OBJECT (self, "Acquired OMX buffer %d %d bytes, flags %d, "
        "%d/%d frame bytes, plane spans: Y:%d/%d, U/V: %d/%d",
        buf_num, buf->omx_buf->nFilledLen, buf->omx_buf->nFlags,
        frame_bytes_copied, frame_size,
        valid_spans_y, max_spans_y, valid_spans_uv, max_spans_uv);

    err = gst_omx_port_release_buffer (self->camera_out_port, buf);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while releasing buffer: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      return FALSE;
    }

    if (eof) {
      GST_DEBUG_OBJECT (self, "Full frame received");
    }
  }

  if (frame_bytes_copied != GST_VIDEO_INFO_SIZE (self->info)) {
      GST_ERROR_OBJECT (self, "Didn't receive enough data from buffers to "
        "fill the frame, %d vs. %d bytes!",
        frame_bytes_copied, GST_VIDEO_INFO_SIZE (self->info));
      return FALSE;
  }

  return TRUE;
}

static gchar * gst_omx_camera_src_config_color_enhancement_to_string (const OMX_CONFIG_COLORENHANCEMENTTYPE * ce)
{
  return g_strdup_printf ("%d:%d", ce->nCustomizedU, ce->nCustomizedV);
}

static void gst_omx_camera_src_update_mirror_mode (GstOMXCameraSrc * self)
{
  self->config.mirror.eMirror = OMX_MirrorNone;

  if (self->config.horizontal_flip && !self->config.vertical_flip) {
    self->config.mirror.eMirror = OMX_MirrorHorizontal;
  } else if (!self->config.horizontal_flip && self->config.vertical_flip) {
    self->config.mirror.eMirror = OMX_MirrorVertical;
  } else if (self->config.horizontal_flip && self->config.vertical_flip) {
    self->config.mirror.eMirror = OMX_MirrorBoth;
  }
}

/* Virtual Methods */

static GstFlowReturn
gst_omx_camera_src_fill (GstPushSrc * push_src, GstBuffer * buffer)
{
  GstOMXCameraSrc *self;
  GstClockTime next_time;
  GstVideoFrame frame;
  gboolean res;

  self = GST_OMX_CAMERA_SRC (push_src);

  if (G_UNLIKELY (GST_VIDEO_INFO_FORMAT (self->info) ==
          GST_VIDEO_FORMAT_UNKNOWN))
    return GST_FLOW_NOT_NEGOTIATED;

  GST_LOG_OBJECT (self,
      "Creating buffer from pool for frame %d", (gint) self->n_frames);

  while (!self->camera) {
    GST_DEBUG_OBJECT (self, "Camera not opened, waiting");
    GST_LIVE_WAIT (self);
  }

  while (gst_omx_component_get_state (self->camera, GST_CLOCK_TIME_NONE) !=
      OMX_StateExecuting) {
    GST_DEBUG_OBJECT (self, "Camera not in executing state, waiting");
    GST_LIVE_WAIT (self);
  }

  while (!gst_omx_camera_src_is_capture_active (self)) {
    GST_DEBUG_OBJECT (self, "Camera capturing, waiting");
    GST_LIVE_WAIT (self);
  }

  // Get the frame data
  // TODO: On RPi, this іs going to be more complex when encoder is being used
  if (!gst_video_frame_map (&frame, self->info, buffer, GST_MAP_WRITE)) {
    GST_DEBUG_OBJECT (self, "Invalid frame");
    return GST_FLOW_OK;
  }

  res = gst_omx_camera_src_poll_frame (self, &frame);

  gst_video_frame_unmap (&frame);

  // Set flags
  GST_BUFFER_FLAG_SET (buffer, GST_BUFFER_FLAG_LIVE);
  if (res) {
    GST_BUFFER_FLAG_SET (buffer,  GST_BUFFER_FLAG_MARKER);
  } else {
    GST_BUFFER_FLAG_SET (buffer, GST_BUFFER_FLAG_CORRUPTED |
      GST_BUFFER_FLAG_DROPPABLE);
    // TODO: Should we also empty the content of the frame? i.e.
    gst_buffer_memset (buffer, 0, 0, gst_buffer_get_size (buffer));
  }

  // Set timestamps
  GST_BUFFER_DTS (buffer) =
      self->accum_rtime + self->running_time;
  GST_BUFFER_PTS (buffer) = GST_BUFFER_DTS (buffer);

  gst_object_sync_values (GST_OBJECT (push_src), GST_BUFFER_DTS (buffer));

  // Set offset
  GST_BUFFER_OFFSET (buffer) = self->accum_frames + self->n_frames;
  GST_BUFFER_OFFSET_END (buffer) = GST_BUFFER_OFFSET (buffer) + 1;
  self->n_frames++;
  if (GST_VIDEO_INFO_FPS_N (self->info) > 0) {
    next_time = gst_util_uint64_scale_int (self->n_frames * GST_SECOND,
        GST_VIDEO_INFO_FPS_D (self->info), GST_VIDEO_INFO_FPS_N (self->info));
    GST_BUFFER_DURATION (buffer) = next_time - self->running_time;
  } else {
    next_time = 0;
    /* NONE means forever */
    GST_BUFFER_DURATION (buffer) = GST_CLOCK_TIME_NONE;
  }

  self->running_time = next_time;

  GST_LOG_OBJECT (self, "frame:%" G_GINT64_FORMAT " offset:%"
      G_GINT64_FORMAT " pts:%" GST_TIME_FORMAT " = accumulated %" GST_TIME_FORMAT
      " + running time: %" GST_TIME_FORMAT,
        self->n_frames,
        GST_BUFFER_OFFSET (buffer),
        GST_TIME_ARGS (GST_BUFFER_PTS (buffer)),
        GST_TIME_ARGS (self->accum_rtime),
        GST_TIME_ARGS (self->running_time));

  return GST_FLOW_OK;
}

static gboolean
gst_omx_camera_src_start (GstBaseSrc * base_src)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (base_src);
  gboolean res;

  GST_LIVE_LOCK (self);

  GST_DEBUG_OBJECT (self, "Starting");

  self->running_time = 0;
  self->n_frames = 0;
  self->accum_frames = 0;
  self->accum_rtime = 0;

  res = gst_omx_camera_src_open_camera_unlocked (self);
  if (!res)
    goto done;

  res = gst_omx_camera_src_configure_camera_unlocked (self);

done:
  GST_LIVE_UNLOCK (self);

  // TODO: Not calling gst_base_src_start_complete() because it seems to be
  // causing a deadlock in GstBaseSrc when testing on my development system
  // i.e. a RPi clocked at default 700MHz and console access over an ssh
  // connection in 100Mbit/s ethernet LAN. My understanding about this issue is
  // the following. Line numbers refer to the file libs/gst/base/gstbasesrc.c
  // in gstreamer-1.2.0.
  //
  // The state is PAUSED when this i.e. GstBaseSrc->start() is being executed.
  // In gst_base_src_start_complete(), line 3317 calls
  // gst_base_src_perform_seek() in our case (mode == GST_PAD_MODE_PUSH at line
  // 3308 is TRUE as we're based on GstPushSrc). gst_base_src_perform_seek()
  // starts the task function gst_base_src_loop() at line 1680. Ok, the loop
  // starts rolling and gst_base_src_get_range() gets called at line 2665. At
  // the lines 2349-2351 gst_base_src_get_range() sees that we're live since
  // we've called gst_base_src_set_live(TRUE) in gst_omx_camera_src_init() and
  // that we're not running (GstBaseSrc->live_running == FALSE) and decides to
  // go sleeping by calling gst_base_src_wait_playing(). But remember we're in
  // state PAUSED, right? How could we ever be running in PAUSED state? So this
  // check makes no sense in this context. And evern worse, there's no way
  // GstBaseSrc->live_running ever gets TRUE if GstBaseSrc is now blocking
  // itself in PAUSED state as gst_base_src_change_state() won't ever be called
  // due to the block and thus transition from PAUSED to PLAYING never happens.
  // You only can set GstBaseSrc->live_running = TRUE when state is PLAYING,
  // right?
  //
  // And there you have it, a deadlock in GstBaseSrc. However, this isn't
  // always triggered on my RPi based system. Run with no debug messages
  // printed to the console, and the state will change from PAUSED to PLAYING
  // before the GstBaseSrc->is_live_running check is done at line 2350:
  // 
  // gst-launch-1.0 -e omxcamerasrc ! video/x-raw,width=480,height=270,framerate=5/1 ! testsink
  // 
  // The same applies if you run with lots of debugging messages slowing things down:
  //
  // gst-launch-1.0 --gst-debug *:6 -e omxcamerasrc ! video/x-raw,width=480,height=270,framerate=5/1 ! testsink
  //
  // Now reduce the debugging output but still have some messages being
  // generated and slowing things down just the right amount and I had about
  // 80% chance of triggering the deadlock. But still not 100%, it would work
  // every now and then but not very often.
  //
  // gst-launch-1.0 --gst-debug *omx*:6,*src*:6 -e omxcamerasrc ! video/x-raw,width=480,height=270,framerate=5/1 ! testsink
  //
  // Is this analysis correct and there's a bug in basesrc.c or am I just
  // stupid?
  //gst_base_src_start_complete (base_src, (res ? GST_FLOW_OK : GST_FLOW_ERROR));

  GST_DEBUG_OBJECT (self, "Started, %s", (res ? "ok" : "failing"));

  return res;
}

static gboolean
gst_omx_camera_src_stop (GstBaseSrc * base_src)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (base_src);
  gboolean res;

  GST_DEBUG_OBJECT (self, "Stopping");

  res = gst_omx_camera_src_close_camera (self);

  GST_DEBUG_OBJECT (self, "Stopped, %s", (res ? "ok" : "failing"));

  return res;
}

static GstCaps * gst_omx_camera_src_get_caps (GstBaseSrc * base_src,
    GstCaps * filter)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (base_src);
  GstCaps *caps;

  GST_LIVE_LOCK (self);

  // TODO: Probe caps from camera and video encoder component on RPi
  // Something like this:
  // 1.
  // formats_camera = gst_omx_port_probe_format (self, self->camera_out_port);
  // #ifdef USE_OMX_TARGET_RPI
  // formats_encoder = gst_omx_port_probe_format (self, self->encoder_out_port);
  // #endif
  // gst_omx_port_probe_format() would use OMX_VIDEO_PARAM_PORTFORMATTYPE
  // structure and OMX_IndexParamVideoPortFormat parameter and filter
  // out those formats we just can't support and map the
  // returned eCompressionFormat's to something more Gstreamery.
  // GstVideoFormat would be fine for uncompressed formats but what about
  // the compressed ones line H.264 and MJPEG that we would like also
  // support?
  // 2.
  // Merge formats_camera and formats_encoder and pack the info
  // into the GstCaps we're going to return here.
  caps = GST_BASE_SRC_CLASS (parent_class)->get_caps (base_src, NULL);

  GST_DEBUG_OBJECT (self, "Returning caps %" GST_PTR_FORMAT, caps);

  GST_LIVE_UNLOCK (self);

  return caps;
}

static GstCaps * gst_omx_camera_src_fixate (GstBaseSrc * base_src,
    GstCaps * caps)
{
  GstOMXCameraSrc * self = GST_OMX_CAMERA_SRC (base_src);

  GstCaps *default_caps;
  GstStructure *structure, *default_structure;
  gint i;

  // TODO: On RPi, this function requires updates if encoder is being used

  GST_LIVE_LOCK (self);

  GST_DEBUG_OBJECT (self, "Fixating caps %" GST_PTR_FORMAT, caps);

  caps = gst_caps_make_writable (caps);
  default_caps = gst_omx_camera_src_default_caps (self);
  default_structure = gst_caps_get_structure (default_caps, 0);

  for (i = 0; i < gst_caps_get_size (caps); ++i) {
    int width = 0, height = 0, fps_n, fps_d, par_n, par_d, res_n, res_d;
    structure = gst_caps_get_structure (caps, i);

    if (gst_structure_has_field (structure, "width"))
      gst_structure_get_int (structure, "width", &width);
    if (gst_structure_has_field (structure, "height"))
      gst_structure_get_int (structure, "height", &height);

    if (!width || !height) {
      if (!width && !height) {
        // Both width and height missing, use defaults
        if (!gst_structure_get_int (default_structure, "width", &width)) {
          GST_ERROR_OBJECT (self, "Error while getting width from "
              "default caps structure");
          goto error;
        }
        if (!gst_structure_get_int (default_structure, "height", &height)) {
          GST_ERROR_OBJECT (self, "Error while getting height from "
              "default caps structure");
          goto error;
        }
      } else {
        // Calculate missing width or height from defaults
        if (!gst_structure_get_fraction (default_structure, "pixel-aspect-ratio", &res_n, &res_d)) {
          GST_ERROR_OBJECT (self, "Error while getting pixel-aspect-ratio from "
              "default caps structure");
          goto error;
        }
        par_n = res_n;
        par_d = res_d;
        if (!width) {
          if (!gst_util_fraction_multiply (height, 1, par_n, par_d, &res_n, &res_d)) {
            GST_ERROR_OBJECT (self, "Error while getting calculating new width "
                "for caps structure");
            goto error;
          }
          width = (gint)gst_util_uint64_scale_int_round (1, res_n, res_d);
        }
        if (!height) {
          if (!gst_util_fraction_multiply (width, 1, par_d, par_n, &res_n, &res_d)) {
            GST_ERROR_OBJECT (self, "Error while getting calculating new height "
                "for caps structure");
            goto error;
          }
          height = (gint)gst_util_uint64_scale_int_round (1, res_n, res_d);
        }
      }
    }

    if (!gst_structure_get_fraction (default_structure, "framerate", &res_n, &res_d)) {
      GST_ERROR_OBJECT (self, "Error while getting framerate "
          "from default caps structure");
      goto error;
    }
    fps_n = res_n;
    fps_d = res_d;

    gst_util_double_to_fraction (((gdouble)width / (gdouble)height),
        &par_n, &par_d);

    gst_structure_fixate_field_nearest_int (structure, "width", width);
    gst_structure_fixate_field_nearest_int (structure, "height", height);
    gst_structure_fixate_field_nearest_fraction (structure, "framerate",
        fps_n, fps_d);
    gst_structure_fixate_field (structure, "format");
    gst_structure_set (structure,
        "pixel-aspect-ratio", GST_TYPE_FRACTION, par_n, par_d, NULL);
  }

  gst_caps_unref (default_caps);

  GST_DEBUG_OBJECT (self, "Fixated caps %" GST_PTR_FORMAT, caps);

done:
  GST_LIVE_UNLOCK (self);

  return caps;

error:
  gst_caps_unref (caps);
  goto done;
}

static gboolean gst_omx_camera_src_set_caps (GstBaseSrc * base_src,
    GstCaps * caps)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (base_src);
  gboolean res = FALSE;

  GST_LIVE_LOCK (self);

  GST_DEBUG_OBJECT (self, "Setting format %" GST_PTR_FORMAT, caps);

  // TODO: On RPi, this requires updates if encoder is being used
  if (!self->info)
    self->info = g_new (GstVideoInfo, 1);

  gst_video_info_init (self->info);

  if (!gst_video_info_from_caps (self->info, caps)) {
    GST_ERROR_OBJECT (self, "Invalid format %" GST_PTR_FORMAT, caps);
    if (self->info) {
      g_free (self->info);
      self->info = NULL;
    }
    res = FALSE;
    goto done;
  }

  self->accum_rtime += self->running_time;
  self->accum_frames += self->n_frames;
  self->running_time = 0;
  self->n_frames = 0;

  res = gst_omx_camera_src_configure_video_unlocked (self);

  // TODO: This needs to go a way when we support other formats than the
  // hard-coded I420. Now it's used in gst_omx_camera_src_poll_frame();
  if (res) {
    GstCaps *omx_buf_caps;

    if (!self->omx_buf_info)
      self->omx_buf_info = g_new (GstVideoInfo, 1);

    gst_video_info_init (self->omx_buf_info);

    omx_buf_caps = gst_omx_camera_src_format_caps (self,
        self->camera_out_port->port_def.format.video.nStride,
        self->camera_out_port->port_def.format.video.nSliceHeight,
        1);

    if (!gst_video_info_from_caps (self->omx_buf_info, omx_buf_caps)) {
      GST_ERROR_OBJECT (self, "Invalid format %" GST_PTR_FORMAT, omx_buf_caps);
      if (self->omx_buf_info) {
        g_free (self->omx_buf_info);
        self->omx_buf_info = NULL;
      }
      res = FALSE;
    }
  }

done:
  GST_DEBUG_OBJECT (self, "Set caps, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  GST_LIVE_UNLOCK (self);

  return res;
}

static gboolean gst_omx_camera_src_decide_allocation (GstBaseSrc *base_src,
    GstQuery *query)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (base_src);

  GstBufferPool *pool;
  guint size, min, max;
  GstStructure *config;
  gboolean update;

  GST_LIVE_LOCK (self);

  while (!self->info) {
    GST_DEBUG_OBJECT (self, "Video capabilities not negotiated, waiting");
    GST_LIVE_WAIT (self);
  }

  // TODO: On RPi, this requires updates if encoder is being used
  if (gst_query_get_n_allocation_pools (query) > 0) {
    gst_query_parse_nth_allocation_pool (query, 0, &pool, &size, &min, &max);

    size = MAX (size, GST_VIDEO_INFO_SIZE (self->info));
    update = TRUE;
  } else {
    pool = NULL;
    size = GST_VIDEO_INFO_SIZE (self->info);
    min = max = 0;
    update = FALSE;
  }

  /* no downstream pool, make our own */
  if (pool == NULL) {
    pool = gst_video_buffer_pool_new ();
  }

  config = gst_buffer_pool_get_config (pool);
  if (gst_query_find_allocation_meta (query, GST_VIDEO_META_API_TYPE, NULL)) {
    gst_buffer_pool_config_add_option (config,
        GST_BUFFER_POOL_OPTION_VIDEO_META);
  }
  gst_buffer_pool_set_config (pool, config);

  if (update)
    gst_query_set_nth_allocation_pool (query, 0, pool, size, min, max);
  else
    gst_query_add_allocation_pool (query, pool, size, min, max);

  GST_DEBUG_OBJECT (self,
      "Buffer pool parameters for configured format: Size:%d Min:%d Max:%d",
      size, min, max);

  if (pool)
    gst_object_unref (pool);

  GST_LIVE_UNLOCK (self);

  return GST_BASE_SRC_CLASS (parent_class)->decide_allocation (base_src, query);
}

static GstStateChangeReturn
gst_omx_camera_src_change_state (GstElement * element, GstStateChange transition)
{
  GstOMXCameraSrc *self;
  GstStateChangeReturn res;

  self = GST_OMX_CAMERA_SRC (element);

  // The idea behind separa enable/disable capturing and start/stop capturing
  // functions is that maybe we'd like to support pause and then you'd only
  // need to call stop/stop functions when switching between play/pause states
  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      if (!(gst_omx_camera_src_enable_capturing (self) &&
            gst_omx_camera_src_start_capturing (self))) {
        res = GST_STATE_CHANGE_FAILURE;
        goto done;
      }
      break;
    default: break;
  }

  res = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      if (!(gst_omx_camera_src_stop_capturing (self) &&
            gst_omx_camera_src_disable_capturing (self)))
        res = GST_STATE_CHANGE_FAILURE;
      break;
    default: break;
  }

done:
  GST_DEBUG_OBJECT (self, "Change state, %s", (res ? "ok" : "failing"));

  return res;
}

static void
gst_omx_camera_src_finalize (GObject * object)
{

  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (object);

  if (self->info)
    g_free (self->info);
  if (self->omx_buf_info)
    g_free (self->omx_buf_info);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
gst_omx_camera_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (object);

  switch (prop_id) {
    case PROP_DEVICE_NUMBER:
      self->config.device.nU32 = g_value_get_uint (value);
      break;
#ifdef USE_OMX_TARGET_RPI
    case PROP_SHARPNESS:
      self->config.sharpness.nSharpness = g_value_get_int (value);
      break;
#endif
    case PROP_GAMMA:
      self->config.gamma.nGamma = g_value_get_int (value) << 16;
      break;
    case PROP_CONTRAST:
      self->config.contrast.nContrast = g_value_get_int (value);
      break;
    case PROP_BRIGHTNESS:
      self->config.brightness.nBrightness = g_value_get_uint (value);
      break;
    case PROP_SATURATION:
      self->config.saturation.nSaturation = g_value_get_int (value);
      break;
    case PROP_IMAGE_FILTER:
      self->config.image_filter.eImageFilter = g_value_get_enum (value);
      break;
    case PROP_COLOR_ENHANCEMENT:
      GST_LIVE_LOCK (self);
      if (gst_omx_camera_src_config_color_enhancement_set_from_string (&self->config.color_enhancement,
            g_value_get_string (value)))
        self->config.color_enhancement.bColorEnhancement = TRUE;
      else
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      GST_LIVE_UNLOCK (self);
      break;
    case PROP_WHITE_BALANCE_MODE:
      self->config.white_balance.eWhiteBalControl = g_value_get_enum (value);
      break;
    case PROP_EXPOSURE_CONTROL_MODE:
      self->config.exposure_control.eExposureControl = g_value_get_enum (value);
      break;
    case PROP_EXPOSURE_METERING_MODE:
      self->config.exposure_value.eMetering = g_value_get_enum (value);
      break;
    case PROP_EXPOSURE_VALUE_COMPENSATION:
      self->config.exposure_value.xEVCompensation = g_value_get_int (value);
      break;
    case PROP_EXPOSURE_ISO_SENSITIVITY:
      self->config.exposure_value.nSensitivity = g_value_get_uint (value);
      self->config.exposure_value.bAutoSensitivity = FALSE;
      break;
    case PROP_APERTURE:
      self->config.exposure_value.nApertureFNumber = g_value_get_uint (value);
      self->config.exposure_value.bAutoAperture = FALSE;
      break;
    case PROP_FRAME_STABILISATION:
      self->config.frame_stabilisation.bStab = g_value_get_boolean (value);
      break;
    case PROP_HORIZONTAL_FLIP:
      gst_omx_camera_src_vo_set_hflip (GST_VIDEO_ORIENTATION (self),
          g_value_get_boolean (value));
      break;
    case PROP_VERTICAL_FLIP:
      gst_omx_camera_src_vo_set_vflip (GST_VIDEO_ORIENTATION (self),
          g_value_get_boolean (value));
      break;
    default:
      break;
  }
}

static void
gst_omx_camera_src_get_property (GObject * object, guint prop_id, GValue * value,
    GParamSpec * pspec)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (object);
  gboolean *flip;

  switch (prop_id) {
    case PROP_DEVICE_NUMBER:
      g_value_set_uint (value, self->config.device.nU32);
      break;
#ifdef USE_OMX_TARGET_RPI
    case PROP_SHARPNESS:
      g_value_set_int (value, self->config.sharpness.nSharpness);
      break;
#endif
    case PROP_GAMMA:
      g_value_set_int (value, self->config.gamma.nGamma >> 16);
      break;
    case PROP_CONTRAST:
      g_value_set_int (value, self->config.contrast.nContrast);
      break;
    case PROP_BRIGHTNESS:
      g_value_set_uint (value, self->config.brightness.nBrightness);
      break;
    case PROP_SATURATION:
      g_value_set_int (value, self->config.saturation.nSaturation);
      break;
    case PROP_WHITE_BALANCE_MODE:
      g_value_set_enum (value, self->config.white_balance.eWhiteBalControl);
      break;
    case PROP_IMAGE_FILTER:
      g_value_set_enum (value, self->config.image_filter.eImageFilter);
      break;
    case PROP_COLOR_ENHANCEMENT:
      if (self->config.color_enhancement.bColorEnhancement) {
        gchar *str = gst_omx_camera_src_config_color_enhancement_to_string (&self->config.color_enhancement);
        g_value_set_string (value, str);
        g_free (str);
      }
      else
        g_value_set_string (value, NULL);
      break;
    case PROP_EXPOSURE_CONTROL_MODE:
      g_value_set_enum (value, self->config.exposure_control.eExposureControl);
      break;
    case PROP_EXPOSURE_METERING_MODE:
      g_value_set_enum (value, self->config.exposure_value.eMetering);
      break;
    case PROP_EXPOSURE_VALUE_COMPENSATION:
      g_value_set_int (value, self->config.exposure_value.xEVCompensation);
      break;
    case PROP_EXPOSURE_ISO_SENSITIVITY:
      if (self->config.exposure_value.bAutoSensitivity)
        g_value_set_uint (value, 0);
      else
        g_value_set_uint (value, self->config.exposure_value.nSensitivity);
      break;
    case PROP_APERTURE:
      if (self->config.exposure_value.bAutoAperture)
        g_value_set_uint (value, 0);
      else
        g_value_set_uint (value, self->config.exposure_value.nApertureFNumber);
      break;
    case PROP_FRAME_STABILISATION:
      g_value_set_boolean (value, self->config.frame_stabilisation.bStab);
      break;
    case PROP_HORIZONTAL_FLIP:
      flip = (gboolean *)g_new0(gboolean, 1);
      gst_omx_camera_src_vo_get_hflip (GST_VIDEO_ORIENTATION (self), flip);
      g_value_set_boolean (value, *flip);
      g_free (flip);
      break;
    case PROP_VERTICAL_FLIP:
      flip = (gboolean *)g_new0(gboolean, 1);
      gst_omx_camera_src_vo_get_vflip (GST_VIDEO_ORIENTATION (self), flip);
      g_value_set_boolean (value, *flip);
      g_free (flip);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstVideoOrientation interface stuff */
static gboolean gst_omx_camera_src_vo_get_hflip (GstVideoOrientation * vo,
    gboolean * flip)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (vo);

  *flip = self->config.horizontal_flip;

  return TRUE;
}

static gboolean gst_omx_camera_src_vo_get_vflip (GstVideoOrientation * vo,
    gboolean * flip)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (vo);

  *flip = self->config.vertical_flip;

  return TRUE;
}

static gboolean gst_omx_camera_src_vo_set_hflip (GstVideoOrientation * vo,
    gboolean flip)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (vo);

  GST_LIVE_LOCK (self);

  self->config.horizontal_flip = flip;
  gst_omx_camera_src_update_mirror_mode (self);

  GST_LIVE_UNLOCK (self);

  return TRUE;
}

static gboolean gst_omx_camera_src_vo_set_vflip (GstVideoOrientation * vo,
    gboolean flip)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (vo);

  GST_LIVE_LOCK (self);

  self->config.vertical_flip = flip;
  gst_omx_camera_src_update_mirror_mode (self);

  GST_LIVE_UNLOCK (self);

  return TRUE;
}

static gboolean gst_omx_camera_src_vo_get_hcenter_vcenter_not_supported (GstVideoOrientation * vo,
    gint * center)
{
  return FALSE;
}

static gboolean gst_omx_camera_src_vo_set_hcenter_vcenter_not_supported (GstVideoOrientation * vo,
    gint center)
{
  return FALSE;
}

static void gst_omx_camera_src_video_orientation_interface_init (GstVideoOrientationInterface * iface)
{
  iface->get_hflip   = gst_omx_camera_src_vo_get_hflip;
  iface->get_vflip   = gst_omx_camera_src_vo_get_vflip;
  iface->set_hflip   = gst_omx_camera_src_vo_set_vflip;
  iface->set_vflip   = gst_omx_camera_src_vo_set_hflip;
  iface->get_hcenter = gst_omx_camera_src_vo_get_hcenter_vcenter_not_supported;
  iface->get_vcenter = gst_omx_camera_src_vo_get_hcenter_vcenter_not_supported;
  iface->set_hcenter = gst_omx_camera_src_vo_set_hcenter_vcenter_not_supported;
  iface->set_vcenter = gst_omx_camera_src_vo_set_hcenter_vcenter_not_supported;
}
