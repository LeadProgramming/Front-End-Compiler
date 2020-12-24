echo "Compiler is running."
g++ -c -w main.cpp Compiler.h Compiler.cpp
g++ -o Compiler main.o Compiler.o
./Compiler
echo "Compiler has finished running."