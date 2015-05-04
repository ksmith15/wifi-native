CXXFLAGS  = -O -g $(shell pkg-config --cflags gio-2.0) $(shell pkg-config --cflags dbus-1) -fpermissive -std=c++11 -fPIC
SOURCES = mainloop.c watch.c object.c client.c polkit.c
OBJS    = $(SOURCES:.c=.o)
LIB     = libgdbus.a

all : $(LIB)

$(LIB) : $(OBJS)
	ar rcs $(LIB) $(OBJS)

%.o : %.c
	$(CXX) -c $(CXXFLAGS) $<

clean:
	$(RM) $(OBJS) $(LIB) core

