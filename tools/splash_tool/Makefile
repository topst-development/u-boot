CC=gcc

all: bmpbpp mksplash rgbto565 rgbto888 rgbto8888
bmpbpp: bmpbpp.*
	$(CC) $< -o bmpbpp
mksplash: mksplash.*
	$(CC) $< -o $@
rgbto565: to565.*
	$(CC) $< -o $@
rgbto888: to888.*
	$(CC) $< -o $@
rgbto8888: to8888.*
	$(CC) $< -o $@
clean:
	rm -rf *.o bmpbpp mksplash rgbto565 rgbto888 rgbto8888
