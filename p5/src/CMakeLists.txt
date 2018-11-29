
file(GLOB p5_src "*.h" "*.cpp")

add_executable(p5 ${p5_src})

# Libraries to be linked
target_link_libraries(p5 ${OpenCV_LIBS})