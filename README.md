# klv

KLV Parser contains specific pasers for the following MISB:

* MISB 0102 - Security Metadata Universal and Local Sets for Motion Imagery Data
* MISB 0601 - UAS Datalink Local Set
* MISB 0602 - Annotation Metadata Set
* MISB 0604 - Timestamps for Class 1/Class 2 Motion Imagery
* MISB 0605 - Class 0 Motion Imagery Metadata and Audio over SDI
* MISB 0903 - Video Moving Target Indicator Metadata

In some cases deprecated tags are not managed, but can be added in the future as well as orther parsers.

A full list of the standards used as reference can be found at [https://nsgreg.nga.mil/misb.jsp](https://nsgreg.nga.mil/misb.jsp) .

## How to build

```bash
git clone https://github.com/fe-dagostino/klv.git
cd klv
mkdir build && cd build
cmake ..
make
```

Since the `KLV_BUILD_TOOLS` is `ENABLED` by default then `klv-parser` should be present in `..build/tools/klv-parser`.

Available options are the following:

```
KLV_BUILD_TOOLS          "Enable/Disable build of tools."                          ON
KLV_BUILD_TESTS          "Enable/Disable build of tests."                         OFF
KLV_BUILD_DOCS           "Enable/Disable doxygen generator"                        ON
KLV_ENABLE_CPPCHECK      "Enable/Disable cppcheck"                                 ON
KLV_ENABLE_IWYU          "Enable/Disable include what you use."                    ON
KLV_ENABLE_PACKAGING     "Enable/Disable packaging."                               ON
```

## FetchContent

To use as dependency in your project, you just need to create a file `klv.cmake` in your `cmake` folder and to include it in the main CMakeLists.txt.

```cmake
FetchContent_Declare(
  klv
  GIT_REPOSITORY https://github.com/fe-dagostino/klv.git
  GIT_TAG        master
  OVERRIDE_FIND_PACKAGE
)

set(KLV_BUILD_TOOLS         OFF CACHE BOOL   "" FORCE)
set(KLV_BUILD_TESTS         OFF CACHE BOOL   "" FORCE)
set(KLV_BUILD_DOCS          OFF CACHE BOOL   "" FORCE)
set(KLV_ENABLE_CPPCHECK     OFF CACHE BOOL   "" FORCE)
set(KLV_ENABLE_IWYU         OFF CACHE BOOL   "" FORCE)
set(KLV_ENABLE_PACKAGING    OFF CACHE BOOL   "" FORCE)

FetchContent_MakeAvailable(klv)
```

Here what to add in the main CMakeLists.txt

```cmake
include(FetchContent)
include(klv)
```

and then for the `target_link_libraries` we specify the name used in `FetchContent_Declare`.

```cmake
target_link_libraries( ${CMAKE_PROJECT_NAME} PRIVATE klv )
```

## Usage

The parser requires a callback structure. A predefined one is available `klv::default_output_callbacks` with a simple action
to output the content of the tags.

Application can define its own actions based on the needs. The must to have for such interface is to be compliant with 'concept callbacks_interface',
if not your application simply will not build.

Supposing to have binary buffer named `file_buffer` a tipical usage of the parser can be the following.

```cpp
  klv::default_output_callbacks def_cb;
  klv::parser parser(def_cb); 
  
  /* Execute the parser over the full buffer */
  bool status = parser.parse(file_buffer.data(), file_buffer.size());
```

