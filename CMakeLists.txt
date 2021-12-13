
cmake_minimum_required(VERSION 3.16)

project("cc")

set(LINK_LIB_LIST)

set(CMAKE_BUILD_TYPE "Debug")

option(BUILD_SHARED_AND_STATIC_LIBS "Build both shared and static libraries" on)

# set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib/)

add_subdirectory(cJSON)
add_subdirectory(utils_cpp)

include_directories("${PROJECT_SOURCE_DIR}/cJSON/include/")
include_directories("${PROJECT_SOURCE_DIR}/utils_cpp/include/")
include_directories("${PROJECT_SOURCE_DIR}/openssl/include/")



list(APPEND LINK_LIB_LIST
	"${PROJECT_SOURCE_DIR}/lib/libcjson.a" 
	"${PROJECT_SOURCE_DIR}/lib/libutils.a" 

	"${PROJECT_SOURCE_DIR}/lib/libcrypto.a" 
	"${PROJECT_SOURCE_DIR}/lib/libssl.a" 
	pthread
	dl
	m
	)


add_subdirectory(tests)
