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
  OBJDIR     = ../obj/Debug/libSessionServer
  TARGETDIR  = ../bin/Debug
  TARGET     = $(TARGETDIR)/libSessionServer.a
  DEFINES   += -D_GLIBCXX_USE_CXX11_ABI=0 -D_ALL_SUPER_GM -D_SQL_DEBUG -D_DEBUG -DDEBUG -D_ROBOT_DEBUG
  INCLUDES  += -I.. -I/usr/local/include -I/usr/local/include/mysql -I/usr/local/mysql/include -I/usr/include -I/usr/include/log4cxx -I/usr/include/libxml2 -I/usr/include/mysql -I../SessionServer -I../base -I../base/xlib -I../base/xlib/Recast -I../base/xlib/Detour -I../base/protobuf -I../base/config -I../Command -I../Common
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -std=c++0x -g3 -Wall -fno-strict-aliasing -fno-short-enums -fno-schedule-insns -pg $(DEBUG_FLAG)
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -L/usr/local/lib -L/usr/local/mysql/lib -L/usr/lib -L/usr/lib/mysql -L/lib64 -L/usr/lib64 -L/usr/lib64/mysql
  LIBS      += -lxml2 -lpthread -lmysqlclient -llog4cxx -lprotobuf -lhiredis -ljansson -ljemalloc
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += 
  LINKCMD    = $(AR) -rcs $(TARGET) $(OBJECTS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release)
  OBJDIR     = ../obj/Release/libSessionServer
  TARGETDIR  = ../bin/Release
  TARGET     = $(TARGETDIR)/libSessionServer.a
  DEFINES   += -D_GLIBCXX_USE_CXX11_ABI=0 -DNDEBUG
  INCLUDES  += -I.. -I/usr/local/include -I/usr/local/include/mysql -I/usr/local/mysql/include -I/usr/include -I/usr/include/log4cxx -I/usr/include/libxml2 -I/usr/include/mysql -I../SessionServer -I../base -I../base/xlib -I../base/xlib/Recast -I../base/xlib/Detour -I../base/protobuf -I../base/config -I../Command -I../Common
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -std=c++0x -g3 -Wall -O2 -fno-strict-aliasing -fno-short-enums -fno-schedule-insns -Wno-unknown-pragmas
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -L/usr/local/lib -L/usr/local/mysql/lib -L/usr/lib -L/usr/lib/mysql -L/lib64 -L/usr/lib64 -L/usr/lib64/mysql
  LIBS      += -lxml2 -lpthread -lmysqlclient -llog4cxx -lprotobuf -lhiredis -ljansson -ljemalloc
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += 
  LINKCMD    = $(AR) -rcs $(TARGET) $(OBJECTS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),debug64)
  OBJDIR     = ../obj/x64/Debug/libSessionServer
  TARGETDIR  = ../bin/Debug
  TARGET     = $(TARGETDIR)/libSessionServer.a
  DEFINES   += -D_GLIBCXX_USE_CXX11_ABI=0 -D_ALL_SUPER_GM -D_SQL_DEBUG -D_DEBUG -DDEBUG -D_ROBOT_DEBUG
  INCLUDES  += -I.. -I/usr/local/include -I/usr/local/include/mysql -I/usr/local/mysql/include -I/usr/include -I/usr/include/log4cxx -I/usr/include/libxml2 -I/usr/include/mysql -I../SessionServer -I../base -I../base/xlib -I../base/xlib/Recast -I../base/xlib/Detour -I../base/protobuf -I../base/config -I../Command -I../Common
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -m64 -std=c++0x -g3 -Wall -fno-strict-aliasing -fno-short-enums -fno-schedule-insns -pg $(DEBUG_FLAG)
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -m64 -L/usr/lib64 -L/usr/local/lib -L/usr/local/mysql/lib -L/usr/lib -L/usr/lib/mysql -L/lib64 -L/usr/lib64 -L/usr/lib64/mysql
  LIBS      += -lxml2 -lpthread -lmysqlclient -llog4cxx -lprotobuf -lhiredis -ljansson -ljemalloc
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += 
  LINKCMD    = $(AR) -rcs $(TARGET) $(OBJECTS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release64)
  OBJDIR     = ../obj/x64/Release/libSessionServer
  TARGETDIR  = ../bin/Release
  TARGET     = $(TARGETDIR)/libSessionServer.a
  DEFINES   += -D_GLIBCXX_USE_CXX11_ABI=0 -DNDEBUG
  INCLUDES  += -I.. -I/usr/local/include -I/usr/local/include/mysql -I/usr/local/mysql/include -I/usr/include -I/usr/include/log4cxx -I/usr/include/libxml2 -I/usr/include/mysql -I../SessionServer -I../base -I../base/xlib -I../base/xlib/Recast -I../base/xlib/Detour -I../base/protobuf -I../base/config -I../Command -I../Common
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -m64 -std=c++0x -g3 -Wall -O2 -fno-strict-aliasing -fno-short-enums -fno-schedule-insns -Wno-unknown-pragmas
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -m64 -L/usr/lib64 -L/usr/local/lib -L/usr/local/mysql/lib -L/usr/lib -L/usr/lib/mysql -L/lib64 -L/usr/lib64 -L/usr/lib64/mysql
  LIBS      += -lxml2 -lpthread -lmysqlclient -llog4cxx -lprotobuf -lhiredis -ljansson -ljemalloc
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += 
  LINKCMD    = $(AR) -rcs $(TARGET) $(OBJECTS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

OBJECTS := \
	$(OBJDIR)/ActivityManager.o \
	$(OBJDIR)/Auction.o \
	$(OBJDIR)/Authorize.o \
	$(OBJDIR)/Boss.o \
	$(OBJDIR)/ChatManager_SE.o \
	$(OBJDIR)/FerrisWheelManager.o \
	$(OBJDIR)/GMCommandManager.o \
	$(OBJDIR)/GuildGmMgr.o \
	$(OBJDIR)/Mail.o \
	$(OBJDIR)/MailTemplateManager.o \
	$(OBJDIR)/ProcessCmd.o \
	$(OBJDIR)/ProcessSceneCmd.o \
	$(OBJDIR)/SealManager.o \
	$(OBJDIR)/SessionActivityMgr.o \
	$(OBJDIR)/SessionGvg.o \
	$(OBJDIR)/SessionScene.o \
	$(OBJDIR)/SessionSceneManager.o \
	$(OBJDIR)/SessionServer.o \
	$(OBJDIR)/SessionThread.o \
	$(OBJDIR)/SessionUser.o \
	$(OBJDIR)/SessionUserCmd.o \
	$(OBJDIR)/SessionUserManager.o \
	$(OBJDIR)/SessionWeddingMgr.o \
	$(OBJDIR)/ShopMgr.o \
	$(OBJDIR)/TimerM.o \
	$(OBJDIR)/TimeTick.o \
	$(OBJDIR)/TowerManager.o \
	$(OBJDIR)/TradeLog.o \
	$(OBJDIR)/UserActive.o \
	$(OBJDIR)/WorldLevelManager.o \

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
	@echo Linking libSessionServer
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
	@echo Cleaning libSessionServer
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

$(OBJDIR)/ActivityManager.o: ../SessionServer/ActivityManager.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Auction.o: ../SessionServer/Auction.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Authorize.o: ../SessionServer/Authorize.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Boss.o: ../SessionServer/Boss.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/ChatManager_SE.o: ../SessionServer/ChatManager_SE.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/FerrisWheelManager.o: ../SessionServer/FerrisWheelManager.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/GMCommandManager.o: ../SessionServer/GMCommandManager.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/GuildGmMgr.o: ../SessionServer/GuildGmMgr.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/Mail.o: ../SessionServer/Mail.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/MailTemplateManager.o: ../SessionServer/MailTemplateManager.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/ProcessCmd.o: ../SessionServer/ProcessCmd.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/ProcessSceneCmd.o: ../SessionServer/ProcessSceneCmd.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SealManager.o: ../SessionServer/SealManager.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SessionActivityMgr.o: ../SessionServer/SessionActivityMgr.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SessionGvg.o: ../SessionServer/SessionGvg.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SessionScene.o: ../SessionServer/SessionScene.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SessionSceneManager.o: ../SessionServer/SessionSceneManager.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SessionServer.o: ../SessionServer/SessionServer.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SessionThread.o: ../SessionServer/SessionThread.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SessionUser.o: ../SessionServer/SessionUser.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SessionUserCmd.o: ../SessionServer/SessionUserCmd.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SessionUserManager.o: ../SessionServer/SessionUserManager.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/SessionWeddingMgr.o: ../SessionServer/SessionWeddingMgr.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/ShopMgr.o: ../SessionServer/ShopMgr.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/TimerM.o: ../SessionServer/TimerM.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/TimeTick.o: ../SessionServer/TimeTick.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/TowerManager.o: ../SessionServer/TowerManager.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/TradeLog.o: ../SessionServer/TradeLog.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/UserActive.o: ../SessionServer/UserActive.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/WorldLevelManager.o: ../SessionServer/WorldLevelManager.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
