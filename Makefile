DIST_BASENAME = rv_house
DIST_FILES = LICENSE README Makefile SConstruct build_config.py.dist build_support.py src build install dist_files
DIST_INNO = "$(HOME)/.wine/drive_c/Program Files (x86)/Inno Setup 5/Compil32.exe"
DIST_PATH = install.bin

default: release

clean: clean_release

all: release

clean_all: clean_release clean_debug

release: build_release

debug: build_debug

build_release:
	scons

build_debug:
	scons debug=yes

clean_release:
	scons -c

clean_debug:
	scons -c debug=yes

dist: clean_all
	mkdir -p dist_tmp/$(DIST_BASENAME) 
	cp -r $(DIST_FILES) dist_tmp/$(DIST_BASENAME)
	cd dist_tmp;tar "--exclude=.*" -cvf ../$(DIST_BASENAME).tar $(DIST_BASENAME)
	gzip $(DIST_BASENAME).tar
	rm -r dist_tmp

# Temporary solution
# TODO: to scons file generalized method of creating
# tar package of the binary distribution
linux_dist:
	#strip build/linux/Release/rv_house
	#upx build/linux/Release/rv_house
	rm -rf $(DIST_PATH)/rv_house
	mkdir -p $(DIST_PATH)/rv_house
	cp -rp dist_files/* $(DIST_PATH)/rv_house
	rm $(DIST_PATH)/rv_house/*.dll
	rm -rf $(DIST_PATH)/rv_house/imageformats
	cp -p dist_files/gpl.txt $(DIST_PATH)/rv_house
	cp -p build/linux/Release/rv_house $(DIST_PATH)/rv_house/rv_house.32
	cd $(DIST_PATH); tar -cvf rv_house_linux.tar --exclude '.*' rv_house; gzip -f rv_house_linux.tar
	rm -rf $(DIST_PATH)/rv_house

win32_dist:
	#strip build/win32/Release/rv_house.exe
	#upx --force build/win32/Release/rv_house.exe
	#start install/installer.iss
	$(DIST_INNO) /cc "install\installer.iss"
