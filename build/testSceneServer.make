# GNU Make project makefile autogenerated by Premake
ifndef config
  config=debug
endif

ifndef verbose
  SILENT = @
endif

ifndef CC
  CC = gcc
endif

ifndef CXX
  CXX = g++
endif

ifndef AR
  AR = ar
endif

ifeq ($(config),debug)
  OBJDIR     = ../obj/Debug/testSceneServer
  TARGETDIR  = ../bin/Test
  TARGET     = $(TARGETDIR)/testSceneServer
  DEFINES   += -D_GLIBCXX_USE_CXX11_ABI=0 -D_ALL_SUPER_GM -D_SQL_DEBUG -D_DEBUG -DDEBUG -D_ROBOT_DEBUG
  INCLUDES  += -I.. -I/usr/local/include -I/usr/local/include/mysql -I/usr/local/mysql/include -I/usr/include -I/usr/include/log4cxx -I/usr/include/libxml2 -I/usr/include/mysql -I../SceneServer -I../base -I../base/xlib -I../base/xlib/Recast -I../base/xlib/Detour -I../base/protobuf -I../base/config -I../Command -I../Common
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -std=c++0x -g3 -Wall -fno-strict-aliasing -fno-short-enums -fno-schedule-insns -pg $(DEBUG_FLAG)
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -L/usr/local/lib -L/usr/local/mysql/lib -L/usr/lib -L/usr/lib/mysql -L/lib64 -L/usr/lib64 -L/usr/lib64/mysql -L../bin/Debug
  LIBS      += -lxml2 -lpthread -lmysqlclient -llog4cxx -lprotobuf -lhiredis -ljansson -ljemalloc -lSceneServer -lCommon -lbase -lgtest_main -lgtest
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += ../bin/Debug/libSceneServer.a ../bin/Debug/libCommon.a ../bin/Debug/libbase.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release)
  OBJDIR     = ../obj/Release/testSceneServer
  TARGETDIR  = ../bin/Test
  TARGET     = $(TARGETDIR)/testSceneServer
  DEFINES   += -D_GLIBCXX_USE_CXX11_ABI=0 -DNDEBUG
  INCLUDES  += -I.. -I/usr/local/include -I/usr/local/include/mysql -I/usr/local/mysql/include -I/usr/include -I/usr/include/log4cxx -I/usr/include/libxml2 -I/usr/include/mysql -I../SceneServer -I../base -I../base/xlib -I../base/xlib/Recast -I../base/xlib/Detour -I../base/protobuf -I../base/config -I../Command -I../Common
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -std=c++0x -g3 -Wall -O2 -fno-strict-aliasing -fno-short-enums -fno-schedule-insns -Wno-unknown-pragmas
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -L/usr/local/lib -L/usr/local/mysql/lib -L/usr/lib -L/usr/lib/mysql -L/lib64 -L/usr/lib64 -L/usr/lib64/mysql -L../bin/Release
  LIBS      += -lxml2 -lpthread -lmysqlclient -llog4cxx -lprotobuf -lhiredis -ljansson -ljemalloc -lSceneServer -lCommon -lbase -lgtest_main -lgtest
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += ../bin/Release/libSceneServer.a ../bin/Release/libCommon.a ../bin/Release/libbase.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),debug64)
  OBJDIR     = ../obj/x64/Debug/testSceneServer
  TARGETDIR  = ../bin/Test
  TARGET     = $(TARGETDIR)/testSceneServer
  DEFINES   += -D_GLIBCXX_USE_CXX11_ABI=0 -D_ALL_SUPER_GM -D_SQL_DEBUG -D_DEBUG -DDEBUG -D_ROBOT_DEBUG
  INCLUDES  += -I.. -I/usr/local/include -I/usr/local/include/mysql -I/usr/local/mysql/include -I/usr/include -I/usr/include/log4cxx -I/usr/include/libxml2 -I/usr/include/mysql -I../SceneServer -I../base -I../base/xlib -I../base/xlib/Recast -I../base/xlib/Detour -I../base/protobuf -I../base/config -I../Command -I../Common
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -m64 -std=c++0x -g3 -Wall -fno-strict-aliasing -fno-short-enums -fno-schedule-insns -pg $(DEBUG_FLAG)
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -m64 -L/usr/lib64 -L/usr/local/lib -L/usr/local/mysql/lib -L/usr/lib -L/usr/lib/mysql -L/lib64 -L/usr/lib64 -L/usr/lib64/mysql -L../bin/Debug
  LIBS      += -lxml2 -lpthread -lmysqlclient -llog4cxx -lprotobuf -lhiredis -ljansson -ljemalloc -lSceneServer -lCommon -lbase -lgtest_main -lgtest
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += ../bin/Debug/libSceneServer.a ../bin/Debug/libCommon.a ../bin/Debug/libbase.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release64)
  OBJDIR     = ../obj/x64/Release/testSceneServer
  TARGETDIR  = ../bin/Test
  TARGET     = $(TARGETDIR)/testSceneServer
  DEFINES   += -D_GLIBCXX_USE_CXX11_ABI=0 -DNDEBUG
  INCLUDES  += -I.. -I/usr/local/include -I/usr/local/include/mysql -I/usr/local/mysql/include -I/usr/include -I/usr/include/log4cxx -I/usr/include/libxml2 -I/usr/include/mysql -I../SceneServer -I../base -I../base/xlib -I../base/xlib/Recast -I../base/xlib/Detour -I../base/protobuf -I../base/config -I../Command -I../Common
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -m64 -std=c++0x -g3 -Wall -O2 -fno-strict-aliasing -fno-short-enums -fno-schedule-insns -Wno-unknown-pragmas
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -m64 -L/usr/lib64 -L/usr/local/lib -L/usr/local/mysql/lib -L/usr/lib -L/usr/lib/mysql -L/lib64 -L/usr/lib64 -L/usr/lib64/mysql -L../bin/Release
  LIBS      += -lxml2 -lpthread -lmysqlclient -llog4cxx -lprotobuf -lhiredis -ljansson -ljemalloc -lSceneServer -lCommon -lbase -lgtest_main -lgtest
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += ../bin/Release/libSceneServer.a ../bin/Release/libCommon.a ../bin/Release/libbase.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

OBJECTS := \
	$(OBJDIR)/SceneObjectTest.o \

RESOURCES := \

SHELLTYPE := msdos
ifeq (,$(ComSpec)$(COMSPEC))
  SHELLTYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(SHELL)))
  SHELLTYPE := posix
endif

.PHONY: clean prebuild prelink

all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)
	@:

$(TARGET): $(GCH) $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking testSceneServer
	$(SILENT) $(LINKCMD)
	$(POSTBUILDCMDS)

$(TARGETDIR):
	@echo Creating $(TARGETDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(TARGETDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(TARGETDIR))
endif

$(OBJDIR):
	@echo Creating $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR))
endif

clean:
	@echo Cleaning testSceneServer
ifeq (posix,$(SHELLTYPE))
	$(SILENT) rm -f  $(TARGET)
	$(SILENT) rm -rf $(OBJDIR)
else
	$(SILENT) if exist $(subst /,\\,$(TARGET)) del $(subst /,\\,$(TARGET))
	$(SILENT) if exist $(subst /,\\,$(OBJDIR)) rmdir /s /q $(subst /,\\,$(OBJDIR))
endif

prebuild:
	$(PREBUILDCMDS)

prelink:
	$(PRELINKCMDS)

ifneq (,$(PCH))
$(GCH): $(PCH)
	@echo $(notdir $<)
	-$(SILENT) cp $< $(OBJDIR)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
endif

$(OBJDIR)/SceneObjectTest.o: ../tests/SceneServer/SceneObjectTest.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
