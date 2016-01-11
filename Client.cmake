file(GLOB_RECURSE PlayPG_CLIENT_SOURCES ${PROJECT_SOURCE_DIR}/client/*.cpp)
file(GLOB_RECURSE PlayPG_CLIENT_HEADERS ${PROJECT_SOURCE_DIR}/client-include/*.cpp)

file(MAKE_DIRECTORY assets)
file(COPY ${PlayPG_TEST_ASSETS} DESTINATION assets)

include_directories("client-include")

set(PlayPG_CLIENT_LIBS  ${PlayPG_LIBS}
                        )

set(PlayPG_CLIENT_NAME ${CMAKE_PROJECT_NAME})

if ( PlayPG_DEBUG )
    set(PlayPG_CLIENT_NAME "${PlayPG_CLIENT_NAME}-d")
endif ()

add_executable(${PlayPG_CLIENT_NAME} ${PlayPG_SOURCES} ${PlayPG_CLIENT_SOURCES} ${PlayPG_CLIENT_HEADERS} ${PlayPG_HEADERS})
target_link_libraries(${PlayPG_CLIENT_NAME} ${PlayPG_CLIENT_LIBS})