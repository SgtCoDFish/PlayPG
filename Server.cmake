find_package(Boost REQUIRED COMPONENTS program_options)

file(GLOB_RECURSE PlayPG_SERVER_SOURCES ${PROJECT_SOURCE_DIR}/server/*.cpp)

include_directories("server-include"
                    ${Boost_INCLUDE_DIR}
                    ${MysqlConnectorCpp_INCLUDES}
                    )
                    
set(PlayPG_SERVER_LIBS ${PlayPG_LIBS}
                ${Boost_LIBRARIES}
                mysqlcppconn
                )

set (PlayPG_SERVER_NAME "serverPG")

if ( PLAYPG_DEBUG )
    set (PlayPG_SERVER_NAME "${PlayPG_SERVER_NAME}-d")
endif ()

add_executable(${PlayPG_SERVER_NAME} ${PlayPG_SOURCES} ${PlayPG_SERVER_SOURCES})
target_link_libraries(${PlayPG_SERVER_NAME} ${PlayPG_SERVER_LIBS})
