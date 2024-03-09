# 依赖
OBJS=$(patsubst %.c, %.o, $(wildcard ./*.c))
# 目标
TARGET=chatRoom_client

# -L手动指定搜索路径
LDFALGS=-L./client_src_so
# -l链接的动态库名字，这里只需要加libXXX.so中间的名字
LIBS=-lChatRoom_client -lsqlite3

$(TARGET):$(OBJS)
	$(CC) -g $^ $(LDFALGS) $(LIBS) -o $@

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	$(RM) $(TARGET) $(OBJS)