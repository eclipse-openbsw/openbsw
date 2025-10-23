add_library(safeUtils INTERFACE)

target_include_directories(safeUtils INTERFACE include)

target_link_libraries(safeUtils INTERFACE util)
