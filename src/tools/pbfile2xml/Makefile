CXX = g++

CXXFLAG = -Wall -g -DTIXML_USE_STL -O2 -Wl,-E -fPIC -D_GLIBCXX_USE_CXX11_ABI=0
#ifneq ($(DEP_DIR), "")
	#DEP_DIR_VAR = $(DEP_DIR)/var
#endif

INC = \
      -I ./ \
	  -I ../../common/ \
	  -I $(DEP_DIR_VAR)/usr/local/include \
	  -I $(DEP_DIR_VAR)/usr/include \

LIB = \
	  ../../common/objs/common.a \
      ../../common/tinyxml/libtinyxml.a \
      ../../common/jsoncpp/objs/jsoncpp.a \
      -llog4cpp -lprotobuf 

OBJFLAG = 

OBJDIR = objs

all:  $(OBJDIR)/imock-pbfile2xml 

$(OBJDIR)/imock-pbfile2xml: pbfile2xml.cpp
	mkdir -p $(@D);
	$(CXX) $(CXXFLAG) -o $@ $^ $(LIB) $(OBJFLAG) $(INC)

clean:
	rm -rf $(OBJDIR)
