
include_directories(ui)


include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
ADD_LIBRARY(  nanopb
                pb_common.c
                pb_decode.c
                pb_encode.c
                )
