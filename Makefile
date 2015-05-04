LIB			= 	libwifi_extension
CXXFLAGS	=	-O -g -I./ $(shell pkg-config --cflags dbus-1) $(shell pkg-config --cflags gio-2.0) -std=c++11 \
				-fpermissive -fPIC
LDFLAGS		=	-Wl,--whole-archive gdbus/libgdbus.a build/libwifi_extension.a -Wl,--no-whole-archive \
				-shared -Wl,-soname,$(LIB).so -o $(LIB).so -g $(shell pkg-config --libs dbus-1) $(shell pkg-config --libs gio-2.0)
SOURCES		=	src/WifiMaster.cpp src/wifi_extension.cpp src/wifi_instance.cpp src/wifi_api.cpp common/extension.cpp
OBJECTS		= $(SOURCES:.cpp=.o)

all: $(LIB)

$(LIB): prepare gdbus wifi_api.cpp wifi_lib
	$(CXX) $(CXXFLAGS) -shared -o build/$(LIB) $(LDFLAGS)

prepare:
	mkdir -p build

gdbus: libgdbus.a

libgdbus.a:
	$(MAKE) -C gdbus

wifi_api.cpp :
	python tools/generate_api.py src/wifi_api.js kSource_wifi_api src/wifi_api.cpp

wifi_lib: $(OBJECTS)
	$(AR) rcs build/$(LIB).a  $(OBJECTS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

install:
	install -D build/$(LIB) $(DESTDIR)/$(PREFIX)/lib/tizen-extensions-crosswalk/$(LIB)

clean:
	$(RM) -r build
	$(RM) $(OBJECTS)
	$(RM) $(LIB).so
	$(RM) src/wifi_api.cpp
	$(MAKE) -C gdbus clean

.PHONY: all prepare clean

