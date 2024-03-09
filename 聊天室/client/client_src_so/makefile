OBJS=$(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET=libChatRoom_client.so
HFILES=$(wildcard ./*.h)

$(TARGET):$(OBJS)
	$(CC) -shared -fPIC $^ -o $@

# 放到lib库中
	cp $(TARGET) /usr/lib/
# 将头文件拷贝到上级目录
	cp $(HFILES) ..

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) /usr/lib/$(TARGET)