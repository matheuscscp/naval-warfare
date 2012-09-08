project(naval-warfare)

if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif (NOT CMAKE_BUILD_TYPE)

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")
	add_definitions(-DLOGTRACE)
endif(${CMAKE_BUILD_TYPE} STREQUAL "Debug" OR ${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo")

# add a new library called helloperson that will be created using
# helloperson.cc
add_library(barco MODULE barco.cc)


# say that the target helloperson will be installed inside
# a folder called "greeter"
install(TARGETS barco
		LIBRARY
		DESTINATION unit)
		
		# add a new library called helloperson that will be created using
# helloperson.cc
add_library(porto MODULE porto.cc)


# say that the target helloperson will be installed inside
# a folder called "greeter"
install(TARGETS porto
		LIBRARY
		DESTINATION porto)


add_library(loot MODULE loot.cc)		
		
install(TARGETS loot
		LIBRARY
		DESTINATION unit)
		

add_library(painel MODULE painel.cc)		
		
install(TARGETS painel
		LIBRARY
		DESTINATION menu)


add_library(partida MODULE partida.cc)		
		
install(TARGETS partida
		LIBRARY
		DESTINATION gamestate)


add_library(mainmenu MODULE mainmenu.cc)
		
install(TARGETS mainmenu
		LIBRARY
		DESTINATION gamestate)


add_library(geraIlha MODULE geraIlha.cc)
		
install(TARGETS geraIlha
		LIBRARY
		DESTINATION geraIlha)


add_library(paused MODULE paused.cc)

install(TARGETS paused
		LIBRARY
		DESTINATION pause)


add_library(singleselectmanager MODULE singleselectmanager.cc)
		
install(TARGETS singleselectmanager
		LIBRARY
		DESTINATION menumanager)


add_library(fade MODULE fade.cc)
		
install(TARGETS fade
		LIBRARY
		DESTINATION animation)
