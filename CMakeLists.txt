cmake_minimum_required(VERSION 3.10)
project(srec_ipc C)

set(CMAKE_C_STANDARD 11)
# Compilation Warning deactivated
#add_compile_options(-Wall -Wextra)

add_executable(sender sender.c common/srec.c common/ipc.c)
add_executable(receiver receiver.c common/srec.c common/ipc.c)
