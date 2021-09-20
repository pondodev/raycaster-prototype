executable_name = output

full: build run

fullclean: clean build run

build:
		g++ -o $(executable_name) *.cpp

run:
		./$(executable_name)

clean:
		rm -f ./$(executable_name)