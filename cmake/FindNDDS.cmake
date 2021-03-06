# Platform detection and defines
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	add_definitions(-DRTI_UNIX -DRTI_LINUX)
	if(NOT NDDS_ARCH)
		if (CMAKE_SIZEOF_VOID_P EQUAL 4)
			set(NDDS_ARCH "i86Linux2.6gcc4.1.1" CACHE STRING "Architecture type for RTI DDS")
		else()
			set(NDDS_ARCH "x64Linux2.6gcc4.1.1" CACHE STRING "Architecture type for RTI DDS")
		endif()
	endif()
	mark_as_advanced(NDDS_ARCH)
else()
	message(FATAL_ERROR "TODO: FindNDDS.cmake windows support!")
endif()

# Root directory
find_path(NDDS_HOME rev_host_rtidds.4.5d HINTS $ENV{NDDSHOME} DOC "Root directory of RTI DDS")
if(NOT NDDS_HOME)
	set(ERR_MSG "${ERR_MSG}Failed to find root containing rev_host_rtidds.4.5d. ")
endif()

function(ndds_include_rtiddsgen_directories LANGUAGE)
	foreach(DIR ${ARGN})
		include_directories("${CMAKE_CURRENT_BINARY_DIR}/rtiddsgen_out/${LANGUAGE}/${DIR}")
	endforeach()
endfunction()

function(ndds_include_project_rtiddsgen_directories PROJECTNAME LANGUAGE)
	foreach(DIR ${ARGN})
		set(dir ${${PROJECTNAME}_BINARY_DIR}/rtiddsgen_out/${LANGUAGE}/${DIR})
		if(EXISTS ${dir})
			include_directories(${dir})
		endif()
	endforeach()
endfunction()

# rtiddsgen
find_program(NDDS_RTIDDSGEN rtiddsgen HINTS ${NDDS_HOME}/scripts DOC "Path to rtiddsgen utility")
mark_as_advanced(NDDS_RTIDDSGEN)
if(NDDS_RTIDDSGEN)
	function(ndds_run_rtiddsgen OUTPUT_SOURCES_VARNAME LANGUAGE)
		if(LANGUAGE STREQUAL "C++")
			set(EXTENSION ".cxx")
		elseif(LANGUAGE STREQUAL "C")
			set(EXTENSION ".c")
		else()
			message(SEND_ERROR "Unsupported language given to ndds_run_rtiddsgen: ${LANGUAGE}")
		endif()

		foreach(IDLFILE ${ARGN})
			set(OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/rtiddsgen_out/${LANGUAGE})
			file(MAKE_DIRECTORY ${OUTDIR})

			file(RELATIVE_PATH IDLFILE_REL ${CMAKE_CURRENT_SOURCE_DIR} ${IDLFILE})
			set(SOURCENAME ${OUTDIR}/${IDLFILE_REL})
			string(REPLACE .idl "" SOURCENAME ${SOURCENAME})
			set(SOURCEFILES "${SOURCENAME}${EXTENSION}" "${SOURCENAME}Plugin${EXTENSION}" "${SOURCENAME}Support${EXTENSION}")

			get_filename_component(SOURCEFILE_DIR ${SOURCENAME} PATH)
			make_directory(${SOURCEFILE_DIR})

			get_filename_component(IDLDIR ${IDLFILE} PATH)

			add_custom_command(OUTPUT ${SOURCEFILES} DEPENDS "${IDLFILE}" COMMAND ${NDDS_RTIDDSGEN} -verbosity 2 -optimization 1 -language ${LANGUAGE} -inputIDL ${IDLFILE} -replace -d "${SOURCEFILE_DIR}" -I "${IDLDIR}")

			set(OUTPUT_SOURCES ${OUTPUT_SOURCES} ${SOURCEFILES})
		endforeach()

		set(${OUTPUT_SOURCES_VARNAME} ${OUTPUT_SOURCES} PARENT_SCOPE)
	endfunction()

	function(ndds_run_rtiddsgen_c OUTPUT_SOURCES_VARNAME)
		foreach(IDLFILE ${ARGN})
			set(OUTDIR ${CMAKE_CURRENT_BINARY_DIR}/rtiddsgen_out_c)
			file(MAKE_DIRECTORY ${OUTDIR})

			file(RELATIVE_PATH IDLFILE_REL ${CMAKE_CURRENT_SOURCE_DIR} ${IDLFILE})
			set(SOURCENAME ${OUTDIR}/${IDLFILE_REL})
			string(REPLACE .idl "" SOURCENAME ${SOURCENAME})
			set(SOURCEFILES "${SOURCENAME}.c" "${SOURCENAME}Plugin.c" "${SOURCENAME}Support.c")

			get_filename_component(SOURCEFILE_DIR ${SOURCENAME} PATH)
			make_directory(${SOURCEFILE_DIR})

			get_filename_component(IDLDIR ${IDLFILE} PATH)

			add_custom_command(OUTPUT ${SOURCEFILES} DEPENDS "${IDLFILE}" COMMAND ${NDDS_RTIDDSGEN} -language C -inputIDL ${IDLFILE} -replace -d "${SOURCEFILE_DIR}" -I "${IDLDIR}")

			set(OUTPUT_SOURCES ${OUTPUT_SOURCES} ${SOURCEFILES})
		endforeach()

		set(${OUTPUT_SOURCES_VARNAME} ${OUTPUT_SOURCES} PARENT_SCOPE)
	endfunction()
else()
	set(ERR_MSG "${ERR_MSG}Failed to find rtiddsgen binary. ")
endif()

# Include directory
find_path(NDDS_INCLUDE_DIR ndds/ndds_cpp.h HINTS ${NDDS_HOME}/include DOC "Include directory for RTI DDS")
mark_as_advanced(NDDS_INCLUDE_DIR)
if(NDDS_INCLUDE_DIR)
	set(NDDS_INCLUDE_DIRS ${NDDS_INCLUDE_DIRS} ${NDDS_INCLUDE_DIR} ${NDDS_INCLUDE_DIR}/ndds)
else()
	set(ERR_MSG "${ERR_MSG}Failed to include directory with ndds/ndds_cpp.h. ")
endif()

# Libraries
set(NDDS_LIBDIR_HINT "${NDDS_HOME}/lib/${NDDS_ARCH}")
foreach(LIB nddscpp nddsc nddscore)
	find_library(NDDS_LIB_${LIB}_RELEASE ${LIB} HINTS ${NDDS_LIBDIR_HINT} DOC "Path to RTI DDS library ${LIB}")
	find_library(NDDS_LIB_${LIB}_DEBUG ${LIB}d HINTS ${NDDS_LIBDIR_HINT} DOC "Path to RTI DDS library ${LIB} built for debugging")

	if(NDDS_LIB_${LIB}_RELEASE AND NDDS_LIB_${LIB}_DEBUG)
		set(NDDS_LIBRARIES ${NDDS_LIBRARIES} optimized ${NDDS_LIB_${LIB}_RELEASE} debug ${NDDS_LIB_${LIB}_DEBUG})
	elseif(NDDS_LIB_${LIB}_RELEASE)
		set(NDDS_LIBRARIES ${NDDS_LIBRARIES} ${NDDS_LIB_${LIB}_RELEASE})
		message(WARNING "Missing debug version of NDDS library ${LIB}")
	elseif(NDDS_LIB_${LIB}_DEBUG)
		set(NDDS_LIBRARIES ${NDDS_LIBRARIES} ${NDDS_LIB_${LIB}_DEBUG})
		message(WARNING "Missing release version of NDDS library ${LIB}")
	else()
		set(ERR_MSG "${ERR_MSG}Failed to find library ${LIB}. ")
	endif()

	mark_as_advanced(NDDS_LIB_${LIB}_RELEASE NDDS_LIB_${LIB}_DEBUG)
endforeach()
set(NDDS_LIBRARIES ${NDDS_LIBRARIES} dl)

# Error/Status handling
if(ERR_MSG)
	set(NDDS_FOUND FALSE CACHE INTERNAL "Whether RTI DDS was found or not" FORCE)
else()
	if(NDDS_FOUND)
		return()
	else()
		set(NDDS_FOUND TRUE CACHE INTERNAL "Whether RTI DDS was found or not" FORCE)
	endif()
endif()

if(NOT NDDS_FIND_QUIETLY)
	if(NDDS_FOUND)
		message(STATUS "Found RTI DDS")
	else()
		if(NDDS_FIND_REQUIRED)
			message(FATAL_ERROR "Failed to find RTI DDS component(s). ${ERR_MSG}")
		else()
			message(WARNING "Failed to find RTI DDS component(s). ${ERR_MSG}")
		endif()
	endif()
endif()
