# Building RedBoot for the Intel ixp4xx


# Introduction

[RedBoot](https://ecos.sourceware.org/docs-3.0/redboot-guide/redboot-guide.html) is an embedded application running in the [eCos Real-Time Operating System](https://ecos.sourceware.org/) (RTOS) that provides bootloader functionality to a wide variety of platforms.  It is often used as the bootloader for boards using the Intel IXP-4xx Xscale CPU.

This repository consists of the original eCos source code imported into git from CVS using [Migrate CVS repositories to Subversion or Git](https://github.com/mhagger/cvs2svn).

The Intel [IXDP425 Development Platform](https://www.swd.ru/files/pdf/start/IXDP425.pdf) is currently supported.


# Building from Source

This eCos repository has been updated to work with recent versions of the GNU C Compiler available as part of [OpenWrt](https://openwrt.org/).  As of this writing, gcc 12.1 is supported.


## Build OpenWrt to obtain GCC Cross Compiler

[Obtain OpenWrt](https://openwrt.org/docs/guide-developer/start#using_the_toolchain) and build for Intel Xscale to obtain the required GCC cross compiler.


## Install Host Prerequisites:


```
    $ sudo apt-get install tcl tk tk-dev tcl-dev
```



## Clone eCos Repository


```
    $ cd ~/src
    $ git clone https://github.com/hharte/ecos.git
    $ cd ecos/
```



## Configure the Environment


```
    $ export OPENWRT_PATH=<path_to_openwrt>
    $ export ECOS_REPOSITORY=~/src/ecos/packages/
    $ export PATH=$PATH:$OPENWRT_PATH/staging_dir/toolchain-armeb_xscale_gcc-12.3.0_musl/bin
    $ export STAGING_DIR=$OPENWRT_PATH/staging_dir/toolchain-armeb_xscale_gcc-12.3.0_musl/
```



## Build eCos Host Tools


```
    $ mkdir build-host
    $ cd build-host
    $ ../configure
    $ make && sudo make install
    $ cd ..
```


Tools are now in: `/usr/local/bin/`.


## Build RedBoot for the Intel IXDP425


```
    $ mkdir build-ROM
    $ cd build-ROM
    $ ecosconfig new ixdp425 redboot
    $ ecosconfig import $ECOS_REPOSITORY/hal/arm/xscale/ixdp425/current/misc/redboot_ROM.ecm
    $ ecosconfig tree
    $ make -j
```


When the build completes, redboot will be in the `install/bin` directory.  Four files are built:


<table>
  <tr>
   <td><code>redboot.bin</code>
   </td>
   <td>Binary file suitable for flashing with JTAG or programming with a device programmer.
   </td>
  </tr>
  <tr>
   <td><code>redboot.img</code>
   </td>
   <td>Redboot ELF binary, stripped of debug information.  This can be downloaded by RedBoot itself.
   </td>
  </tr>
  <tr>
   <td><code>redboot.elf</code>
   </td>
   <td>Full RedBoot ELF binary, useful with <code>gdb</code>.
   </td>
  </tr>
  <tr>
   <td><code>redboot.srec</code>
   </td>
   <td>S-Record file useful with some device programmers.
   </td>
  </tr>
</table>



# FLASHing to the Intel IXDP425 Development Board


## FLASH with JTAG using OpenOCD

It is possible to program the IXDP425 via JTAG using [OpenOCD](https://openocd.org/).


## FLASH with a Device Programmer

The [28F128J3](https://media-www.micron.com/-/media/client/global/documents/products/data-sheet/nor-flash/parallel/j3/316577_j3d_monolithic_ds.pdf?rev=77975b59947145aab439595289cf09bd) 16MB NOR FLASH device on the ixdp425 Development Board is socketed, so it can be programmed in an external device programmer.  I used an [XGecu T56 Programmer](https://xgecu.myshopify.com/collections/xgecu-t56-programmer) available on [Amazon](https://www.amazon.com/XGecu-Universal-Programmer-Drivers-Adapters/dp/B086K35WZS) that comes with the required 56-pin TSOP adapter.

After loading the redboot.bin file, convert to big-endian by choosing File->Fill Block/Swap(F).


# References


<table>
  <tr>
   <td><a href="https://openwrt.org/docs/techref/targets/ixp4xx">ixp4xx Devices on OpenWrt</a>
   </td>
  </tr>
  <tr>
   <td><a href="https://dflund.se/~triad/krad/ixp4xx/">Recent Linux on Intel IXP4xx Systems</a>
   </td>
  </tr>
  <tr>
   <td><a href="http://h-wrt.com/en/mini-how-to/ecos">About Ecos building</a>
   </td>
  </tr>
  <tr>
   <td><a href="https://openwrt.org/toh/actiontec/mi424wr#building_redboot">Building RedBoot</a>
   </td>
  </tr>
  <tr>
   <td><a href="https://openwrt.org/docs/guide-developer/toolchain/crosscompile">[OpenWrt Wiki] Cross compiling</a>
   </td>
  </tr>
  <tr>
   <td><a href="http://ecos.sourceware.org/ecos/anoncvs.html">Original eCos source code (CVS)</a>
   </td>
  </tr>
  <tr>
   <td><a href="https://hg-pub.ecoscentric.com/">Original eCos source code (Mercurial)</a>
   </td>
  </tr>
</table>



# Help Needed

The following resources are currently not available online anymore, and I would love to obtain them.  If you have any of these, please let me know.


<table>
  <tr>
   <td>Item
   </td>
   <td>Details
   </td>
  </tr>
  <tr>
   <td>Intel RedBoot 2.04 Source Code
   </td>
   <td>Original filename: <a href="https://web.archive.org/web/20070918115755/http://www.intel.com/design/network/products/npfamily/download_ixp400.htm">redboot-intel-ixp4xx-070320.tar.gz</a>
   </td>
  </tr>
  <tr>
   <td>Redboot 2.04 pre-built binaries for IXP4XX-based platforms
   </td>
   <td>Original filename: <a href="https://web.archive.org/web/20070918115755/http://www.intel.com/design/network/products/npfamily/download_ixp400.htm">Redboot-v2_04-npe_bins-070320.tar.gz</a>
   </td>
  </tr>
  <tr>
   <td>Intel® IXDP425 / IXCDP1100 Development Platform Quick Start Guide
   </td>
   <td>Document Number: 253177
   </td>
  </tr>
  <tr>
   <td>Intel® IXDP425 / IXCDP1100 Development Platform User’s Guide
   </td>
   <td>Document Number: 273743
   </td>
  </tr>
  <tr>
   <td>Intel® IXDP425 / IXCDP1100 Development Platform Boot-Loader Flash Conversion Guide
   </td>
   <td>Document Number:  253201
   </td>
  </tr>
  <tr>
   <td>Intel® IXDP425 / IXCDP1100 Development Platform Schematics
   </td>
   <td>
   </td>
  </tr>
  <tr>
   <td>Intel® IXDP425 Network Processor Development Platform Upgrade White Paper
   </td>
   <td>Document Number: 252779
   </td>
  </tr>
</table>
