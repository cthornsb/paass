if(USE_ROOT AND BUILD_SCOPE)
	add_executable(scope scope.cpp RootScanner.cpp)
	target_link_libraries(scope ScanStatic ${ROOT_LIBRARIES})

	root_generate_dictionary(HistScannerDictionary HistScannerChanData.hpp LINKDEF HistScannerLinkDef.h)
	add_executable(hist RootScanner.cpp HistScanner.cpp HistUnpacker.cpp HistScannerDictionary.cxx hist.cpp)
	target_link_libraries(hist ScanStatic ${ROOT_LIBRARIES})

	install(TARGETS scope hist DESTINATION bin)
endif(USE_ROOT AND BUILD_SCOPE)

if(BUILD_SKELETON)
    add_executable(skeleton Skeleton.cpp)
    target_link_libraries(skeleton ScanStatic)
    install (TARGETS skeleton DESTINATION bin)
endif(BUILD_SKELETON)

# Install headReader executable.
add_executable(headReader headReader.cpp)
target_link_libraries(headReader ScanStatic)
install (TARGETS headReader DESTINATION bin)
