#ifndef MISB_ST_0903_H
#define MISB_ST_0903_H

#include "misb-base.h"

namespace klv
{

namespace misb
{

// --- MISB ST 0903 VMTI Tag Dictionary Keys ---
namespace VmtiTags
{
  constexpr uint8_t Checksum                 =  1;  // MANDATORY: 2-byte CRC Checksum
  constexpr uint8_t PrecisionTimeStamp       =  2;  // 8-byte Microseconds Integer
  constexpr uint8_t SystemName               =  3;  // String (Name of VMTI source process)
  constexpr uint8_t SourceSensor             =  4;  // String identifying payload source
  constexpr uint8_t NumberDetectedTargets    =  5;  // Variable length integer counter
  constexpr uint8_t TargetReportSeries       =  6;  // Nested Pack/Local Set series container
  constexpr uint8_t AlgorithmSeries          =  7;  // Nested metadata describing target classifiers
  constexpr uint8_t VmtiHorizontalFov        = 10; // 2-byte Mapped angle [0, 180] degrees
  constexpr uint8_t VmtiVerticalFov          = 11; // 2-byte Mapped angle [0, 180] degrees
  constexpr uint8_t VmtiLdsVolumeCountryCode = 12; // 2-byte coding system identifier
  constexpr uint8_t VmtiVersion              = 14; // 1-byte Unsigned Integer standard version
}

// --- MISB ST 0903 Target Report Sub-Tags (Nested inside Master Tag 6) ---
namespace TargetReportTags
{
  constexpr uint8_t TargetCentroidPixel         =  1;  // 4-byte packed image crosshair (X, Y)
  constexpr uint8_t BoundingBoxTopLeftPixel     =  2;  // 4-byte packed rectangle limit (X, Y)
  constexpr uint8_t BoundingBoxBottomRightPixel =  3; // 4-byte packed rectangle limit (X, Y)
  constexpr uint8_t TargetPriority              =  4;  // 1-byte counter rating tracking import
  constexpr uint8_t TargetConfidence            =  5;  // 1-byte value rating detection accuracy (0-100%)
  constexpr uint8_t TargetHistoryLength         =  6;  // 2-byte counter tracing tracking path age
  constexpr uint8_t TargetCentroidLatLon        =  7;  // 8-byte global GPS coordinate position (Lat, Lon)
  constexpr uint8_t TargetCentroidElevation     =  8;  // 2-byte scaled height above sea level
  constexpr uint8_t TargetId                    = 11; // Variable bytes unique numeric ID tracking tag
  constexpr uint8_t TargetLocationArea          = 14; // Complete sub-polygon mapping footprint
  constexpr uint8_t TargetIntensity             = 16; // Average color/IR pixel value brightness matching chip
}


/* VMTI Metadata Set */
template <callbacks_interface callbacks_t>
class ST_0903_Parser : public parser_base<callbacks_t>
{
  using parser_base<callbacks_t>::callbacks;
  using parser_base<callbacks_t>::standard;
  using parser_base<callbacks_t>::max_tags;
  using parser_base<callbacks_t>::m_tags;
  using parser_base<callbacks_t>::m_tag_readers;
public:
  ST_0903_Parser(const callbacks_t& cb)
    : parser_base<callbacks_t>(klv::misb::standard_t::st_0903_vmti_track, 102, cb)
  {
    constexpr int64_t d_1b_u = 255;
    constexpr int64_t d_2b_u = 65535;

    // Map out the MISB ST 0903 standard tag layout coordinates
    m_tags[VmtiTags::Checksum]                 = { element_t::Checksum, "VMTI Checksum", "hex" };
    m_tags[VmtiTags::PrecisionTimeStamp]       = { element_t::Timestamp, "VMTI Precision Time Stamp", "microseconds" };
    m_tags[VmtiTags::SystemName]               = { element_t::String, "VMTI System Name" };
    m_tags[VmtiTags::SourceSensor]             = { element_t::String, "VMTI Source Sensor" };
    
    // Tags 5, 6, and 7 are structural packs/series containers rather than simple scalars.
    // For baseline parsing safety, we configure their names to prevent unconfigured alerts.
    m_tags[VmtiTags::NumberDetectedTargets]    = { element_t::Unconfigured, "Number of Detected Targets" };
    m_tags[VmtiTags::TargetReportSeries]       = { element_t::Unconfigured, "Target Report Series" };
    m_tags[VmtiTags::AlgorithmSeries]          = { element_t::Unconfigured, "Algorithm Series (Pack)" };

    m_tags[VmtiTags::VmtiHorizontalFov]        = { element_t::MappedNumeric, "VMTI Horizontal FOV", "degrees", 0, d_2b_u, 0.0, 180.0 };
    m_tags[VmtiTags::VmtiVerticalFov]          = { element_t::MappedNumeric, "VMTI Vertical FOV", "degrees", 0, d_2b_u, 0.0, 180.0 };
    m_tags[VmtiTags::VmtiLdsVolumeCountryCode] = { element_t::String, "VMTI LDS Volume Country Code" };
    m_tags[VmtiTags::VmtiVersion]              = { element_t::MappedNumeric, "VMTI Version Number", "version", 0, d_1b_u, 0.0, 255.0 };
  }

  [[nodiscard]]
  bool parse_packet(const uint8_t* stream, size_t size) noexcept(true)
  {
    size_t index = 0;
    while (index < size)
    {
      uint8_t tag = stream[index++];
      if ( (tag == 0) || (tag >= max_tags()) ) [[unlikely]]
        continue;

      if (index >= size)
        break;

      size_t length = 0;
      uint8_t len_byte = stream[index++];
      if (len_byte & 0x80)
      {
        size_t num_bytes = len_byte & 0x7F;
        if (index + num_bytes > size)
          break;

        for (size_t i = 0; i < num_bytes; ++i)
        { length = (length << 8) | stream[index++]; }
      }
      else
      {
        length = len_byte; 
      }

      if (index + length > size)
        break;

      auto&          meta    = m_tags[tag];
      const uint8_t* val_ptr = &stream[index];

      if (tag == VmtiTags::TargetReportSeries)
      {
        parse_nested_target_report(val_ptr, length);
      }
      else
      {
        if (m_tag_readers[static_cast<size_t>(meta.type)](this,tag,val_ptr,length,meta) != klv::cb_result_t::success)
          return false;
      }
      index += length;
    }

    return true;
  }

private:
  /**/
  inline void parse_nested_target_report(const uint8_t* val_ptr, size_t length)
  {
    size_t idx = 0;

    // Outer series loop tracking multiple targets packed contiguously inside Tag 6
    while (idx < length)
    {
      [[maybe_unused]] uint8_t target_pack_tag = val_ptr[idx++];
      if (idx >= length)
        break;

      // Parse nested target package block length
      size_t target_pack_len = 0;
      uint8_t len_byte = val_ptr[idx++];
      if (len_byte & 0x80)
      {
        size_t num_bytes = len_byte & 0x7F;
        if (idx + num_bytes > length) break;
        for (size_t i = 0; i < num_bytes; ++i)
        { 
          target_pack_len = (target_pack_len << 8) | val_ptr[idx++]; 
        }
      }
      else
      {
        target_pack_len = len_byte;
      }

      if (idx + target_pack_len > length)
        break;

      // Isolate the sub-window pointer context for this specific target
      const uint8_t* sub_ptr = &val_ptr[idx];
      size_t sub_idx = 0;

      uint32_t target_id = 0;
      uint8_t confidence = 0;

      // Scan inside the target report pack for fields defined in TargetReportTags
      while (sub_idx < target_pack_len)
      {
        uint8_t sub_tag = sub_ptr[sub_idx++];
        if (sub_idx >= target_pack_len)
          break;

        size_t sub_len = sub_ptr[sub_idx++]; // Standard 1-byte length inside packs
        if (sub_idx + sub_len > target_pack_len)
          break;

        if (sub_tag == TargetReportTags::TargetId)
        {
          target_id = static_cast<uint32_t>(parser_base<callbacks_t>::extract_raw_bytes(&sub_ptr[sub_idx], sub_len));
        }
        else if (sub_tag == TargetReportTags::TargetConfidence)
        {
          confidence = sub_ptr[sub_idx]; // 1-byte raw percentage value (0-100)
        }

        sub_idx += sub_len;
      }

      callbacks().on_vmti_target(standard(), target_id, confidence);

      idx += target_pack_len;
    }
  }

private:

};

} /* namespace misb */

} /* namespace klv */

#endif /* MISB_ST_0903_H */
