PROJECT(AxiSampling)
cmake_minimum_required(VERSION 3.1)

set (CMAKE_CXX_STANDARD 11)

set(EXAMPLES
  localDensity
)

foreach(EXAMPLE ${EXAMPLES})
    add_executable(${EXAMPLE} ${EXAMPLE}.cpp)
endforeach()
