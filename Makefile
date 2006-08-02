DIST_BASENAME=rv_house
DIST_FILES=LICENSE README Makefile SConstruct build_config.py.dist build_support.py src build install dist_files

default: release

clean: clean_release

all: release debug

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

