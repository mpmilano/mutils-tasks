cmake_minimum_required (VERSION 3.1)
project (mutils-tasks)

#Versions
set(mutils_tasks_VERSION 1.0)

#CXX FLAGS
set(CMAKE_CXX_FLAGS "-std=c++14 -fPIC -DMAX_THREADS=10000")
set(CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS})
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++14 -fPIC -DMAX_THREADS=10000 -g")
set(CMAKE_SHARED_LINKER_FLAGS "-shared --enable-new-dtags")

#Check dependencies
find_package(mutils 1.0)
if(NOT ${mutils_FOUND})
  message( FATAL_ERROR "mutils not found.")
endif(NOT ${mutils_FOUND})

add_library(mutils-tasks SHARED FutureFreePool.cpp GlobalPool.cpp)
target_include_directories(mutils-tasks PRIVATE
  $<BUILD_INTERFACE:${mutils-tasks_SOURCE_DIR}/include>)

#Make install
install(TARGETS mutils-tasks EXPORT mutils-tasks
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/mutils-tasks
        DESTINATION include)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/mutils-tasks/mutils-tasksConfigVersion.cmake"
  VERSION ${mutils_tasks_VERSION}
  COMPATIBILITY AnyNewerVersion
)

export (EXPORT mutils-tasks
  FILE "${CMAKE_CURRENT_BINARY_DIR}/mutils-tasks/mutils-tasksTargets.cmake"
)

configure_file (mutils-tasksConfig.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/mutils-tasks/mutils-tasksConfig.cmake"
  COPYONLY
)

set(ConfigPackageLocation lib/cmake/mutils-tasks)
install(EXPORT mutils-tasks
  FILE mutils-tasksTargets.cmake
  DESTINATION ${ConfigPackageLocation}
)

install(
  FILES
    mutils-tasksConfig.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/mutils-tasks/mutils-tasksConfigVersion.cmake"
  DESTINATION ${ConfigPackageLocation}
)
