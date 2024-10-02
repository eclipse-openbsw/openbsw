add_library(timer INTERFACE)

target_include_directories(timer INTERFACE include)

target_link_libraries(timer INTERFACE estd)
