# CMakeLists.txt for event package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 2.8.10)
project(AnitaTreeMaker)
set(libname "AnitaTreeMaker")


# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})


if(DEFINED ENV{ANITA_UTIL_INSTALL_DIR})
    message("ANITA_UTIL_INSTALL_DIR is set to $ENV{ANITA_UTIL_INSTALL_DIR}")
    set(UTIL_LIB_DIR $ENV{ANITA_UTIL_INSTALL_DIR}/lib)
    set(UTIL_INC_DIR $ENV{ANITA_UTIL_INSTALL_DIR}/include)
    set(UTIL_BIN_DIR $ENV{ANITA_UTIL_INSTALL_DIR}/bin)
    set(UTIL_SHARE_DIR $ENV{ANITA_UTIL_INSTALL_DIR}/share)
    set(LD_UTIL $ENV{ANITA_UTIL_INSTALL_DIR}/lib)
    set(INC_UTIL $ENV{ANITA_UTIL_INSTALL_DIR}/include)
elseif(DEFINED ENV{ARA_UTIL_INSTALL_DIR})
    message("ARA_UTIL_INSTALL_DIR is set to $ENV{ANITA_UTIL_INSTALL_DIR}")
    set(UTIL_LIB_DIR $ENV{ARA_UTIL_INSTALL_DIR}/lib)
    set(UTIL_INC_DIR $ENV{ARA_UTIL_INSTALL_DIR}/include)
    set(UTIL_BIN_DIR $ENV{ARA_UTIL_INSTALL_DIR}/bin)
    set(UTIL_SHARE_DIR $ENV{ARA_UTIL_INSTALL_DIR}/share)
    set(LD_UTIL $ENV{ARA_UTIL_INSTALL_DIR}/lib)
    set(INC_UTIL $ENV{ARA_UTIL_INSTALL_DIR}/include)
else()
    message("Will install to /usr/local  ANITA_UTIL_INSTALL_DIR not set")
    set(UTIL_LIB_DIR /usr/local/lib)
    set(UTIL_INC_DIR /usr/local/include)
    set(UTIL_BIN_DIR /usr/local/bin)
    set(UTIL_SHARE_DIR /usr/local/share)
    set(LD_UTIL /usr/local/lib)
    set(INC_UTIL /usr/local/include)
endif()



set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${UTIL_SHARE_DIR}/cmake/modules/")


#---Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS MathMore MathCore RIO Hist Tree Net Minuit)
find_package(FFTW REQUIRED)
find_package(ZLIB REQUIRED)


if(TARGET RootFftwWrapper)
  message("RootFftwWrapper is a target")
else()
 link_directories(${UTIL_LIB_DIR})
endif()

if(TARGET AnitaEvent)
  message("AnitaEvent is a target")
else()
 link_directories(${UTIL_LIB_DIR})
endif()
  

#---Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
message("ROOT_VERSION is set to ${ROOT_VERSION}")
if( ${ROOT_VERSION} VERSION_GREATER "5.99/99")
    message("Using ROOT_VERSION 6")
    include(${ROOT_USE_FILE})
else()
#    include(RootNewMacros)
     add_definitions(${ROOT_DEFINITIONS})	
endif()

message("ROOT_INCLUDE_DIRS is set to ${ROOT_INCLUDE_DIRS}")


include_directories(${PROJECT_SOURCE_DIR} ${ROOT_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/include ${FFTW_INCLUDES} ${ZLIB_INCLUDES} ${UTIL_INC_DIR})
add_definitions(${ROOT_CXX_FLAGS})

message("UTIL_INC_DIR is set to ${UTIL_INC_DIR}")
message("DICTNAME is set to ${DICTNAME}" )
message("PROJECT_BINARY_DIR is set to ${PROJECT_BINARY_DIR}")

foreach(loop_prog checkForCorruptEvents            makeGpsEventTreeBothAdu5s
    makeSSHkTree fixTriggerTimeAnita3             makeGpsTree
    makeSlowRateTree makeAdu5PatFiles                 makeGpsTttTree
    makeSummedTurfRateTree makeAuxiliaryTree                makeGpuTree
    makeSuperSmoothAdu5PatFiles makeAveragedSurfHkTree           makeHkTree
    makeSurfHkTree makeCalibratedEventTrees         makeMonitorTree
    makeTelemHeadTree makeConcatanatedFiles            makeOtherTree
    makeTurfRateTree makeEventHeadTree                makePrettyHkTree
    quickCheckForErrors makeEventRunTree                 makeRawHeadTree
    updateAdu5PatTreesFromTelemetry makeFixedEventHeaderTrees
    makeRawScalerTree                updateEventRunTree makeGpsEventTree
    makeRunSummaryFile makeTuffStatusTree makeRtlSdrTree
    fixTelemHeadEventTrees makeA4TimedHeaderFile
)
add_executable (${loop_prog} ${loop_prog}.cxx)
target_link_libraries (${loop_prog} LINK_PUBLIC AnitaEvent RootFftwWrapper ${ROOT_LIBRARIES} ${ZLIB_LIBRARIES} )
install (TARGETS ${loop_prog} DESTINATION ${UTIL_BIN_DIR})
endforeach(loop_prog )

foreach(script remakeHkFiles.sh runTelemEventMaker.sh runTelemMonitorMaker.sh
    runTelemSurfHkMaker.sh runAdu5PatSmoother.sh runTelemGpsMaker.sh
    runTelemOtherMonitorMaker.sh runTelemTurfRateMaker.sh
    runAnita4FileMakerTelem.sh   runTelemGpuMaker.sh
    runTelemRawScalerMaker.sh runTelemAuxMaker.sh
    runTelemHeaderMaker.sh       runTelemSlowMaker.sh
    runTelemAvgSurfHkMaker.sh    runTelemHkMaker.sh
    runTelemSumTurfRateMaker.sh  runTelemTuffStatusMaker.sh
    anita4FlightFileMakerUChicago.sh ANITA4rootifier.sh 
    processData.sh processDataChecker.sh processTestRun.sh rootFileChecker.sh rawFolderChecker.sh) 
  install(PROGRAMS ${script} DESTINATION ${UTIL_BIN_DIR})
endforeach(script)


#################################################################################3
### The default build configuration is INSANE. No optimization? What is this 1971? 

message (" Adding new build type") 

set(CMAKE_CXX_FLAGS_DEFAULT 
  "-Os -g -Wall" 
  CACHE STRING "c++ Flags used during default eventReaderRoot builds" 
  FORCE ) 

set(CMAKE_C_FLAGS_DEFAULT 
  "-Os -g -Wall"
  CACHE STRING "c Flags used during default eventReaderRoot builds" 
  FORCE ) 

set(CMAKE_EXE_LINKER_FLAGS_DEFAULT 
  "-g"
  CACHE STRING "ld Flags used during default eventReaderRoot builds" 
  FORCE ) 

set(CMAKE_SHARED_LINKER_FLAGS_DEFAULT 
  "-g"
  CACHE STRING "ld Flags used during default eventReaderRoot builds" 
  FORCE ) 


mark_as_advanced ( CMAKE_CXX_FLAGS_DEFAULT CMAKE_C_FLAGS_DEFAULT CMAKE_EXE_LINKER_FLAGS_DEFAULT CMAKE_SHARED_LINKER_FLAGS_DEFAULT) 

if (NOT CMAKE_BUILD_TYPE) 
  set (CMAKE_BUILD_TYPE Default
    CACHE STRING "Choose tye type of build: None Debug Release RelWithDebInfo MinSizeRel Default"
    FORCE ) 
endif()


add_definitions("-DUSE_FFT_TOOLS") 

option ( FORCE_OLD_GPP_ABI " Force old g++ ABI; this might be necessary if using new g++ with ROOT compiled with older g++ or other similar situations" OFF ) 
if (FORCE_OLD_GPP_ABI) 
  add_definitions( -D_GLIBCXX_USE_CXX11_ABI=0 ) 
endif() 
 
