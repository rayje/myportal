#  Libraries - MinGW
ifdef SystemRoot
LIBS=-lglut32cu -lglu32 -lopengl32
CLEAN=del *.exe *.o *.a
else
#  Libraries - OSX
ifeq ($(shell uname), Darwin)
LIBS=-framework GLUT -framework OpenGL
else 
LIBS=-lglut -lGLU -lm
endif
endif

all: portal

#  Generic compile rules
.c.o:
	gcc -c -O -Wall $<

#  Generic compile and link
%: %.c mgltools.a
	gcc -Wall -O3 -o $@ $^ $(LIBS)

#  Delete unwanted files
clean:;rm -f portal *.o *.a

#  Create archive (include glWindowPos here if you need it)
mgltools.a:fatal.o loadtexbmp.o print.o project.o errcheck.o object.o
	ar -rcs mgltools.a $^

#  SDL version of ex20
sdl20: sdl20.o fatal.o loadtexbmp.o project.o errcheck.o printsdl.o
	gcc -Wall -O3 -o $@ $^ -lSDL -lSDL_mixer $(LIBS)
