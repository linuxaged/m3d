file(GLOB COMMON_SOURCE *.cpp *.h *.hpp)
add_library(Math ${COMMON_SOURCE})
set_target_properties(Math PROPERTIES FOLDER "common")