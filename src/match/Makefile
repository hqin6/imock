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
      -I ./dat/ \
      -I ./fmt/ \
      -I ./rr \
	  -I $(DEP_DIR_VAR)/usr/local/include \
	  -I $(DEP_DIR_VAR)/usr/include \

LIB = \

OBJFLAG = \

OBJDIR = objs

SRCS = $(wildcard *.cpp ./dat/*.cpp ./fmt/*.cpp ./rr/*.cpp)

OBJS=$(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

LIB_TARGET=$(OBJDIR)/match.a

all:  $(LIB_TARGET)

$(LIB_TARGET): $(OBJS) 
	echo "ar: $(LIB_TARGET)"
	ar -r $(LIB_TARGET) $(OBJS)
	ranlib $(LIB_TARGET)

$(OBJDIR)/%.o:%.cpp
	mkdir -p $(@D);
	$(CXX) $(CXXFLAG) -c $(INC) -o $@ $<

clean:
	rm -rf $(OBJDIR)
