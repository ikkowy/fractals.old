add_library(websocket
    include/websocket/websocket.h
    src/websocket.c)

target_include_directories(websocket PUBLIC include)

target_link_libraries(websocket utils)
