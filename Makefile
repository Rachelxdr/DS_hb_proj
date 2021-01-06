GCC := g++
RM := rm 
APP := Node
HEADERS := ./header_files
FLAGS := -g -pthread -std=c++11 -I$(HEADERS)
# SRC := ./cpp_files/*
SRC := ./cpp_files/Logger.cpp ./cpp_files/main.cpp ./cpp_files/thread.cpp ./cpp_files/Udp_socket.cpp ./cpp_files/Node.cpp
# OFILES := main.o Logger.o Node.o Udp_socket.o thread.o


# $(APP): $(OFILES)
# 	$(GCC) $(OFILES) -o $(APP)

app:
	$(GCC) -o $(APP) $(SRC) $(FLAGS) 

# $(OFILES): $(SRC)
# 	$(GCC) $(FLAGS) $(SRC)

clean:
	$(RM) -f $(APP) *.o *_log.txt