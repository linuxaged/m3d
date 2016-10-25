add_library ( VkUtils common.cpp vulkanContext.cpp vulkanDebug.cpp vulkanShaders.cpp vulkanSwapChain.cpp vulkanTools.cpp )
target_include_directories (VkUtils PUBLIC $(CMAKE_CURRENT_SOURCE_DIR))