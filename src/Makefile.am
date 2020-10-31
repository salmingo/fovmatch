bin_PROGRAMS=fovmatch
fovmatch_SOURCES=ACatalog.cpp ACatTycho2.cpp MatchRefsys.cpp fovmatch.cpp

if DEBUG
  AM_CFLAGS = -g3 -O0 -Wall -DNDEBUG
  AM_CXXFLAGS = -g3 -O0 -Wall -DNDEBUG
else
  AM_CFLAGS = -O3 -Wall
  AM_CXXFLAGS = -O3 -Wall
endif

fovmatch_LDADD = -lm 
