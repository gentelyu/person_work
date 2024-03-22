OBJS=$(patsubst %.c, %.o, $(wildcard ./*.c))
TARGET=libBingDu.so


$(TARGET):$(OBJS)
	$(CC) -shared -fPIC $^ -o $@

#	发布
	cp $(TARGET) /usr/lib/

%.o:%.c
	$(CC) -c $^ -o $@

clean:
	$(RM) $(OBJS) $(TARGET)
	$(RM) /usr/lib/$(TARGET)