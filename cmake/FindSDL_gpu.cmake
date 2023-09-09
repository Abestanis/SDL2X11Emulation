if(NOT TARGET SDL_gpu)
    include(FetchContent)
    FetchContent_Declare(
        SDL_gpu
        GIT_REPOSITORY https://github.com/grimfang4/sdl-gpu.git
        GIT_TAG        47a3e2b2a9326c33ad6f177794705987399de8cf
    )
    FetchContent_MakeAvailable(SDL_gpu)

    set(SDL_gpu_LIBRARY SDL_gpu)
    set(SDL_gpu_INCLUDE_DIR "${SDL_gpu_SOURCE_DIR}/include")
    target_include_directories(SDL_gpu PUBLIC "${SDL_gpu_INCLUDE_DIR}")

    include(FindPackageHandleStandardArgs)
    find_package_handle_standard_args(SDL_gpu
        REQUIRED_VARS
        SDL_gpu_LIBRARY
        SDL_gpu_INCLUDE_DIR
    )
endif ()
