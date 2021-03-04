aux_source_directory(. NETSRCS)
add_library(net ${NETSRCS})
add_library(net::library ALIAS net)
target_link_libraries(net base::library)

# add_subdirectory(Test)