file(GLOB COMMON_SOURCE *.cpp *.h *.hpp)
add_library(VkUtils ${COMMON_SOURCE})
set_target_properties(VkUtils PROPERTIES FOLDER "common")