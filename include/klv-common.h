#ifndef KLV_COMMON_H
#define KLV_COMMON_H

#include "klv-config.h"

#include <array>
#include <string>
#include <cstdint>

namespace klv {

namespace misb {

enum class standard_t : uint8_t
{
  /* --- Core Telemetry & Time --- */
  st_0601_uas_datalink,           // UAS Datalink Local Set (primary telemetry: lat/lon/alt/heading/sensor geometry)
  st_0603_misp_time,              // MISP Time System and Timestamps (precision time stamp pack)
  st_0604_class12_timestamp,      // Timestamps for Class 1/Class 2 Motion Imagery
  st_0605_microsecond_timestamp,  // Microsecond Precision Time Stamp (Class 0 MI over SDI)

  /* --- Security --- */
  st_0102_security,               // Security Metadata Universal and Local Sets (classification markings)

  /* --- Annotation & Targeting --- */
  st_0602_annotation,             // Annotation Metadata Set (Cursor-on-Target, points, lines, polygons)
  st_0903_vmti_track,             // Video Moving Target Indicator and Track Metadata

  /* --- Display & Terminal --- */
  st_0806_rvt,                    // Remote Video Terminal Metadata Set
  st_0808_ancillary_text,         // Ancillary Text Metadata Sets (deprecated)

  /* --- Sensor-Specific --- */
  st_0801_photogrammetry,         // Photogrammetry Metadata for Motion Imagery
  st_0809_meteorological,         // Meteorological Metadata Local Set
  st_1206_sar,                    // Synthetic Aperture Radar (SAR) Motion Imagery Metadata
  st_1602_composite_imaging,      // Composite Imaging Local Set
  st_1801_surface_profiles,       // Surface Profiles

  /* --- Identification --- */
  st_1204_miis_core,              // Motion Imagery Identification System (MIIS) Core Identifier
  st_1301_miis_augmentation,      // MIIS Augmentation Identifiers
  st_1504_orbital_vectors,        // Natural Representation of Orbital State Vectors
  st_2101_core_identifier,        // Core Identifier for Class 1/Class 2 Motion Imagery

  /* --- Legacy --- */
  eg_0104_predator,               // Predator UAV Basic Universal Metadata Set (deprecated)

  unknown = 0xFF
};

} /* namespace misb */

/*********/

enum class element_t : uint8_t
{
  Unconfigured   = 0x00,
  String,
  MappedNumeric,
  Timestamp,
  Checksum,
  Bitfield,
  MappedCode,

  NestedPack
};

struct metadata_t
{
  element_t   type           = element_t::Unconfigured;
  const char* name           = nullptr;
  const char* units          = nullptr;
  int64_t     domain_min     = 0;
  int64_t     domain_max     = 0;
  double      range_min      = 0.0;
  double      range_max      = 0.0;
  int64_t     special_value  = 0;
 
  bool        out_of_range   = false;
  double      cached_numeric = 0.0;
  std::string cached_string  = "";
};

enum class cb_result_t : uint8_t
{
  failed,
  success,
};

template <typename T>
concept callbacks_interface = requires(const T&              cb, 
                                       klv::misb::standard_t std,
                                       uint8_t               tag,
                                       const metadata_t&     meta,
                                       uint64_t              raw_time,
                                       uint32_t              checksum,
                                       uint8_t               flags,
                                       uint64_t              pts,
                                       uint32_t              target_id,
                                       uint8_t               confidence,
                                       double                x,
                                       double                y
                                      )
{
  requires(!std::copyable<T>);
  { cb.on_unconfigured_tag (std, tag)                   } -> std::same_as<cb_result_t>;
  { cb.on_numeric_tag      (std, tag, meta)             } -> std::same_as<cb_result_t>;
  { cb.on_string_tag       (std, tag, meta)             } -> std::same_as<cb_result_t>;
  { cb.on_timestamp_tag    (std, tag, meta, raw_time)   } -> std::same_as<cb_result_t>;
  { cb.on_checksum_tag     (std, tag, meta, checksum)   } -> std::same_as<cb_result_t>;
  { cb.on_bitfield_tag     (std, tag, meta, flags)      } -> std::same_as<cb_result_t>;
  { cb.on_pts              (std, pts)                   } -> std::same_as<cb_result_t>; /* Added for ST 0604 timestamps */
  { cb.on_vmti_target      (std, target_id, confidence) } -> std::same_as<cb_result_t>; /* Added for ST 0903 */
  { cb.on_viewport_position(std, tag, meta, x, y )      } -> std::same_as<cb_result_t>; /* ST 0602 Tag 18 viewport coordinate in pixels */

  { cb.on_invalid_key      ()                           } -> std::same_as<cb_result_t>; /* An invalid key have been detected */
  { cb.on_length_overflow  ()                           } -> std::same_as<cb_result_t>; /* Parsing error due wrong length */
  { cb.on_payload_truncated()                           } -> std::same_as<cb_result_t>; /* Parsing error due missing bytes accordingly with length */
};

} /* namespace klv */

#endif /* KLV_COMMON_H */
