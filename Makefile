
TEST_SRC = $(wildcard tests/*Test.cpp)
TEST_OBJ = $(TEST_SRC:.cpp=.o)

all: tests.out profile.out
	@echo "Executing test: "
	@./tests.out
	@echo "Tests finished. Executing performace comparisions."
	@./profile.out

profile.out:
	g++ -I. -std=c++11 -O3 -Wall -Wextra tests/profile.cpp -pthread -o profile.out

tests.out: $(TEST_OBJ)
	g++ $(TEST_OBJ) -I. -lgtest -std=c++11 -o tests.out -lpthread -Wall -Wextra

tests/%.o: tests/%.cpp
	g++ $(CC_FLAGS) -c -o $@ $< -I. -std=c++11 -Wall -Wextra

clean:
	rm tests.out $(TEST_OBJ) profile.out
