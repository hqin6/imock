CXX = g++

CXXFLAG = -Wall -g -DTIXML_USE_STL -O2 -Wl,-E -fPIC -D_GLIBCXX_USE_CXX11_ABI=0
#ifneq ($(DEP_DIR), "")
	#DEP_DIR_VAR = $(DEP_DIR)/var
#endif

INC = \
      -I ./ \
      -I ../../interface/ \
	  -I ../../match/dat/ \
	  -I ../../match/fmt/ \
	  -I ../../match/rr/ \
	  -I ../../common/ \
	  -I ../../common/tinyxml \
	  -I $(DEP_DIR_VAR)/usr/local/include \
	  -I $(DEP_DIR_VAR)/usr/include \

LIB = \
	  ../../interface/objs/libimock-interface.a \
      -llog4cpp -lprotobuf 

OBJFLAG = 

OBJDIR = objs

SRCS = $(wildcard *.cpp ./*/*.cpp)

OBJS=$(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))


all:  $(OBJDIR)/imock-diff

$(OBJDIR)/imock-diff: $(OBJS) 
	$(CXX) $(CXXFLAG) -o $@ $^ $(LIB) $(OBJFLAG)

$(OBJDIR)/%.o:%.cpp
	mkdir -p $(@D);
	$(CXX) $(CXXFLAG) -c $(INC) -o $@ $<

clean:
	rm -rf $(OBJDIR)
