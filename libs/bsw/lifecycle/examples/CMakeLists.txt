add_library(lifecycleExamples OBJECT src/examples.cpp)

target_include_directories(lifecycleExamples PRIVATE include)

target_link_libraries(lifecycleExamples PRIVATE asyncMock lifecycle)
