#ifndef MISB_ST_0601_H
#define MISB_ST_0601_H

#include "misb-base.h"

namespace klv 
{

namespace misb
{

/* UAS Telemetry Set */
template <callbacks_interface callbacks_t>
class ST_0601_Parser : public parser_base<callbacks_t>
{
public:
  ST_0601_Parser(callbacks_t cb = callbacks_t{})
    : parser_base<callbacks_t>(cb)
  {
    // Reusable Mapping Domain Dimensions
    constexpr int64_t d_1b_u = 255;          // 1-byte Unsigned Domain [0, 255]
    constexpr int64_t d_2b_s = 32767;        // 2-byte Signed Domain [-32767, 32767]
    constexpr int64_t d_2b_u = 65535;        // 2-byte Unsigned Domain [0, 65535]
    constexpr int64_t d_4b_s = 2147483647;   // 4-byte Signed Domain [-2147483647, 2147483647]
    constexpr int64_t d_4b_u = 4294967295;   // 4-byte Unsigned Domain [0, 4294967295]

    // --- Core Architecture, Time & Mission Identifiers (Tags 1 - 4) ---
    parser_base<callbacks_t>::m_tags[1]  = { element_t::Checksum     , "Checksum", "hex" }; // Handled by inline verification pass
    parser_base<callbacks_t>::m_tags[2]  = { element_t::Timestamp    , "Precision Time Stamp", "microseconds" };
    parser_base<callbacks_t>::m_tags[3]  = { element_t::String       , "Mission ID" };
    parser_base<callbacks_t>::m_tags[4]  = { element_t::String       , "Platform Tail Number" }; 

    // --- Platform Attitude & Model Dynamics (Tags 5 - 10) ---
    parser_base<callbacks_t>::m_tags[5]  = { element_t::MappedNumeric, "Platform Heading Angle", "degrees", 0, d_2b_u, 0.0, 360.0 };
    parser_base<callbacks_t>::m_tags[6]  = { element_t::MappedNumeric, "Platform Pitch Angle", "degrees", -d_2b_s, d_2b_s, -20.0, 20.0, 0x8000 };
    parser_base<callbacks_t>::m_tags[7]  = { element_t::MappedNumeric, "Platform Roll Angle", "degrees", -d_2b_s, d_2b_s, -50.0, 50.0 };
    parser_base<callbacks_t>::m_tags[8]  = { element_t::MappedNumeric, "Platform True Airspeed", "meters/sec", 0, d_1b_u, 0.0, 255.0 }; 
    parser_base<callbacks_t>::m_tags[9]  = { element_t::MappedNumeric, "Platform Indicated Airspeed", "meters/sec", 0, d_1b_u, 0.0, 255.0 };
    parser_base<callbacks_t>::m_tags[10] = { element_t::String       , "Platform Designation" };

    // --- Sensor Configuration & Optics Metadata (Tags 11 - 17) ---
    parser_base<callbacks_t>::m_tags[11] = { element_t::String       , "Image Source Sensor" };
    parser_base<callbacks_t>::m_tags[12] = { element_t::String       , "Image Coordinate System" };
    parser_base<callbacks_t>::m_tags[13] = { element_t::MappedNumeric, "Sensor Latitude", "degrees", -d_4b_s, d_4b_s, -90.0, 90.0 };
    parser_base<callbacks_t>::m_tags[14] = { element_t::MappedNumeric, "Sensor Longitude", "degrees", -d_4b_s, d_4b_s, -180.0, 180.0 };
    parser_base<callbacks_t>::m_tags[15] = { element_t::MappedNumeric, "Sensor True Altitude", "meters", 0, d_2b_u, -900.0, 19000.0 };
    parser_base<callbacks_t>::m_tags[16] = { element_t::MappedNumeric, "Sensor Horizontal FOV", "degrees", 0, d_2b_u, 0.0, 180.0 };
    parser_base<callbacks_t>::m_tags[17] = { element_t::MappedNumeric, "Sensor Vertical FOV", "degrees", 0, d_2b_u, 0.0, 180.0 };

    // --- Gimbal Orientation Variables (Tags 18 - 22) ---
    parser_base<callbacks_t>::m_tags[18] = { element_t::MappedNumeric, "Sensor Rel Azimuth Angle", "degrees", 0, d_4b_u, 0.0, 360.0 };
    parser_base<callbacks_t>::m_tags[19] = { element_t::MappedNumeric, "Sensor Rel Elevation Angle", "degrees", -d_4b_s, d_4b_s, -180.0, 180.0 };
    parser_base<callbacks_t>::m_tags[20] = { element_t::MappedNumeric, "Sensor Rel Roll Angle", "degrees", 0, d_4b_u, 0.0, 360.0 };
    parser_base<callbacks_t>::m_tags[21] = { element_t::MappedNumeric, "Slant Range", "meters", 0, d_4b_u, 0.0, 5000000.0 };
    parser_base<callbacks_t>::m_tags[22] = { element_t::MappedNumeric, "Target Width", "meters", 0, d_2b_u, 0.0, 10000.0 };

    // --- Image Center Footprint Metrics (Tags 23 - 25) ---
    parser_base<callbacks_t>::m_tags[23] = { element_t::MappedNumeric, "Frame Center Latitude", "degrees", -d_4b_s, d_4b_s, -90.0, 90.0 };
    parser_base<callbacks_t>::m_tags[24] = { element_t::MappedNumeric, "Frame Center Longitude", "degrees", -d_4b_s, d_4b_s, -180.0, 180.0 };
    parser_base<callbacks_t>::m_tags[25] = { element_t::MappedNumeric, "Frame Center Elevation", "meters", 0, d_2b_u, -900.0, 19000.0 };

    // --- Relative Four-Corner Boundary Offsets (Tags 26 - 33) ---
    constexpr double r_offset = 0.075;
    parser_base<callbacks_t>::m_tags[26] = { element_t::MappedNumeric, "Offset Corner Lat Point 1", "degrees", -d_2b_s, d_2b_s, -r_offset, r_offset };
    parser_base<callbacks_t>::m_tags[27] = { element_t::MappedNumeric, "Offset Corner Lon Point 1", "degrees", -d_2b_s, d_2b_s, -r_offset, r_offset };
    parser_base<callbacks_t>::m_tags[28] = { element_t::MappedNumeric, "Offset Corner Lat Point 2", "degrees", -d_2b_s, d_2b_s, -r_offset, r_offset };
    parser_base<callbacks_t>::m_tags[29] = { element_t::MappedNumeric, "Offset Corner Lon Point 2", "degrees", -d_2b_s, d_2b_s, -r_offset, r_offset };
    parser_base<callbacks_t>::m_tags[30] = { element_t::MappedNumeric, "Offset Corner Lat Point 3", "degrees", -d_2b_s, d_2b_s, -r_offset, r_offset };
    parser_base<callbacks_t>::m_tags[31] = { element_t::MappedNumeric, "Offset Corner Lon Point 3", "degrees", -d_2b_s, d_2b_s, -r_offset, r_offset };
    parser_base<callbacks_t>::m_tags[32] = { element_t::MappedNumeric, "Offset Corner Lat Point 4", "degrees", -d_2b_s, d_2b_s, -r_offset, r_offset };
    parser_base<callbacks_t>::m_tags[33] = { element_t::MappedNumeric, "Offset Corner Lon Point 4", "degrees", -d_2b_s, d_2b_s, -r_offset, r_offset };

    // --- Flight Environmental & Heading Reference Values (Tags 34 - 39) ---
    parser_base<callbacks_t>::m_tags[34] = { element_t::MappedNumeric, "Icing Detected", "enum", 0, d_1b_u, 0.0, 255.0 }; 
    parser_base<callbacks_t>::m_tags[35] = { element_t::MappedNumeric, "Wind Direction", "degrees", 0, d_2b_u, 0.0, 360.0 };
    parser_base<callbacks_t>::m_tags[36] = { element_t::MappedNumeric, "Wind Speed", "meters/sec", 0, d_1b_u, 0.0, 100.0 };
    parser_base<callbacks_t>::m_tags[38] = { element_t::MappedNumeric, "Density Altitude", "meters", 0, d_2b_u, -900.0, 19000.0 };
    parser_base<callbacks_t>::m_tags[39] = { element_t::MappedNumeric, "Outside Air Temperature", "Celsius", -128, 127, -128.0, 127.0 };

    // --- Target Geo-Location & Target Tracking Metrics (Tags 40 - 46) ---
    parser_base<callbacks_t>::m_tags[40] = { element_t::MappedNumeric, "Target Location Lat", "degrees", -d_4b_s, d_4b_s, -90.0, 90.0 };
    parser_base<callbacks_t>::m_tags[41] = { element_t::MappedNumeric, "Target Location Lon", "degrees", -d_4b_s, d_4b_s, -180.0, 180.0 };
    parser_base<callbacks_t>::m_tags[42] = { element_t::MappedNumeric, "Target Location Elev", "meters", 0, d_2b_u, -900.0, 19000.0 };
    parser_base<callbacks_t>::m_tags[43] = { element_t::MappedNumeric, "Target Track Gate Width", "pixels", 0, d_1b_u, 0.0, 510.0 };   
    parser_base<callbacks_t>::m_tags[44] = { element_t::MappedNumeric, "Target Track Gate Height", "pixels", 0, d_1b_u, 0.0, 510.0 };  
    parser_base<callbacks_t>::m_tags[45] = { element_t::MappedNumeric, "Target Error Estimate CE90", "meters", 0, d_2b_u, 0.0, 4095.0 };
    parser_base<callbacks_t>::m_tags[46] = { element_t::MappedNumeric, "Target Error Estimate LE90", "meters", 0, d_2b_u, 0.0, 4095.0 };

    // --- Operational Overlays, Security, and Navigation References (Tags 47 - 65) ---
    parser_base<callbacks_t>::m_tags[47] = { element_t::Bitfield     , "Generic Flag Data 01", "bits" }; 
    parser_base<callbacks_t>::m_tags[48] = { element_t::MappedNumeric, "Security Compliance Version", "version", 0, d_2b_u, 0.0, 65535.0 };
    parser_base<callbacks_t>::m_tags[49] = { element_t::MappedNumeric, "Platform Roll Angle Full", "degrees", -d_4b_s, d_4b_s, -180.0, 180.0 };
    parser_base<callbacks_t>::m_tags[50] = { element_t::MappedNumeric, "Platform Pitch Angle Full", "degrees", -d_2b_s, d_2b_s, -90.0, 90.0 };
    parser_base<callbacks_t>::m_tags[57] = { element_t::MappedNumeric, "Ground Range", "meters", 0, d_4b_u, 0.0, 5000000.0 };
    parser_base<callbacks_t>::m_tags[62] = { element_t::MappedCode   , "Laser PRF Code", "code", 0, d_2b_u, 1111.0, 8888.0 };
    parser_base<callbacks_t>::m_tags[63] = { element_t::MappedNumeric, "Sensor FOV Name", "enum", 0, d_1b_u, 0.0, 255.0 };
    parser_base<callbacks_t>::m_tags[64] = { element_t::MappedNumeric, "Platform Mag Heading", "degrees", 0, d_2b_u, 0.0, 360.0 };
    parser_base<callbacks_t>::m_tags[65] = { element_t::MappedNumeric, "UAS Datalink LS Version", "version", 0, d_1b_u, 0.0, 255.0 };
  }

  void parse_packet(const uint8_t* stream, size_t size)
  {
    size_t index = 0;

    while (index < size)
    {
      uint8_t tag = stream[index++];
      if (index >= size)
        break;// Task 2 Integration: Local Element BER Short/Long length parser step

      size_t length = 0;
      uint8_t len_byte = stream[index++];
      if (len_byte & 0x80)
      {
        size_t num_bytes = len_byte & 0x7F;
        if (index + num_bytes > size) 
          break;

        for (size_t i = 0; i < num_bytes; ++i)
        {
          length = (length << 8) | stream[index++];
        }
      }
      else
      {
        length = len_byte;
      }

      if (index + length > size)
        break;

      auto&          meta    = parser_base<callbacks_t>::m_tags[tag];
      const uint8_t* val_ptr = &stream[index];

      auto tag_reader_result = parser_base<callbacks_t>::m_tag_readers[static_cast<size_t>(meta.type)](this,tag,val_ptr,length,meta);

      index += length;
    }
  }

private:
  // Checksum Validator matching MISB ST 0601 implementation constraints
  inline bool verify_klv_checksum(const uint8_t* buffer, size_t full_packet_length) noexcept(true)
  {
    if (full_packet_length < 4)
      return false;
  
    uint16_t computed_sum = 0;
    // Sum every single byte up to the 2-byte value boundary of Tag 1 Checksum
    for (size_t i = 0; i < full_packet_length - 2; ++i)
    {
      computed_sum += buffer[i] << (8 * ((i + 1) % 2));
    }
  
    uint16_t transmitted_sum = (buffer[full_packet_length - 2] << 8) | buffer[full_packet_length - 1];

    return (computed_sum == transmitted_sum);
  }
private:

};

} /* namespace misb */

} /* namespace klv */

#endif /* MISB_ST_0601_H */
