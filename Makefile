DIST_BASENAME=rv_house
DIST_FILES=LICENSE README Makefile SConstruct build_config.py.dist build_support.py src build install dist_files

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
linux_bin_dist:
	strip build/linux2/Release/rv_house
	mkdir -p linux_bin_dist/rv_house
	cp -rp dist_files/* linux_bin_dist/rv_house
	rm linux_bin_dist/rv_house/pthreadGC.dll
	cp -p build/linux2/Release/rv_house linux_bin_dist/rv_house
	cd linux_bin_dist; tar -cvf rv_house_linux.tar --exclude '.*' rv_house; gzip rv_house_linux.tar
