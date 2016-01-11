find_package(Boost REQUIRED COMPONENTS program_options filesystem system)

file(GLOB_RECURSE PlayPG_SERVER_SOURCES ${PROJECT_SOURCE_DIR}/server/*.cpp)
file(GLOB_RECURSE PlayPG_SERVER_HEADERS ${PROJECT_SOURCE_DIR}/server-include/*.hpp)

include_directories("server-include"
                    ${Boost_INCLUDE_DIR}
                    ${MysqlConnectorCpp_INCLUDES}
                    )
                    
set(PlayPG_SERVER_LIBS ${PlayPG_LIBS}
                ${Boost_LIBRARIES}
                mysqlcppconn
                )

set (PlayPG_SERVER_NAME "serverPG")

if ( PlayPG_DEBUG )
    set (PlayPG_SERVER_NAME "${PlayPG_SERVER_NAME}-d")
endif ()

add_executable(${PlayPG_SERVER_NAME} ${PlayPG_SOURCES} ${PlayPG_SERVER_SOURCES} ${PlayPG_HEADERS} ${PlayPG_SERVER_HEADERS})
target_link_libraries(${PlayPG_SERVER_NAME} ${PlayPG_SERVER_LIBS})
