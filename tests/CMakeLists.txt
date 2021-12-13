
add_executable(test_server httpd.c)

add_executable(test_client simpleclient.c)

target_link_libraries(test_server ${LINK_LIB_LIST} 
	)

target_link_libraries(test_client ${LINK_LIB_LIST} 
	)



