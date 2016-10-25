file(GLOB COMMON_SOURCE *.cpp *.h *.hpp)
add_library(VkUtils ${COMMON_SOURCE})
set_target_properties(VkUtils PROPERTIES FOLDER "common")

# TODO: target_link_libraries
add_dependencies( VkUtils Math)
link_libraries(Math)
include_directories(Math)