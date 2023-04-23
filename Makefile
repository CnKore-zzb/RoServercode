#######################################
# file   : makefile for sssg
# author : sunhuiwei
#######################################

debug: premake4 config
	cd ./build; config=debug $(MAKE)

release: premake4 config-release
	cd ./build; config=release $(MAKE)

studio: premake4 config-studio
	cd ./build; config=debug $(MAKE)

banshu: premake4 config-banshu
	cd ./build; config=debug $(MAKE)

publish: premake4 config-publish
	cd ./build; config=debug $(MAKE)

tf: premake4 config-tf
	cd ./build; config=debug $(MAKE)

premake4: premake4.lua
	premake4 --platform=x64 --file=premake4.lua gmake

clean:
	cd ./build; $(MAKE) clean

config:
	mkdir -p bin/Debug
	./update_resource.sh Debug client-trunk
	cp -au ./Config/* ./bin/Debug
	cp -au ./trade ./bin/Debug/
	rm -fr bin/Debug/map/.svn
	rm -fr bin/Debug/sql_log/.svn
	rm -fr bin/Debug/sql_main/.svn
	rm -fr bin/Debug/sql_main/platform/.svn

config-studio:
	mkdir -p bin/Debug
	./update_resource.sh Debug Studio
	cp -au ./Config/* ./bin/Debug
	cp -au ./trade ./bin/Debug/
	rm -fr bin/Debug/map/.svn
	rm -fr bin/Debug/sql_log/.svn
	rm -fr bin/Debug/sql_main/.svn
	rm -fr bin/Debug/sql_main/platform/.svn

config-banshu:
	mkdir -p bin/Debug
	./update_resource.sh Debug banshu
	cp -au ./Config/* ./bin/Debug
	cp -au ./trade ./bin/Debug/
	rm -fr bin/Debug/map/.svn
	rm -fr bin/Debug/sql_log/.svn
	rm -fr bin/Debug/sql_main/.svn
	rm -fr bin/Debug/sql_main/platform/.svn

config-publish:
	mkdir -p bin/Debug
	./update_resource.sh Debug Release_New
	cp -au ./Config/* ./bin/Debug
	cp -au ./trade ./bin/Debug/
	rm -fr bin/Debug/map/.svn
	rm -fr bin/Debug/sql_log/.svn
	rm -fr bin/Debug/sql_main/.svn
	rm -fr bin/Debug/sql_main/platform/.svn

config-tf:
	mkdir -p bin/Debug
	cp -au ./Config/* ./bin/Debug
	cp -au ./trade ./bin/Debug/
	rm -fr bin/Debug/map/.svn
	rm -fr bin/Debug/sql_log/.svn
	rm -fr bin/Debug/sql_main/.svn
	rm -fr bin/Debug/sql_main/platform/.svn

config-release:
	mkdir -p bin/Release
	cp -au ./Config/* ./bin/Release
	cp -au ./libso/* ./bin/Release
	cp -au ./trade ./bin/Release/
	rm -fr bin/Release/map/.svn
	rm -fr bin/Release/sql_log/.svn
	rm -fr bin/Release/sql_main/.svn
	rm -fr bin/Release/sql_main/platform/.svn

proto:
	./protobuf

tag:
	rm tags
	@ctags -R --c++-kinds=+p --fields=+iaS --extra=+q

touchserver:
	touch base/xlib/xServer.cpp

touch:
		rm -rf ./obj/*
		find ./ | xargs touch

define linksymbol
	cd $1;\
	rm -f $2.symbol;\
	objcopy --only-keep-debug $2 $2.symbol;\
	objcopy --strip-debug $2;\
	objcopy --add-gnu-debuglink=$2.symbol $2;
endef
symbols: touchserver release
	$(call linksymbol,./bin/Release,ProxyServer)
	$(call linksymbol,./bin/Release,SceneServer)
	$(call linksymbol,./bin/Release,SessionServer)
	$(call linksymbol,./bin/Release,RecordServer)
	$(call linksymbol,./bin/Release,GateServer)
	$(call linksymbol,./bin/Release,SuperServer)
	$(call linksymbol,./bin/Release,SocialServer)
	$(call linksymbol,./bin/Release,TeamServer)
	$(call linksymbol,./bin/Release,GlobalServer)
	$(call linksymbol,./bin/Release,GZoneServer)
	$(call linksymbol,./bin/Release,StatServer)
	$(call linksymbol,./bin/Release,TradeServer)
	$(call linksymbol,./bin/Release,MatchServer)
	$(call linksymbol,./bin/Release,GuildServer)
	$(call linksymbol,./bin/Release,AuctionServer)
	$(call linksymbol,./bin/Release,WeddingServer)
	$(call linksymbol,./bin/Release,Robots)

VERSION:=$(version)
tarball: symbols
	cd ./bin/Release; tar czf ../ro-$(VERSION).tar.gz --exclude-vcs --exclude='lib' *

base:
	cd ./build; $(MAKE) base
sceneserver:
	cd ./build; $(MAKE) SceneServer
sessionserver:
	cd ./build; $(MAKE) SessionServer
recordserver:
	cd ./build; $(MAKE) RecordServer
gateserver:
	cd ./build; $(MAKE) GateServer
superserver:
	cd ./build; $(MAKE) SuperServer
proxyserver:
	cd ./build; $(MAKE) ProxyServer
guildserver:
	cd ./build; $(MAKE) GuildServer
socialserver:
	cd ./build; $(MAKE) SocialServer
teamserver:
	cd ./build; $(MAKE) TeamServer
matchserver:
	cd ./build; $(MAKE) MatchServer
tradeserver:
	cd ./build; $(MAKE) TradeServer
auctionserver:
	cd ./build; $(MAKE) AuctionServer
weddingserver:
	cd ./build; $(MAKE) WeddingServer
gzone:
	cd ./build; $(MAKE) GZoneServer
stat:
	cd ./build; $(MAKE) StatServer
dataserver:
	cd ./build; $(MAKE) DataServer
robots:
	cd ./build; $(MAKE) Robots
test: base sceneserver
	cd ./build; $(MAKE) testbase
	cd ./build; $(MAKE) testSceneServer
runtest: test
	cd ./bin/Test; ./testbase
	cd ./bin/Test; ./testSceneServer

