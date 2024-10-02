cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)

project(Tvisa)

include(CTest)
enable_testing()
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_C_COMPILER icx)
set(CMAKE_CXX_COMPILER icpx)

# include sycl headers
find_program(SYCL_LIB icpx)
get_filename_component(SYCL_ROOT ${SYCL_LIB} DIRECTORY)
get_filename_component(SYCL_ROOT ${SYCL_ROOT} DIRECTORY)
message(STATUS "SYCL_ROOT: ${SYCL_ROOT}")

# Set compile flags
include_directories(${SYCL_ROOT}/include/sycl;${SYCL_ROOT}/include/;include/;utils/)
set(CXXFLAGS -ffast-math -Wall -Wno-deprecated-declarations -Wno-unused-variable -Wno-vla-cxx-extension)
set(SYCLFLAGS -fsycl -fsycl-id-queries-fit-in-int -fsycl-default-sub-group-size=16 -D__SYCL_INTERNAL_API -fsycl-targets=spir64_gen)
add_compile_options(${CXXFLAGS} ${SYCLFLAGS})

# Set link flags
set(LINKFLAGS -fsycl -fsycl-max-parallel-link-jobs=8 -lmkl_intel_lp64 -lmkl_sequential -lmkl_core -lpthread -lm)
set(LINKFLAGS ${LINKFLAGS} "-fsycl-targets=spir64_gen")
set(LINKFLAGS ${LINKFLAGS} -Xsycl-target-backend=spir64_gen "-device pvc -internal_options -cl-intel-256-GRF-per-thread")
message(STATUS "LINKFLAGS: ${LINKFLAGS}")
add_link_options(${LINKFLAGS})

add_subdirectory(utils)

# add executable for each file
add_subdirectory(test)
add_subdirectory(benchmark)
