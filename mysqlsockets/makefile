cc = g++ -std=c++11
INCLUDE := -I /home/server/libserver/ -I ../codec/
CONFLAGS := -g -lpthread -lcccsss -lzlog -lmysqlclient -ljson -lCodec
CSRCS = $(wildcard *.c)
OBJ_SRC = $(patsubst %.c,%.o,$(CSRCS))
CSRCS_H =  $(wildcard *.h)
sqlsocks:$(OBJ_SRC) 
	$(cc)  $(OBJ_SRC) $(CONFLAGS) -o $@
%.o:%.c
	$(cc) $(INCLUDE) -c $<
clean:
	rm -rf *.o sqlsocks