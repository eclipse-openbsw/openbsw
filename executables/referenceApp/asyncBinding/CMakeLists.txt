add_library(asyncBinding INTERFACE)

target_include_directories(asyncBinding INTERFACE include)

target_link_libraries(asyncBinding INTERFACE runtime)
