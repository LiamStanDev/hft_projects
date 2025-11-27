add_library(lats_core
    STATIC
    timer.cpp
    latency_stats.cpp
)

target_link_libraries(lats_core
    PUBLIC Boost::boost
)
