EL5 = $(shell uname -r | grep el5 | wc -l )

all:
	cd parse && make && cd -;
	cd serialize && make && cd -;
	cd diff  && make && cd -;
	cd pbfile2xml && make && cd -;
clean:
	cd parse && make clean && cd -;
	cd serialize && make clean && cd -;
	cd diff && make clean && cd -;
	cd pbfile2xml && make clean && cd -;

