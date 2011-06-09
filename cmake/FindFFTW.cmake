find_path(FFTW_INCLUDES
  NAMES
  fftw3.h
  PATHS
  $ENV{FFTWDIR}
  ${INCLUDE_INSTALL_DIR}
)

find_library(FFTWF_LIB NAMES fftw3f PATHS $ENV{FFTWDIR} ${LIB_INSTALL_DIR})
find_library(FFTW_LIB NAMES fftw3 PATHS $ENV{FFTWDIR} ${LIB_INSTALL_DIR})
set(FFTW_LIBRARIES ${FFTWF_LIB} ${FFTW_LIB})

find_library(FFTWL_LIB NAMES fftw3l  PATHS $ENV{FFTWDIR} ${LIB_INSTALL_DIR})

if(FFTWL_LIB)
set(FFTW_LIBRARIES ${FFTW_LIBRARIES} ${FFTWL_LIB})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFTW DEFAULT_MSG
                                  FFTW_INCLUDES FFTW_LIBRARIES)

mark_as_advanced(FFTW_INCLUDES FFTWF_LIB FFTW_LIB FFTWL_LIB)
