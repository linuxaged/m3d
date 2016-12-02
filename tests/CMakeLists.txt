file ( GLOB M3D_TEST_SOURCE tests/*.cpp tests/gtest/*.cc )

add_executable ( m3d_test ${M3D_TEST_SOURCE})

target_link_libraries ( m3d_test glog Math)

add_test (m3d_unit_test m3d_test)