add_executable(
  rib_tests
  rib_tests.cpp

  ../adpcm_codec.cpp
  ../codec.cpp
)
target_link_libraries(
  rib_tests
  GTest::gtest_main
)
target_include_directories(rib_tests PRIVATE ${PROJECT_SOURCE_DIR})

gtest_discover_tests(rib_tests
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests
)
