#ifndef MISB_PARSER_BASE_H
#define MISB_PARSER_BASE_H

#include "klv-common.h"

#include <cstring>

namespace klv 
{

namespace misb
{

template <callbacks_interface callbacks_t>
class parser_base
{
protected:
  parser_base( standard_t std, uint32_t max_tags, const callbacks_t& cb)
    : m_std(std), m_max_tags(max_tags), m_callbacks(cb)
  {
    m_tags.fill({ element_t::Unconfigured });

    m_tag_readers.fill( [](parser_base<callbacks_t>* , uint8_t , const uint8_t* , size_t , metadata_t& ) noexcept(true) -> cb_result_t { return cb_result_t::success; } );

    m_tag_readers[static_cast<size_t>(klv::element_t::Unconfigured)]  = [](parser_base<callbacks_t>* self, uint8_t tag_id, const uint8_t* buffer, size_t length, metadata_t& meta) noexcept(true) -> cb_result_t
      {
        meta.cached_string.assign(reinterpret_cast<const char*>(buffer), length);
        return self->callbacks().on_unconfigured_tag(self->m_std, tag_id);
      };

    m_tag_readers[static_cast<size_t>(klv::element_t::String)]        = [](parser_base<callbacks_t>* self, uint8_t tag_id, const uint8_t* buffer, size_t length, metadata_t& meta) noexcept(true) -> cb_result_t
      {
        meta.cached_string.assign(reinterpret_cast<const char*>(buffer), length);
        return self->callbacks().on_string_tag(self->m_std, tag_id, meta);
      };

    m_tag_readers[static_cast<size_t>(klv::element_t::MappedNumeric)] = [](parser_base<callbacks_t>* self, uint8_t tag_id, const uint8_t* buffer, size_t length, metadata_t& meta) noexcept(true) -> cb_result_t
      {
        uint64_t raw        = self->extract_raw_bytes(buffer, length);
        int64_t  s_raw      = self->apply_sign_extension(raw, length, meta.domain_min);
        meta.cached_numeric = self->scale_value(s_raw, meta);

        std::tie(meta.cached_numeric,meta.out_of_range) = self->boundary_check(meta.cached_numeric, meta);
        if ( meta.out_of_range == true )
          return cb_result_t::failed;

        return self->callbacks().on_numeric_tag(self->m_std, tag_id, meta);
      };

    m_tag_readers[static_cast<size_t>(klv::element_t::Timestamp)]     = [](parser_base<callbacks_t>* self, uint8_t tag_id, const uint8_t* buffer, size_t length, metadata_t& meta) noexcept(true) -> cb_result_t
      {
        uint64_t raw_time = self->extract_raw_bytes(buffer, length);
        meta.cached_numeric = static_cast<double>(raw_time);
        return self->callbacks().on_timestamp_tag(self->m_std, tag_id, meta, raw_time);
      };

    m_tag_readers[static_cast<size_t>(klv::element_t::Checksum)]      = [](parser_base<callbacks_t>* self, uint8_t tag_id, const uint8_t* buffer, size_t length, metadata_t& meta) noexcept(true) -> cb_result_t
      {
        switch ( self->m_std )
        {
          case klv::misb::standard_t::st_0602_annotation:
          {
            if (length != 4)
              return cb_result_t::failed;
          }; break;

          default:
          {
            if (length != 2)
              return cb_result_t::failed;
          }; break;
        }

        uint32_t raw_checksum = static_cast<uint32_t>(self->extract_raw_bytes(buffer, length));
        return self->callbacks().on_checksum_tag(self->m_std, tag_id, meta, raw_checksum);
      };

    m_tag_readers[static_cast<size_t>(klv::element_t::Bitfield)]      = [](parser_base<callbacks_t>* self, uint8_t tag_id, const uint8_t* buffer, size_t length, metadata_t& meta) noexcept(true) -> cb_result_t
      {
        if (length == 0)
          return self->callbacks().on_bitfield_tag(self->m_std, tag_id, meta, 0x00);
        
        return self->callbacks().on_bitfield_tag(self->m_std, tag_id, meta, buffer[0]);
      };

    m_tag_readers[static_cast<size_t>(klv::element_t::MappedCode)]    = [](parser_base<callbacks_t>* self, uint8_t tag_id, const uint8_t* buffer, size_t length, metadata_t& meta) noexcept(true) -> cb_result_t
      {
        uint64_t raw = self->extract_raw_bytes(buffer, length);
        std::tie(meta.cached_numeric,meta.out_of_range) = self->boundary_check(raw, meta);

        return self->callbacks().on_numeric_tag(self->m_std, tag_id, meta);
      };

  }

public:
  [[nodiscard]]
  inline constexpr standard_t         standard() const noexcept(true)
  { return m_std; }

  [[nodiscard]]
  inline constexpr uint32_t           max_tags() const noexcept(true)
  { return m_max_tags;}

  [[nodiscard]]
  inline constexpr const callbacks_t& callbacks() const noexcept(true)
  { return m_callbacks;}

  [[nodiscard]]
  inline constexpr const metadata_t&  get_tag_data(uint8_t tag) const noexcept(true)
  { return m_tags[tag]; }

protected:
  /***/
  [[nodiscard]]
  inline constexpr uint64_t extract_raw_bytes(const uint8_t* val_ptr, size_t length) const noexcept(true)
  {
    uint64_t raw = 0;
#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    const size_t bytes_to_read = length > 8 ? 8 : length;
    for (size_t i = 0; i < bytes_to_read; ++i)
    { raw = (raw << 8) | val_ptr[i]; }
#else
    switch (length)
    {
      case 1: 
        raw = val_ptr[0];
      break;

      case 2: 
        raw = (static_cast<uint64_t>(val_ptr[0]) << 8) | val_ptr[1];
      break;

      case 3: 
        raw = (static_cast<uint64_t>(val_ptr[0]) << 16) |
              (static_cast<uint64_t>(val_ptr[1]) <<  8) | val_ptr[2];
      break;

      case 4:
        raw = (static_cast<uint64_t>(val_ptr[0]) << 24) | 
              (static_cast<uint64_t>(val_ptr[1]) << 16) |
              (static_cast<uint64_t>(val_ptr[2]) <<  8) | val_ptr[3];
      break;

      case 8:
        raw = (static_cast<uint64_t>(val_ptr[0]) << 56) |
              (static_cast<uint64_t>(val_ptr[1]) << 48) |
              (static_cast<uint64_t>(val_ptr[2]) << 40) |
              (static_cast<uint64_t>(val_ptr[3]) << 32) |
              (static_cast<uint64_t>(val_ptr[4]) << 24) |
              (static_cast<uint64_t>(val_ptr[5]) << 16) |
              (static_cast<uint64_t>(val_ptr[6]) <<  8) | val_ptr[7];
      break;
    }
#endif
    return raw;
  }

  /***/
  [[nodiscard]]
  inline constexpr int64_t apply_sign_extension(uint64_t raw, size_t length, int64_t domain_min) const noexcept(true)
  {
    if (domain_min >= 0)
      return raw;

    int64_t s_raw = raw;
    size_t  bits  = length * 8;
  
    if (raw & (1ULL << (bits - 1)))
    { s_raw |= ~((1ULL << bits) - 1); }

    return s_raw;
  }

  /***/
  [[nodiscard]]
  inline constexpr double scale_value(int64_t s_raw, const metadata_t& meta) const noexcept(true)
  {
    double scale = (meta.range_max - meta.range_min) / (static_cast<double>(meta.domain_max) - static_cast<double>(meta.domain_min));
    return meta.range_min + (static_cast<double>(s_raw) - static_cast<double>(meta.domain_min)) * scale;
  }

  /***/
  [[nodiscard]]
  inline constexpr std::pair<int64_t,bool> boundary_check(uint64_t raw, const metadata_t& meta) const noexcept(true)
  {
    if ( (meta.cached_numeric >= meta.range_min) && (meta.cached_numeric <= meta.range_max) )
      return {raw,false};

    return {meta.special_value,true};
  }

protected:
  using tag_reader_ptr_t = klv::cb_result_t(*)(parser_base<callbacks_t>* self, uint8_t tag_id, const uint8_t* buffer, size_t length, metadata_t& meta) noexcept(true);

  const standard_t                  m_std;
  const uint32_t                    m_max_tags;    /* used to specify the max expected value when reading the tag id and discarding all that is bigger */
  std::array<metadata_t, 256>       m_tags;
  std::array<tag_reader_ptr_t, 256> m_tag_readers;
  const callbacks_t&                m_callbacks;
};

} /* namespace misb */

} /* namespace klv */

#endif /* MISB_PARSER_BASE_H */
