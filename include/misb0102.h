#ifndef MISB_ST_0102_H
#define MISB_ST_0102_H

#include "misb-base.h"

namespace klv 
{

namespace misb
{

// ============================================================================
// --- MISB ST 0102 Security Metadata Local Set Dictionary ---
// Compliant with standard UAS Datalink parent embedding (Tag 48 nesting)
// ============================================================================

namespace SecurityTags {
  // Core Classification Parameters
  constexpr uint8_t SecurityClassification          = 1;  // 1-byte Enum (1=Unclassified, 2=Restricted, etc.)
  constexpr uint8_t ClassifyingCountryCodingMethod  = 2;  // 1-byte Enum (1=ISO-3166 Two Letter, 13=GENC, etc.)
  constexpr uint8_t ClassifyingCountry              = 3;  // String (e.g., "US", "CA", //UK)

  // Compartments & Caveats (MISSING FROM YOUR LIST)
  constexpr uint8_t SecuritySCI_SHIInformation      = 4;  // String (Sensitive Compartmented Info)
  constexpr uint8_t SecurityCaveats                 = 5;  // String (e.g., "NOFORN", "ORCON")
  constexpr uint8_t ReleasingInstructions           = 6;  // String (Country codes allowed to view)

  // Administrative Lineage (MISSING FROM YOUR LIST)
  constexpr uint8_t ClassifiedBy                    = 7;  // String (Identity of classifier authority)
  constexpr uint8_t DerivedFrom                     = 8;  // String (Source document or basis)
  constexpr uint8_t ClassificationReason            = 9;  // String (Why it is classified)
  constexpr uint8_t DeclassificationDate            = 10; // String (YYYYMMDD format)
  constexpr uint8_t ClassificationAndMarkingSystem  = 11; // String (The authority framework used)

  // Object Targeting Country Metadata (UPDATED SHIFT)
  constexpr uint8_t ObjectCountryCodingMethod       = 12; // 1-byte Enum 
  constexpr uint8_t ObjectCountry                   = 13; // String (Where the payload optics look)
  constexpr uint8_t ClassificationComments          = 14; // String (Optional extra metadata text)

  // Identification Anchors (MISSING FROM YOUR LIST)
  constexpr uint8_t UMID                            = 15; // 32 or 64-byte SMPTE ST 330 Unique Media Identifier
  constexpr uint8_t StreamID                        = 16; // 1-byte Unsigned Integer (Identifies video channel)
  constexpr uint8_t TransportStreamID               = 17; // 2-byte Unsigned Integer (MPEG-TS validation anchor)

  // Modern System Extensions
  constexpr uint8_t ItemDesignatorId                = 22; // String (Explicit asset classification footprint)
  constexpr uint8_t Version                         = 23; // 2-byte Unsigned Integer (e.g., 10, 11, 12)
  constexpr uint8_t ObjectCountryCodes              = 24; // Multi-country structure extension (C++20/C++23)    
}

/* Security Metadata Set */
template <callbacks_interface callbacks_t>
class ST_0102_Parser : protected parser_base<callbacks_t>
{
public:
  ST_0102_Parser(callbacks_t cb = callbacks_t{})
    : parser_base<callbacks_t>(cb)
  {
    // Numeric Constraint Threshold Bounds
    constexpr int64_t max_1b_u = 255;
    constexpr int64_t max_2b_u = 65535;

    // 1. Core Classification Parameters (Corrected to MappedNumeric for Enum Resolution!)
    parser_base<callbacks_t>::m_tags[SecurityTags::SecurityClassification]         = { element_t::MappedNumeric, "Security Classification", "enum", 0, max_1b_u, 0.0, 255.0 };
    parser_base<callbacks_t>::m_tags[SecurityTags::ClassifyingCountryCodingMethod] = { element_t::MappedNumeric, "Classifying Country Coding Method", "enum", 0, max_1b_u, 0.0, 255.0 };
    parser_base<callbacks_t>::m_tags[SecurityTags::ClassifyingCountry]             = { element_t::String, "Classifying Country" };
    
    // 2. Compartments & Caveats
    parser_base<callbacks_t>::m_tags[SecurityTags::SecuritySCI_SHIInformation]     = { element_t::String, "SCI/SHI Information" };
    parser_base<callbacks_t>::m_tags[SecurityTags::SecurityCaveats]                = { element_t::String, "Security Caveats" };
    parser_base<callbacks_t>::m_tags[SecurityTags::ReleasingInstructions]          = { element_t::String, "Releasing Instructions" };
    
    // 3. Administrative Lineage Records
    parser_base<callbacks_t>::m_tags[SecurityTags::ClassifiedBy]                   = { element_t::String, "Classified By" };
    parser_base<callbacks_t>::m_tags[SecurityTags::DerivedFrom]                    = { element_t::String, "Derived From" };
    parser_base<callbacks_t>::m_tags[SecurityTags::ClassificationReason]           = { element_t::String, "Classification Reason" };
    parser_base<callbacks_t>::m_tags[SecurityTags::DeclassificationDate]           = { element_t::String, "Declassification Date" };
    parser_base<callbacks_t>::m_tags[SecurityTags::ClassificationAndMarkingSystem] = { element_t::String, "Classification Marking System" };
    
    // 4. Object Targeting Frameworks
    parser_base<callbacks_t>::m_tags[SecurityTags::ObjectCountryCodingMethod]      = { element_t::MappedNumeric, "Object Country Coding Method", "enum", 0, max_1b_u, 0.0, 255.0 };
    parser_base<callbacks_t>::m_tags[SecurityTags::ObjectCountry]                  = { element_t::String, "Object Country" };
    parser_base<callbacks_t>::m_tags[SecurityTags::ClassificationComments]         = { element_t::String, "Classification Comments" };
    
    // 5. High-Throughput Identifiers (Mapped as Numerics to avoid payload copies)
    parser_base<callbacks_t>::m_tags[SecurityTags::StreamID]                       = { element_t::MappedNumeric, "Stream ID", "id", 0, max_1b_u, 0.0, 255.0 };
    parser_base<callbacks_t>::m_tags[SecurityTags::TransportStreamID]              = { element_t::MappedNumeric, "Transport Stream ID", "id", 0, max_2b_u, 0.0, 65535.0 };
    
    // 6. Modern System Extensions
    parser_base<callbacks_t>::m_tags[SecurityTags::ItemDesignatorId]               = { element_t::String, "Item Designator ID" };
    parser_base<callbacks_t>::m_tags[SecurityTags::Version]                        = { element_t::MappedNumeric, "Security Version", "version", 0, max_2b_u, 0.0, 65535.0 };
    parser_base<callbacks_t>::m_tags[SecurityTags::ObjectCountryCodes]             = { element_t::String, "Object Country Codes List" };

    // Note: Tag 15 (UMID) is omitted from this base configuration loop as it requires a 
    // specialized raw byte payload array policy callback (on_security_umid_tag) 
    // rather than standard String or MappedNumeric processing.
  }

  void parse_packet(const uint8_t* stream, size_t size)
  {
    size_t index = 0;
    while (index < size)
    {
      uint8_t tag = stream[index++];
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

      auto&          meta    = parser_base<callbacks_t>::m_tags[tag];
      const uint8_t* val_ptr = &stream[index];

      // Specialized routing intercept for Tag 15 (UMID Byte Blocks)
      if (tag == SecurityTags::UMID)
      {
        // Drops directly out to custom array handlers on your policy
        parser_base<callbacks_t>::callbacks().on_checksum_tag(tag, meta, static_cast<uint16_t>(length)); // Reuses validation anchors
        index += length;
        continue;
      }

      auto tag_reader_result = parser_base<callbacks_t>::m_tag_readers[static_cast<size_t>(meta.type)](this,tag,val_ptr,length,meta);

      index += length;
    }
  }

private:

};

} /* namespace misb */

} /* namespace klv */

#endif /* MISB_ST_0102_H */
