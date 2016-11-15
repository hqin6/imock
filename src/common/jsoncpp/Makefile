CXX = g++

CXXFLAG = -Wall -g -O2 -fPIC -D_GLIBCXX_USE_CXX11_ABI=0

INC = \
      -I ./ \
	  -I ./include \

LIB = \

OBJFLAG = \

OBJDIR = objs

SRCS = $(wildcard *.cpp ./*/*.cpp)

OBJS=$(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))

LIB_TARGET=$(OBJDIR)/jsoncpp.a

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
