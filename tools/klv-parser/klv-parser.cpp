
#include "klv-parser.h"

#include <fstream>

struct test_callbacks_t
{
  inline klv::cb_result_t on_unconfigured_tag([[maybe_unused]] klv::misb::standard_t std, uint8_t tag) noexcept(true)
  {
    std::cout << "[Warn] Tag [" << std::setw(3) << (int)tag << "] not configured \n";
    return klv::cb_result_t::success;
  }

  klv::cb_result_t on_numeric_tag([[maybe_unused]] klv::misb::standard_t std, uint8_t tag, [[maybe_unused]] const klv::metadata_t& meta) noexcept(true)
  {
    std::cout << "[LDS] Numeric Tag: " << (int)tag << "\n";
    return klv::cb_result_t::success;
  }

  klv::cb_result_t on_string_tag([[maybe_unused]] klv::misb::standard_t std, uint8_t tag, [[maybe_unused]] const klv::metadata_t& meta) noexcept(true)
  {
    std::cout << "[LDS] String Tag: " << (int)tag << "\n";
    return klv::cb_result_t::success;
  }

  klv::cb_result_t on_timestamp_tag([[maybe_unused]] klv::misb::standard_t std, uint8_t tag, [[maybe_unused]] const klv::metadata_t& meta, [[maybe_unused]] uint64_t raw_time) noexcept(true)
  {
    std::cout << "[LDS] Timestamp Tag: " << (int)tag << " | Time: " << raw_time << "\n";
    return klv::cb_result_t::success;
  }

  klv::cb_result_t on_checksum_tag([[maybe_unused]] klv::misb::standard_t std, uint8_t tag, [[maybe_unused]] const klv::metadata_t& meta, [[maybe_unused]] uint16_t checksum) noexcept(true)
  {
    std::cout << "[LDS] Checksum Tag: " << (int)tag << " | Value: 0x" << std::hex << checksum << std::dec << "\n";
    return klv::cb_result_t::success;
  }

  klv::cb_result_t on_bitfield_tag([[maybe_unused]] klv::misb::standard_t std, uint8_t tag, [[maybe_unused]] const klv::metadata_t& meta, [[maybe_unused]] uint8_t flags) noexcept(true)
  {
    std::cout << "[LDS] Bitfield Tag: " << (int)tag << " | Flags: 0x" << std::hex << (int)flags << std::dec << "\n";
    return klv::cb_result_t::success;
  }

  klv::cb_result_t on_pts([[maybe_unused]] klv::misb::standard_t std, uint64_t pts) noexcept(true)
  {
    std::cout << "[Preamble] ST 0604 Transport PTS: " << pts << "\n";
    return klv::cb_result_t::success;
  }

  klv::cb_result_t on_vmti_target( [[maybe_unused]] klv::misb::standard_t std, uint32_t target_id, uint8_t confidence) noexcept(true)
  {
    std::cout << "[VMTI] Target ID: " << target_id << " | Confidence: " << (int)confidence << "%\n";
    return klv::cb_result_t::success;
  }

  klv::cb_result_t on_viewport_position(klv::misb::standard_t std, uint8_t tag, const klv::metadata_t& meta, double x, double y) noexcept(true)
  {
    std::cout << "Standard: " << static_cast<uint32_t>(std) << " [Tag " << (int)tag << "] " << meta.name << " Pos : [" << x << ", " << y << "]\n";
    return klv::cb_result_t::success;
  }

  // Parser error hooks
  klv::cb_result_t on_invalid_key() noexcept(true)
  {
    std::cerr << "[Error] Invalid SMPTE key prefix found.\n";
    return klv::cb_result_t::success;
  }

  klv::cb_result_t on_length_overflow() noexcept(true)
  { 
    std::cerr << "[Error] BER Length overflow detected.\n";
    return klv::cb_result_t::success;
  }

  klv::cb_result_t on_payload_truncated() noexcept(true)
  {
    std::cerr << "[Error] Payload truncated prematurely.\n";
    return klv::cb_result_t::success;
  }
};


int main(int argc, char* argv[])
{
  /* Ensure a file path was passed via command line */
  if (argc < 2)
  {
    std::cerr << "Usage: " << argv[0] << " <path_to_klv_file.bin>\n";
    return 1;
  }

  std::string filepath = argv[1];

  /* Open file at the very end to find its exact size instantly */
  std::ifstream file(filepath, std::ios::binary | std::ios::ate);
  if (!file.is_open())
  {
    std::cerr << "Failed to open file: " << filepath << "\n";
    return 1;
  }

  std::streamsize file_size = file.tellg();
  if (file_size <= 0)
  {
    std::cerr << "File is empty or invalid.\n";
    return 1;
  }

  file.seekg(0, std::ios::beg);

  /* Allocate memory for the whole file and read it in one shot */
  std::vector<uint8_t> file_buffer(static_cast<size_t>(file_size));
  if (!file.read(reinterpret_cast<char*>(file_buffer.data()), file_size))
  {
    std::cerr << "Error reading file data into buffer.\n";
    return 1;
  }

  std::cout << "Successfully loaded " << file_size << " bytes from " << filepath << "\n";
  std::cout << "Starting KLV stream parsing (Auto-detection mode)...\n";
  std::cout << "==================================================\n";

  /* Initialize parser running your new klv::callbacks concept target */
  klv::parser<test_callbacks_t> parser;
  //klv::parser parser; /* uncomment to use the default output implementation */

  /* Execute the parser over the complete file memory chunk */
  bool status = parser.parse(file_buffer.data(), file_buffer.size());

  std::cout << "==================================================\n";
  if (status)
  {
    std::cout << "Parsing completed successfully with no structural errors!\n";
  }
  else
  {
    std::cerr << "Parsing failed due to stream corruption or payload truncation.\n";
  }

  return status ? 0 : 1;
}