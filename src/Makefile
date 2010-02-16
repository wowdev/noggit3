CC = g++
objects = areadb.o dbcfile.o font.o frustum.o liquid.o particle.o maptile.o menu.o model.o mpq_libmpq.o sky.o test.o video.o wmo.o world.o wowmapview.o

all:	wowmapview

clean:
	rm -f wowmapview *.o

wowmapview: $(objects) libmpq/libmpq.a zlib/zlib.a
	$(CC) -o $@ $+ -L/usr/X11R6/lib -lSDL -lGL -lGLU

clean_mpq:
	libmpq/make clean
clean_zlib:
	zlib/make clean

libmpq/libmpq.a:
	libmpq/make
zlib/zlib.a:
	zlib/make

%.o:%.cpp
	$(CC) -c $+


