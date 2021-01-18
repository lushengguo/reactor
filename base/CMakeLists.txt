aux_source_directory(. BASE_SRCS)
add_library(reactor_base ${BASE_SRCS})
target_link_libraries(reactor_base pthread)
add_library(base::library ALIAS reactor_base)

enable_testing()
add_subdirectory(test)