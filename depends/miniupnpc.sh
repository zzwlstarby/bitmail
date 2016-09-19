#!/bin/bash
DepSrcRoot=$(dirname $(readlink -f $0) )
OutDir=$DepSrcRoot/../out

#==================================================
cd $DepSrcRoot
TARBALL="miniupnpc-1.9.20160209.tar.gz"
TAROPTS=zxvf
TARDEST="/tmp"
DESTDIR="/tmp/miniupnpc-1.9.20160209"
DOWNLINK="http://miniupnp.tuxfamily.org/files/miniupnpc-1.9.20160209.tar.gz"
DOWNOPTS=""

if ! [ -f "$TARBALL" ]; then
	echo "Downloading... $DOWNLINK"
	wget "$DOWNOPTS" "$DOWNLINK"
fi

if [ -f "$TARBALL" ]; then
	echo "Decompressing... $TARBALL "
	tar "$TAROPTS" "$TARBALL" -C "$TARDEST"
fi

if [ -d "$DESTDIR" ]; then
	cp -f Makefile.miniupnpc "$DESTDIR"/Makefile
	cd "$DESTDIR"
	make DESTDIR="$OutDir" INSTALLPREFIX="" install-static && make clean
fi