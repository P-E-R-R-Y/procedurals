set(NAME system)
set(TAG v1.0.0)

if(NOT ${NAME}_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        ${NAME}
        GIT_REPOSITORY https://github.com/p-e-r-r-y/system.git
        GIT_TAG        ${TAG}
    )
    FetchContent_MakeAvailable(${NAME})
endif()