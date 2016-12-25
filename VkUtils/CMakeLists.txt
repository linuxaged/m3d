add_library(VkUtils
	src/idl_gen_text.cpp
	src/idl_parser.cpp
	src/RendererVulkan.cpp
	src/Scene.cpp
	src/stb_image.c
	src/vulkanDebug.cpp
	src/vulkanShaders.cpp)

set_target_properties(VkUtils PROPERTIES FOLDER "common")

target_include_directories(VkUtils PUBLIC ./include)
target_include_directories(VkUtils PRIVATE ./src)

target_link_libraries(VkUtils Math)
