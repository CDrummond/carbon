configure_file (carbon-runner.cmake ${CMAKE_CURRENT_BINARY_DIR}/carbon-runner @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/carbon-runner ${CMAKE_CURRENT_BINARY_DIR}/carbon-pkexec DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}/scripts)
