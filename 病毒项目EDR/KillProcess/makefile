OBJS=$(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET=test_main

# -L手动指定搜索路径
LDFALGS=-L./src_so
# -l链接的动态库名字，这里只需要加libXXX.so中间的名字
LIBS=-lBingDu -lcrypto -lssl

$(TARGET):$(OBJS)
	$(CC) -g $^ $(LDFALGS) $(LIBS) -o $@

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	$(RM) $(TARGET) $(OBJS)