set(src
    embed/types.cc embed/types.h
)
add_library(embed STATIC ${src})
target_include_directories(embed
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)
