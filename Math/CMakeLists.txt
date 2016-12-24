add_library(Math SHARED
	src/Matrix.cpp
	src/SIMD_NEON.cpp
	src/SIMD_SSE.cpp
	)

set_target_properties(Math PROPERTIES FOLDER "common")

target_include_directories(Math PUBLIC ./include)
target_include_directories(Math PRIVATE ./src)
