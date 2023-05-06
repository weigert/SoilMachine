# soillib
# Author: Nicholas McDonald
# Version 1.0
# Tested on GNU/Linux and MacOS

# Install Location

LIBPATH = $(HOME)/.local/lib
INCPATH = $(HOME)/.local/include

# Compiler Settings

CC = g++-10 -std=c++20
CF = -Wfatal-errors -O2

TINYLINK = -lX11 -lpthread -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lGL -lGLEW -lboost_system -lboost_filesystem

# OS Specific Configuration

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)			# Detect GNU/Linux
LIBPATH = $(HOME)/.local/lib
INCPATH = $(HOME)/.local/include
endif

ifeq ($(UNAME), Darwin)			# Detect MacOS

INCPATH = /opt/homebrew/include
LIBPATH = /opt/homebrew/lib

CC = g++-12 -std=c++20

endif

# Compile Soilmachine


all: SoilMachine.cpp
			$(CC) -L$(LIBPATH) -I$(INCPATH) SoilMachine.cpp $(CF) -lTinyEngine $(TINYLINK) -o soilmachine
