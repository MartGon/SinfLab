
file(GLOB p2_src "*.h" "*.cpp")

add_executable(p2 ${p2_src})

# Libraries to be linked
target_link_libraries(p2 ${OPENSSL_LIBRARIES})