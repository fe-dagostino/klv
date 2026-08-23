#ifndef MISB_ST_0604_H
#define MISB_ST_0604_H

#include "klv-common.h"

namespace klv
{

namespace misb
{

/*
┌────────────────────────────────────────────────────────┐
│  MISB ST 0604 Header Wrapper (5 Bytes: 00 92 DF 02 4C) │
└───────────────────────────┬────────────────────────────┘
                            │ (Tells you the exact video sync timeline millisecond)
                            ▼
 ┌───────────────────────────────────────────────────────┐
 │ MULTIPLEXED PAYLOAD CONTIGUOUS BLOCK                  │
 ├───────────────────────────────────────────────────────┤
 │ 📦 Sub-Stream 1: MISB ST 0601 UAS Telemetry (Tags)    │
 ├───────────────────────────────────────────────────────┤
 │ 📦 Sub-Stream 2: MISB ST 0102 Security (Tags)         │
 ├───────────────────────────────────────────────────────┤
 │ 📦 Sub-Stream 3: MISB ST 0903 VMTI Target Tracker     │
 └───────────────────────────────────────────────────────┘
*/

} /* namespace misb */

} /* namespace klv */

#endif /* MISB_ST_0604_H */
