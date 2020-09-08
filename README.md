RV House
========

Lobby application for Re-Volt multiplayer games.

The source is licensed fully under GPL.  
See LICENSE for details.

Building
--------

For building, SCons (with Python 3) is needed:  
<http://www.scons.org/>

Copy *build_config.py.dist* to *build_config.py* and edit the file
to have correct paths etc. for your system.

Release build:  
just run `scons`

Debug build:  
run `scons debug=yes`

Windows build (cross-compiling):  
run `scons win32=yes`

When built, you can create an installation executable with Inno Setup 5 using 
*install/installer.iss* script.

Packages can also be created using the provided Makefile.

To create a Windows installer:  
`make win32_dist DIST_INNO=<path/to/inno/Compile32.exe>`

To create a Linux tarball:  
`make linux_dist`

For needed libraries and other installation details, see also:  
<http://rvhouse.revoltzone.net/index.php?section=sources>

Dependencies
------------

- **GCC** or MinGW compiler toolset is required (tested with 4.x and 9 series). 
  Only 32-bit compilation is currently supported and tested. You also need 
  pthread and zlib packages.

- **Qt4**: Get prebuilt binaries from your package manager (4.8 series). For Windows,
  MinGW binaries are [available here](https://download.qt.io/archive/qt/4.8/).

- **Boost** serialization 1.55: For compatibility, you need the 
  [1.55 version](https://www.boost.org/users/history/version_1_55_0.html) exactly. 
  Only serialization library is needed.

- **ACE**: Cross-platform network programming toolkit. Get the 
  [latest version](http://download.dre.vanderbilt.edu/) from 6.x series.

- **[KadC]**: Kademlia Distributed Hash Table C library. Get the `rv_house` branch.

- **[libdht]**: Distributed Hash Table abstraction library.

- **[libreudp]**: Resending UDP library.

- **[libnetcomgrp]**: Network communication group library.

[KadC]: https://github.com/gk7huki/KadC
[libdht]: https://github.com/gk7huki/libdht
[libreudp]: https://github.com/gk7huki/libreudp
[libnetcomgrp]: https://github.com/gk7huki/libnetcomgrp

Contact
-------

Arto Jalkanen (ajalkane@gmail.com)  
Huki (gk7huki@gmail.com)
