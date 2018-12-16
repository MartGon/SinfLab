
file(GLOB p4_verifier_src "*.h" "*.cpp")

add_executable(p4_verifier ${p4_verifier_src})

# Libraries to be linked
target_link_libraries(p4_verifier ${OPENSSL_LIBRARIES} ${SDL2_NET_LIBRARIES} ${SDL2_LIBRARY})