
file(GLOB p3_src "*.h" "*.cpp")

add_executable(p3 ${p3_src})

# Libraries to be linked
target_link_libraries(p3 ${OPENSSL_LIBRARIES})