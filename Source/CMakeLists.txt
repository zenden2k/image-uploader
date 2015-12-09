cmake_minimum_required(VERSION 2.8)								

project(ImageUploader)				

set(SOURCE_EXE "Image Uploader.cpp")			

include_directories(./) #include_directories(${CMAKE_SOURCE_DIR})

add_executable(ImageUploader ${SOURCE_EXE})	

add_subdirectory(Core/Utils)	

target_link_libraries(ImageUploader iuCore)

#target_link_libraries(main foo)	