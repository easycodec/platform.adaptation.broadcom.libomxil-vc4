set(EXEC hello_video.bin)
set(SRCS tsemaphore.c queue.c video.c)
#SET(pc_dependents "libtbm")

add_executable(${EXEC} ${SRCS})
#add_dependencies(${EXEC} ${pc_dependents})
target_link_libraries(${EXEC} ${HELLO_PI_LIBS} -ltbm)

install(TARGETS ${EXEC}
        RUNTIME DESTINATION bin)
