configure_file (carbon-runner.cmake ${CMAKE_CURRENT_BINARY_DIR}/carbon-runner @ONLY)
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/carbon-runner carbon-terminate
	DESTINATION ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}/scripts)
