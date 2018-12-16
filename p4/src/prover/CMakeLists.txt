
file(GLOB p4_prover_src "*.h" "*.cpp")

add_executable(p4_prover ${p4_prover_src})

# Libraries to be linked
target_link_libraries(p4_prover ${OPENSSL_LIBRARIES} ${SDL2_NET_LIBRARIES} ${SDL2_LIBRARY})