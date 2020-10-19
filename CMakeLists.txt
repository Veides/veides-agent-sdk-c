cmake_minimum_required(VERSION 3.5.0)

project(veides_agent_client C CXX)

set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

set(VEIDES_AGENT_CLIENT_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

option(VEIDES_BUILD_DEPENDENCIES "Either build dependent libraries or use system libraries" OFF)
option(VEIDES_INSTALL_DEPENDENCIES "Whether dependencies should be installed in the system or not" OFF)
option(VEIDES_BUILD_TESTS "Set to ON to build the tests" OFF)
option(VEIDES_BUILD_SAMPLES "To build samples or not to build" ON)

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    message(STATUS "Setting OPENSSL_ROOT_DIR to /usr/local/opt/openssl")
    set(OPENSSL_ROOT_DIR /usr/local/opt/openssl)
    link_directories(${OPENSSL_ROOT_DIR}/lib)
endif()

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

if(NOT ${VEIDES_BUILD_DEPENDENCIES})
    find_library(PAHO_SYSTEM_LIBRARY paho-mqtt3as REQUIRED)
    set(PAHO_CLIENT_VERSION "system")
endif()

message(STATUS "Veides Agent Client ${VEIDES_AGENT_CLIENT_VERSION}")

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/veides_agent_client/include
    ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty
    ${PROJECT_BINARY_DIR}
)

set(
    sources

    veides_agent_client/src/veides_agent_client.c
    veides_agent_client/src/veides_base_client.c
    veides_agent_client/src/veides_logger.c
    veides_agent_client/src/veides_properties.c
    veides_agent_client/src/veides_utils.c
)

set(
    headers

    veides_agent_client/include/veides_agent_client.h
    veides_agent_client/include/veides_logger.h
    veides_agent_client/include/veides_utils.h
    veides_agent_client/include/veides_properties.h
    veides_agent_client/include/veides_rc.h
)

set(
    headers_internal

    veides_agent_client/include/veides_base_client.h
    veides_agent_client/include/veides_internal.h
)

set(
    3rdparty

    3rdparty/cJSON/cJSON.c
    3rdparty/cJSON/cJSON.h

)

add_library(veides_agent_client SHARED ${sources} ${headers} ${headers_internal} ${3rdparty})

set_target_properties(
    veides_agent_client

    PROPERTIES
    VERSION ${VEIDES_AGENT_CLIENT_VERSION}
    SOVERSION ${VERSION_MAJOR}
)

if(NOT ${VEIDES_BUILD_DEPENDENCIES})
    message(STATUS "Using system Paho MQTT Library")

    target_link_libraries(
        veides_agent_client

        ${PAHO_SYSTEM_LIBRARY}
    )
else()
    message(STATUS "Adding Paho MQTT Library to build pipeline")

    set(PAHO_CLIENT_VERSION 1.3.7)

    add_subdirectory(3rdparty/paho)

    include_directories(
        ${PAHO_INCLUDE_DIR}
    )

    add_dependencies(veides_agent_client paho)

    target_link_libraries(
        veides_agent_client

        ${PAHO_LIBS_DIR}/libpaho-mqtt3as${CMAKE_SHARED_LIBRARY_SUFFIX}
    )
endif()

target_link_libraries(veides_agent_client ${CMAKE_THREAD_LIBS_INIT})

install(
    TARGETS

    veides_agent_client

    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(
    FILES

    ${headers}

    DESTINATION include
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/veides_agent_client/include/veides_version.h.in
    ${CMAKE_BINARY_DIR}/veides_version.h
    @ONLY
)

if(${VEIDES_BUILD_SAMPLES})
    add_subdirectory(samples)
endif()

if(${VEIDES_BUILD_TESTS})
    enable_testing()

    add_subdirectory(3rdparty/gtest)

    set(
        test_sources

        tests/veides_agent_client.cpp
        tests/veides_agent_client_properties.cpp
    )

    include_directories(
        ${GTEST_INCLUDE_DIR}
        tests
    )

    add_executable(veidesAgentClientTests ${test_sources} ${sources} ${3rdparty})
    add_dependencies(veidesAgentClientTests googletest)
    add_test(veidesAgentClientTests veidesAgentClientTests)

    target_link_libraries(
        veidesAgentClientTests

        ${GTEST_LIBS_DIR}/libgtest${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${GTEST_LIBS_DIR}/libgtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}
    )

    if(NOT ${VEIDES_BUILD_DEPENDENCIES})
        target_link_libraries(
            veidesAgentClientTests

            ${PAHO_SYSTEM_LIBRARY}
        )
    else()
        target_link_libraries(
            veidesAgentClientTests

            ${PAHO_LIBS_DIR}/libpaho-mqtt3as${CMAKE_SHARED_LIBRARY_SUFFIX}
        )

        add_dependencies(veidesAgentClientTests paho)
    endif()

    target_link_libraries(veidesAgentClientTests ${CMAKE_THREAD_LIBS_INIT})
endif()