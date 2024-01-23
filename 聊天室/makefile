# 服务器
OBJECTS_SERVER=StdTcp.o testServer.o StdThread.o DoubleLinkList.o Queue.o ThreadPool.o StdSqlite.o StdFile.o
TARGET_SERVER=server

# 客户端
OBJECTS_CLIENT=StdTcp.o testClient.o StdThread.o DoubleLinkList.o StdSqlite.o StdFile.o
TARGET_CLIENT=client

.DEFAULT_GOAL := all

.PHONY: all clean_server clean_client clean

all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_SERVER): $(OBJECTS_SERVER)
ifeq ($(wildcard $@),)
	gcc $^ -o $@ -lsqlite3
endif

$(TARGET_CLIENT): $(OBJECTS_CLIENT)
ifeq ($(wildcard $@),)
	gcc $^ -o $@ -lsqlite3
endif

StdTcp.o: StdTcp.c
	gcc -c $^ -o $@

testServer.o: testServer.c
	gcc -c $^ -o $@

testClient.o: testClient.c
	gcc -c $^ -o $@

StdThread.o: StdThread.c
	gcc -c $^ -o $@

DoubleLinkList.o: DoubleLinkList.c
	gcc -c $^ -o $@

Queue.o: Queue.c
	gcc -c $^ -o $@

ThreadPool.o: ThreadPool.c
	gcc -c $^ -o $@

StdSqlite.o: StdSqlite.c
	gcc -c $^ -o $@

StdFile.o: StdFile.c
	gcc -c $^ -o $@

clean_server:
	@rm -rf *.o $(TARGET_SERVER)

clean_client:
	@rm -rf *.o $(TARGET_CLIENT)

clean: clean_server clean_client
