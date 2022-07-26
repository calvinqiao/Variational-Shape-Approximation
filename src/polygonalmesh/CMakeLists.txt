add_library(polygonalmesh 
	polygonalmesh.cpp
	polygonalmesh.h
	polygonalmeshio.cpp)
target_link_libraries(polygonalmesh ysclass)
target_include_directories(polygonalmesh PUBLIC .)
