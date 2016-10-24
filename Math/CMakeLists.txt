add_library ( Math MathBase.cpp Matrix.cpp Quaternion.cpp SIMD_SSE.cpp )

target_include_directories (Math PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})