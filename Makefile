DIR_INC = core/include
DIR_SRC = core/src
DIR_OBJ = obj
DIR_BIN = bin
DIR_LOG = log
DIR_EX_LIB = libreadline-devel

SRC = $(wildcard $(DIR_SRC)/*.c) 
OBJ = $(patsubst %.c,$(DIR_OBJ)/%.o,$(notdir $(SRC))) 

TARGET = autoTest
BIN_TARGET = $(DIR_BIN)/$(TARGET)

CC = gcc
CFLAGS = -I $(DIR_INC) 
LDFLAGS = -l readline -l history

all:ENV_INIT $(BIN_TARGET)  

$(BIN_TARGET):$(OBJ) Makefile	
	@$(CC) $(OBJ) $(LDFLAGS) -o $@ 
	@cp $(BIN_TARGET) ./
	@echo "target make completed!"
	
$(DIR_OBJ)/%.o:
	@echo "Building $<..."
	@$(CC) $(CFLAGS) -c $< -o $@ 
			
$(DIR_SRC)/%.dep:$(DIR_SRC)/%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\$(DIR_OBJ)/$*.o : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	
.PHONY:ENV_INIT
ENV_INIT:		
	@if [ ! -d $(DIR_OBJ) ];then mkdir -p $(DIR_OBJ);chmod 666 $(DIR_OBJ);fi
	@if [ ! -d $(DIR_BIN) ];then mkdir -p $(DIR_BIN);chmod 666 $(DIR_BIN);fi
	@if [ ! -d $(DIR_LOG) ];then mkdir -p $(DIR_LOG);chmod 666 $(DIR_LOG);fi
	@./$(DIR_EX_LIB)/libInstall.sh
	
.PHONY:clean
clean:
	rm -rf $(OBJ) $(DIR_OBJ) $(DIR_BIN)
	rm -rf $(TARGET) $(BIN_TARGET)
	rm -rf msgQ *.dep $(DIR_SRC)/*.dep
	@echo "make clean all.....done!"
	
-include $(SRC:.c=.dep)	