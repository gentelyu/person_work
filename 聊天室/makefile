#静态库
LIBRARY = -lsqlite3

# 服务器
OBJECTS_SERVER=DoubleLinkList.o Queue.o StdSqlite.o StdThread.o ThreadPool.o StdFile.o StdTcp_Server.o testServer.o
TARGET_SERVER=server

# 客户端
OBJECTS_CLIENT=DoubleLinkList.o StdSqlite.o StdThread.o StdFile.o StdTcp_Client.o testClient.o
TARGET_CLIENT=client

.DEFAULT_GOAL := all

.PHONY: all clean_server clean_client clean

all: $(TARGET_SERVER) $(TARGET_CLIENT)

$(TARGET_SERVER): $(OBJECTS_SERVER)
# 如果$(wildcard $@)和空相等(当前变量不存在)
ifeq ($(wildcard $@),)
	gcc -g $^ -o $@ $(LIBRARY)
endif

$(TARGET_CLIENT): $(OBJECTS_CLIENT)
ifeq ($(wildcard $@),)
	echo $^
	gcc -g $^ -o $@ $(LIBRARY)
endif

DoubleLinkList.o: DoubleLinkList.c
	gcc -c $^ -o $@

Queue.o: Queue.c
	gcc -c $^ -o $@

StdSqlite.o: StdSqlite.c
	gcc -c $^ -o $@

StdThread.o: StdThread.c
	gcc -c $^ -o $@

ThreadPool.o: ThreadPool.c
	gcc -c $^ -o $@

StdFile.o: StdFile.c
	gcc -c $^ -o $@

StdTcp_Server.o: StdTcp_Server.c
	gcc -c $^ -o $@

StdTcp_Client.o: StdTcp_Client.c
	gcc -c $^ -o $@

testServer.o: testServer.c
	gcc -c $^ -o $@

testClient.o: testClient.c
	gcc -c $^ -o $@

clean_server:
	@rm -rf *.o $(TARGET_SERVER)

clean_client:
	@rm -rf *.o $(TARGET_CLIENT)

clean: clean_server clean_client
