add_library(proxyutil 
	proxy.cpp
	proxy.h
	proxymesh.cpp
	proxymesh.h)
target_link_libraries(proxyutil polygonalmesh ysclass)
target_include_directories(proxyutil PUBLIC .)
