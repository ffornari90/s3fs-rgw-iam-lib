cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-attributes")

#
# Create version information
#
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make_version_files.sh commit_hash)
execute_process(COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/make_version_files.sh product_version)
file(READ commit_hash COMMIT_HASH)
file(READ product_version PRODUCT_VERSION)

#
# Set project information
#
project(s3fs_rgw_sts LANGUAGES CXX)
project(s3fs_rgw_sts VERSION ${PRODUCT_VERSION})
project(s3fs_rgw_sts DESCRIPTION "Software using AWS C++ SDK to perform Ceph RadosGW STS")

find_package(nlohmann_json 3.11.2 REQUIRED)
find_package(CURL 7.74.0 REQUIRED)

include(ExternalProject)

ExternalProject_Add(s3fs-fuse
    GIT_REPOSITORY https://github.com/s3fs-fuse/s3fs-fuse.git
    GIT_TAG 6e89e69bba8e3bd8679fe3d7d47ca01e32951cee
    GIT_SHALLOW FALSE
    CONFIGURE_COMMAND cp -R ../s3fs-fuse/. . && ./autogen.sh && ./configure
    COMMAND make
)

#
# Create config.h
#
configure_file(config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

#
# Set Library information
#
set(LIB_NAME   "rgw-sts")
set(LIB_SRC    "src/main.cpp" "src/readSecretValue.cpp" "src/configureClient.cpp" "src/getOIDCRefreshToken.cpp" "src/getOIDCAccessToken.cpp" "src/s3fsFuncs.cpp")
set(LIB_HEADER "src/main.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/config.h")
set(LIB_SAMPLE "src/rgw_sts_test.cpp")
set(LIB_TYPE   "SHARED")

#
# AWS libraries
#
find_package(AWSSDK REQUIRED COMPONENTS core identity-management sts iam s3)

#
# For building Library
#
add_library(${LIB_NAME} ${LIB_TYPE} ${LIB_SRC})
target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} /usr/include /usr/local/include)
target_link_libraries(${LIB_NAME} curl nlohmann_json::nlohmann_json ${AWSSDK_LINK_LIBRARIES})

#
# For building test file.
#
link_directories(${INSTALL_DIR}/lib)
add_executable("${LIB_NAME}_test" ${LIB_SAMPLE})
target_include_directories("${LIB_NAME}_test" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${INSTALL_DIR}/include)
target_link_libraries("${LIB_NAME}_test" ${LIB_NAME} ${AWSSDK_LINK_LIBRARIES})

#
# Specify Install Folder
#
install(TARGETS "${LIB_NAME}" LIBRARY DESTINATION "/usr/local/lib")
