
SET(SRCS 
                qfplib-m3.s
        )
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_library(qfpm3           ${SRCS}      )
