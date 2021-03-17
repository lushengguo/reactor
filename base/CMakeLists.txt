aux_source_directory(. BASESRCS)
add_library(base ${BASESRCS})
add_library(base::library ALIAS base)
target_link_libraries(base Threads::Threads)

add_subdirectory(Test)