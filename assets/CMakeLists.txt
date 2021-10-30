ADD_LIBRARY(assets 
            fonts.cpp 
            gfx.cpp
)
target_include_directories(assets PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}  )
target_include_directories(assets PRIVATE   ${CMAKE_CURRENT_SOURCE_DIR}/../externals/simplerILI9341  )
