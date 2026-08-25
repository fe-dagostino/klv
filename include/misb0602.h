#ifndef MISB_ST_0602_H
#define MISB_ST_0602_H

#include "misb-base.h"

namespace klv 
{

namespace misb
{

// --- MISB ST 0602 Annotation Tag Dictionary Keys ---
namespace AnnotationTags
{
  constexpr uint8_t Checksum               =  1; // MANDATORY: 2-byte CRC Checksum
  constexpr uint8_t PrecisionTimeStamp     =  2; // MANDATORY: 8-byte Microseconds Integer
  constexpr uint8_t AnnotationSource       =  3; // String identifying annotation origin source
  constexpr uint8_t AnnotationSeries       =  4; // Nested Pack/Local Set series container for shapes
  constexpr uint8_t AnnotationVersion      =  5; // 1-byte Unsigned Integer standard version

  constexpr uint8_t ViewportPosition       = 18;  // Compound dual-axis position configuration pack
  constexpr uint8_t ViewportWidth          = 19;  // 3-byte unsigned percentage boundary metric
  constexpr uint8_t ViewportHeight         = 20;  // 3-byte unsigned percentage boundary metric
  constexpr uint8_t SpatialExtent          = 21;  // 3-byte localized scaling parameter factor  
}

/* Annotation Metadata Set Parser */
template <callbacks_interface callbacks_t>
class ST_0602_Parser : public parser_base<callbacks_t>
{
  using parser_base<callbacks_t>::callbacks;
  using parser_base<callbacks_t>::standard;
  using parser_base<callbacks_t>::max_tags;
  using parser_base<callbacks_t>::m_tags;
  using parser_base<callbacks_t>::m_tag_readers;

public:
  /***/
  ST_0602_Parser(callbacks_t cb = callbacks_t{})
    : parser_base<callbacks_t>(klv::misb::standard_t::st_0602_annotation, 22, cb)
  {
    constexpr int64_t d_1b_u = 255;
    constexpr int64_t d_3b_u = 16777215; 

    // Map out the MISB ST 0602 standard tag layout rules
    m_tags[AnnotationTags::Checksum]           = { element_t::Checksum     , "Annotation Checksum", "hex" };
    m_tags[AnnotationTags::PrecisionTimeStamp] = { element_t::Timestamp    , "Annotation Precision Time Stamp", "microseconds" };
    m_tags[AnnotationTags::AnnotationSource]   = { element_t::String       , "Annotation Origin Source" };
    m_tags[AnnotationTags::AnnotationSeries]   = { element_t::NestedPack   , "Annotation Series Master Pack" };
    m_tags[AnnotationTags::AnnotationVersion]  = { element_t::MappedNumeric, "Annotation Version Number", "version", 0, d_1b_u, 0.0, 255.0 };

    m_tags[AnnotationTags::ViewportPosition]   = { element_t::NestedPack,    "Viewport Position Pack" };
    m_tags[AnnotationTags::ViewportWidth]      = { element_t::MappedNumeric, "Viewport Width Pack"          , "%", 0, d_3b_u, 0.0, 100.0 };
    m_tags[AnnotationTags::ViewportHeight]     = { element_t::MappedNumeric, "Viewport Height Pack"         , "%", 0, d_3b_u, 0.0, 100.0 };
    m_tags[AnnotationTags::SpatialExtent]      = { element_t::MappedNumeric, "Spatial Extent Parameter Pack", "%", 0, d_3b_u, 0.0, 100.0 };

    m_tag_readers[static_cast<size_t>(element_t::NestedPack)] = [](parser_base<callbacks_t>* self, uint8_t tag_id, const uint8_t* buffer, size_t length, metadata_t& meta) noexcept(true) -> cb_result_t
      {
        auto* parser = static_cast<ST_0602_Parser<callbacks_t>*>(self);

        if (tag_id == AnnotationTags::ViewportPosition) [[likely]]
        {
          return parser->parse_viewport_position_pack(buffer, length, meta);
        }
        else if (tag_id == AnnotationTags::AnnotationSeries)
        {
          return parser->parse_nested_annotation_series(buffer, length) ? klv::cb_result_t::success : klv::cb_result_t::failed;
        }

        return klv::cb_result_t::success;
      };
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
      { length = len_byte; }

      if (index + length > size)
        break;

      auto&          meta    = m_tags[tag];
      const uint8_t* val_ptr = &stream[index];

      if (m_tag_readers[static_cast<size_t>(meta.type)](this, tag, val_ptr, length, meta) != klv::cb_result_t::success)
        return false;

      index += length;
    }
    return true;
  }

private:
  /***/
  [[nodiscard]]
  klv::cb_result_t parse_viewport_position_pack(const uint8_t* pack_ptr, size_t pack_len, [[maybe_unused]] const metadata_t& meta) noexcept
  {
    if (pack_len == 1) [[unlikely]]
    {
      // This is and edge case where the standard assigns a single control layout byte to Tag, 
      // and then we consider the position centered to a default 50.0, 50.0.
      callbacks().on_viewport_position( standard(), 18, meta, 50.0, 50.0);
      return klv::cb_result_t::success;
    }

    size_t internal_idx = 0;
  
    // Track our dual percent scales to pass to the atomic callback at the end
    double pct_x = 0.0;
    double pct_y = 0.0;

    // Cache the direct function pointer to your generic MappedNumeric engine loop reader
    auto generic_numeric_reader = m_tag_readers[static_cast<size_t>(klv::element_t::MappedNumeric)];

    // Process the sub-packet layout sequentially
    while (internal_idx < pack_len)
    {
      if (internal_idx + 2 > pack_len)
        break;

      uint8_t sub_tag = pack_ptr[internal_idx++];
      size_t  sub_len = pack_ptr[internal_idx++];

      if (internal_idx + sub_len > pack_len)
        break;

      const uint8_t* sub_val_ptr = &pack_ptr[internal_idx];

      // Boundary check to protect your global dictionary map from out-of-bounds reads
      if (sub_tag < m_tags.size()) [[likely]]
      {
        auto& sub_meta = m_tags[sub_tag];

        // DIRECT ENGINE INTERACTION:
        // Call your generic numeric handler natively, passing the precise sub-length (2)
        // This will instantly execute your linear scaling calculations and run on_numeric_tag
        if (generic_numeric_reader(this, sub_tag, sub_val_ptr, sub_len, sub_meta) != klv::cb_result_t::success)
        {
          return klv::cb_result_t::failed;
        }

        // Extract the calculated real-world float value out of the cache to pass to your atomic callback
        if (sub_tag == 1)
        {
          pct_x = sub_meta.cached_numeric;
        }
        else if (sub_tag == 2)
        { 
          pct_y = sub_meta.cached_numeric;
        }
      }

      internal_idx += sub_len;
    }

    return callbacks().on_viewport_position( standard(), 18, meta, pct_x, pct_y);
  }

  [[nodiscard]]
  bool parse_nested_annotation_series(const uint8_t* val_ptr, size_t length) noexcept
  {
    // Because Tag 4's payload is structured exactly like a standard KLV packet stream,
    // we can recursively pass its isolated buffer straight back into our main, 
    // data-driven parsing engine.
    return parse_packet(val_ptr, length);
  }

};

} /* namespace misb */

} /* namespace klv */

#endif /* MISB_ST_0602_H */
