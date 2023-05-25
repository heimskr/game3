## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2002 lignum Computing, Inc. <oglft@lignumcomputing.com>
## $Id: Makefile.am,v 1.9 2003/10/01 14:07:53 allen Exp $
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 59 Temple Place, Suite 30, Boston, MA 02111-1307, USA.


if OGLFT_NO_QT
INCLUDES = -I$(top_srcdir)/liboglft
check_PROGRAMS = demo demo2 demo4 tutorial1 tutorial2 tutorial3
else
INCLUDES = -I$(top_srcdir)/liboglft -I${QTDIR}/include
check_PROGRAMS = demo demo2 demo3 demo4 tutorial1 tutorial2 tutorial3 speedtest
endif

demo_SOURCES = demo.cpp
demo2_SOURCES = demo2.cpp
demo3_SOURCES = demo3.cpp vignette.h Demo3UnicodeExample.h
demo4_SOURCES = demo4.cpp
nodist_demo3_SOURCES = moc_vignette.cpp 
tutorial1_SOURCES = tutorial1.cpp
tutorial2_SOURCES = tutorial2.cpp
tutorial3_SOURCES = tutorial3.cpp
speedtest_SOURCES = speedtest.cpp speedtest.h
nodist_speedtest_SOURCES = moc_speedtest.cpp

LDADD = $(top_builddir)/liboglft/libOGLFT.la

speedtest_LDADD = $(LDADD) -L${QTDIR}/lib @QTLIB@
demo_LDADD = $(LDADD) -lglut -L/usr/X11R6/lib -lXi -lXmu
demo2_LDADD = $(LDADD) -lglut -L/usr/X11R6/lib -lXi -lXmu
demo3_LDADD = $(LDADD) -L${QTDIR}/lib @QTLIB@
demo4_LDADD = $(LDADD) -lglut -L/usr/X11R6/lib -lXi -lXmu
tutorial1_LDADD = $(LDADD) -lglut -L/usr/X11R6/lib -lXi -lXmu
tutorial2_LDADD = $(LDADD) -lglut -L/usr/X11R6/lib -lXi -lXmu
tutorial3_LDADD = $(LDADD) -lglut -L/usr/X11R6/lib -lXi -lXmu

TESTS =

EXTRA_DIST = background00b.png background01.png background10b.png background11.png \
texture.png

MOSTLYCLEANFILES = moc_speedtest.cpp moc_vignette.cpp

moc_speedtest.cpp: speedtest.h
	${QTDIR}/bin/moc -o moc_speedtest.cpp $(top_srcdir)/tests/speedtest.h
moc_vignette.cpp: vignette.h
	${QTDIR}/bin/moc -o moc_vignette.cpp $(top_srcdir)/tests/vignette.h
