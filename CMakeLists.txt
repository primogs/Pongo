project(Pongo)

cmake_minimum_required(VERSION 3.0)

set(INSTALL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
add_executable(${PROJECT_NAME} 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/ClientHandler.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/HttpHeaderRequest.cpp"  
	"${CMAKE_CURRENT_SOURCE_DIR}/src/PageImpressum.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/ElmtBase.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/HttpHeaderResponse.cpp" 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/PageRoot.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/ElmtCanvas.cpp"     
	"${CMAKE_CURRENT_SOURCE_DIR}/src/HttpVariables.cpp"         
	"${CMAKE_CURRENT_SOURCE_DIR}/src/PageSecond.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/ElmtPicture.cpp"           
	"${CMAKE_CURRENT_SOURCE_DIR}/src/ResourceManager.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/HtmlPage.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/NetworkManagement.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/UniformResourceIdentifier.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/HtmlWebsite.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/NetworkManagementSsl.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/NetworkManagerBase.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp" )

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_link_libraries(${PROJECT_NAME} PUBLIC pthread mariadb tls)
