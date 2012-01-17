function(python_install_module_dirs)
	set(path lib/python${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}/site-packages)
	foreach(dir ${ARGN})
		install(DIRECTORY ${dir}/ DESTINATION ${path} FILES_MATCHING PATTERN "*.py")
	endforeach()
endfunction()
