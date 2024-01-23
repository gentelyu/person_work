#OOBJECTS_SERVER=StdTcp.c testServer.c StdThread.c DoubleLinkList.c Queue.c ThreadPool.c StdSqlite.c StdFile.c
#server:$(OOBJECTS_SERVER)
#	gcc $(OOBJECTS_SERVER) -o server -lsqlite3

#OOBJECTS_CLIENT=StdTcp.c testClient.c StdThread.c DoubleLinkList.c StdSqlite.c StdFile.c
#client:$(OOBJECTS_CLIENT)
#	gcc $(OOBJECTS_CLIENT) -o client -lsqlite3


#.PHONY:clean
#clean:
#	rm -f server client

#。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
#服务器
OBJECTS_SERVER=StdTcp.o testServer.o StdThread.o DoubleLinkList.o Queue.o ThreadPool.o StdSqlite.o StdFile.o
TARGET_SERVER=server

$(TARGET_SERVER):$(OBJECTS_SERVER)
	gcc $^ -o $@ -lsqlite3

StdTcp.o:StdTcp.c
	gcc -c $^ -o $@

testServer.o:testServer.c
	gcc -c $^ -o $@

StdThread.o:StdThread.c
	gcc -c $^ -o $@

DoubleLinkList.o:DoubleLinkList.c
	gcc -c $^ -o $@

Queue.o:Queue.c
	gcc -c $^ -o $@

ThreadPool.o:ThreadPool.c
	gcc -c $^ -o $@

StdSqlite.o:StdSqlite.c
	gcc -c $^ -o $@

StdFile.o:StdFile.c
	gcc -c $^ -o $@

clean_server:
	@rm -rf *.o $(TARGET_SERVER)

#。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。。
#客户端
OBJECTS_CLIENT=StdTcp.o testClient.o StdThread.o DoubleLinkList.o StdSqlite.o StdFile.o
TARGET_CLIENT=client


$(TARGET_CLIENT):$(OBJECTS_CLIENT)
	gcc $^ -o $@ -lsqlite3

StdTcp.o:StdTcp.c
	gcc -c $^ -o $@

testClient.o:testClient.c
	gcc -c $^ -o $@

StdThread.o:StdThread.c
	gcc -c $^ -o $@

DoubleLinkList.o:DoubleLinkList.c
	gcc -c $^ -o $@

StdSqlite.o:StdSqlite.c
	gcc -c $^ -o $@

StdFile.o:StdFile.c
	gcc -c $^ -o $@
	
clean_client:
	@rm -rf *.o $(TARGET_CLIENT)

clean:
	@rm -rf *.o $(TARGET_SERVER) $(TARGET_SERVER)