OBJS=$(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET=libChatRoom_server.so
# 动态库中所有的头文件
HFILES=$(wildcard ./*.h)


$(TARGET):$(OBJS)
	$(CC) -shared -fPIC $^ -o $@

# 放到lib库中
	cp $(TARGET) /usr/lib/

# 将头文件更新到上级目录
	cp $(HFILES) ..

%.o:%.c
	$(CC) -c $^ -o $@



.PHONY: clean show

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) /usr/lib/$(TARGET)

show:
	@echo $(RM) ../1.c 