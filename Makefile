#--------------------------
# Variables
#--------------------------
#
MAKE         = make -s
CC           = gcc
SOURCE_PATH  = ./
OBJECTS      = factory.c

INCLUDE_BASE = ./include/
LIB_BASE     = ./lib/

MYFLAGS      = -Wall -g
MYFLAGSBD	 =

MYHEADERS    =  -I${INCLUDE_BASE}
MYLIBS       =  -L${LIB_BASE} -ldb_factory -lpthread
MYLIBS32     =  -L${LIB_BASE} -ldb_factory32 -lpthread


#--------------------------
# Compilation rules
#--------------------------
#
#
all:  $(OBJECTS)
	@echo "                              Compiling "
	@echo -n "                              "
	@echo -n "Concurent example [.."
	@$(CC) factory.c $(MYFLAGS) $(MYHEADERS) $(MYLIBS) -o factory.exe
	@echo "..]"
	@echo "                              Example compiled successfully!"
	@echo ""
	
32bit: $(OBJECTS)
	@echo "                              Compiling "
	@echo -n "                              "
	@echo -n "Concurent example [.."
	@$(CC) factory.c $(MYFLAGS) $(MYHEADERS) $(MYLIBS32) -o factory.exe
	@echo "..]"
	@echo "                              Example compiled successfully!"
	@echo ""

clean:
	@echo "                              Deleted files!"
	@echo -n "                              "
	@rm -f *.o *exe
	@echo  ""
