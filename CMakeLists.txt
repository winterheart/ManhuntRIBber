cmake_minimum_required(VERSION 3.20)

project(ManhuntRIBber VERSION 0.4.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

configure_file(manhuntribber_version.h.in manhuntribber_version.h)

add_executable(manhuntribber
	CLI11.hpp
	adpcm_decoder.h
	adpcm_decoder.cpp
	main.cpp
)
target_include_directories(manhuntribber PUBLIC	"${PROJECT_BINARY_DIR}")
