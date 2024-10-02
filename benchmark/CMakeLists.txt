
file(GLOB sources ./*.cpp)

foreach(source ${sources})
    get_filename_component(name ${source} NAME_WE)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE utils)
    add_test(NAME ${name} COMMAND ${name})
endforeach()
