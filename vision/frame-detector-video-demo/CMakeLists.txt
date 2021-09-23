set(subProject frame-detector-video-demo)
project(${subProject})

file(GLOB SRCS *.c*)
file(GLOB HDRS *.h*)

get_filename_component(PARENT_DIR ${PROJECT_SOURCE_DIR} DIRECTORY)
set(SHARED "${PARENT_DIR}/shared/")
file(GLOB SHARED_HDRS_FILES ${SHARED}/*.h*)
file(GLOB SHARED_CPP_FILES ${SHARED}/*.c*)

add_executable(${subProject} ${SRCS} ${HDRS} ${SHARED_HDRS_FILES} ${SHARED_CPP_FILES})

target_include_directories(${subProject} PRIVATE ${Boost_INCLUDE_DIRS} ${${rootProject}_INCLUDE_DIRS} ${${subProject}_INCLUDE_DIRS} ${SHARED})

target_link_libraries( ${subProject} ${${rootProject}_LIBS} ${${subProject}_LIBS} ${OpenCV_LIBS} ${Boost_LIBRARIES} )

#Add to the apps list
list( APPEND ${rootProject}_APPS ${subProject} )
set( ${rootProject}_APPS ${${rootProject}_APPS} PARENT_SCOPE )

# Installation steps
install( TARGETS ${subProject}
        RUNTIME DESTINATION ${RUNTIME_INSTALL_DIRECTORY} )
