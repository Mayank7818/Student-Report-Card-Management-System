# Student Report Card Management System
#
# Override the MySQL location if it is installed elsewhere:
#   make MYSQL_DIR="C:/Program Files/MySQL/MySQL Server 8.0"

MYSQL_DIR ?= C:/Program Files/MySQL/MySQL Server 8.0

CXX      ?= g++
CXXFLAGS ?= -std=c++11 -Wall -Wextra -O2
INCLUDES  = -I"$(MYSQL_DIR)/include"
LDFLAGS   = -L"$(MYSQL_DIR)/lib"
LDLIBS    = -lmysql

TARGET  = report_card
SOURCES = main.cpp Console.cpp Database.cpp Student.cpp
OBJECTS = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Type-check without linking — useful when the MySQL libs are not installed.
check:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -fsyntax-only $(SOURCES)

clean:
	-rm -f $(OBJECTS) $(TARGET) $(TARGET).exe

.PHONY: all check clean
