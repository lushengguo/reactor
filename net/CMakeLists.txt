aux_source_directory(. NETSRCS)
add_library(net ${NETSRCS})
add_library(net::library ALIAS net)
target_link_libraries(net poller::library)

add_subdirectory(Test)
add_subdirectory(Poller)