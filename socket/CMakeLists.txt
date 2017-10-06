cmake_minimum_required(VERSION 3.8)
project(socket)

set(CMAKE_CXX_STANDARD 11)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

set(SOURCE_FILES main.cpp)
add_executable(socket ${SOURCE_FILES})

find_package(Boost 1.58.0 REQUIRED system)
find_package(OpenCV REQUIRED)

include_directories(socket ${Boost_INCLUDE_DIRS})

link_directories(socket ${Boost_LIBRARY_DIRS})

target_link_libraries(socket ${OpenCV_LIBS})

target_link_libraries(${PROJECT_NAME}
        /home/toams/lib/boost/lib/libboost_chrono.so.1.64.0
        /home/toams/lib/boost/lib/libboost_math_c99f.so
        /home/toams/lib/boost/lib/libboost_math_c99.so
        /home/toams/lib/boost/lib/libboost_log.so.1.64.0
        /home/toams/lib/boost/lib/libboost_regex.so
        /home/toams/lib/boost/lib/libboost_prg_exec_monitor.so
        /home/toams/lib/boost/lib/libboost_wave.so.1.64.0
        /home/toams/lib/boost/lib/libboost_numpy.so
        /home/toams/lib/boost/lib/libboost_math_c99f.so.1.64.0
        /home/toams/lib/boost/lib/libboost_coroutine.so
        /home/toams/lib/boost/lib/libboost_python.so
        /home/toams/lib/boost/lib/libboost_numpy.so.1.64.0
        /home/toams/lib/boost/lib/libboost_chrono.so
        /home/toams/lib/boost/lib/libboost_math_tr1l.so.1.64.0
        /home/toams/lib/boost/lib/libboost_math_tr1l.so
        /home/toams/lib/boost/lib/libboost_random.so
        /home/toams/lib/boost/lib/libboost_math_c99l.so.1.64.0
        /home/toams/lib/boost/lib/libboost_context.so.1.64.0
        /home/toams/lib/boost/lib/libboost_serialization.so.1.64.0
        /home/toams/lib/boost/lib/libboost_thread.so
        /home/toams/lib/boost/lib/libboost_program_options.so
        /home/toams/lib/boost/lib/libboost_wave.so
        /home/toams/lib/boost/lib/libboost_type_erasure.so.1.64.0
        /home/toams/lib/boost/lib/libboost_math_tr1.so.1.64.0
        /home/toams/lib/boost/lib/libboost_log_setup.so
        /home/toams/lib/boost/lib/libboost_locale.so.1.64.0
        /home/toams/lib/boost/lib/libboost_atomic.so.1.64.0
        /home/toams/lib/boost/lib/libboost_python.so.1.64.0
        /home/toams/lib/boost/lib/libboost_timer.so.1.64.0
        /home/toams/lib/boost/lib/libboost_program_options.so.1.64.0
        /home/toams/lib/boost/lib/libboost_signals.so.1.64.0
        /home/toams/lib/boost/lib/libboost_graph.so
        /home/toams/lib/boost/lib/libboost_graph.so.1.64.0
        /home/toams/lib/boost/lib/libboost_wserialization.so.1.64.0
        /home/toams/lib/boost/lib/libboost_date_time.so.1.64.0
        /home/toams/lib/boost/lib/libboost_signals.so
        /home/toams/lib/boost/lib/libboost_prg_exec_monitor.so.1.64.0
        /home/toams/lib/boost/lib/libboost_container.so
        /home/toams/lib/boost/lib/libboost_system.so
        /home/toams/lib/boost/lib/libboost_type_erasure.so
        /home/toams/lib/boost/lib/libboost_math_c99l.so
        /home/toams/lib/boost/lib/libboost_context.so
        /home/toams/lib/boost/lib/libboost_math_tr1.so
        /home/toams/lib/boost/lib/libboost_unit_test_framework.so.1.64.0
        /home/toams/lib/boost/lib/libboost_filesystem.so
        /home/toams/lib/boost/lib/libboost_filesystem.so.1.64.0
        /home/toams/lib/boost/lib/libboost_atomic.so
        /home/toams/lib/boost/lib/libboost_math_tr1f.so
        /home/toams/lib/boost/lib/libboost_thread.so.1.64.0
        /home/toams/lib/boost/lib/libboost_system.so.1.64.0
        /home/toams/lib/boost/lib/libboost_random.so.1.64.0
        /home/toams/lib/boost/lib/libboost_timer.so
        /home/toams/lib/boost/lib/libboost_math_tr1f.so.1.64.0
        /home/toams/lib/boost/lib/libboost_date_time.so
        /home/toams/lib/boost/lib/libboost_log.so
        /home/toams/lib/boost/lib/libboost_wserialization.so
        /home/toams/lib/boost/lib/libboost_iostreams.so.1.64.0
        /home/toams/lib/boost/lib/libboost_iostreams.so
        /home/toams/lib/boost/lib/libboost_serialization.so
        /home/toams/lib/boost/lib/libboost_math_c99.so.1.64.0
        /home/toams/lib/boost/lib/libboost_coroutine.so.1.64.0
        /home/toams/lib/boost/lib/libboost_unit_test_framework.so
        /home/toams/lib/boost/lib/libboost_locale.so
        /home/toams/lib/boost/lib/libboost_regex.so.1.64.0
        /home/toams/lib/boost/lib/libboost_log_setup.so.1.64.0
        /home/toams/lib/boost/lib/libboost_container.so.1.64.0
        /lib/x86_64-linux-gnu/libpthread.so.0
        /lib/x86_64-linux-gnu/libpthread-2.23.so
        )