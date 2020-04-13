TARGET = spirit-test
TEMPLATE = app

CONFIG += c++14
CONFIG -= qt

DEFINES += HAVE_GLIBC

QMAKE_CXXFLAGS += -Wall -Wextra -Werror
QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-but-set-variable
QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-missing-field-initializers

SOURCES += main.cc \
    sqltest.cc

HEADERS += \
    sqltest.hh
