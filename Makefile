
TEST_SRC = $(wildcard tests/*.cpp)
TEST_OBJ = $(TEST_SRC:.cpp=.o)

all: tests.out

tests.out: $(TEST_OBJ)
	g++ $(TEST_OBJ) -I. -lgtest -std=c++11 -o tests.out -lpthread

tests/%.o: tests/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $< -I. -std=c++11

clean:
	rm tests.out $(TEST_OBJ)
