add_library(udsConfiguration INTERFACE)

target_include_directories(udsConfiguration INTERFACE include)

add_library(udsConfigurationImpl src/uds/session/DiagSession.cpp)

target_link_libraries(udsConfigurationImpl PRIVATE uds)
