# - Try to find ZeroMQ
# Once done this will define
# ZMQ_FOUND - System has ZMQ
# ZMQ_INCLUDE_DIRS - The ZMQ include directories
# ZMQ_LIBRARIES - The libraries needed to use ZMQ

find_path (ZMQ_INCLUDE_DIR
      NAMES zmq.hpp
      )
      
find_library (ZMQ_LIBRARY
      NAMES zmq
      )
      
if((NOT ZMQ_INCLUDE_DIR) OR (NOT ZMQ_LIBRARY))
   ## load in pkg-config support
   find_package(PkgConfig QUIET)
   
   if(PkgConfig_FOUND)
      ## use pkg-config to get hints for 0mq locations
      pkg_check_modules(PC_ZeroMQ QUIET libzmq)

      ## use the hint from above to find where 'zmq.hpp' is located
      find_path(ZMQ_INCLUDE_DIR
        NAMES zmq.hpp
        PATHS ${PC_ZeroMQ_INCLUDE_DIRS}
        )

      ## use the hint from about to find the location of libzmq
      find_library(ZMQ_LIBRARY
        NAMES zmq
        PATHS ${PC_ZeroMQ_LIBRARY_DIRS}
        )
   endif(PkgConfig_FOUND)
   
endif((NOT ZMQ_INCLUDE_DIR) OR (NOT ZMQ_LIBRARY))

set ( ZMQ_LIBRARIES ${ZMQ_LIBRARY} )
set ( ZMQ_INCLUDE_DIRS ${ZMQ_INCLUDE_DIR} )

include ( FindPackageHandleStandardArgs )
# handle the QUIETLY and REQUIRED arguments and set ZMQ_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args ( ZMQ DEFAULT_MSG ZMQ_LIBRARY ZMQ_INCLUDE_DIR )
