SWOOLE_SRC = "/opt/swoole-src"
PHP_INCLUDE = `php-config --includes`
PHP_LIBS = `php-config --libs`
PHP_LDFLAGS = `php-config --ldflags`

all: ro_des.cpp
	c++ -DHAVE_CONFIG_H -g  -O0 -fPIC -c ro_des.cpp ${PHP_INCLUDE} -std=c++11 -I${SWOOLE_SRC}/include -I${SWOOLE_SRC}
	gcc -fPIC -c d3des.c
	c++ -shared ro_des.o d3des.o -o ro_des.so
clean: ro_des.so
	rm ro_des.so d3des.o