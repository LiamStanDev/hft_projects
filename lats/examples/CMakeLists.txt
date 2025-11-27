find_package(Boost REQUIRED)

add_executable(boost_intrusive_demo
    boost_intrusive_demo.cpp
)

target_link_libraries(boost_intrusive_demo
    PRIVATE
    boost::boost
)
