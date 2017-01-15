add_library(Render
	src/File.cpp
	src/Mesh.cpp
	src/idl_gen_text.cpp
	src/idl_parser.cpp
	src/Pipeline.cpp
	src/CommandBuffer.cpp
	src/RendererVulkan.cpp
	src/Scene.cpp
	src/stb_image.c
	src/vulkanDebug.cpp
	src/vulkanShaders.cpp)

set_target_properties(Render PROPERTIES FOLDER "common")

target_include_directories(Render PUBLIC ./include)
target_include_directories(Render PRIVATE ./src)

target_link_libraries(Render Math)
