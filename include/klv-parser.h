#ifndef KLV_PARSER_H
#define KLV_PARSER_H

#include "klv-common.h"
#include "misb0601.h"
#include "misb0102.h"
#include "misb0903.h"

#include <chrono>
#include <iostream>

namespace klv {

struct default_output_callbacks
{
  inline cb_result_t on_unconfigured_tag(uint8_t tag) noexcept(true)
  {
    std::cout << "[Tag " << std::setw(3) << (int)tag << "] not configured \n";
    return cb_result_t::success;
  }

  inline cb_result_t on_numeric_tag(uint8_t tag, const metadata_t& meta) noexcept(true)
  {
    std::cout << "[Tag " << std::setw(3) << (int)tag << "] " 
              << std::left << std::setw(30) << meta.name << " : " 
              << std::right << std::setw(12) << meta.cached_numeric << " [" << meta.units << "]\n";
    return cb_result_t::success;
  }

  inline cb_result_t on_string_tag(uint8_t tag, const metadata_t& meta) noexcept(true)
  {
    std::cout << "[Tag " << std::setw(3) << (int)tag << "] " 
              << std::left << std::setw(30) << meta.name << " : " << meta.cached_string << "\n";
    return cb_result_t::success;
  }

  inline cb_result_t on_timestamp_tag(uint8_t tag, const metadata_t& meta, uint64_t raw_microseconds) noexcept(true)
  {
    std::chrono::system_clock::time_point tp{std::chrono::duration_cast<std::chrono::system_clock::duration>(std::chrono::microseconds(raw_microseconds))};
    std::time_t tt = std::chrono::system_clock::to_time_t(tp);
    std::tm gmt_time;
    #if defined(_MSC_VER)
        gmtime_s(&gmt_time, &tt);
    #else
        gmtime_r(&tt, &gmt_time);
    #endif
    uint64_t ms = (raw_microseconds / 1000) % 1000;
    std::cout << "[Tag " << std::setw(3) << (int)tag << "] " << std::left << std::setw(30) << meta.name << " : " 
              << std::put_time(&gmt_time, "%Y-%m-%d %H:%M:%S") << "." << std::setw(3) << std::setfill('0') << ms << " UTC\n" << std::setfill(' ');
    return cb_result_t::success;
  }

  inline cb_result_t on_checksum_tag(uint8_t tag, const metadata_t& meta, uint16_t checksum) noexcept(true)
  {
    std::cout << "[Tag " << std::setw(3) << (int)tag << "] " << std::left << std::setw(30) << meta.name << " : 0x" 
              << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << checksum << "\n" << std::nouppercase << std::dec << std::setfill(' ');
    return cb_result_t::success;
  }

  inline cb_result_t on_bitfield_tag(uint8_t tag, const metadata_t& meta, uint8_t byte_flags) noexcept(true)
  {
    std::cout << "[Tag " << std::setw(3) << (int)tag << "] " << std::left << std::setw(30) << meta.name << " :\n"
              << "  -> [Bit 0] Laser Rangefinder Enabled   : " << ((byte_flags & 0x01) ? "ACTIVE" : "OFF") << "\n"
              << "  -> [Bit 1] Auto-Track Mode Status      : " << ((byte_flags & 0x02) ? "ON (AUTO)" : "OFF (MANUAL)") << "\n"
              << "  -> [Bit 2] IR Camera Cooler Lamp       : " << ((byte_flags & 0x04) ? "OPERATIONAL" : "STANDBY") << "\n"
              << "  -> [Bit 3] Sensor Selection Stream     : " << ((byte_flags & 0x08) ? "SECONDARY" : "PRIMARY") << "\n"
              << "  -> [Bit 4] Image Frame Freeze State    : " << ((byte_flags & 0x10) ? "PAUSED" : "LIVE") << "\n"
              << "  -> [Bit 5] Weapon Store Armed System   : " << ((byte_flags & 0x20) ? "ARMED (DANGER)" : "SAFE") << "\n"
              << "  -> [Bit 6] Payload Store Release State : " << ((byte_flags & 0x40) ? "RELEASED/FIRED" : "SECURED") << "\n"
              << "  -> [Bit 7] Navigation INS/GPS Sync     : " << ((byte_flags & 0x80) ? "VALID SYNC" : "DEGRADED FAULT") << "\n";
    return cb_result_t::success;
  }

  inline cb_result_t on_st0604_pts(uint64_t raw_pts_ticks) noexcept(true)
  {
    // Convert the 90kHz MPEG clock ticks directly into video timeline milliseconds
    double video_ms = (static_cast<double>(raw_pts_ticks) / 90000.0) * 1000.0;
    std::cout << "[MISB ST 0604 Stream Wrapper] PTS Ticks: " << raw_pts_ticks << " (" << video_ms << " ms video offset)\n";
    return cb_result_t::success;
  }

  inline cb_result_t on_vmti_target(uint32_t target_id, uint8_t confidence) noexcept(true)
  {
    std::cout << "    -> [VMTI Target Track] ID: " << target_id 
              << " | Automation Confidence: " << (int)confidence << "%\n";
    return cb_result_t::success;
  }

  inline cb_result_t on_security_version(uint8_t tag, uint16_t version_val) noexcept(true)
  {
    std::cout << "[Security Tag " << (int)tag << "] ST 0102 System Version : " <<  version_val << "\n";
    return cb_result_t::success;
  }

  inline cb_result_t on_security_enum(uint8_t tag, std::string_view tag_name, uint8_t enum_val) noexcept(true)
  {
    std::cout << "[Security Tag " << (int)tag << "] " << tag_name << " Raw Enum Value : " << (int)enum_val << "\n";
    return cb_result_t::success;
  }

  inline cb_result_t on_invalid_key() noexcept(true)
  {
    std::cout << "on_invalid_key()" << "\n";
    return cb_result_t::success;
  }

  inline cb_result_t on_length_overflow() noexcept(true)
  {
    std::cout << "on_length_overflow()" << "\n";
    return cb_result_t::success;
  }
 
  inline cb_result_t on_payload_truncated() noexcept(true)
  {
    std::cout << "on_payload_truncated()" << "\n";
    return cb_result_t::success;
  }

};

template <callbacks_interface callbacks_t = default_output_callbacks>
class parser
{
public:
  parser(
          callbacks_t      cb       = callbacks_t{},
          misb::standard_t standard = misb::standard_t::unknown
        )
    : m_standard{standard},
      m_callbacks(cb),
      m_st_0601(m_callbacks),
      m_st_0102(m_callbacks),
      m_st_0903(m_callbacks)
  {
    m_parsers.fill([](parser*, const uint8_t*, size_t) noexcept(true) {});

    m_parsers[static_cast<size_t>(klv::misb::standard_t::st_0601_uas_datalink)] = [](parser* self, const uint8_t* payload, size_t length) noexcept(true)
                                                                                  { self->m_st_0601.parse_packet(payload, length); };
    m_parsers[static_cast<size_t>(klv::misb::standard_t::st_0102_security)]     = [](parser* self, const uint8_t* payload, size_t length) noexcept(true)
                                                                                  { self->m_st_0102.parse_packet(payload, length); };
    m_parsers[static_cast<size_t>(klv::misb::standard_t::st_0903_vmti_track)]   = [](parser* self, const uint8_t* payload, size_t length) noexcept(true)
                                                                                  { self->m_st_0903.parse_packet(payload, length); };
  }

  bool parse(const uint8_t* buffer, size_t size) noexcept(true)
  {
    size_t cursor = 0;

    /* Standard is unknown so we should Auto-Detect inspecting the buffer */
    if (m_standard == misb::standard_t::unknown) [[likely]]
    {
      /******
       * Clean, isolated MISB ST 0604 Transport Preamble Skip, this only applies 
       * if the absolute beginning of the raw transport payload contains the prefix.
       */
      if (size > 5 && buffer[0] == 0x00 && buffer[1] == 0x92) 
      {
        uint64_t raw_pts_ticks = 0;
        for (size_t i = 0; i < 5; ++i) 
        {
          raw_pts_ticks = (raw_pts_ticks << 8) | buffer[i];
        }

        /* Notify the policy handler at zero runtime cost */
        m_callbacks.on_st0604_pts(raw_pts_ticks);
        cursor = 5; 
      }

      while (cursor < size)
      {
        if (cursor + 17 > size)
          break;

        /* Scan ahead for the next valid 4-byte SMPTE key prefix */
        const uint8_t* key_ptr = find_klv_start(&buffer[cursor], size - cursor);
        if (!key_ptr)
          break;

        cursor = static_cast<size_t>(key_ptr - buffer);
        if (cursor + 17 > size)
          break;

        /* Fast, branchless identification from our lookup array layout */
        const misb::standard_t selected_standard = identify_standard(key_ptr);
        if (selected_standard == misb::standard_t::unknown) 
        {
          cursor += 4; // Advance past current false prefix match
          continue;
        }

        // Extract length, validate, dispatch, and advance cursor
        if (process_payload(buffer, size, cursor, selected_standard) == false) [[unlikely]]
          return false;
      }

      return true;
    } /* if (m_standard == klv::misb::standard_t::unknown) [[likely]] */


    while (cursor < size)
    {
      if (cursor + 17 > size)
        break;

      const uint8_t* key_ptr = &buffer[cursor];

      // Basic structural integrity guard for predictable streams
      if (key_ptr[0] != 0x06 || key_ptr[1] != 0x0E || key_ptr[2] != 0x2B || key_ptr[3] != 0x34) [[unlikely]]
      {
        m_callbacks.on_invalid_key();
        return false;
      }

      if (process_payload(buffer, size, cursor, m_standard) == false) [[unlikely]]
        return false;
    }

    return true;
  }

private:
  inline static constexpr std::array<misb::standard_t,19>  klv_std_lookup_table = {
                                                        /* 0x00 */ misb::standard_t::unknown,
                                                        /* 0x01 */ misb::standard_t::st_0601_uas_datalink,
                                                        /* 0x02 */ misb::standard_t::st_0602_annotation,
                                                        /* 0x03 */ misb::standard_t::st_0603_misp_time,
                                                        /* 0x04 */ misb::standard_t::st_0604_class12_timestamp,
                                                        /* 0x05 */ misb::standard_t::st_0102_security,
                                                        /* 0x06 */ misb::standard_t::st_0806_rvt,
                                                        /* 0x07 */ misb::standard_t::st_0801_photogrammetry,
                                                        /* 0x08 */ misb::standard_t::st_0808_ancillary_text,
                                                        /* 0x09 */ misb::standard_t::st_0903_vmti_track,
                                                        /* 0x0A */ misb::standard_t::st_0809_meteorological,
                                                        /* 0x0B */ misb::standard_t::unknown,
                                                        /* 0x0C */ misb::standard_t::st_1204_miis_core,
                                                        /* 0x0D */ misb::standard_t::st_1206_sar,
                                                        /* 0x0E */ misb::standard_t::st_1301_miis_augmentation,
                                                        /* 0x0F */ misb::standard_t::st_1504_orbital_vectors,
                                                        /* 0x10 */ misb::standard_t::st_1602_composite_imaging,
                                                        /* 0x11 */ misb::standard_t::st_1801_surface_profiles,
                                                        /* 0x12 */ misb::standard_t::st_2101_core_identifier
                                                     };
  inline static constexpr std::array<misb::standard_t,19>  klv_non_std_lookup_table = {
                                                        /* 0x00 */ misb::standard_t::unknown,
                                                        /* 0x01 */ misb::standard_t::unknown,
                                                        /* 0x02 */ misb::standard_t::unknown,
                                                        /* 0x03 */ misb::standard_t::eg_0104_predator,
                                                        /* 0x04 */ misb::standard_t::unknown,
                                                        /* 0x05 */ misb::standard_t::st_0605_microsecond_timestamp
                                                     };

  /**/
  [[nodiscard]]
  inline constexpr const uint8_t* find_klv_start(const uint8_t* data, size_t len) const noexcept(true)
  {
    for (size_t i = 0; i + 16 <= len; ++i)
    {
      if ( data[i] == 0x06 && data[i + 1] == 0x0E && data[i + 2] == 0x2B && data[i + 3] == 0x34)
      { return data + i; }
    }
    return nullptr;
  }

  [[nodiscard]]
  inline constexpr misb::standard_t identify_standard(const uint8_t* key) const noexcept(true)
  {
    // All MISB keys: 06 0E 2B 34 [org_hi] [org_lo] ...
    // MISB org code: 02 0B (most), 02 05 (ST 0605), 02 03 (EG 0104)
    const uint8_t org_hi = key[4];
    const uint8_t org_lo = key[5];

    /* Standard MISB (02 0B): byte[12] is the sequential standard number */
    if ( (org_hi == 0x02) && (org_lo == 0x0B) ) [[likely]]
    {
      const uint8_t idx = key[12];
      if (idx <= 0x12)
        return klv_std_lookup_table[idx];

      return misb::standard_t::unknown;
    }

    /* Non-standard org codes */
    if (org_hi == 0x02 && org_lo <= 5)
      return klv_non_std_lookup_table[org_lo];

    return misb::standard_t::unknown;
  }

  [[nodiscard]]
  inline constexpr bool process_payload( const uint8_t* buffer, size_t size, size_t& cursor, misb::standard_t standard) noexcept(true)
  {
    // The key is 16 bytes long, so the length field starts exactly at cursor + 16
    size_t len_idx = cursor + 16;
    uint8_t len_byte = buffer[len_idx++];
    size_t payload_length = 0;

    if ((len_byte & 0x80) == 0) [[likely]]
    {
      // BER Short Form (Bit 8 is 0): Length is contained entirely within this single byte
      payload_length = len_byte;
    }
    else
    {
      // BER Long Form (Bit 8 is 1): Lower 7 bits specify the number of length bytes following
      const size_t num_bytes = len_byte & 0x7F;
      
      // Safety guard against corrupted long-form markers or integer overflows
      if (num_bytes == 0 || num_bytes > sizeof(size_t) || (len_idx + num_bytes) > size) [[unlikely]]
      {
        m_callbacks.on_length_overflow();
        return false;
      }

      // Bitwise shift loop to construct the size_t payload length
      for (size_t i = 0; i < num_bytes; ++i)
      {
        payload_length = (payload_length << 8) | buffer[len_idx++];
      }
    }

    // Guard checking if the payload physically cuts off early inside the buffer boundary
    if (len_idx + payload_length > size) [[unlikely]]
    {
      m_callbacks.on_payload_truncated();
      return false;
    }

    // Route to correct parser node with zero dynamic dispatch or virtual table overhead
    const uint8_t* payload_ptr = &buffer[len_idx];

    m_parsers[static_cast<size_t>(standard)](this, payload_ptr, payload_length);

    // Advance cursor straight past Key, Length, and Payload to the start of the next block
    cursor = len_idx + payload_length;
    return true;
  }

private:
  using parser_ptr_t = void(*)(parser*, const uint8_t*, size_t) noexcept(true);

  misb::standard_t                       m_standard;
  callbacks_t                            m_callbacks;

  std::array<parser_ptr_t, 19>           m_parsers;
  klv::misb::ST_0601_Parser<callbacks_t> m_st_0601;
  klv::misb::ST_0102_Parser<callbacks_t> m_st_0102;
  klv::misb::ST_0903_Parser<callbacks_t> m_st_0903;
};

} /* namespace klv */

#endif /* KLV_PARSER_H */

