cmake_minimum_required(VERSION 3.0.2)

project(yacwm)

include(FindX11)
if (X11_FOUND)
  include_directories(${X11_INCLUDE_DIR} ${X11_XComposite_INCLUDE_PATH})
else()
  message("X11 not found!")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

set(SOURCES
  main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
  ${X11_LIBRARIES} ${X11_Xcomposite_LIB} ${X11_Xrender_LIB} ${X11_Xdamage_LIB} Threads::Threads
)
