add_executable(ylangylang ylangylang.cxx)
target_link_libraries(ylangylang patterns)

set_property(TARGET ylangylang PROPERTY CXX_STANDARD 17)
