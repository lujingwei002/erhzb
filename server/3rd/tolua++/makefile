
CC=gcc
INCLUDE_DIRS=-I../../3rd/lua/src

SRC=tolua.c toluabind.c tolua_event.c tolua_is.c tolua_map.c tolua_push.c tolua_to.c 

LIB_SRC=toluabind.c tolua_event.c tolua_is.c tolua_map.c tolua_push.c tolua_to.c 

OBJ = $(patsubst %.c,%.o,$(LIB_SRC))

all:tolua++ tolua++.a

%.o:%.c
	g++ -O2 -c $< -o $@ $(INCLUDE_DIRS)

tolua++:$(SRC)
	gcc -O2 -Wall -o tolua++ $(SRC) ../../3rd/lua/src/liblua.a -lm -ldl ${INCLUDE_DIRS}

tolua++.a:$(OBJ)
	ar rcu $@ $(OBJ)
	ranlib $@

test:
	./tolua++ -n gamesrv -o testfile/interface.cc testfile/interface.pkg 

clean:
	rm -rf tolua++ tolua++.a $(OBJ)
