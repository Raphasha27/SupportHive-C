cmake_minimum_required(VERSION 3.14)
project(SupportHive-C C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------
include(FetchContent)

# 1. libuv (Async I/O)
message(STATUS "Fetching libuv...")
FetchContent_Declare(
  libuv
  GIT_REPOSITORY https://github.com/libuv/libuv.git
  GIT_TAG        v1.44.2
)
FetchContent_MakeAvailable(libuv)

# 2. http-parser (Request Parsing)
message(STATUS "Fetching http-parser...")
FetchContent_Declare(
  http_parser
  GIT_REPOSITORY https://github.com/nodejs/http-parser.git
  GIT_TAG        v2.9.4
)
FetchContent_GetProperties(http_parser)
if(NOT http_parser_POPULATED)
  FetchContent_Populate(http_parser)
  add_library(http_parser STATIC 
    ${http_parser_SOURCE_DIR}/http_parser.c
    ${http_parser_SOURCE_DIR}/http_parser.h
  )
  target_include_directories(http_parser PUBLIC ${http_parser_SOURCE_DIR})
endif()

# 3. cJSON (JSON Parsing)
message(STATUS "Fetching cJSON...")
FetchContent_Declare(
  cjson
  GIT_REPOSITORY https://github.com/DaveGamble/cJSON.git
  GIT_TAG        v1.7.17
)
FetchContent_GetProperties(cjson)
if(NOT cjson_POPULATED)
  FetchContent_Populate(cjson)
  add_library(libcjson STATIC 
    ${cjson_SOURCE_DIR}/cJSON.c
    ${cjson_SOURCE_DIR}/cJSON.h
  )
  target_include_directories(libcjson PUBLIC ${cjson_SOURCE_DIR})
endif()

# 4. SQLite3 (Local Data Engine)
message(STATUS "Fetching sqlite3...")
FetchContent_Declare(
  sqlite3
  URL      https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip
)
FetchContent_GetProperties(sqlite3)
if(NOT sqlite3_POPULATED)
  FetchContent_Populate(sqlite3)
  add_library(sqlite3 STATIC ${sqlite3_SOURCE_DIR}/sqlite3.c)
  target_include_directories(sqlite3 PUBLIC ${sqlite3_SOURCE_DIR})
endif()

# ------------------------------------------------------------------------------
# Include Directories
# ------------------------------------------------------------------------------
include_directories(
  ${PROJECT_SOURCE_DIR}/include
)

# ------------------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------------------
file(GLOB_RECURSE SOURCES "src/*.c")

# ------------------------------------------------------------------------------
# Executable
# ------------------------------------------------------------------------------
add_executable(supporthive ${SOURCES})

# ------------------------------------------------------------------------------
# Linking
# ------------------------------------------------------------------------------
# Link against libuv (uv_a is the static library target usually) and http-parser
if(WIN32)
    target_link_libraries(supporthive PRIVATE uv_a http_parser libcjson sqlite3 ws2_32 psapi iphlpapi userenv)
else()
    target_link_libraries(supporthive PRIVATE uv_a http_parser libcjson sqlite3 pthread dl)
endif()

message(STATUS "Build configuration complete. Ready to compile SupportHive-C.")
