include_guard()
include(FetchContent)

FetchContent_Declare(
    gwtoolboxpp
    GIT_REPOSITORY https://github.com/gwdevhub/gwtoolboxpp
    GIT_TAG dev
)

FetchContent_GetProperties(gwtoolboxpp)

if(gwtoolboxpp_POPULATED)
    message(STATUS "Skipping gwtoolboxpp download")
    return()
else()
    message(STATUS "Fetching gwtoolboxpp...")
endif()

FetchContent_Populate(gwtoolboxpp)

message(STATUS "Done.")

add_subdirectory(${gwtoolboxpp_SOURCE_DIR} ${gwtoolboxpp_BINARY_DIR} EXCLUDE_FROM_ALL)