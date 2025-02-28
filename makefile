CC = g++    #gnu c++ 編譯器
CFLAGS = -std=c++11 -Wall -O2    #使用c++11標準   #wall : 開啟所有警告   #o2 : 最佳化程式執行效率(第二級別優化)
LDFLAGS = -pthread     #支持多線程 

TARGET = camera_stream    #camera_stream : 目標執行文件
SOURCES = libcamera_server.cpp  #libcamera_server.cpp : 源文件
OBJECTS = $(SOURCES:.cpp=.o)   #把libcamera_server.cpp 轉換成 .o (轉變成可以執行的代碼形式)

all: $(TARGET)    #輸入make指令後，會自動創建camera_stream程序

$(TARGET): $(OBJECTS)     #g++ libcamera_server.o -o camera_stream -pthread
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)    

%.o: %.cpp          #g++ -std=c++11 -Wall -O2 -c libcamera_server.cpp -o libcamera_server.o
	$(CC) $(CFLAGS) -c $< -o $@

clean:               #定義clean : 刪除所有生成文件
	rm -f $(OBJECTS) $(TARGET)            
 
.PHONY: all clean   #表all, clean 是命令名稱，不是文件名