# eCos Host-side Software

This `README` file only describes the eCos host-side software. For details of the eCos target-side software or the required toolchains, please see other documentation. A good starting point is [http://sourceware.org/ecos](http://sourceware.org/ecos).

There are two categories of host-side software. The host subdirectory contains generic software, primarily related to the eCos configuration technology. All eCos users will need to use some of this technology to configure and build eCos, either using pre-built binaries or by building the host-side software from source. The generic software should be portable to a wide range of host platforms.

There is also package-specific host-side software. Much of this is I/O related. For example the generic USB-slave package contains some programs related to testing; a test application is run on a target with suitable USB slave-side hardware, and needs to interact with another program running on the USB host; the latter is package-specific host-side software and can be found in the subdirectory `packages/io/usb/slave`. Code like this may have significant platform dependencies and may only work on a single platform or on a small number of platforms. There may also be special requirements, for example it may be necessary to install some programs suid root so that they have appropriate access to the hardware.

The host subdirectory includes the following:


```
    infra/
```



        This is an implementation of the eCos infrastructure that can be used on the host-side, and provides assertion, tracing and testcase support.


        NOTE: the eCos infrastructure facilities are not especially well-suited to host-side development, in particular they are not C++-oriented. There are plans to remove the current infrastructure completely and replace it with something more suitable. People planning new projects should be aware of this, and may wish to avoid using the current infrastructure.


```
    libcdl/
```



        The CDL library lies at the heart of the eCos configuration technology.


```
    tools/configtool/
```



        The sources to the various configuration tools can be found here.


```
    tools/configtool/common/common/
```



        Contains sources related to makefile generation, shared by the command line and graphical tools.


```
    tools/configtool/standalone/common/
```



        Contains the command line ecosconfig tool.


```
    tools/configtool/standalone/wxwin/
```



        Contains sources for the wxWindows-based, Linux and Windows graphical configuration tool. The Windows version is built with cygwin g++.


```
    tools/configtool/common/win32/
    tools/configtool/standalone/win32/
```



        Contains sources for the older, MFC-based, Windows-only graphical configuration tool. This can only be built with Visual C++.

The two graphical configuration tools have their own build procedures, described in [tools/configtool/standalone/wxwin/README.txt](https://github.com/hharte/ecos/blob/main/host/tools/configtool/standalone/wxwin/README.txt) and [tools/configtool/standalone/win32/ReadMe](https://github.com/hharte/ecos/blob/main/host/tools/configtool/standalone/win32/ReadMe) respectively.

Package-specific host-side code lives in the host subdirectory of the appropriate package, for example packages/io/usb/slave/&lt;version>/host. Most packages only provide target-side code and hence will not have a host subdirectory. Users can install various packages from a variety of sources, and where a package does have host-side software the package documentation should be consulted for further information.


# Installing on Linux, Other Unix Systems, and Cygwin

Both generic host-side software (`infra`, `libcdl` and `ecosconfig`) and package-specific software can be built with the conventional "`configure/make/make install`" sequence. However the code does not conform fully to GNU coding standards so some operations such as "`make dist`" are not supported. There is limited support for DejaGnu-based testing.

Much of the host-side software has a dependency on Tcl. This is not supplied with the sources since many users will already have a suitable installation, for example it is shipped as standard with all major Linux distributions. The generic host-side software should work with any release of Tcl from 8.0 onwards. The package-specific software requires a more recent version, 8.3 or later. If no suitable Tcl installation is available then the configure step will still succeed but some of the package-specific software will not be built.

There are two main approaches to building the host-side software:



1. build the generic and the package-specific code in one build tree. This uses the top-level `configure` script. The script automatically invokes the configure script in the main host subdirectory. In addition it searches the packages hierarchy for host subdirectories containing their own configure scripts and will invoke those.

    Note: the search for host subdirectories happens during configure time, not during the make. If new packages with host-side code are added to the repository then it will be necessary to re-run the toplevel configure script.

2. build the generic code in one build tree, using the configure script in the toplevel's host subdirectory. Then build some or all of the package-specific code in separate build trees, using the configure scripts in each package's host subdirectory.

The first approach is generally simpler. However some of the package-specific code requires special installation, for example a program may have to be installed suid root so that it has the right privileges to access hardware, and hence the "make install" step has to be run by the superuser. Also some of the package-specific code is rather specialized and may be of no interest to many users. For example, the USB testing code is only useful when developing USB-based applications. Hence some users may prefer the second approach, building just the generic code and a subset of the package-specific code.

It is necessary to use a separate build tree rather than build directly in the source tree. This is enforced by the configure scripts.


```
    $ mkdir build
    $ cd build
```


The next step is to run the desired configure script. To build all the host-side software this means the toplevel configure script:


```
    $ <path>/configure <args>
```


Alternatively to build just the generic host-side software, use the configure script in the host subdirectory.


```
    $ mkdir host
    $ cd host
    $ <path>/host/configure <args>
```


Or, to build just one package's host-side code:


```
    $ mkdir -p packages/io/usb/slave/current/host
    $ cd packages/io/usb/slave/current/host
    $ <path>/packages/io/usb/slave/current/host/configure <args>
```


(It is not actually necessary to use the same directory structure in the build tree as in the source tree, but doing so can avoid confusion).



A list of all the command-line options can be obtained by running "configure --help". The most important ones are as follows:



1. `--prefix`. This can be used to specify the location of the install tree, defaulting to /usr/local, so the ecosconfig program ends up in /usr/local/bin/ecosconfig and the CDL library ends up in /usr/local/lib/libcdl.a. If an alternative location is preferred this can be specified with --prefix, for example:

    ```
    $ <path>/configure --prefix=/usr/local/ecos <args>
    ```


2. `--enable-debug`. By default all assertions and tracing are disabled. When debugging any of the generic host-side software these should be enabled. Some package-specific code may not have any extra debug support, in which case --enable-debug would be ignored.

    ```
    $ <path>/configure --enable-debug
    ```



    It is also possible to control most of the assertion and tracing macros at a finer grain. This is intended mainly for use by the developers.


    `--disable-asserts`			disable all assertions


    `--disable-preconditions`		disable a subset of the assertions


    `--disable-postconditions`	disable a subset of the assertions


    `--disable-invariants`		disable a subset of the assertions


    `--disable-loopinvariants`	disable a subset of the assertions


    `--disable-tracing`			disable tracing


    `--disable-fntracing`		disable function entry/exit tracing

3. `--with-tcl=&lt;path>`

    ```
    --with-tcl-version=<number>
    --with-tk=<path>
    ```



    The host-side tools have a dependency on Tcl, which is not supplied with the sources because many people will already have a suitable installation. Specifically it is necessary to have the header file tcl.h and appropriate libraries such that -ltcl will work - this can involve either static or shared libraries. Some tools may require Tk as well as Tcl.


    Unfortunately there is considerable variation in Tcl installations. In theory all Tcl installations have a file tclConfig.sh which defines exactly how to compile and link code that uses Tcl, and Tk has a similar file tkConfig.sh. The eCos configure scripts look for these files, first in $(prefix)/lib, then in /usr/lib. If the system already has a Tcl installation in /usr then the configure script will automatically find /usr/lib/tclConfig.sh and it is not necessary to pass additional options when configuring the eCos host-side software. Alternatively, if for example you have installed a more recent version of Tcl/Tk in the same place that you want to install the eCos software, e.g. /usr/local, then $(prefix)/lib/tclConfig.sh will be read in.


    It is also possible that a more recent version of Tcl has been installed in a different location. For example, you may wish to install the eCos host tools in /opt/ecos but use a version of Tcl installed in /usr/local. The eCos configure scripts need to be told explicitly where to look for the Tcl:


        ```
        $ <path>/configure --with-tcl=/usr/local ...
        ```



    Some systems, for example Debian Linux 3.0, do not install tclConfig.sh in /usr/lib because that makes it more difficult to have several different versions of Tcl installed at the same time. Instead tclConfig.sh is found in a versioned directory such as /usr/lib/tcl8.3. Since several versions may be installed the desired one must be specified explicitly.


        ```
        $ <path>/configure --with-tcl-version=8.3
        ```



    The` --with-tcl` and `--with-tcl-version` options are combined to give a search path:


        ```
        <with-tcl>/lib/tclConfig.sh
        <with-tcl>/lib/tcl<vsn>/tclConfig.sh
        <prefix>/lib/tclConfig.sh
        <prefix>/lib/tcl<vsn>/tclConfig.sh
        /usr/lib/tclConfig.sh
        /usr/lib/tcl<vsn>/tclConfig.sh
        ```



    If `tclConfig.sh` cannot be found in any of these places then it is assumed that Tcl has not been properly installed and the eCos configure script will fail.


    To search for Tk the configure scripts use much the same rules as for Tcl. It is also possible to specify a path using the `--with-tk` option, useful if for some reason Tk has been installed in a different location from Tcl. There is no `--with-tk-version`: it is assumed that the same version should be used for both Tcl and Tk.


    Again, the configure scripts must be able to find `tkConfig.sh`


    Once tclConfig.sh and tkConfig.sh have been found and read in, the eCos configure scripts should have all the information needed to compile and link code that uses Tcl. First the location of key headers such as `&lt;tcl.h>` is needed. A `tclConfig.sh` file may define `TCL_INCLUDE_SPEC` or `TCL_INC_DIR` to give a specific location, otherwise the header files should be in `$(TCL_PREFIX)/include`. If `&lt;tcl.h>` cannot be found then the eCos configure scripts will fail.


    Next it is necessary to work out how to link applications with Tcl. This information should be provided by a `tclConfig.sh` variable `TCL_LIB_SPEC`. Unfortunately not all Tcl installations set this, for example the cygwin Tcl 8.4 release. If `TCL_LIB_SPEC` is not defined then instead the configure script will look for a library `libtcl&lt;vsn>.a`, where `&lt;vsn>` is specified using `--with-tcl-version`, then for a library libtcl.a. tclConfig.sh may also list additional libraries in `TCL_LIBS` and additional linker flags in `TCL_LD_FLAGS`.


    For Tk the relevant `tkConfig.sh` settings are `TK_INCLUDE_SPEC` or `TK_INC_DIR, TK_XINCLUDES`, `TK_LIB_SPEC`, and `TK_LIBS`.


Following the configure step the build tree should be set up correctly. All that remains is the actual build and install:


```
    $ make
    $ make install
```


This should result in an `ecosconfig` executable, plus appropriate libraries and header files. If the install prefix is a system location, for example `/usr/local/`, then "`make install`" will normally require root privileges. Also some of the package-specific software has special installation requirements, for example programs that need to be installed suid root, and this will also need root privileges.


# Installing with Visual C++

Under Windows it is possible to build the generic host-side software (`infra`, `libcdl` and `ecosconfig`) with Visual C++ but this is deprecated. Building with g++ under cygwin is preferred.

It is still necessary to run the `configure` script and a suitable `make` utility. That requires a shell and a Unix-like environment, as provided by cygwin. The Visual C++ compiler `cl.exe` needs to be on the shell's search path, and some environment variables such as `INCLUDE` and `LIB` may need to be set to point at the Visual C++ installation - the details may vary depending on the version of the compiler. Then the configure command should be run like this:


```
    $ CC=cl CXX=cl <path>/host/configure <args>
```


Note that the path should be a cygwin path: cygwin mount points are accepted and forward slashes should be used. The various configure scripts now detect that Visual C++ should be used, and adapt accordingly.

Depending on what cygwin mount points are set up, `/usr/local` may or may not be an appropriate install location for VC++ applications. If not, the install location should be specified with `--prefix`:


```
    $ CC=cl CXX=cl <path>/configure --prefix=<install-path> <args>
```


It is also necessary to use the right version of Tcl. For a VC++ build the cygwin release of Tcl should not be used. Instead a suitable prebuilt Tcl package can be obtained from [http://www.tcl.tk/](http://www.tcl.tk/). It is necessary to tell the `configure` script where this has been installed, for example:


```
    $ CC=cl CXX=cl <path>/configure --prefix=<install-path> \
      --with-tcl=/cygdrive/d/local/scriptics/Tcl/tcl8.1 <args>
```


The library name will be of the form `tcl81.lib`, and there will not be a symbolic link from `tcl.lib` to the appropriate version. It will be necessary to specify the Tcl version explicitly since the default version is currently 8.0.


```
    $ CC=cl CXX=cl <path>/configure --prefix=<install-path> \
      --with-tcl=/d/local/scriptics/Tcl/tcl8.1 --with-tcl-version=81 <args>
```


Following a successful `configure`, the tools can be built and installed in the normal fashion:


```
    $ make
    $ make install
```



# More Information

Please see the eCos web site, [http://sourceware.org/ecos/](http://sourceware.org/ecos/), for further details. This includes the FAQ, a form for reporting problems, and details of the various mailing lists ([https://ecos.sourceware.org/intouch.html](https://ecos.sourceware.org/intouch.html)) At the time of writing there are no separate mailing lists for the eCos host-side sources, the main mailing list `ecos-discuss@sourceware.com` should be used instead.

Copyright (C) 2000, 2001, 2002, 2009 Free Software Foundation, Inc.

This material may be distributed only subject to the terms and conditions set forth in the Open Publication License, v1.0 or later (the latest version is presently available at [http://www.opencontent.org/openpub/](http://www.opencontent.org/openpub/)) Distribution of the work or derivative of the work in any standard (paper) book form is prohibited unless prior permission obtained from the copyright holder.

