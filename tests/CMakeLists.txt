
add_executable(tests tests.cxx)
target_link_libraries(tests patterns)

set_property(TARGET tests PROPERTY CXX_STANDARD 17)
