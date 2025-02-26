cmake_minimum_required(VERSION 3.20)

project(ManhuntRIBber VERSION 0.6.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(manhuntribber_version.h.in manhuntribber_version.h)

add_executable(manhuntribber
	CLI11.hpp
	adpcm_codec.h
	adpcm_codec.cpp
	byteswap.h
	codec.h
	codec.cpp
	main.cpp
)
target_include_directories(manhuntribber PUBLIC	"${PROJECT_BINARY_DIR}")
target_link_options(manhuntribber PRIVATE
	$<$<PLATFORM_ID:Windows>:-static>
	$<$<PLATFORM_ID:Windows>:-s>
)

if(BUILD_TESTING)
	find_package(GTest REQUIRED)
	enable_testing()
	include(GoogleTest)
	add_subdirectory(tests)
endif()
