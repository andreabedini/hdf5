#-------------------------------------------------------------------------------
MACRO (SET_GLOBAL_VARIABLE name value)
  SET (${name} ${value} CACHE INTERNAL "Used to pass variables between directories" FORCE)
ENDMACRO (SET_GLOBAL_VARIABLE)

#-------------------------------------------------------------------------------
MACRO (IDE_GENERATED_PROPERTIES SOURCE_PATH HEADERS SOURCES)
  #set(source_group_path "Source/AIM/${NAME}")
  STRING (REPLACE "/" "\\\\" source_group_path ${SOURCE_PATH})
  source_group(${source_group_path} FILES ${HEADERS} ${SOURCES})

  #-- The following is needed if we ever start to use OS X Frameworks but only
  #--  works on CMake 2.6 and greater
  #SET_PROPERTY (SOURCE ${HEADERS}
  #       PROPERTY MACOSX_PACKAGE_LOCATION Headers/${NAME}
  #)
ENDMACRO (IDE_GENERATED_PROPERTIES)

#-------------------------------------------------------------------------------
MACRO (IDE_SOURCE_PROPERTIES SOURCE_PATH HEADERS SOURCES)
  #  INSTALL (FILES ${HEADERS}
  #       DESTINATION include/R3D/${NAME}
  #       COMPONENT Headers       
  #  )

  STRING (REPLACE "/" "\\\\" source_group_path ${SOURCE_PATH}  )
  source_group (${source_group_path} FILES ${HEADERS} ${SOURCES})

  #-- The following is needed if we ever start to use OS X Frameworks but only
  #--  works on CMake 2.6 and greater
  #SET_PROPERTY (SOURCE ${HEADERS}
  #       PROPERTY MACOSX_PACKAGE_LOCATION Headers/${NAME}
  #)
ENDMACRO (IDE_SOURCE_PROPERTIES)

#-------------------------------------------------------------------------------
MACRO (H5_NAMING target libtype)
  IF (WIN32 AND NOT MINGW)
    IF (${libtype} MATCHES "SHARED")
      IF (H5_LEGACY_NAMING)
        SET_TARGET_PROPERTIES (${target} PROPERTIES OUTPUT_NAME "dll")
        SET_TARGET_PROPERTIES (${target} PROPERTIES PREFIX "${target}")
      ELSE (H5_LEGACY_NAMING)
        SET_TARGET_PROPERTIES (${target} PROPERTIES OUTPUT_NAME "${target}dll")
      ENDIF (H5_LEGACY_NAMING)
    ENDIF (${libtype} MATCHES "SHARED")
  ENDIF (WIN32 AND NOT MINGW)
ENDMACRO (H5_NAMING)

#-------------------------------------------------------------------------------
MACRO (H5_SET_LIB_OPTIONS libtarget libname libtype)
  # message (STATUS "${libname} libtype: ${libtype}")
  IF (${libtype} MATCHES "SHARED")
    IF (WIN32 AND NOT MINGW)
      IF (H5_LEGACY_NAMING)
        SET (LIB_RELEASE_NAME "${libname}dll")
        SET (LIB_DEBUG_NAME "${libname}ddll")
      ELSE (H5_LEGACY_NAMING)
        SET (LIB_RELEASE_NAME "${libname}")
        SET (LIB_DEBUG_NAME "${libname}_D")
      ENDIF (H5_LEGACY_NAMING)
    ELSE (WIN32 AND NOT MINGW)
      SET (LIB_RELEASE_NAME "${libname}")
      SET (LIB_DEBUG_NAME "${libname}_debug")
    ENDIF (WIN32 AND NOT MINGW)
  ELSE (${libtype} MATCHES "SHARED")
    IF (WIN32 AND NOT MINGW)
      IF (H5_LEGACY_NAMING)
        SET (LIB_RELEASE_NAME "${libname}")
        SET (LIB_DEBUG_NAME "${libname}d")
      ELSE (H5_LEGACY_NAMING)
        SET (LIB_RELEASE_NAME "lib${libname}")
        SET (LIB_DEBUG_NAME "lib${libname}_D")
      ENDIF (H5_LEGACY_NAMING)
    ELSE (WIN32 AND NOT MINGW)
      # if the generator supports configuration types or if the CMAKE_BUILD_TYPE has a value
      IF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        SET (LIB_RELEASE_NAME "${libname}")
        SET (LIB_DEBUG_NAME "${libname}_debug")
      ELSE (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
        SET (LIB_RELEASE_NAME "lib${libname}")
        SET (LIB_DEBUG_NAME "lib${libname}_debug")
      ENDIF (CMAKE_CONFIGURATION_TYPES OR CMAKE_BUILD_TYPE)
    ENDIF (WIN32 AND NOT MINGW)
  ENDIF (${libtype} MATCHES "SHARED")
  
  SET_TARGET_PROPERTIES (${libtarget}
      PROPERTIES
      DEBUG_OUTPUT_NAME          ${LIB_DEBUG_NAME}
      RELEASE_OUTPUT_NAME        ${LIB_RELEASE_NAME}
      MINSIZEREL_OUTPUT_NAME     ${LIB_RELEASE_NAME}
      RELWITHDEBINFO_OUTPUT_NAME ${LIB_RELEASE_NAME}
  )
  
  #----- Use MSVC Naming conventions for Shared Libraries
  IF (MINGW AND ${libtype} MATCHES "SHARED")
    SET_TARGET_PROPERTIES (${libtarget}
        PROPERTIES
        IMPORT_SUFFIX ".lib"
        IMPORT_PREFIX ""
        PREFIX ""
    )
  ENDIF (MINGW AND ${libtype} MATCHES "SHARED")

  IF (${libtype} MATCHES "SHARED")
    IF (WIN32)
      SET (LIBHDF_VERSION ${HDF5_PACKAGE_VERSION_MAJOR})
    ELSE (WIN32)
      SET (LIBHDF_VERSION ${HDF5_PACKAGE_VERSION})
    ENDIF (WIN32)
    SET_TARGET_PROPERTIES (${libtarget} PROPERTIES VERSION ${LIBHDF_VERSION})
    SET_TARGET_PROPERTIES (${libtarget} PROPERTIES SOVERSION ${LIBHDF_VERSION})
  ENDIF (${libtype} MATCHES "SHARED")

  #-- Apple Specific install_name for libraries
  IF (APPLE)
    OPTION (HDF5_BUILD_WITH_INSTALL_NAME "Build with library install_name set to the installation path" OFF)
    IF (HDF5_BUILD_WITH_INSTALL_NAME)
      SET_TARGET_PROPERTIES(${libtarget} PROPERTIES
          LINK_FLAGS "-current_version ${HDF5_PACKAGE_VERSION} -compatibility_version ${HDF5_PACKAGE_VERSION}"
          INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
          BUILD_WITH_INSTALL_RPATH ${HDF5_BUILD_WITH_INSTALL_NAME}
      )
    ENDIF (HDF5_BUILD_WITH_INSTALL_NAME)
  ENDIF (APPLE)

ENDMACRO (H5_SET_LIB_OPTIONS)

#-------------------------------------------------------------------------------
MACRO (TARGET_FORTRAN_WIN_PROPERTIES target)
  IF (WIN32)
    IF (MSVC)
      SET_TARGET_PROPERTIES (${target}
          PROPERTIES
              COMPILE_FLAGS "/dll"
              LINK_FLAGS "/SUBSYSTEM:CONSOLE"
      ) 
    ENDIF (MSVC)
  ENDIF (WIN32)
ENDMACRO (TARGET_FORTRAN_WIN_PROPERTIES)

