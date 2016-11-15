CXX = g++

CXXFLAG = -Wall -g -fPIC -D_GLIBCXX_USE_CXX11_ABI=0
#ifneq ($(DEP_DIR), "")
	#DEP_DIR_VAR = $(DEP_DIR)/var
#endif

INC = \
      -I ./../ \
      -I ./ \
	  -I $(DEP_DIR_VAR)/usr/local/include \
	  -I $(DEP_DIR_VAR)/usr/include \

LIB = \
	  ../objs/common.a \
      -lgtest -llog4cpp -lpthread

OBJS = \
       utest.o \
       utest_ProcTitle.o \
	   utest_INIConf.o 


OBJFLAG = 

all: utest 

utest: $(OBJS)
	$(CXX) $(CXXFLAG) -o $@ $^ $(LIB) $(OBJFLAG)
	rm -f $(OBJS);

.cpp.o:
	$(CXX) $(CXXFLAG) -c $(INC) -o $@ $<

clean:
	rm -rf $(OBJS) utest

