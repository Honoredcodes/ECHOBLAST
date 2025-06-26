CXX = g++
CXXFLAGS = -Wall -std=c++17 \
  -IModules/email \
  -IModules/services \
  -IModules/utils \
  -IModules/includes \
  -I/opt/homebrew/Cellar/jsoncpp/1.9.6/include \
  -I/usr/include

LDFLAGS = \
  -L/opt/homebrew/Cellar/jsoncpp/1.9.6/lib \
  -L/opt/homebrew/opt/curl/lib \
  -ljsoncpp -lcurl

SRC = \
  main.cpp \
  Modules/email/email_program.cpp \
  Modules/services/curl_utils.cpp \
  Modules/utils/GenericMethods.cpp

TARGET = program

all:
	@echo "please wait, while the program is being compiled..."
	@$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)