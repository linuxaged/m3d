file(GLOB COMMON_SOURCE *.cpp *.h *.hpp)
add_library(Gui ${COMMON_SOURCE})
set_target_properties(Gui PROPERTIES FOLDER "common")

# TODO: target_link_libraries
add_dependencies( Gui Math )
link_libraries(Math)
include_directories(Math)