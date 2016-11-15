CXX = g++

#CXXFLAG = -Wall -g -DTIXML_USE_STL -O2 -Wl,-E -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 -DHTTPS_SUPPORT
CXXFLAG = -Wall -g -DTIXML_USE_STL -O2 -Wl,-E -fPIC -D_GLIBCXX_USE_CXX11_ABI=0 

INC = \
      -I ./ \
      -I ./http \
      -I ../common \
      -I ../common/tinyxml \
	  -I ../match \
	  -I ../match/fmt \
	  -I ../match/dat \
	  -I ../match/rr \
	  -I $(DEP_DIR_VAR)/usr/local/include \
	  -I $(DEP_DIR_VAR)/usr/include \

LIB = \
	  ../match/objs/match.a \
      ../common/objs/common.a \
      ../common/tinyxml/libtinyxml.a \
      ../common/jsoncpp/objs/jsoncpp.a \
	  -levent -llog4cpp -lprotobuf -lpthread

OBJFLAG = 

OBJDIR = objs

SRCS = $(wildcard *.cpp ./*/*.cpp)

OBJS=$(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))


all:  $(OBJDIR)/imock-server

$(OBJDIR)/imock-server: $(OBJS) 
	$(CXX) $(CXXFLAG) -o $@ $^ $(LIB) $(OBJFLAG)

$(OBJDIR)/%.o:%.cpp
	mkdir -p $(@D);
	$(CXX) $(CXXFLAG) -c $(INC) -o $@ $<

clean:
	rm -rf $(OBJDIR)
