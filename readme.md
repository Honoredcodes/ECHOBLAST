g++ -std=c++17 -o program main.cpp \
-I/opt/homebrew/Cellar/jsoncpp/1.9.6/include \
 -I/usr/include \
 -L/opt/homebrew/Cellar/jsoncpp/1.9.6/lib \
 -L/opt/homebrew/opt/curl/lib \
 -ljsoncpp -lcurl

 cd ./Projects/"C++ PROJECTS"/ECHOBLAST