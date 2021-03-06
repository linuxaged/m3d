set(EXTERNAL_NAME glog)

include(ExternalProject)
ExternalProject_Add(
  ${EXTERNAL_NAME}
  # the CMake support is only in master, has not been released
  URL https://github.com/google/glog/archive/b6a5e0524c28178985f0d228e9eaa43808dbec3c.zip
  URL_MD5 e26cd32670bfacd1e80aa571984d770a
  BINARY_DIR ${EXTERNAL_PROJECT_PREFIX}/build
  CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  LOG_DOWNLOAD 1
  LOG_CONFIGURE 1
  LOG_BUILD 1
)

# Hide this external target (for ide users)
set_target_properties(${EXTERNAL_NAME} PROPERTIES FOLDER "externals")

ExternalProject_Get_Property(${EXTERNAL_NAME} INSTALL_DIR)

string(TOUPPER ${EXTERNAL_NAME} EXTERNAL_NAME_UPPER)
set(${EXTERNAL_NAME_UPPER}_INCLUDE_DIRS ${INSTALL_DIR}/include CACHE PATH "List of glog include directories")