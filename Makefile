CC = clang 
CFLAGS = -std=c99 -Werror -Wno-unused-label -Wno-unused-parameter -Wno-cpp -Wall
OBJ_FILES = edrmacos.o
DYN_LYB = -lbsm -lEndpointSecurity
FRAMEWORKS = -framework Foundation

all: edr codesign

edr: $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o edrmacos $(FRAMEWORKS) $(DYN_LYB)

edrmacos.o: edrmacos.c
	$(CC) $(CFLAGS) -c edrmacos.c

codesign:
	codesign --sign - \
    	--entitlements $(shell pwd)/reformatted.entitlements \
    	--deep $(shell pwd)/edrmacos \
    	--force

clean:
	rm *.o
