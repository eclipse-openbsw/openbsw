add_executable(asyncImplExample src/example.cpp)

target_include_directories(asyncImplExample PRIVATE include)

target_link_libraries(asyncImplExample PRIVATE asyncImpl async estd)
