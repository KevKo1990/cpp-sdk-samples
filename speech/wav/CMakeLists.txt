set(subProject wav)
project(${subProject})

# --------------------
# DEPENDENCIES
# --------------------

ADD_LIB(${subProject} LibSndFile)

# --------------------
# BUILD
# --------------------

file(GLOB SRCS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp)
file(GLOB HDRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h ../headers/*.h)

add_executable(${subProject} ${SRCS} ${HDRS})
target_include_directories(${subProject} PRIVATE ${${rootProject}_INCLUDE_DIRS} ${${subProject}_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/headers)
target_link_libraries(${subProject} ${${rootProject}_LIBS} ${${subProject}_LIBS})

# Installation steps
install( TARGETS ${subProject}
        RUNTIME DESTINATION ${RUNTIME_INSTALL_DIRECTORY}
        LIBRARY DESTINATION ${LIBRARY_INSTALL_DIRECTORY}
        ARCHIVE DESTINATION ${ARCHIVE_INSTALL_DIRECTORY})
