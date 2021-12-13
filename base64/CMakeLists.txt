
cmake_minimum_required(VERSION 3.0)

project(base64)


#cJSON
set(BASE64_LIB base64)

file(GLOB HEADERS ${PROJECT_SOURCE_DIR}/include/base64/base64.h)
set(SOURCES src/base64.c)

include_directories(${PROJECT_SOURCE_DIR}/include/)


add_library("${BASE64_LIB}-static" STATIC "${HEADERS}" "${SOURCES}")
set_target_properties("${BASE64_LIB}-static" PROPERTIES OUTPUT_NAME "${BASE64_LIB}")


add_subdirectory(tests)
