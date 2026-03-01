CXX = g++
CXXFLAGS = -std=c++11 -O3 -Wall
TARGET = matrix_mult
SOURCES = main.cpp

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET) *.txt *.py

run: $(TARGET)
	./$(TARGET)

run5: $(TARGET)
	./$(TARGET) 5 matrix1.txt matrix2.txt result.txt

run10: $(TARGET)
	./$(TARGET) 10 matrix1.txt matrix2.txt result.txt

.PHONY: clean run run5 run10