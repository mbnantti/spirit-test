TARGET = spirit-test
TEMPLATE = app

CONFIG += c++14
CONFIG -= qt

DEFINES += HAVE_GLIBC

QMAKE_CXXFLAGS += -Wall -Wextra -Werror
QMAKE_CXXFLAGS += -Wno-unused-variable -Wno-unused-but-set-variable
QMAKE_CXXFLAGS += -Wno-unused-function -Wno-unused-parameter
QMAKE_CXXFLAGS += -Wno-missing-field-initializers

INCLUDEPATH += /opt/maxutils/include
LIBS += -L/opt/maxutils/lib -lmaxbase -lmaxsql

INCLUDEPATH +=/usr/local/include/mariadb
LIBS += -L/usr/local/lib/mariadb/
LIBS += -lmariadbclient -lmariadb -lz -ldl -lm -lpthread -lssl -lcrypto -lcurl -lpcre2-8 -lpam

INCLUDEPATH += /usr/include/mariadb

SOURCES += main.cc \
    sqltest.cc

HEADERS += \
    sqltest.hh
