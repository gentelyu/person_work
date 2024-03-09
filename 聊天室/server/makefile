# 依赖
OBJS=$(patsubst %.c, %.o, $(wildcard ./*.c))
HFILES=$(wildcard ./*.h)

# 目标
TARGET=chatRoom_server

# -L手动指定搜索路径
LDFALGS=-L./server_src_so
# -l链接的动态库名字，这里只需要加libXXX.so中间的名字
LIBS=-lChatRoom_server -lsqlite3

$(TARGET):$(OBJS)
	$(CC) -g $^ $(LDFALGS) $(LIBS) -o $@ 

%.o:%.c
	$(CC) -c $^ -o $@

clean_hfiles:
	$(RM) $(HFILES)

clean:
	$(RM) $(TARGET) $(OBJS)