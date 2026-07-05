find_path(
    RED_ORANGE_INCLUDE_DIR
    NAMES red_dragon.hpp
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../drachenland/fiery"
    REQUIRED
)

find_path(
    BLUE_INCLUDE_DIR
    NAMES blue_dragon.hpp
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../drachenland/icy"
    REQUIRED
)

find_path(
    TOOTH_INCLUDE_DIR
    NAMES toothless.h
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../drachenland/fury"
    REQUIRED
)

find_path(
    UTILS_INCLUDE_DIR
    NAMES breath_power.hpp
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../utils"
    REQUIRED
)

set(
    DRAGONS_INCLUDE_DIR
    "${RED_ORANGE_INCLUDE_DIR}"
    "${BLUE_INCLUDE_DIR}"
    "${TOOTH_INCLUDE_DIR}"
    "${UTILS_INCLUDE_DIR}"
)

find_library(
    DRAGONS_LIBRARY
    NAMES Dragons
    HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../drachenland/lib"
    REQUIRED
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
    DRAGONS
    FOUND_VAR DRAGONS_FOUND
    REQUIRED_VARS
        RED_ORANGE_INCLUDE_DIR
        BLUE_INCLUDE_DIR
        TOOTH_INCLUDE_DIR
        DRAGONS_LIBRARY 
)

# make it target to attach headers, dependencies
if (DRAGONS_FOUND AND NOT TARGET DRAGONS::Dragons)
    # unknown - CMake does it itself
    add_library(DRAGONS::Dragons UNKNOWN IMPORTED)
    set_target_properties(
        DRAGONS::Dragons PROPERTIES
        IMPORTED_LOCATION "${DRAGONS_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${DRAGONS_INCLUDE_DIR}"
    )
endif()