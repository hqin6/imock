CXX = g++

CXXFLAG = -Wall -g -fPIC -D_GLIBCXX_USE_CXX11_ABI=0
#ifneq ($(DEP_DIR), "")
	#DEP_DIR_VAR = $(DEP_DIR)/var
#endif

INC = \
      -I ../../common/ \
      -I ../../common/tinyxml \
      -I ../fmt/ \
      -I ../dat/ \
      -I ../rr/ \
      -I ./ \
	  -I $(DEP_DIR_VAR)/usr/local/include \
	  -I $(DEP_DIR_VAR)/usr/include \

LIB = \
      -lgtest  \
	  ../objs/match.a \
      ../../common/objs/common.a \
      ../../common/tinyxml/libtinyxml.a \
      ../../common/jsoncpp/objs/jsoncpp.a \
	  -llog4cpp -lprotobuf -lpthread

OBJS = \
	   foo.pb.o \
	   addr.pb.o \
	   utest_fmt_dat.o \
       utest.o \


OBJFLAG =

all: utest 

utest: $(OBJS)
	$(CXX) $(CXXFLAG) -o $@ $^ $(LIB) $(OBJFLAG)

.cpp.o:
	$(CXX) $(CXXFLAG) -c $(INC) -o $@ $<
.cc.o:
	$(CXX) $(CXXFLAG) -c $(INC) -o $@ $<

clean:
	rm -rf $(OBJS) utest

