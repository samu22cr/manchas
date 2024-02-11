build:
	gcc -o out/build/portfolio ./src/*.c
run:
	./out/build/portfolio
clean:
	rm ./out/build/portfolio
