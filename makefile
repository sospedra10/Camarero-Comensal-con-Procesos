all: camarero.c comensal.c
	gcc -o camarero camarero.c
	gcc -o comensal comensal.c

clean:
	rm -rf *.o
	rm -rf comensal 
	rm -rf camarero
