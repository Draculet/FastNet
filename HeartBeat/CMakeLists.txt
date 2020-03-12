cmake_minimum_required (VERSION 2.8)

project (HeartBeat)
add_definitions("-std=c++11 -lpthread")
aux_source_directory(../FastNet/src BASE_DIR)
aux_source_directory(. BASE_DIR2)
find_package (Threads)
add_executable(main ${BASE_DIR} ${BASE_DIR2})
target_link_libraries (main ${CMAKE_THREAD_LIBS_INIT})

