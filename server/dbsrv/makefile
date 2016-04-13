INCLUDE_DIRS =-I. -I../rd/protobuf/src -I../3rd/lua/src -I../3rd/mysql/include -I../3rd/hiredis -I../3rd/tolua++ -I../srvlib

LIB_DIRS =

LIBS =../3rd/lua/src/liblua.a ../3rd/mysql/lib/libmysqlclient.a ../3rd/hiredis/libhiredis.a ../3rd/tolua++/tolua++.a    ../3rd/libprotobuf.a
DY_LIBS = -lm -ldl -lreadline -lhistory -lncurses -lpthread

CFLAGS=-O2 -Wall

SOURCE = $(wildcard \
		 ../srvlib/net/*.cc\
		 ../srvlib/log/*.cc\
		 ../srvlib/pblua/*.cc\
		 script/*.cc\
		 gamesrv/*.cc\
		 sqlconn/*.cc\
		 main.cc\
		 interface.cc\
		 ) 


OBJ = $(patsubst %.cc,%.o,$(SOURCE))


EXE = dbsrv 



all:$(EXE) 


%.o:%.cc
	g++ $(CFLAGS) -c $< -o $@ $(INCLUDE_DIRS)


${EXE}:$(OBJ)
	g++ -o $(EXE) $(OBJ) $(LIBS) $(DY_LIBS) $(LIB_DIRS) 

tolua:
	../3rd/tolua++/tolua++ -n dbsrv -o interface.cc interface.pkg


clean:
	rm -rf $(EXE)
	rm -rf $(OBJ)
