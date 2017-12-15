include ../makepath

C = g++
INC = -I. -I$(INC_QT) $(_I_INC_QT) -I$(INC_X11)
LIB_PATH = -L$(LIB_QT)
CFLAGS = -g -w -Wall -fopenmp -pthread -DQT_THREAD_SUPPORT $(INC) $(COMMON_COMPILER_OPTIONS)
LIB = -lgcc -lQtCore$(SUFFIX_QT) 
NAME = ../libexample$(DOT_SO)


OBJ = srv_example.o srv_commands.o srv_interface.o

.SUFFIXES: .cpp .o

.cpp.o:
	$(C) $(CFLAGS) -c -fPIC -o $@ $<

all: $(NAME)

$(NAME): $(OBJ) 
	$(C) $(CFLAGS) -shared -fPIC $(LIB_PATH) -o $(NAME) $(OBJ) $(LIB) $(SYS_LIB)

clean:
	 rm -rf $(OBJ) $(NAME)