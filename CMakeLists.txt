cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project(Minecraft4k C CXX)

include_directories(include)

set(PROJECT_NAME Minecraft4k)

set(Header_Files
    "Constants.h"
    "Shader.h"
    "TextureGenerator.h"
    "Util.h"
    "World.h"
    "Vector.h"
)
source_group("Header Files" FILES ${Header_Files})

set(Resource_Files
    "res/raytrace.comp"
    "res/screen.frag"
    "res/screen.vert"
)
source_group("Resource Files" FILES ${Resource_Files})

set(Source_Files
    "glad.c"
    "Minecraft4k.cpp"
    "Shader.cpp"
    "TextureGenerator.cpp"
    "Util.cpp"
    "World.cpp"
    "Vector.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Header_Files}
    ${Resource_Files}
    ${Source_Files}
)


add_executable(${PROJECT_NAME} ${ALL_FILES})
set(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})


if(UNIX)
    # TODO -m32, use C libs 
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Os -s -fno-rtti -fno-stack-protector -ffunction-sections -fdata-sections -Wl,--gc-sections
            -fno-math-errno -fno-unroll-loops -fmerge-all-constants -fno-ident -mfpmath=387 -mfancy-math-387
	    -fsingle-precision-constant -ffast-math -fno-exceptions -nostartfiles -nostdlib -nodefaultlibs
	    -fcf-protection=none -N -fno-align-functions -fomit-frame-pointer -fno-threadsafe-statics
	    -finline-limit=10 -fno-asynchronous-unwind-tables 
        )

    # TODO --nmagic
    target_link_options(${PROJECT_NAME} PRIVATE
	    -s -Wl,--hash-style=gnu,--build-id=none,-z,max-page-size=2048,-z,norelro,-z,now,--as-needed,--gc-sections
        )

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND strip -S --strip-unneeded --remove-section=.note.gnu.gold-version --remove-section=.comment --remove-section=.note --remove-section=.note.gnu.build-id --remove-section=.note.ABI-tag -R .note -R .comment -R .eh_frame -R .eh_frame_hdr -s ${PROJECT_NAME}
	
	COMMAND objcopy --remove-section .note.gnu.property --remove-section .gnu.version --remove-section .fini --remove-section .init_array --remove-section .got ${PROJECT_NAME}
    )

endif()

# Compile and link options
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /Oi-;
            /Gy
        >
        /permissive-;
        /O1;
        /sdl;
        /W3;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        /Os;
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:
            /INCREMENTAL
        >
        $<$<CONFIG:Release>:
            /INCREMENTAL:NO
        >
        /DEBUG;
        /SUBSYSTEM:CONSOLE;
        /OPT:REF;
        /OPT:ICF
    )
endif()

# Dependencies
if(UNIX)
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "glfw"
    "dl"
)
else()
set(ADDITIONAL_LIBRARY_DEPENDENCIES
    "glfw"
)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

link_directories(${PROJECT_NAME} PRIVATE
    "lib"
)
