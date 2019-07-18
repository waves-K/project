.PHONY:clean

all:math_cgi  exe

math_cgi:math_cgi.c
	gcc $^ -o $@

exe:
	cd ../C/; make clean; make; cd -;
	cd ../C++/; make clean; make; cd -;

clean:
	rm math_cgi 
	cd ../C/; make clean; cd -;
	cd ../C++; make clean; cd -;
