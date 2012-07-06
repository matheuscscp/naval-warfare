project(naval-warfare)

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