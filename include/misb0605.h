#ifndef MISB_ST_0605_H
#define MISB_ST_0605_H

#include "misb-base.h"

namespace klv 
{

namespace misb
{

  // --- MISB ST 0605 Time Stamp Dictionary Keys ---
namespace TimeStampTags
{
  constexpr uint8_t TimeStatus         = 1; // 1-byte Enumerated status flag
  constexpr uint8_t PrecisionTimeStamp = 2; // 8-byte Microseconds UTC Integer
}

/* Annotation Metadata Set Parser */
template <callbacks_interface callbacks_t>
class ST_0605_Parser : public parser_base<callbacks_t>
{
  using parser_base<callbacks_t>::max_tags;
  using parser_base<callbacks_t>::m_tags;
  using parser_base<callbacks_t>::m_tag_readers;
public:
  ST_0605_Parser(callbacks_t cb = callbacks_t{})
    : parser_base<callbacks_t>(klv::misb::standard_t::st_0605_microsecond_timestamp, 3, cb)
  {
    // Map out the standard flat root tag definitions for ST 0605
    m_tags[TimeStampTags::TimeStatus]         = { element_t::Bitfield,  "Time Sync Clock Status" };
    m_tags[TimeStampTags::PrecisionTimeStamp] = { element_t::Timestamp, "Sync Precision Time Stamp", "microseconds" };
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

      // Pure generic BER length reader matching your core engine layout
      size_t  length   = 0;
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

      // Hard boundary check to protect against corrupted indices
      if (tag >= m_tags.size()) [[unlikely]]
      {
        index += length;
        continue;
      }

      auto&          meta    = m_tags[tag];
      const uint8_t* val_ptr = &stream[index];

      // Purely data-driven dispatch! Calls your generic Timestamp/Bitfield handlers automatically.
      if (m_tag_readers[static_cast<size_t>(meta.type)](this, tag, val_ptr, length, meta) != klv::cb_result_t::success)
      {
        m_tag_readers[static_cast<size_t>(meta.type)](this, tag, val_ptr, length, meta);
        return false;
      }

      index += length;
    }
    return true;
  }

private:

};

} /* namespace misb */

} /* namespace klv */

#endif /* MISB_ST_0605_H */
