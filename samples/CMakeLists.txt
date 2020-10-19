include_directories(
    .
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_BINARY_DIR}
)

add_executable(agentBasic agentBasic.c)

target_link_libraries(agentBasic veides_agent_client)
