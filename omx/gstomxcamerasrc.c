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
#define GST_OMX_CAMERA_SRC_WIDTH_DEFAULT                       320
#define GST_OMX_CAMERA_SRC_HEIGHT_DEFAULT                      240
#define GST_OMX_CAMERA_SRC_FRAMERATE_DEFAULT                   30
#ifdef USE_OMX_TARGET_RPI
#define GST_OMX_CAMERA_SRC_CONTROL_RATE_DEFAULT                OMX_Video_ControlRateVariable
#define GST_OMX_CAMERA_SRC_TARGET_BITRATE_DEFAULT              17000000
#define GST_OMX_CAMERA_SRC_SHARPNESS_DEFAULT                   0
#endif
#define GST_OMX_CAMERA_SRC_GAMMA_DEFAULT                       2
#define GST_OMX_CAMERA_SRC_CONTRAST_DEFAULT                    0
#define GST_OMX_CAMERA_SRC_BRIGHTNESS_DEFAULT                  50
#define GST_OMX_CAMERA_SRC_SATURATION_DEFAULT                  0
#define GST_OMX_CAMERA_SRC_IMAGE_FILTER_DEFAULT                OMX_ImageFilterNone
#define GST_OMX_CAMERA_SRC_COLOR_ENHANCEMENT_U_CHANNEL_DEFAULT 0
#define GST_OMX_CAMERA_SRC_COLOR_ENHANCEMENT_V_CHANNEL_DEFAULT 0
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
  PROP_CONTROL_RATE,
  PROP_TARGET_BITRATE,
  PROP_SHARPNESS,
#endif
  PROP_GAMMA,
  PROP_CONTRAST,
  PROP_BRIGHTNESS,
  PROP_SATURATION,
  PROP_IMAGE_FILTER,
  PROP_COLOR_ENHANCEMENT_U_CHANNEL,
  PROP_COLOR_ENHANCEMENT_V_CHANNEL,
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

#define GST_TYPE_OMX_CAMERA_SRC_CONTROL_RATE (gst_omx_camera_src_control_rate_get_type ())
static GType
gst_omx_camera_src_control_rate_get_type (void)
{
  static GType omx_camera_src_control_rate_type = 0;
  static const GEnumValue control_rate_types[] = {
    {OMX_Video_ControlRateDisable,            "Disable",              "disable"             },
    {OMX_Video_ControlRateVariable,           "Variable",             "variable"            },
    {OMX_Video_ControlRateConstant,           "Constant",             "constant"            },
    {OMX_Video_ControlRateVariableSkipFrames, "Variable Skip Frames", "variable-skip-frames"},
    {OMX_Video_ControlRateConstantSkipFrames, "Constant Skip Frames", "constant-skip-frames"},
    {0, NULL, NULL}
  };

  if (!omx_camera_src_control_rate_type) {
    omx_camera_src_control_rate_type =
        g_enum_register_static ("GstOMXCameraSrcControlRate",
            control_rate_types);
  }
  return omx_camera_src_control_rate_type;
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

  gobject_class->finalize = gst_omx_camera_src_finalize;
  gobject_class->set_property = gst_omx_camera_src_set_property;
  gobject_class->get_property = gst_omx_camera_src_get_property;

  g_object_class_install_property (G_OBJECT_CLASS (klass), PROP_DEVICE_NUMBER,
      g_param_spec_uint ("device-number", "Device number",
          "Device number of the camera device",
          0, G_MAXINT, GST_OMX_CAMERA_SRC_DEVICE_NUMBER_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
#ifdef USE_OMX_TARGET_RPI
  g_object_class_install_property (gobject_class, PROP_CONTROL_RATE,
      g_param_spec_enum ("control-rate", "Control Rate",
          "Bitrate control method for compressed video",
          GST_TYPE_OMX_CAMERA_SRC_CONTROL_RATE,
          GST_OMX_CAMERA_SRC_CONTROL_RATE_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_TARGET_BITRATE,
      g_param_spec_uint ("target-bitrate", "Target Bitrate",
          "Target bitrate for compressed video",
          0, 17000000, GST_OMX_CAMERA_SRC_TARGET_BITRATE_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
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
      PROP_COLOR_ENHANCEMENT_U_CHANNEL,
      g_param_spec_uint ("color-enhancement-u-channel",
          "Color enhancement effect for U channel",
          "Specify color enhancement effect for image U channel. If this "
          "property is set, also  color-enhancement-v-channel should be set. "
          "Setting e.g. both properties to 128 results in a monochrome image.",
          0, 255, GST_OMX_CAMERA_SRC_COLOR_ENHANCEMENT_U_CHANNEL_DEFAULT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (G_OBJECT_CLASS (klass),
      PROP_COLOR_ENHANCEMENT_V_CHANNEL,
      g_param_spec_uint ("color-enhancement-v-channel",
          "Color enhancement effect for V channel",
          "Specify color enhancement effect for image V channel. If this "
          "property is set, also  color-enhancement-u-channel should be set. "
          "Setting e.g. both properties to 128 results in a monochrome image.",
          0, 255, GST_OMX_CAMERA_SRC_COLOR_ENHANCEMENT_V_CHANNEL_DEFAULT,
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

  gst_element_class_set_static_metadata (element_class,
      "OpenMAX Camera video source",
      "Source/Video",
      "Source for raw video data from OpenMAX camera component",
      "Tuomas Jormola <tj@solitudo.net>");
}

static void
gst_omx_camera_src_init (GstOMXCameraSrc * self)
{
  gint n_comp = 1;    // Camera
  gint n_port = 2;    // Camera in & out
#ifdef USE_OMX_TARGET_RPI
  n_comp += 2;        // Null sink, encoder
  n_port += 4;        // Camera preview, null sink in, encoder in & out
#endif
  self->comp = (GstOMXComponent **)g_new0 (GstOMXComponent, n_comp);
  self->port = (GstOMXPort **)g_new0 (GstOMXPort, n_port);
  self->all_port_formats = NULL;
  self->camera_configured = FALSE;
  self->video_configured = FALSE;
  GST_OMX_INIT_STRUCT (&self->config.device);
#ifdef USE_OMX_TARGET_RPI
  GST_OMX_INIT_STRUCT (&self->config.bitrate);
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
  self->port_format = NULL;
  self->info = NULL;
  self->width = 0;
  self->height = 0;
  self->fps_n = 0;
  self->fps_d = 0;
  self->framerate = 0;
  self->omx_buf_info = NULL;
  self->omx_buf_port = NULL;
#ifdef USE_OMX_TARGET_RPI
  self->config.bitrate.eControlRate = GST_OMX_CAMERA_SRC_CONTROL_RATE_DEFAULT;
  self->config.bitrate.nTargetBitrate = GST_OMX_CAMERA_SRC_TARGET_BITRATE_DEFAULT;
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
  self->config.color_enhancement.nCustomizedU =
    GST_OMX_CAMERA_SRC_COLOR_ENHANCEMENT_U_CHANNEL_DEFAULT;
  self->config.color_enhancement.nCustomizedV =
    GST_OMX_CAMERA_SRC_COLOR_ENHANCEMENT_V_CHANNEL_DEFAULT;
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

/* LIVE_LOCK needs to be hold */
static gboolean
gst_omx_camera_src_is_capture_active_unlocked (GstOMXCameraSrc * self)
{
  OMX_CONFIG_PORTBOOLEANTYPE capture;
  OMX_ERRORTYPE err;

  GST_OMX_INIT_STRUCT (&capture);
  capture.nPortIndex = self->port[CAMERA_VIDEO_OUT]->index;
  err = gst_omx_component_get_parameter (self->comp[CAMERA],
      OMX_IndexConfigPortCapturing, &capture);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while checking video capture state on "
        "camera video output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    return FALSE;
  }

  return capture.bEnabled;
}

/* LIVE_LOCK needs to be hold */
static gboolean
gst_omx_camera_src_component_state_to_loaded_unlocked (GstOMXCameraSrc * self,
    GstOMXComponent * comp)
{
  OMX_STATETYPE state = OMX_StateInvalid,
                next_state = OMX_StateInvalid;
  OMX_ERRORTYPE err;
  gboolean res;

  if (!comp) {
    GST_DEBUG_OBJECT (self, "Component not opened, return");
    res = FALSE;
    goto done;
  }

  while (state != OMX_StateLoaded) {
    state = gst_omx_component_get_state (comp, GST_CLOCK_TIME_NONE);
    switch (state) {
      case OMX_StateExecuting:
        next_state = OMX_StateIdle; break;
      case OMX_StateIdle:
      case OMX_StateLoaded:
        next_state = OMX_StateLoaded; break;
      default: break;
    }

    if (next_state == OMX_StateInvalid) {
      GST_DEBUG_OBJECT (self, "Invalid state: 0x%08x", state);
      res = FALSE;
      goto done;
    }

    if (state != next_state) {
      err = gst_omx_component_set_state (comp, next_state);
      if (err != OMX_ErrorNone) {
        GST_ERROR_OBJECT (self, "Error while setting %s state "
            "from %s to %s: %s (0x%08x)",
            comp->name,
            gst_omx_state_to_string (state), gst_omx_state_to_string (next_state),
            gst_omx_error_to_string (err), err);
        res = FALSE;
        goto done;
      }
    }
  }

  res = TRUE;

done:
  return res;
}

static gboolean
gst_omx_camera_src_caps_from_port_format (GstOMXCameraSrc * self,
    const GstOMXCameraSrcPortFormat *format, GstCaps *caps)
{
  GstCaps *new_caps = NULL;
  GString *type = NULL, *raw_format = NULL;
  gint min_width, max_width, min_height, max_height;
  gint min_fps_n, min_fps_d = 1, max_fps_n, max_fps_d = 1;
  gint mpegversion = 0;
  gboolean res = FALSE;

  GST_DEBUG_OBJECT (self, "Creating caps for port format with "
      "compression 0x%08x and color format 0x%08x",
      format->compression_format, format->color_format);

  switch (format->compression_format) {
    case OMX_VIDEO_CodingUnused:
      switch (format->color_format) {
        case OMX_COLOR_FormatYUV411Planar:
        case OMX_COLOR_FormatYUV411PackedPlanar:
          raw_format = g_string_new ("Y41B"); break;
        case OMX_COLOR_FormatYUV420Planar:
        case OMX_COLOR_FormatYUV420PackedPlanar:
          raw_format = g_string_new ("I420"); break;
        case OMX_COLOR_FormatYUV420SemiPlanar:
        case OMX_COLOR_FormatYUV420PackedSemiPlanar:
          raw_format = g_string_new ("NV12"); break;
        case OMX_COLOR_FormatYUV422Planar:
        case OMX_COLOR_FormatYUV422PackedPlanar:
          raw_format = g_string_new ("Y42B"); break;
        case OMX_COLOR_FormatYUV422SemiPlanar:
        case OMX_COLOR_FormatYUV422PackedSemiPlanar:
          raw_format = g_string_new ("NV16"); break;
        case OMX_COLOR_FormatYUV444Interleaved:
          raw_format = g_string_new ("NV24"); break;
        default:
          GST_DEBUG_OBJECT (self, "Unknown color format: 0x%08x", format->color_format);
          break;
      }
      if (raw_format)
        type = g_string_new ("video/x-raw");
      break;
    case OMX_VIDEO_CodingMPEG2:
      type = g_string_new ("video/x-mpeg"); break;
      mpegversion = 2;
    case OMX_VIDEO_CodingH263:
      type = g_string_new ("video/x-h263"); break;
    case OMX_VIDEO_CodingMPEG4:
      type = g_string_new ("video/x-mpeg");
      mpegversion = 4;
      break;
    case OMX_VIDEO_CodingWMV:
      type = g_string_new ("video/x-wmv"); break;
    case OMX_VIDEO_CodingRV:
      type = g_string_new ("video/x-pn-realvideo"); break;
    case OMX_VIDEO_CodingAVC:
      type = g_string_new ("video/x-h264"); break;
    case OMX_VIDEO_CodingMJPEG:
      type = g_string_new ("image/jpeg"); break;
    case OMX_VIDEO_CodingVP6:
      type = g_string_new ("video/x-vp6"); break;
    case OMX_VIDEO_CodingVP7:
      type = g_string_new ("video/x-vp7"); break;
    case OMX_VIDEO_CodingVP8:
      type = g_string_new ("video/x-vp8"); break;
    case OMX_VIDEO_CodingSorenson:
      type = g_string_new ("video/x-svq"); break;
    case OMX_VIDEO_CodingTheora:
      type = g_string_new ("video/x-theora"); break;
    default:
      GST_DEBUG_OBJECT (self, "Unknown compression format 0x%08x",
          format->compression_format);
      break;
  }
#ifdef USE_OMX_TARGET_RPI
  // RPi reports that these formats are supported. However, as of firmware
  // 1.20130902-1, they don't seem to work. You can configure the formats
  // but when enabling the port, OMX_ErrorInsufficientResources error will
  // be given. So for the time being, let's black-list the formats.
  switch (format->compression_format) {
    case OMX_VIDEO_CodingMJPEG:
    case OMX_VIDEO_CodingMPEG4:
    case OMX_VIDEO_CodingH263:
    case OMX_VIDEO_CodingVP7:
      if (type) {
        g_string_free (type, TRUE);
        type = NULL;
      }
      break;
    default: break;
  }
#endif

  if (type) {
#ifdef USE_OMX_TARGET_RPI
    min_width = 64;
    max_width = 1920;
    min_height = 64;
    max_height = 1080;
    min_fps_n = 2;
    max_fps_n = 30;
#else
    min_width = 0;
    max_width = G_INT_MAX;
    min_height = 0;
    max_height = G_INT_MAX;
    min_fps_n = 0;
    max_fps_n = G_INT_MAX;
#endif
    new_caps = gst_caps_new_simple (type->str,
        "width", GST_TYPE_INT_RANGE, min_width, max_width,
        "height", GST_TYPE_INT_RANGE, min_height, max_height,
        "framerate", GST_TYPE_FRACTION_RANGE,
          min_fps_n, min_fps_d, max_fps_n, max_fps_d,
        NULL);
    if (raw_format)
      gst_caps_set_simple (new_caps, "format", G_TYPE_STRING, raw_format->str, NULL);
    if (mpegversion)
      gst_caps_set_simple (new_caps, "mpegversion", G_TYPE_INT, mpegversion, NULL);

    GST_DEBUG_OBJECT (self, "Created caps %" GST_PTR_FORMAT, new_caps);

    gst_caps_append (caps, new_caps);
    new_caps = NULL;

    res = TRUE;
  }

  if (type)
    g_string_free (type, TRUE);

  if (raw_format)
    g_string_free (raw_format, TRUE);

  return res;
}

/* LIVE_LOCK needs to be hold */
static GSList *
gst_omx_camera_src_probe_port_formats_unlocked (GstOMXCameraSrc * self,
    GstOMXPort * port)
{
  OMX_VIDEO_PARAM_PORTFORMATTYPE portformat;
  OMX_ERRORTYPE err;
  GSList *res = NULL;

  GST_DEBUG_OBJECT (self, "Probing %s port %u for supported formats",
      port->comp->name, port->index);

  if (!self->all_port_formats) {
    self->all_port_formats = g_hash_table_new (NULL, NULL);
  }

  res = (GSList *)g_hash_table_lookup (self->all_port_formats, port);
  if (res != NULL) {
    GST_DEBUG_OBJECT (self, "Formats already probed, returning cached copy");
    return res;
  }

  GST_OMX_INIT_STRUCT (&portformat);
  portformat.nPortIndex = port->index;
  portformat.nIndex = 0;

  while (1) {
    GstOMXCameraSrcPortFormat *format;
    err = gst_omx_component_get_parameter (port->comp,
          OMX_IndexParamVideoPortFormat, &portformat);
    if (err == OMX_ErrorNoMore)
      break;
    if (err != OMX_ErrorNone) {
      if (res) {
        g_slist_free (res);
        res = NULL;
      }
      break;
    }

    format = g_new0 (GstOMXCameraSrcPortFormat, 1);
    format->compression_format = portformat.eCompressionFormat;
    format->color_format = portformat.eColorFormat;

    switch (portformat.eColorFormat) {
        case OMX_COLOR_FormatYUV411PackedPlanar:
        case OMX_COLOR_FormatYUV420PackedPlanar:
        case OMX_COLOR_FormatYUV422PackedPlanar:
        case OMX_COLOR_FormatYUV420PackedSemiPlanar:
        case OMX_COLOR_FormatYUV422PackedSemiPlanar:
        case OMX_COLOR_FormatYVU420PackedPlanar:
        case OMX_COLOR_FormatYVU420PackedSemiPlanar:
          format->is_packed = TRUE; break;
        default:
          format->is_packed = FALSE; break;
    }

    format->caps_template = gst_caps_new_empty ();
    if (!gst_omx_camera_src_caps_from_port_format (self, format,
          format->caps_template)) {
      gst_caps_unref (format->caps_template);
      format->caps_template = NULL;
    }

    res = g_slist_append (res, format);

    GST_DEBUG_OBJECT (self, "Component %s port %u supports "
        "compression 0x%08x and color format 0x%08x",
        port->comp->name, port->index,
        format->compression_format, format->color_format);

    portformat.nIndex++;
  }

  if (res)
    g_hash_table_insert (self->all_port_formats, port, res);

  return res;
}

/* LIVE_LOCK needs to be hold */
static GSList *
gst_omx_camera_src_probe_all_formats_unlocked (GstOMXCameraSrc * self)
{
  GSList *formats;
#ifdef USE_OMX_TARGET_RPI
  GSList *encoder_formats;
#endif

  formats = gst_omx_camera_src_probe_port_formats_unlocked (self,
        self->port[CAMERA_VIDEO_OUT]);
#ifdef USE_OMX_TARGET_RPI
  // Return encoded formats first on RPi
  encoder_formats = gst_omx_camera_src_probe_port_formats_unlocked (self,
        self->port[ENCODER_OUT]);
  if (formats) {
    GSList *last = g_slist_last (encoder_formats);
    last->next = formats;
  }
  formats = encoder_formats;
#endif

  return formats;
}

/* LIVE_LOCK needs to be hold */
static gboolean gst_omx_camera_src_configure_camera_unlocked (GstOMXCameraSrc
    * self)
{
  OMX_ERRORTYPE err;
  gboolean res;

  GST_DEBUG_OBJECT (self, "Configuring camera");

  while (!self->comp[CAMERA]) {
    GST_DEBUG_OBJECT (self, "Camera not opened, waiting");
    GST_LIVE_WAIT (self);
  }

  if (gst_omx_component_get_state (self->comp[CAMERA], GST_CLOCK_TIME_NONE) !=
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
  cbtype.bEnable = TRUE;
  err = gst_omx_component_set_config (self->comp[CAMERA],
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
  err = gst_omx_component_set_parameter (self->comp[CAMERA],
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
  err = gst_omx_component_set_config (self->comp[CAMERA],
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
    err = gst_omx_component_set_config (self->comp[CAMERA],
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

  err = gst_omx_component_set_config (self->comp[CAMERA],
      OMX_IndexConfigCommonContrast, &self->config.contrast);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config contrast %d: %s (0x%08x)",
        self->config.contrast.nContrast,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->comp[CAMERA],
      OMX_IndexConfigCommonBrightness, &self->config.brightness);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config brightness %u: %s (0x%08x)",
        self->config.brightness.nBrightness,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->comp[CAMERA],
      OMX_IndexConfigCommonSaturation, &self->config.saturation);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config saturation %d: %s (0x%08x)",
        self->config.saturation.nSaturation,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->comp[CAMERA],
      OMX_IndexConfigCommonImageFilter, &self->config.image_filter);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config image filter %d: %s (0x%08x)",
        self->config.image_filter.eImageFilter,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->comp[CAMERA],
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

  err = gst_omx_component_set_config (self->comp[CAMERA],
      OMX_IndexConfigCommonWhiteBalance, &self->config.white_balance);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config white balance %d: %s (0x%08x)",
        self->config.white_balance.eWhiteBalControl,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->comp[CAMERA],
      OMX_IndexConfigCommonExposure, &self->config.exposure_control);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config exposure control %d: %s (0x%08x)",
        self->config.exposure_control.eExposureControl,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_component_set_config (self->comp[CAMERA],
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

  err = gst_omx_component_set_config (self->comp[CAMERA],
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

  self->config.mirror.nPortIndex = self->port[CAMERA_VIDEO_OUT]->index;
  err = gst_omx_component_set_config (self->comp[CAMERA],
      OMX_IndexConfigCommonMirror, &self->config.mirror);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config mirror %d on "
        "camera video output port: %s (0x%08x)",
        self->config.mirror.eMirror,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#ifdef USE_OMX_TARGET_RPI
  self->config.mirror.nPortIndex = self->port[CAMERA_PREVIEW_OUT]->index;
  err = gst_omx_component_set_config (self->comp[CAMERA],
      OMX_IndexConfigCommonMirror, &self->config.mirror);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config mirror %d on "
        "camera preview output port: %s (0x%08x)",
        self->config.mirror.eMirror,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#endif

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
  gint stride;
  OMX_PARAM_PORTDEFINITIONTYPE port_def;
  OMX_CONFIG_FRAMERATETYPE framerate;
#ifdef USE_OMX_TARGET_RPI
  OMX_VIDEO_PARAM_PORTFORMATTYPE format;
#endif
  OMX_ERRORTYPE err;
  gboolean res;

  while (!self->camera_configured) {
    GST_DEBUG_OBJECT (self, "Camera not configured, waiting");
    GST_LIVE_WAIT (self);
  }

  while (!self->port_format) {
    GST_DEBUG_OBJECT (self, "Video capabilities not negotiated, waiting");
    GST_LIVE_WAIT (self);
  }

  GST_DEBUG_OBJECT (self, "Configuring video %dx%d @ %.2f",
      self->width,
      self->height,
      self->framerate);

  err = gst_omx_port_update_port_definition (self->port[CAMERA_VIDEO_OUT], NULL);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while getting port definition for "
        "camera video output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  port_def = self->port[CAMERA_VIDEO_OUT]->port_def;

  stride = port_def.nBufferAlignment
    ? (self->width + port_def.nBufferAlignment - 1) &
        (~(port_def.nBufferAlignment - 1))
    : GST_ROUND_UP_4 (self->width);

  port_def.format.video.nFrameWidth = self->width;
  port_def.format.video.nFrameHeight = self->height;
  port_def.format.video.nStride = stride;
  port_def.format.video.xFramerate = ((OMX_U32)self->framerate) << 16;
  // On RPi, leave eColorFormat to component default if tunneling to encoder
#ifdef USE_OMX_TARGET_RPI
  if (self->port_format->compression_format == OMX_VIDEO_CodingUnused)
    port_def.format.video.eColorFormat = self->port_format->color_format;
#else
  port_def.format.video.eColorFormat = self->port_format->color_format;
#endif

  err = gst_omx_port_update_port_definition (self->port[CAMERA_VIDEO_OUT],
      &port_def);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error setting parameter port definition for "
        "camera video output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

#ifdef USE_OMX_TARGET_RPI
  // Use the video output port definition also for preview
  port_def.nPortIndex = self->port[CAMERA_PREVIEW_OUT]->index;
  err = gst_omx_port_update_port_definition (self->port[CAMERA_PREVIEW_OUT],
      &port_def);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error setting parameter port definition for "
        "camera preview output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#endif

  GST_OMX_INIT_STRUCT (&framerate);
  framerate.nPortIndex = self->port[CAMERA_VIDEO_OUT]->index;
  err = gst_omx_component_get_config (self->comp[CAMERA],
        OMX_IndexConfigVideoFramerate, &framerate);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error getting config framerate for "
        "camera video output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  framerate.xEncodeFramerate = port_def.format.video.xFramerate;
  err =
      gst_omx_component_set_config (self->comp[CAMERA],
      OMX_IndexConfigVideoFramerate, &framerate);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config framerate %d for "
        "camera video output port: %s (0x%08x)",
        framerate.xEncodeFramerate >> 16,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

#ifdef USE_OMX_TARGET_RPI
  framerate.nPortIndex = self->port[CAMERA_PREVIEW_OUT]->index;
  err =
      gst_omx_component_set_config (self->comp[CAMERA],
      OMX_IndexConfigVideoFramerate, &framerate);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while setting config framerate %d for "
        "camera preview output port: %s (0x%08x)",
        framerate.xEncodeFramerate >> 16,
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  // Configure encoder
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_port_update_port_definition (self->port[ENCODER_OUT], NULL);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while getting port definition for "
          "encoder output port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    port_def = self->port[ENCODER_OUT]->port_def;
    port_def.format.video.nFrameWidth = self->width;
    port_def.format.video.nFrameHeight = self->height;
    port_def.format.video.nStride = stride;
    port_def.format.video.xFramerate = ((OMX_U32)self->framerate) << 16;
    port_def.format.video.nBitrate = self->config.bitrate.nTargetBitrate;
    port_def.format.video.eColorFormat = self->port_format->color_format;

    err = gst_omx_port_update_port_definition (self->port[ENCODER_OUT],
        &port_def);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error setting parameter port definition for "
          "encoder output port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }

    self->config.bitrate.nPortIndex = self->port[ENCODER_OUT]->index;
    err = gst_omx_component_set_parameter (self->comp[ENCODER],
        OMX_IndexParamVideoBitrate, &self->config.bitrate);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while setting parameter bitrate 0x%08x %u: %s (0x%08x)",
          self->config.bitrate.eControlRate,
          self->config.bitrate.nTargetBitrate,
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }

    GST_OMX_INIT_STRUCT (&format);
    format.nPortIndex = self->port[ENCODER_OUT]->index;
    format.eCompressionFormat = self->port_format->compression_format;
    format.eColorFormat = self->port_format->color_format;
    err = gst_omx_component_set_parameter (self->comp[ENCODER],
        OMX_IndexParamVideoPortFormat, &format);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while setting parameter format %u: %s (0x%08x)",
          format.eCompressionFormat,
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  self->video_configured = TRUE;
  res = TRUE;

done:
  GST_DEBUG_OBJECT (self, "Video configured, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  return res;
}

static gboolean
gst_omx_camera_src_start_capturing (GstOMXCameraSrc * self)
{
  OMX_CONFIG_PORTBOOLEANTYPE capture;
  OMX_ERRORTYPE err;
  gboolean res;

  GST_LIVE_LOCK (self);

  GST_DEBUG_OBJECT (self, "Starting video capture");

  while (!self->video_configured) {
    GST_DEBUG_OBJECT (self, "Video not configured, waiting");
    GST_LIVE_WAIT (self);
  }

#ifdef USE_OMX_TARGET_RPI
  // Tunnel camera preview and null sink
  err = gst_omx_component_setup_tunnel (self->comp[CAMERA],
      self->port[CAMERA_PREVIEW_OUT],
      self->comp[NULL_SINK], self->port[NULL_SINK_IN]);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while tunneling camera preview output port "
        "to null sink input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  // Tunnel camera video and encoder
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_component_setup_tunnel (self->comp[CAMERA],
        self->port[CAMERA_VIDEO_OUT],
        self->comp[ENCODER], self->port[ENCODER_IN]);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while tunneling camera video output port "
          "to encoder input port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  // State to idle
  err = gst_omx_component_set_state (self->comp[CAMERA], OMX_StateIdle);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting camera state "
        "to idle: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->comp[CAMERA],
      OMX_StateIdle, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Camera didn't switch to idle state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

#ifdef USE_OMX_TARGET_RPI
  err = gst_omx_component_set_state (self->comp[NULL_SINK], OMX_StateIdle);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting null sink state "
        "to idle: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->comp[NULL_SINK],
      OMX_StateIdle, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Null sink didn't switch to idle state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_component_set_state (self->comp[ENCODER], OMX_StateIdle);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while setting encoder state "
          "to idle: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    err = gst_omx_component_wait_state_changed (self->comp[ENCODER],
        OMX_StateIdle, 1 * GST_SECOND);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Encoder didn't switch to idle state: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  // Enable ports
  err = gst_omx_port_set_enabled (self->port[CAMERA_IN], TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while enabling "
        "camera input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_enabled (self->port[CAMERA_VIDEO_OUT], TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while enabling "
        "camera video output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#ifdef USE_OMX_TARGET_RPI
  err = gst_omx_port_set_enabled (self->port[CAMERA_PREVIEW_OUT], TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while enabling "
        "camera preview output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_enabled (self->port[NULL_SINK_IN], TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while enabling "
        "null sink input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_port_set_enabled (self->port[ENCODER_IN], TRUE);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while enabling "
          "encoder input port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    err = gst_omx_port_set_enabled (self->port[ENCODER_OUT], TRUE);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while enabling "
          "encoder output port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  // Flushing off
  err = gst_omx_port_set_flushing (self->port[CAMERA_IN],
      1 * GST_SECOND, FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while switching flush off "
        "on camera input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_flushing (self->port[CAMERA_VIDEO_OUT],
      1 * GST_SECOND, FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while switching flush off "
        "on camera video output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#ifdef USE_OMX_TARGET_RPI
  err = gst_omx_port_set_flushing (self->port[CAMERA_PREVIEW_OUT],
      1 * GST_SECOND, FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while switching flush off "
        "on camera preview output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_flushing (self->port[NULL_SINK_IN],
      1 * GST_SECOND, FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while switching flush off "
        "on null sink input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_port_set_flushing (self->port[ENCODER_IN],
        1 * GST_SECOND, FALSE);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while switching flush off "
          "on encoder input port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    err = gst_omx_port_set_flushing (self->port[ENCODER_OUT],
        1 * GST_SECOND, FALSE);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while switching flush off "
          "on encoder video output port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  // Allocate buffers
  err = gst_omx_port_allocate_buffers (self->port[CAMERA_IN]);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while allocating buffers for "
        "camera input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  // Only allocate camera output if not tunneling to encoder
#ifdef USE_OMX_TARGET_RPI
  if (self->port_format->compression_format == OMX_VIDEO_CodingUnused) {
#endif
    err = gst_omx_port_allocate_buffers (self->port[CAMERA_VIDEO_OUT]);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while allocating buffers for "
          "camera video output port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
#ifdef USE_OMX_TARGET_RPI
  }
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_port_allocate_buffers (self->port[ENCODER_OUT]);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while allocating buffers for "
          "encoder output port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  // State to executing
  err = gst_omx_component_set_state (self->comp[CAMERA], OMX_StateExecuting);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting camera state to "
        "executing: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->comp[CAMERA],
      OMX_StateExecuting, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Camera didn't switch to executing "
        "state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

#ifdef USE_OMX_TARGET_RPI
  err = gst_omx_component_set_state (self->comp[NULL_SINK], OMX_StateExecuting);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting null sink state to "
        "executing: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->comp[NULL_SINK],
      OMX_StateExecuting, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Null sink didn't switch to executing "
        "state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_component_set_state (self->comp[ENCODER], OMX_StateExecuting);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while setting encoder state to "
          "executing: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    err = gst_omx_component_wait_state_changed (self->comp[ENCODER],
        OMX_StateExecuting, 1 * GST_SECOND);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Encoder didn't switch to executing "
          "state: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  // Capture on
  GST_OMX_INIT_STRUCT (&capture);
  capture.nPortIndex = self->port[CAMERA_VIDEO_OUT]->index;
  capture.bEnabled = TRUE;
  err = gst_omx_component_set_parameter (self->comp[CAMERA],
      OMX_IndexConfigPortCapturing, &capture);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while enabling video capture on "
        "camera video output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  // On RPi, populate either video output if no encoder used
  // or encoder if encoder used
#ifdef USE_OMX_TARGET_RPI
  if (self->port_format->compression_format == OMX_VIDEO_CodingUnused) {
#endif
    err = gst_omx_port_populate (self->port[CAMERA_VIDEO_OUT]);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while populating camera video output port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    err = gst_omx_port_mark_reconfigured (self->port[CAMERA_VIDEO_OUT]);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while marking camera video output port reconfigured: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
#ifdef USE_OMX_TARGET_RPI
  }
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_port_populate (self->port[ENCODER_OUT]);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while populating encoder output port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    err = gst_omx_port_mark_reconfigured (self->port[ENCODER_OUT]);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while marking encoder output port reconfigured: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif
  res = TRUE;

done:
  GST_DEBUG_OBJECT (self, "Video capturing started, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  GST_LIVE_UNLOCK (self);

  return res;
}

static gboolean
gst_omx_camera_src_stop_capturing (GstOMXCameraSrc * self)
{
  OMX_CONFIG_PORTBOOLEANTYPE capture;
  OMX_ERRORTYPE err;
  gboolean res;

  GST_LIVE_LOCK (self);

  GST_DEBUG_OBJECT (self, "Stopping video capture");

  if (!self->comp[CAMERA]) {
    GST_DEBUG_OBJECT (self, "Camera not opened, return");
    res = TRUE;
    goto done;
  }

  if (gst_omx_component_get_state (self->comp[CAMERA], GST_CLOCK_TIME_NONE) !=
      OMX_StateExecuting) {
    GST_DEBUG_OBJECT (self, "Camera not in executing state, return");
    res = TRUE;
    goto done;
  }

#ifdef USE_OMX_TARGET_RPI
  if (!self->comp[NULL_SINK]) {
    GST_DEBUG_OBJECT (self, "Null sink not opened, return");
    res = TRUE;
    goto done;
  }

  if (gst_omx_component_get_state (self->comp[NULL_SINK], GST_CLOCK_TIME_NONE) !=
      OMX_StateExecuting) {
    GST_DEBUG_OBJECT (self, "Null sink not in executing state, return");
    res = TRUE;
    goto done;
  }

  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    if (!self->comp[ENCODER]) {
      GST_DEBUG_OBJECT (self, "Encoder not opened, return");
      res = TRUE;
      goto done;
    }

    if (gst_omx_component_get_state (self->comp[ENCODER], GST_CLOCK_TIME_NONE) !=
        OMX_StateExecuting) {
      GST_DEBUG_OBJECT (self, "Encoder not in executing state, return");
      res = TRUE;
      goto done;
    }
  }
#endif

  // Capture off
  GST_OMX_INIT_STRUCT (&capture);
  capture.nPortIndex = self->port[CAMERA_VIDEO_OUT]->index;
  capture.bEnabled = FALSE;
  err = gst_omx_component_set_parameter (self->comp[CAMERA],
      OMX_IndexConfigPortCapturing, &capture);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while disabling video capture on "
        "camera video output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  // Flush buffers
  err = gst_omx_port_set_flushing (self->port[CAMERA_IN],
      1 * GST_SECOND, TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while flushing camera input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_flushing (self->port[CAMERA_VIDEO_OUT],
      1 * GST_SECOND, TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while flushing camera video output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#ifdef USE_OMX_TARGET_RPI
  err = gst_omx_port_set_flushing (self->port[CAMERA_PREVIEW_OUT],
      1 * GST_SECOND, TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while flushing camera preview output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_flushing (self->port[NULL_SINK_IN],
      1 * GST_SECOND, TRUE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self,
        "Error while flushing null sink input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_port_set_flushing (self->port[ENCODER_IN],
        1 * GST_SECOND, TRUE);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while flushing encoder input port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    err = gst_omx_port_set_flushing (self->port[ENCODER_OUT],
        1 * GST_SECOND, TRUE);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while flushing encoder output port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  // Disable ports
  err = gst_omx_port_set_enabled (self->port[CAMERA_IN], FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while disabling "
        "camera input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_port_set_enabled (self->port[CAMERA_VIDEO_OUT], FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while disabling "
        "camera video output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#ifdef USE_OMX_TARGET_RPI
  err = gst_omx_port_set_enabled (self->port[CAMERA_PREVIEW_OUT], FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while disabling "
        "camera preview output port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }

  err = gst_omx_port_set_enabled (self->port[NULL_SINK_IN], FALSE);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while disabling "
        "null sink input port: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_port_set_enabled (self->port[ENCODER_IN], FALSE);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while disabling "
          "encoder input port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    err = gst_omx_port_set_enabled (self->port[ENCODER_OUT], FALSE);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while disabling "
          "encoder output port: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  // Free buffers
  err = gst_omx_port_deallocate_buffers (self->port[CAMERA_IN]);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while dellocating "
        "camera input port buffers: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#ifdef USE_OMX_TARGET_RPI
  if (self->port_format->compression_format == OMX_VIDEO_CodingUnused) {
#endif
    err = gst_omx_port_deallocate_buffers (self->port[CAMERA_VIDEO_OUT]);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while dellocating "
          "camera video output port buffers: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
#ifdef USE_OMX_TARGET_RPI
  }
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_port_deallocate_buffers (self->port[ENCODER_OUT]);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while dellocating "
          "encoder output port buffers: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  // State to idle
  err = gst_omx_component_set_state (self->comp[CAMERA], OMX_StateIdle);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting camera state "
        "to idle: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->comp[CAMERA],
      OMX_StateIdle, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Camera didn't switch to idle state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#ifdef USE_OMX_TARGET_RPI
  err = gst_omx_component_set_state (self->comp[NULL_SINK], OMX_StateIdle);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting null sink state "
        "to idle: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->comp[NULL_SINK],
      OMX_StateIdle, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Null sink didn't switch to idle state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_component_set_state (self->comp[ENCODER], OMX_StateIdle);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while setting encoder state "
          "to idle: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    err = gst_omx_component_wait_state_changed (self->comp[ENCODER],
        OMX_StateIdle, 1 * GST_SECOND);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Encoder didn't switch to idle state: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

  // State to loaded
  err = gst_omx_component_set_state (self->comp[CAMERA], OMX_StateLoaded);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting camera state "
        "to loaded: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->comp[CAMERA],
      OMX_StateLoaded, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Camera didn't switch "
        "to loaded state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
#ifdef USE_OMX_TARGET_RPI
  err = gst_omx_component_set_state (self->comp[NULL_SINK], OMX_StateLoaded);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Error while setting null sink state "
        "to loaded: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  err = gst_omx_component_wait_state_changed (self->comp[NULL_SINK],
      OMX_StateLoaded, 1 * GST_SECOND);
  if (err != OMX_ErrorNone) {
    GST_ERROR_OBJECT (self, "Null sink didn't switch "
        "to loaded state: %s (0x%08x)",
        gst_omx_error_to_string (err), err);
    res = FALSE;
    goto done;
  }
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    err = gst_omx_component_set_state (self->comp[ENCODER], OMX_StateLoaded);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Error while setting encoder state "
          "to loaded: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
    err = gst_omx_component_wait_state_changed (self->comp[ENCODER],
        OMX_StateLoaded, 1 * GST_SECOND);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self, "Encoder didn't switch to loaded state: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    }
  }
#endif

done:
  GST_DEBUG_OBJECT (self, "Video capturing stopped, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  GST_LIVE_UNLOCK (self);

  return res;
}

static
gboolean gst_omx_camera_src_poll_buffer (GstOMXCameraSrc *self,
    GstBuffer *buffer)
{
  GstOMXBuffer *buf;
  GstOMXAcquireBufferReturn acq_return;
  GstVideoFrame frame;
  gint *max_spans = NULL, *valid_spans = NULL;
  gint total_size;
  gint dst_offset, src_offset;
  gint buf_num = 0, buf_bytes_copied, total_bytes_copied = 0;
  gint buf_discard_slices;
  gint span_size_bytes = 0;
  gint i;
  gboolean end_of_buf = FALSE;
  OMX_ERRORTYPE err;
  gboolean res;

  if (self->omx_buf_info) {
    // The frame content is fragmented into several OMX buffers, each
    // port_def.format.video.nStride wide port_def.format.video.nSliceHeight
    // tall. We shall loop until we get enough buffers so that the whole frame
    // has been fully received, hopefully.
    gint frame_height = GST_VIDEO_INFO_HEIGHT (self->info);
    gint buf_slice_height = GST_VIDEO_INFO_HEIGHT (self->omx_buf_info);
    buf_discard_slices =
                (buf_slice_height && (frame_height % buf_slice_height))
              ? (buf_slice_height - (frame_height % buf_slice_height))
              : 0;
    total_size = GST_VIDEO_INFO_SIZE (self->info);
    if (self->port_format->is_packed) {
      if (!gst_video_frame_map (&frame, self->info, buffer, GST_MAP_WRITE)) {
        GST_DEBUG_OBJECT (self, "Invalid frame");
        res = FALSE;
        goto done;
      }

      max_spans = g_new0 (gint, GST_VIDEO_FRAME_N_COMPONENTS (&frame));
      valid_spans = g_new0 (gint, GST_VIDEO_FRAME_N_COMPONENTS (&frame));
      for (i = 0; i < GST_VIDEO_FRAME_N_COMPONENTS (&frame); i++) {
        max_spans[i] = GST_VIDEO_INFO_COMP_HEIGHT(self->omx_buf_info, i);
      }
    }
  } else {
    total_size = gst_buffer_get_size (buffer);
  }

  res = TRUE;

  while (!end_of_buf) {

    acq_return = gst_omx_port_acquire_buffer (self->omx_buf_port, &buf);

    switch (acq_return) {
      case GST_OMX_ACQUIRE_BUFFER_FLUSHING:
      case GST_OMX_ACQUIRE_BUFFER_RECONFIGURE:
      case GST_OMX_ACQUIRE_BUFFER_EOS:
      case GST_OMX_ACQUIRE_BUFFER_ERROR:
        goto loop_done;
      case GST_OMX_ACQUIRE_BUFFER_OK:
        break;
    }

    if (buf->omx_buf->nFlags & OMX_BUFFERFLAG_DATACORRUPT) {
      GST_ERROR_OBJECT (self, "Corrupted data in buffer");
      goto loop_error;
    }

    end_of_buf = buf->omx_buf->nFlags & OMX_BUFFERFLAG_ENDOFFRAME;

    buf_bytes_copied = 0;
    if (self->omx_buf_info) {
      if (buf->omx_buf->nFilledLen !=
          GST_VIDEO_INFO_SIZE (self->omx_buf_info)) {
        GST_ERROR_OBJECT (self, "Received an unexpected amount of data "
            "in the OMX buffer, %d vs. %d bytes",
            buf->omx_buf->nFilledLen, GST_VIDEO_INFO_SIZE (self->omx_buf_info));
        goto loop_error;
      }
    }

    if (self->port_format->is_packed) {

      for (i = 0; i < GST_VIDEO_FRAME_N_COMPONENTS (&frame); i++) {
        valid_spans[i] = max_spans[i] - (end_of_buf ? buf_discard_slices : 0);
        dst_offset = buf_num * max_spans[i] *
          GST_VIDEO_FRAME_PLANE_STRIDE (&frame, i);
        src_offset = GST_VIDEO_INFO_PLANE_OFFSET (self->omx_buf_info, i);
        span_size_bytes = GST_VIDEO_INFO_PLANE_STRIDE (self->omx_buf_info, i)
          * valid_spans[i];
        buf_bytes_copied += span_size_bytes;
        if (buf_bytes_copied > GST_VIDEO_INFO_SIZE (self->omx_buf_info)) {
          GST_ERROR_OBJECT (self, "Wanted to copy too much from the OMX buffer, "
            "%d vs. %d bytes!",
            buf_bytes_copied, GST_VIDEO_INFO_SIZE (self->omx_buf_info));
          goto loop_error;
        }
        total_bytes_copied += span_size_bytes;
        if (total_bytes_copied > total_size) {
          GST_ERROR_OBJECT (self, "Buffers received from camera won't "
            "fit in the frame, %d vs. %d bytes!", total_bytes_copied, total_size);
          goto loop_error;
        }
        memcpy ((gchar *)frame.data[i] + dst_offset,
          buf->omx_buf->pBuffer + buf->omx_buf->nOffset + src_offset,
          span_size_bytes);
      }
    } else {
      if (total_bytes_copied + buf->omx_buf->nFilledLen > total_size) {
        GST_ERROR_OBJECT (self, "Data received from hardware won't "
          "fit in the buffer, %d vs. %d bytes!",
          total_bytes_copied + buf->omx_buf->nFilledLen,
          total_size);
        goto loop_error;
      }
      buf_bytes_copied = gst_buffer_fill (buffer,
          total_bytes_copied,
          buf->omx_buf->pBuffer + buf->omx_buf->nOffset,
          buf->omx_buf->nFilledLen);
      if (buf_bytes_copied != buf->omx_buf->nFilledLen) {
        GST_ERROR_OBJECT (self, "Bytes lost when copying buffers from "
          "hardware, %d vs. %d bytes!",
          buf_bytes_copied, buf->omx_buf->nFilledLen);
      }
      total_bytes_copied += buf_bytes_copied;
    }

loop_done:
    buf_num++;

    GST_LOG_OBJECT (self, "Processed OMX buffer %d %d bytes, "
        "flags 0x%08x, %d/%d buffer bytes",
        buf_num, buf->omx_buf->nFilledLen, buf->omx_buf->nFlags,
        total_bytes_copied, total_size);

    goto loop_exit;

loop_error:
    res = FALSE;
    end_of_buf = TRUE;

loop_exit:
    err = gst_omx_port_release_buffer (self->omx_buf_port, buf);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while releasing buffer: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      end_of_buf = TRUE;
    }
  }

done:

  if (self->port_format->is_packed) {
    gst_video_frame_unmap (&frame);
    if (max_spans)
      g_free (max_spans);
    if (valid_spans)
      g_free (valid_spans);
  }

  return res;
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
  gboolean res;

  self = GST_OMX_CAMERA_SRC (push_src);

  if (!self->video_configured)
    return GST_FLOW_NOT_NEGOTIATED;

  GST_LOG_OBJECT (self,
      "Creating buffer from pool for frame %d", (gint) self->n_frames);

  while (!self->comp[CAMERA]) {
    GST_DEBUG_OBJECT (self, "Camera not opened, waiting");
    GST_LIVE_WAIT (self);
  }

#ifdef USE_OMX_TARGET_RPI
  if (self->port_format->compression_format != OMX_VIDEO_CodingUnused) {
    while (!self->comp[ENCODER]) {
      GST_DEBUG_OBJECT (self, "Encoder not opened, waiting");
      GST_LIVE_WAIT (self);
    }
  }
#endif

  while (!gst_omx_camera_src_is_capture_active_unlocked (self)) {
    GST_DEBUG_OBJECT (self, "Camera not capturing, waiting");
    GST_LIVE_WAIT (self);
  }

  res = gst_omx_camera_src_poll_buffer (self, buffer);

  // Set flags
  GST_BUFFER_FLAG_SET (buffer, GST_BUFFER_FLAG_LIVE);
  if (res) {
    GST_BUFFER_FLAG_SET (buffer,  GST_BUFFER_FLAG_MARKER);
  } else {
    GST_BUFFER_FLAG_SET (buffer, GST_BUFFER_FLAG_CORRUPTED |
      GST_BUFFER_FLAG_DROPPABLE);
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
  if (self->fps_d > 0) {
    next_time = gst_util_uint64_scale_int (self->n_frames * GST_SECOND,
        self->fps_d, self->fps_n);
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
  GstOMXSrcClass *klass = GST_OMX_SRC_GET_CLASS (self);
  gint in_port_index, out_port_index;
#ifdef USE_OMX_TARGET_RPI
  gint null_sink_in_port_index = -1,
       encoder_in_port_index = -1, encoder_out_port_index = -1;
#endif
  gboolean res;

  GST_LIVE_LOCK (self);

  GST_DEBUG_OBJECT (self, "Starting");

  self->running_time = 0;
  self->n_frames = 0;
  self->accum_frames = 0;
  self->accum_rtime = 0;

  self->comp[CAMERA] =
      gst_omx_component_new (GST_OBJECT_CAST (self), klass->cdata.core_name,
        klass->cdata.component_name, klass->cdata.component_role,
        klass->cdata.hacks);

  if (!self->comp[CAMERA]) {
    GST_ERROR_OBJECT (self, "Error while creating camera component");
    res = FALSE;
    goto done;
  }

  if (gst_omx_component_get_state (self->comp[CAMERA], GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_ERROR_OBJECT (self, "Camera state is not loaded");
    res = FALSE;
    goto done;
  }

  if (!gst_omx_component_add_all_ports (self->comp[CAMERA])) {
    GST_ERROR_OBJECT (self, "Error while adding ports to camera");
    res = FALSE;
    goto done;
  }

  if (!gst_omx_component_all_ports_set_enabled (self->comp[CAMERA], FALSE)) {
    GST_ERROR_OBJECT (self, "Error while disabling camera ports");
    res = FALSE;
    goto done;
  }

  in_port_index = klass->cdata.in_port_index;
  out_port_index = klass->cdata.out_port_index;

  self->port[CAMERA_IN] = gst_omx_component_get_port (self->comp[CAMERA],
      in_port_index);
  self->port[CAMERA_VIDEO_OUT] = gst_omx_component_get_port (self->comp[CAMERA],
      out_port_index);

  if (!self->port[CAMERA_IN] || !self->port[CAMERA_VIDEO_OUT]) {
    GST_ERROR_OBJECT (self, "Error while detecting camera ports");
    res = FALSE;
    goto done;
  }

#ifdef USE_OMX_TARGET_RPI
  // Preview port for camera
  self->port[CAMERA_PREVIEW_OUT] =
    gst_omx_component_get_port (self->comp[CAMERA], out_port_index - 1);

  if (!self->port[CAMERA_PREVIEW_OUT]) {
    GST_ERROR_OBJECT (self, "Error while opening camera preview port");
    res = FALSE;
    goto done;
  }

  // Null sink
  self->comp[NULL_SINK] =
      gst_omx_component_new (GST_OBJECT_CAST (self), klass->cdata.core_name,
        "OMX.broadcom.null_sink", NULL,
        0);

  if (!self->comp[NULL_SINK]) {
    GST_ERROR_OBJECT (self, "Error while creating null sink component");
    res = FALSE;
    goto done;
  }

  if (gst_omx_component_get_state (self->comp[NULL_SINK], GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_ERROR_OBJECT (self, "Null sink state is not loaded");
    res = FALSE;
    goto done;
  }

  if (!gst_omx_component_add_all_ports (self->comp[NULL_SINK])) {
    GST_ERROR_OBJECT (self, "Error while adding ports to null sink");
    res = FALSE;
    goto done;
  }

  if (!gst_omx_component_all_ports_set_enabled (self->comp[NULL_SINK], FALSE)) {
    GST_ERROR_OBJECT (self, "Error while disabling null sink ports");
    res = FALSE;
    goto done;
  }

  {
    OMX_PORT_PARAM_TYPE param;
    OMX_ERRORTYPE err;

    GST_OMX_INIT_STRUCT (&param);

    err =
        gst_omx_component_get_parameter (self->comp[NULL_SINK], OMX_IndexParamVideoInit,
        &param);
    if (err != OMX_ErrorNone) {
      GST_WARNING_OBJECT (self, "Couldn't get port information: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    } else {
      GST_DEBUG_OBJECT (self, "Detected %u ports, starting at %u",
          (guint) param.nPorts, (guint) param.nStartPortNumber);
      null_sink_in_port_index = param.nStartPortNumber;
    }
  }

  // In port for null sink
  self->port[NULL_SINK_IN] =
    gst_omx_component_get_port (self->comp[NULL_SINK], null_sink_in_port_index);

  if (!self->port[NULL_SINK_IN]) {
    GST_ERROR_OBJECT (self, "Error while opening null sink in port");
    res = FALSE;
    goto done;
  }

  // Encoder
  self->comp[ENCODER] =
      gst_omx_component_new (GST_OBJECT_CAST (self), klass->cdata.core_name,
        "OMX.broadcom.video_encode", NULL,
        0);

  if (!self->comp[ENCODER]) {
    GST_ERROR_OBJECT (self, "Error while creating encoder component");
    res = FALSE;
    goto done;
  }

  if (gst_omx_component_get_state (self->comp[ENCODER], GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_ERROR_OBJECT (self, "Encoder state is not loaded");
    res = FALSE;
    goto done;
  }

  if (!gst_omx_component_add_all_ports (self->comp[ENCODER])) {
    GST_ERROR_OBJECT (self, "Error while adding ports to encoder");
    res = FALSE;
    goto done;
  }

  if (!gst_omx_component_all_ports_set_enabled (self->comp[ENCODER], FALSE)) {
    GST_ERROR_OBJECT (self, "Error while disabling encoder ports");
    res = FALSE;
    goto done;
  }

  {
    OMX_PORT_PARAM_TYPE param;
    OMX_ERRORTYPE err;

    GST_OMX_INIT_STRUCT (&param);

    err =
        gst_omx_component_get_parameter (self->comp[ENCODER], OMX_IndexParamVideoInit,
        &param);
    if (err != OMX_ErrorNone) {
      GST_WARNING_OBJECT (self, "Couldn't get port information: %s (0x%08x)",
          gst_omx_error_to_string (err), err);
      res = FALSE;
      goto done;
    } else {
      GST_DEBUG_OBJECT (self, "Detected %u ports, starting at %u",
          (guint) param.nPorts, (guint) param.nStartPortNumber);
      encoder_in_port_index = param.nStartPortNumber + 0;
      encoder_out_port_index = param.nStartPortNumber + 1;
    }
  }

  // In and out ports for encoder
  self->port[ENCODER_IN] =
    gst_omx_component_get_port (self->comp[ENCODER], encoder_in_port_index);

  if (!self->port[ENCODER_IN]) {
    GST_ERROR_OBJECT (self, "Error while opening encoder in port");
    res = FALSE;
    goto done;
  }
  self->port[ENCODER_OUT] =
    gst_omx_component_get_port (self->comp[ENCODER], encoder_out_port_index);

  if (!self->port[ENCODER_OUT]) {
    GST_ERROR_OBJECT (self, "Error while opening encoder out port");
    res = FALSE;
    goto done;
  }
#endif

  res = gst_omx_camera_src_configure_camera_unlocked (self);

done:
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

  GST_LIVE_UNLOCK (self);

  return res;
}

static gboolean
gst_omx_camera_src_stop (GstBaseSrc * base_src)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (base_src);
  gboolean res;

  GST_LIVE_LOCK (self);

  GST_DEBUG_OBJECT (self, "Stopping");

  if (!gst_omx_camera_src_component_state_to_loaded_unlocked (self,
        self->comp[CAMERA])) {
    GST_ERROR_OBJECT (self, "Error while switching camera to loaded state");
  }

#ifdef USE_OMX_TARGET_RPI
  if (!gst_omx_camera_src_component_state_to_loaded_unlocked (self,
        self->comp[NULL_SINK])) {
    GST_ERROR_OBJECT (self, "Error while switching null sink to loaded state");
  }

  if (!gst_omx_camera_src_component_state_to_loaded_unlocked (self,
        self->comp[ENCODER])) {
    GST_ERROR_OBJECT (self, "Error while switching encoder to loaded state");
  }
#endif

  self->port[CAMERA_IN] = NULL;
  self->port[CAMERA_VIDEO_OUT] = NULL;
#ifdef USE_OMX_TARGET_RPI
  self->port[CAMERA_PREVIEW_OUT] = NULL;
  self->port[NULL_SINK_IN] = NULL;
  self->port[ENCODER_IN] = NULL;
  self->port[ENCODER_OUT] = NULL;
#endif

  if (self->comp[CAMERA])
    gst_omx_component_free (self->comp[CAMERA]);
  self->comp[CAMERA] = NULL;
#ifdef USE_OMX_TARGET_RPI
  if (self->comp[NULL_SINK])
    gst_omx_component_free (self->comp[NULL_SINK]);
  self->comp[NULL_SINK] = NULL;
  if (self->comp[ENCODER])
    gst_omx_component_free (self->comp[ENCODER]);
  self->comp[ENCODER] = NULL;
#endif

  self->camera_configured = FALSE;
  self->video_configured = FALSE;

  res = TRUE;

  GST_DEBUG_OBJECT (self, "Stopped, %s", (res ? "ok" : "failing"));

  GST_LIVE_UNLOCK (self);

  return res;
}

static GstCaps * gst_omx_camera_src_get_caps (GstBaseSrc * base_src,
    GstCaps * filter)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (base_src);
  GstCaps *caps, *new_caps = NULL;
  GSList *port_formats, *port_format;

  GST_LIVE_LOCK (self);

  GST_DEBUG_OBJECT (self, "Detecting capabilities");

  if (!self->comp[CAMERA]) {
    GST_DEBUG_OBJECT (self, "Camera not opened, return");
    goto done;
  }

  while (gst_omx_component_get_state (self->comp[CAMERA], GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_DEBUG_OBJECT (self, "Camera not in loaded state, waiting");
    GST_LIVE_WAIT (self);
  }

#ifdef USE_OMX_TARGET_RPI
  if (!self->comp[ENCODER]) {
    GST_DEBUG_OBJECT (self, "Encoder not opened, return");
    goto done;
  }

  while (gst_omx_component_get_state (self->comp[ENCODER], GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_DEBUG_OBJECT (self, "Encoder not in loaded state, waiting");
    GST_LIVE_WAIT (self);
  }
#endif

  port_formats = gst_omx_camera_src_probe_all_formats_unlocked (self);

  new_caps = gst_caps_new_empty ();

  for (port_format = port_formats; port_format;
      port_format = g_slist_next (port_format)) {
    GstOMXCameraSrcPortFormat *format = g_slist_nth_data (port_format, 0);
    if (format && format->caps_template) {
      gst_caps_append (new_caps, gst_caps_copy (format->caps_template));
    }
  }

  if (gst_caps_is_empty (new_caps)) {
    GST_ERROR_OBJECT (self, "No supported output formats handled by the hardware");
    gst_caps_unref (new_caps);
    new_caps = NULL;
  }

done:
  if (!new_caps)
      new_caps = GST_BASE_SRC_CLASS (parent_class)->get_caps (base_src, NULL);

  if (filter) {
    caps = gst_caps_intersect (filter, new_caps);
    gst_caps_unref (new_caps);
  } else {
    caps = new_caps;
  }

  GST_DEBUG_OBJECT (self, "Returning caps %" GST_PTR_FORMAT, caps);

  GST_LIVE_UNLOCK (self);

  return caps;
}

static GstCaps * gst_omx_camera_src_fixate (GstBaseSrc * base_src,
    GstCaps * caps)
{
  GstOMXCameraSrc * self = GST_OMX_CAMERA_SRC (base_src);

  GstStructure *structure;

  GST_LIVE_LOCK (self);

  GST_DEBUG_OBJECT (self, "Fixating caps %" GST_PTR_FORMAT, caps);

  caps = gst_caps_truncate (caps);
  structure = gst_caps_get_structure (caps, 0);

  gst_structure_fixate_field_nearest_int (structure, "width",
      GST_OMX_CAMERA_SRC_WIDTH_DEFAULT);
  gst_structure_fixate_field_nearest_int (structure, "height",
      GST_OMX_CAMERA_SRC_HEIGHT_DEFAULT);
  gst_structure_fixate_field_nearest_fraction (structure, "framerate",
      GST_OMX_CAMERA_SRC_FRAMERATE_DEFAULT, 1);

  if (!gst_caps_is_fixed (caps))
    caps = gst_caps_fixate (caps);

  GST_DEBUG_OBJECT (self, "Fixated caps %" GST_PTR_FORMAT, caps);

  GST_LIVE_UNLOCK (self);

  return caps;
}

static gboolean gst_omx_camera_src_set_caps (GstBaseSrc * base_src,
    GstCaps * caps)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (base_src);
  GSList *formats, *format;
  const GstStructure *structure;
  const GValue *framerate;
  gboolean res;

  GST_LIVE_LOCK (self);

  GST_DEBUG_OBJECT (self, "Setting format %" GST_PTR_FORMAT, caps);

  if (!self->comp[CAMERA]) {
    GST_DEBUG_OBJECT (self, "Camera not opened, return");
    goto error;
  }

  while (gst_omx_component_get_state (self->comp[CAMERA], GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_DEBUG_OBJECT (self, "Camera not in loaded state, waiting");
    GST_LIVE_WAIT (self);
  }

#ifdef USE_OMX_TARGET_RPI
  if (!self->comp[ENCODER]) {
    GST_DEBUG_OBJECT (self, "Encoder not opened, return");
    goto error;
  }

  while (gst_omx_component_get_state (self->comp[ENCODER], GST_CLOCK_TIME_NONE) !=
      OMX_StateLoaded) {
    GST_DEBUG_OBJECT (self, "Encoder not in loaded state, waiting");
    GST_LIVE_WAIT (self);
  }
#endif

  // Find port format matching the caps
  formats = gst_omx_camera_src_probe_all_formats_unlocked (self);

  for (format = formats; format; format = g_slist_next (format)) {
    GstOMXCameraSrcPortFormat *port_format = g_slist_nth_data (format, 0);
    if (port_format && port_format->caps_template &&
        gst_caps_is_subset (caps, port_format->caps_template)) {
      GST_DEBUG_OBJECT (self, "Found the superset caps for port format %"
          GST_PTR_FORMAT, port_format->caps_template);
      self->port_format = port_format;
      break;
    }
  }

  if (!self->port_format) {
    GST_ERROR_OBJECT (self, "Error while locating port format");
    goto error;
  }

  if (self->port_format->compression_format == OMX_VIDEO_CodingUnused) {
    if (!self->info)
      self->info = g_new (GstVideoInfo, 1);

    gst_video_info_init (self->info);

    if (!gst_video_info_from_caps (self->info, caps)) {
      GST_ERROR_OBJECT (self, "Invalid format %" GST_PTR_FORMAT, caps);
      if (self->info) {
        g_free (self->info);
        self->info = NULL;
      }
      goto error;
    }
  }

  structure = gst_caps_get_structure (caps, 0);
  self->width = g_value_get_int (gst_structure_get_value (structure,
        "width"));
  self->height = g_value_get_int (gst_structure_get_value (structure,
        "height"));
  framerate = gst_structure_get_value (structure, "framerate");
  self->fps_n = gst_value_get_fraction_numerator (framerate);
  self->fps_d = gst_value_get_fraction_denominator (framerate);
  gst_util_fraction_to_double (self->fps_n, self->fps_d, &self->framerate);

  self->accum_rtime += self->running_time;
  self->accum_frames += self->n_frames;
  self->running_time = 0;
  self->n_frames = 0;
#ifdef USE_OMX_TARGET_RPI
  self->omx_buf_port =
    self->port_format->compression_format == OMX_VIDEO_CodingUnused
    ? self->port[CAMERA_VIDEO_OUT]
    : self->port[ENCODER_OUT];
#else
  self->omx_buf_port = self->port[CAMERA];
#endif

  res = gst_omx_camera_src_configure_video_unlocked (self);

  if (res &&
      self->omx_buf_port->port_def.format.video.nSliceHeight &&
      self->port_format->compression_format == OMX_VIDEO_CodingUnused) {
    GstCaps *omx_buf_caps;

    if (!self->omx_buf_info)
      self->omx_buf_info = g_new (GstVideoInfo, 1);

    gst_video_info_init (self->omx_buf_info);

    omx_buf_caps = gst_caps_copy (caps);
    gst_caps_set_simple (omx_buf_caps, "width", G_TYPE_INT,
        self->port[CAMERA_VIDEO_OUT]->port_def.format.video.nStride,
        NULL);
    gst_caps_set_simple (omx_buf_caps, "height", G_TYPE_INT,
        self->port[CAMERA_VIDEO_OUT]->port_def.format.video.nSliceHeight,
        NULL);

    if (!gst_video_info_from_caps (self->omx_buf_info, omx_buf_caps)) {
      GST_ERROR_OBJECT (self, "Invalid format %" GST_PTR_FORMAT, omx_buf_caps);
      if (self->omx_buf_info) {
        g_free (self->omx_buf_info);
        self->omx_buf_info = NULL;
      }
      res = FALSE;
    }
    if (omx_buf_caps)
      gst_caps_unref (omx_buf_caps);
  }

done:
  GST_DEBUG_OBJECT (self, "Set caps, %s", (res ? "ok" : "failing"));

  GST_LIVE_BROADCAST (self);

  GST_LIVE_UNLOCK (self);

  return res;

error:
  self->info = NULL;
  self->port_format = NULL;
  res = FALSE;
  goto done;
}

static gboolean gst_omx_camera_src_decide_allocation (GstBaseSrc *base_src,
    GstQuery *query)
{
  GstOMXCameraSrc *self = GST_OMX_CAMERA_SRC (base_src);

  GstBufferPool *pool;
  guint buf_size, size, min, max;
  GstStructure *config;
  gboolean update;
  OMX_ERRORTYPE err;

  GST_LIVE_LOCK (self);

  while (!self->port_format) {
    GST_DEBUG_OBJECT (self, "Video capabilities not negotiated, waiting");
    GST_LIVE_WAIT (self);
  }

  if (self->port_format->compression_format == OMX_VIDEO_CodingUnused) {
    buf_size = GST_VIDEO_INFO_SIZE (self->info);
  } else {
    err = gst_omx_port_update_port_definition (self->omx_buf_port, NULL);
    if (err != OMX_ErrorNone) {
      GST_ERROR_OBJECT (self,
          "Error while getting port definition for "
          "%s port %u: %s (0x%08x)",
          self->omx_buf_port->comp->name, self->omx_buf_port->index,
          gst_omx_error_to_string (err), err);
      buf_size = 0;
    } else {
      buf_size = self->omx_buf_port->port_def.nBufferSize;
    }
  }

  if (gst_query_get_n_allocation_pools (query) > 0) {
    gst_query_parse_nth_allocation_pool (query, 0, &pool, &size, &min, &max);

    size = MAX (size, buf_size);
    update = TRUE;
  } else {
    pool = NULL;
    size = buf_size;
    min = max = 0;
    update = FALSE;
  }

  /* no downstream pool, make our own */
  if (pool == NULL) {
    pool = self->port_format->is_packed
      ? gst_video_buffer_pool_new ()
      : gst_buffer_pool_new ();
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
gst_omx_camera_src_change_state (GstElement * element,
    GstStateChange transition)
{
  GstOMXCameraSrc *self;
  GstStateChangeReturn res;

  self = GST_OMX_CAMERA_SRC (element);

  switch (transition) {
    case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
      if (!gst_omx_camera_src_start_capturing (self)) {
        res = GST_STATE_CHANGE_FAILURE;
        goto done;
      }
      break;
    default: break;
  }

  res = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);

  switch (transition) {
    case GST_STATE_CHANGE_PLAYING_TO_PAUSED:
      if (!gst_omx_camera_src_stop_capturing (self))
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

  if (self->comp)
    g_free (self->comp);

  if (self->port)
    g_free (self->port);

  if (self->info)
    g_free (self->info);

  if (self->omx_buf_info)
    g_free (self->omx_buf_info);

  if (self->all_port_formats) {
    GList *values = g_hash_table_get_values (self->all_port_formats);
    if (values) {
      GList *value;
      for (value = values; value; value = g_list_next (value)) {
        GSList *port_formats = (GSList *)g_list_nth_data (value, 0);
        if (port_formats) {
          g_slist_free (port_formats);
        }
      }
      g_list_free (values);
    }
    g_hash_table_destroy (self->all_port_formats);
  }

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
    case PROP_CONTROL_RATE:
      self->config.bitrate.eControlRate = g_value_get_enum (value);
      break;
    case PROP_TARGET_BITRATE:
      self->config.bitrate.nTargetBitrate = g_value_get_uint (value);
      break;
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
    case PROP_COLOR_ENHANCEMENT_U_CHANNEL:
      GST_LIVE_LOCK (self);
      self->config.color_enhancement.bColorEnhancement = TRUE;
      self->config.color_enhancement.nCustomizedU = g_value_get_uint (value);
      GST_LIVE_UNLOCK (self);
      break;
    case PROP_COLOR_ENHANCEMENT_V_CHANNEL:
      GST_LIVE_LOCK (self);
      self->config.color_enhancement.bColorEnhancement = TRUE;
      self->config.color_enhancement.nCustomizedV = g_value_get_uint (value);
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
    case PROP_CONTROL_RATE:
      g_value_set_enum (value, self->config.bitrate.eControlRate);
      break;
    case PROP_TARGET_BITRATE:
      g_value_set_uint (value, self->config.bitrate.nTargetBitrate);
      break;
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
    case PROP_COLOR_ENHANCEMENT_U_CHANNEL:
      g_value_set_uint (value, self->config.color_enhancement.nCustomizedU);
      break;
    case PROP_COLOR_ENHANCEMENT_V_CHANNEL:
      g_value_set_uint (value, self->config.color_enhancement.nCustomizedV);
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
