file(GLOB COMMON_SOURCE *.cpp *.cc *.h *.hpp)
add_library(VkUtils ${COMMON_SOURCE})
set_target_properties(VkUtils PROPERTIES FOLDER "common")

# TODO: target_link_libraries
add_dependencies( VkUtils Math gli glm)
link_libraries(Math gli glm)
include_directories(Math, gli, glm)