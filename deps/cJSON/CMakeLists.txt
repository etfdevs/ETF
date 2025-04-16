add_library(cJSON OBJECT cJSON.c cJSON.h)
target_include_directories(cJSON SYSTEM INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
