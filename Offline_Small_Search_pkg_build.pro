#-------------------------------------------------
#
# Project created by QtCreator 2015-12-13T12:17:27
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Offline_Small_Search_pkg_build
TEMPLATE = app

CONFIG += no_keywords c++11

SOURCES += main.cpp\
        opkg_build.cpp \
    build_thread.cpp \
    build.cpp \
    gumbo/attribute.c \
    gumbo/char_ref.c \
    gumbo/error.c \
    gumbo/parser.c \
    gumbo/string_buffer.c \
    gumbo/string_piece.c \
    gumbo/tag.c \
    gumbo/tokenizer.c \
    gumbo/utf8.c \
    gumbo/util.c \
    gumbo/vector.c \
    parse/htmlparse.cc \
    parse/myhtmlparse.cc \
    parse/stringutils.cc \
    parse/utf8convert.cc

HEADERS  += opkg_build.h \
    build_thread.h \
    build.h \
    gumbo/attribute.h \
    gumbo/char_ref.h \
    gumbo/error.h \
    gumbo/gumbo.h \
    gumbo/insertion_mode.h \
    gumbo/parser.h \
    gumbo/string_buffer.h \
    gumbo/string_piece.h \
    gumbo/token_type.h \
    gumbo/tokenizer.h \
    gumbo/tokenizer_states.h \
    gumbo/utf8.h \
    gumbo/util.h \
    gumbo/vector.h \
    parse/htmlparse.h \
    parse/myhtmlparse.h \
    parse/namedentities.h \
    parse/strcasecmp.h \
    parse/stringutils.h \
    parse/utf8convert.h

FORMS    += opkg_build.ui

DEPENDPATH += G:/build/i686/xapian-bin/include G:/build/i686/xapian-bin/include/xapian G:/build/i686/xapian-core-1.2.21/common G:/build/i686/zimlib-bin/include G:/build/i686/file-bin/include
INCLUDEPATH += G:/build/i686/xapian-bin/include G:/build/i686/xapian-bin/include/xapian G:/build/i686/xapian-core-1.2.21/common G:/build/i686/zimlib-bin/include G:/build/i686/file-bin/include
LIBS+=-LG:/msys32/usr/local/lib -LG:/msys32/mingw32/lib -LG:/build/i686/zimlib-bin/lib -LG:/build/i686/xapian-bin/lib -LG:/build/i686/file-bin/lib -lmagic -lxapian -lxapian.dll -lzim -llzma -lwinmm -lrpcrt4 -lz -lws2_32
