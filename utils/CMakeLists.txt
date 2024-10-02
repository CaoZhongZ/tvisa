

file(GLOB_RECURSE UTILS_SOURCES "*.cpp" "*.hpp")
message(STATUS "UTILS_SOURCES: ${UTILS_SOURCES}")

add_library(utils SHARED ${UTILS_SOURCES})
