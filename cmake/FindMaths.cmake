set(NAME maths)
set(TAG main)

if(NOT ${NAME}_FOUND)
    include(FetchContent)
    FetchContent_Declare(
        ${NAME}
        GIT_REPOSITORY https://github.com/p-e-r-r-y/${NAME}.git
        GIT_TAG        ${TAG}
    )
    FetchContent_MakeAvailable(${NAME})
endif()