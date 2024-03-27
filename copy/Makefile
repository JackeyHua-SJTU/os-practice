all: pipe fork copy

pipe: PipeCopy.c
	gcc PipeCopy.c -o PipeCopy -D$(MACRO)

fork: ForkCopy.c 
	gcc ForkCopy.c -o ForkCopy -D$(MACRO)

copy: MyCopy.c
	gcc MyCopy.c -o MyCopy -D$(MACRO)

clean:
	rm MyCopy ForkCopy PipeCopy