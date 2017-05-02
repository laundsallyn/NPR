SET(pwd ${CMAKE_CURRENT_LIST_DIR})

SET(src "")
AUX_SOURCE_DIRECTORY(${pwd} src)
add_executable(npr ${src})
message(STATUS "npr added ${src}")

target_link_libraries(npr ${stdgl_libraries})
find_library(TWEAKBAR AntTweakBar ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(npr ${TWEAKBAR})
FIND_PACKAGE(JPEG REQUIRED)
TARGET_LINK_LIBRARIES(npr ${JPEG_LIBRARIES})
