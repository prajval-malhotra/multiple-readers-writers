########## Variables ##########

CXX = gcc					# compiler
CXXFLAGS = -O3 -Wall -MMD			# compiler flags
MAKEFILE_NAME = ${firstword ${MAKEFILE_LIST}}	

OBJECTS = answer.o			
DEPENDS = ${OBJECTS:.o=.d}	
EXEC = run					

########## Targets ##########

.PHONY : clean					

${EXEC} : ${OBJECTS}
	${CXX} ${CXXFLAGS} $^ -o $@		# additional object files before $^

${OBJECTS} : ${MAKEFILE_NAME}

-include ${DEPENDS}

clean :						
	rm -f ${DEPENDS} ${OBJECTS} ${EXEC}