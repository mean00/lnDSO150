

IF(USE_SMALLFOOTPRINT)
        ADD_DEFINITIONS("-DUSE_SMALLFOOTPRINT")
ENDIF()
ADD_LIBRARY(assets STATIC
            fonts.cpp 
            gfx.cpp
)
target_include_directories(assets PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}  )
target_include_directories(assets PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../externals/simplerILI9341  )
target_link_libraries( assets lnArduino )
