include ../makepath

C = g++
INC = -I. -I../srv_net -I$(INC_QT) $(_I_INC_QT) -I$(INC_X11)
LIB_PATH = -L$(LIB_QT) -L../srv_net
CFLAGS = -g -w -Wall -fopenmp -pthread -DQT_THREAD_SUPPORT $(INC) $(COMMON_COMPILER_OPTIONS)
LIB = -lgcc -lsrv_net -lQtCore$(SUFFIX_QT) -lQtNetwork$(SUFFIX_QT)
NAME = ../fsrv$(DOT_EXE)


OBJ = settings.o messages.o \
clntsock.o clntsock.moc.o worker.o worker.moc.o \
modmng.o srv.o srv.moc.o main.o

.SUFFIXES: .cpp .o

.cpp.o:
	$(C) $(CFLAGS) -c -o $@ $<

all: $(NAME)

$(NAME): $(OBJ) 
	$(C) $(CFLAGS) $(LIB_PATH) -o $(NAME) $(OBJ) $(LIB) $(SYS_LIB)

clean:
	 rm -rf $(OBJ) *.moc.cpp $(NAME)

clntsock.moc.cpp: clntsock.h
	$(MOC) clntsock.h -o clntsock.moc.cpp

worker.moc.cpp: worker.h
	$(MOC) worker.h -o worker.moc.cpp

srv.moc.cpp: srv.h
	$(MOC) srv.h -o srv.moc.cpp
