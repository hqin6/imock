CXX = g++

CXXFLAG = -Wall -g -DTIXML_USE_STL -O2 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0
#ifneq ($(DEP_DIR), "")
	#DEP_DIR_VAR = $(DEP_DIR)/var
#endif

INC = \
      -I ./ \
      -I ../common \
      -I ../common/tinyxml \
      -I ../common/jsoncpp/include \
      -I ../match \
      -I ../match/fmt \
      -I ../match/dat \
      -I ../match/rr \
	  -I $(DEP_DIR_VAR)/usr/local/include \
	  -I $(DEP_DIR_VAR)/usr/include \

LIB = ../match/objs/match.a \
      ../common/objs/common.a \
      ../common/tinyxml/libtinyxml.a \
      ../common/jsoncpp/objs/jsoncpp.a \


OBJFLAG = \

OBJDIR = objs

SRCS = $(wildcard *.cpp)

OBJS=$(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

LIBA_TARGET = $(OBJDIR)/libimock-interface.a 
LIBSO_TARGET = $(OBJDIR)/libimock-interface.so

all:  $(LIBA_TARGET) $(LIBSO_TARGET)

$(LIBA_TARGET): $(OBJS) $(LIB)
	echo "ar: $(LIBA_TARGET)"
	$(foreach i, $(LIB), ar -x $(i);)
	ar -r $(LIBA_TARGET) $(OBJS) *.o
	ranlib $(LIBA_TARGET)
	rm -rf *.o

$(LIBSO_TARGET): $(OBJS) $(LIB)
	echo "ar: $(LIBA_TARGET)"
	$(foreach i, $(LIB), ar -x $(i);)
	$(CXX) -shared $(CXXFLAG) $(OBJS) *.o -o $@
	rm -rf *.o

$(OBJDIR)/%.o:%.cpp
	mkdir -p $(@D);
	$(CXX) $(CXXFLAG) -c $(INC) -o $@ $<

clean:
	rm -rf $(OBJDIR)
