file(GLOB SOURCES *.cpp)

add_executable(${PROJECT_NAME} ${SOURCES}) 
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
target_link_libraries(${PROJECT_NAME} opencv_core)
