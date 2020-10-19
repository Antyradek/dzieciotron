file(GLOB SOURCES *.cpp)

add_executable(${PROJECT_NAME} ${SOURCES}) 

# który mamy rok, przypomnij?
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${PROJECT_NAME} PROPERTY COMPILE_OPTIONS -Wall -Wextra -Werror)

# czasami OpenCV instaluje się w dziwnych miejscach
target_include_directories(${PROJECT_NAME} PRIVATE /usr/include/opencv4)

#target_link_libraries(${PROJECT_NAME} opencv_highgui rt boost_program_options)
target_link_libraries(${PROJECT_NAME} pthread opencv_core opencv_videoio opencv_imgproc)
