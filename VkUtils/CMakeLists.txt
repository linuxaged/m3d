file(GLOB COMMON_SOURCE *.cpp *.cc *.h *.hpp)
add_library(VkUtils ${COMMON_SOURCE})
set_target_properties(VkUtils PROPERTIES FOLDER "common")

# TODO: we alreay add deps in root CMakeLists.txt
add_dependencies( VkUtils Math gli glm flatbuffer)
link_libraries(Math gli glm flatbuffer)
include_directories(Math, gli, glm, flatbuffer)