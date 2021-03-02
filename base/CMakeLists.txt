aux_source_directory(. BASESRCS)
add_library(base ${BASESRCS})
add_library(base::library ALIAS base)
target_link_libraries(base thread::library)

add_subdirectory(Test)
add_subdirectory(Thread)