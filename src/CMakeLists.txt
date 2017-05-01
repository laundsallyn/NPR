SET(pwd ${CMAKE_CURRENT_LIST_DIR})

SET(src "")
AUX_SOURCE_DIRECTORY(${pwd} src)
add_executable(skinning ${src})
message(STATUS "skinning added ${src}")

target_link_libraries(skinning ${stdgl_libraries})
find_library(TWEAKBAR AntTweakBar ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(skinning ${TWEAKBAR})
FIND_PACKAGE(JPEG REQUIRED)
TARGET_LINK_LIBRARIES(skinning ${JPEG_LIBRARIES})
