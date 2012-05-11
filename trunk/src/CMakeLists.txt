project(naval-warfare)

# add a new library called helloperson that will be created using
# helloperson.cc
add_library(helloperson MODULE helloperson.cc)


# say that the target helloperson will be installed inside
# a folder called "greeter"
install(TARGETS helloperson
		LIBRARY
		DESTINATION greeter)