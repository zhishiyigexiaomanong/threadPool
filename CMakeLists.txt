cmake_minimum_required(VERSION 3.15)

project(test)

aux_source_directory(. SRC)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE pthread)




