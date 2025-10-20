# Pull GoogleTest automatically
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/heads/main.zip
)
FetchContent_MakeAvailable(googletest)

# Add test executable
file(GLOB SOURCE_TEST_FILES
  "*_test.cpp"
  "interpret/*_test.cpp"
)
add_executable(asas_tests ${SOURCE_TEST_FILES})

# Link test executable against GoogleTest and asas_lib
target_include_directories(asas_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(asas_tests 
  PRIVATE
    asas_lib 
    GTest::gtest_main
)

# Add tests
include(GoogleTest)
gtest_discover_tests(asas_tests)
