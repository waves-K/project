CURR_PATH=$(shell pwd)
LIB=$(CURR_PATH)/../lib
INCLUDE=-I$(LIB)/include #-I,指定头文件目录
MYSQL_LIB=-L$(LIB)/lib -lmysqlclient  #-L,指定库路径
cc=g++

.PHONY:all clean
all:insert_cgi select_cgi

insert_cgi:insert_cgi.cpp comm.cpp
	g++ -o $@ $^ $(INCLUDE) $(MYSQL_LIB)
select_cgi:select_cgi.cpp comm.cpp
	g++ -o $@ $^ $(INCLUDE) $(MYSQL_LIB)

clean:
	rm insert_cgi select_cgi

#CURR_PATH=$(shell pwd)
#LIB=$(CURR_PATH)/../lib
#INCLUDE=-I$(LIB)/include #-I,指定头文件目录
#MYSQL_LIB=-L$(LIB)/lib -lmysqlclient  #-L,指定库路径
#
#
#test:test.c
#	gcc $^ -o $@ $(INCLUDE) $(MYSQL_LIB)
#
#.PHONY:clean
#
#clean:
#	rm -f test
