all: smoke

smoke:
	cd Whatever && make smoke
	cd Dict && make smoke
	cd Bignum && make smoke
	cd Huffman && make smoke

clean:
	cd Whatever && make clean
	cd Dict && make clean
	cd Bignum && make clean
	cd Huffman && make clean
