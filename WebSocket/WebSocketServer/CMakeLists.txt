cmake_minimum_required (VERSION 2.8)

project (WebSocketServer)
add_definitions("-std=c++11 -lpthread")
aux_source_directory(../../FastNet/src BASE_DIR)
aux_source_directory(. BASE_DIR2)
aux_source_directory(../WebSocketCodec BASE_DIR3)
aux_source_directory(../WebSocketCodec/headerParse BASE_DIR4)
find_package (Threads)
add_executable(main ${BASE_DIR} ${BASE_DIR2} ${BASE_DIR3} ${BASE_DIR4})
target_link_libraries (main ${CMAKE_THREAD_LIBS_INIT})

