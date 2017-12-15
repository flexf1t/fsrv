include ../makepath

C = g++
INC = -I. -I$(INC_QT) $(_I_INC_QT)
CFLAGS = $(m) -g -w -Wall -nodefaultlibs $(INC) -DQT_THREAD_SUPPORT $(COMMON_COMPILER_OPTIONS)
LIB = libsrv_net.a

OBJ = dirmng.o sockethandlers.o

.SUFFIXES: .cpp .o

.cpp.o:
	$(C) $(CFLAGS) -c -fPIC -o $@ $<

all: $(OBJ)
	ar rcs $(LIB) $(OBJ)

clean:
	rm -rf $(OBJ) $(LIB)
	
# EOF