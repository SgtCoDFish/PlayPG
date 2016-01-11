add_definitions(-DPLAYPG_BUILD_SERVER -DDATABASE_MYSQL)

find_package(Boost REQUIRED COMPONENTS program_options filesystem system)
find_package(ODB REQUIRED COMPONENTS mysql)

include(${ODB_USE_FILE})

file(GLOB_RECURSE PlayPG_SERVER_SOURCES ${PROJECT_SOURCE_DIR}/server/*.cpp)
file(GLOB_RECURSE PlayPG_SERVER_HEADERS ${PROJECT_SOURCE_DIR}/server-include/*.hpp)
set(PlayPG_ODB_SOURCES "")

odb_compile(PlayPG_ODB FILES ${PlayPG_ODB_HEADERS} ${PlayPG_ODB_SOURCES}
            DB mysql STANDARD c++14
            INCLUDE
                ${PROJECT_SOURCE_DIR}/include
                ${PROJECT_SOURCE_DIR}/include/data)

include_directories("server-include"
                    ${Boost_INCLUDE_DIR}
                    ${ODB_COMPILE_OUTPUT_DIR}
                    ${ODB_INCLUDE_DIRS}
                    )
                    
set(PlayPG_SERVER_LIBS ${PlayPG_LIBS}
                ${Boost_LIBRARIES}
                ${ODB_LIBRARIES}
                )

set (PlayPG_SERVER_NAME "serverPG")

if ( PlayPG_DEBUG )
    set (PlayPG_SERVER_NAME "${PlayPG_SERVER_NAME}-d")
endif ()

add_executable(${PlayPG_SERVER_NAME}
               ${PlayPG_SOURCES}
               ${PlayPG_SERVER_SOURCES}
               ${PlayPG_NON_DATA_HEADERS}
               ${PlayPG_SERVER_HEADERS}
               ${PlayPG_ODB})
target_link_libraries(${PlayPG_SERVER_NAME} ${PlayPG_SERVER_LIBS} mysqlcppconn)
