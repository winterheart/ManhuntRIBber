cmake_minimum_required(VERSION 3.20)

project(ManhuntRIBber VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(manhuntribber
	CLI11.hpp
	adpcm_decoder.h
	adpcm_decoder.cpp
	main.cpp
)

#add_compile_definitions(manhuntribber -DPLACE_PREDICTOR_TWICE=1)