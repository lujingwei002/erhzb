INCLUDE_DIRS =-I. -I../rd/protobuf/src -I../3rd/lua/src -I../3rd/mysql/include -I../3rd/hiredis -I../3rd/tolua++

LIB_DIRS =

LIBS =../3rd/lua/src/liblua.a ../3rd/mysql/lib/libmysqlclient.a ../3rd/hiredis/libhiredis.a ../3rd/tolua++/tolua++.a 
DY_LIBS = -lm -ldl -lreadline -lhistory -lncurses -lpthread

CFLAGS=-O2 -Wall

SOURCE = $(wildcard \
		 net/*.cc\
		 websocket/*.cc\
		 log/*.cc\
		 script/*.cc\
		 encrypt/*.cc\
		 gameclient/*.cc\
		 main.cc\
		 interface.cc\
		 ) 


OBJ = $(patsubst %.cc,%.o,$(SOURCE))


EXE = gatesrv



all:$(EXE) 


%.o:%.cc
	g++ $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)


${EXE}:$(OBJ)
	g++ -o $(EXE) $(OBJ) $(LIBS) $(DY_LIBS) $(LIB_DIRS) 

tolua:
	../3rd/tolua++/tolua++ -n gatesrv -o interface.cc interface.pkg


clean:
	rm -rf $(EXE)
	rm -rf $(OBJ)
