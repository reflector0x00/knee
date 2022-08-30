
BUSYBOX=busybox-1.35.0

if [ ! -d "busybox" ]; then

  if [ ! -f "$BUSYBOX.tar.bz2" ]; then
    wget https://busybox.net/downloads/$BUSYBOX.tar.bz2
  fi

  if [ ! -d "$BUSYBOX" ]; then
    tar -xf $BUSYBOX.tar.bz2
  fi

  mv $BUSYBOX busybox
fi

cp configs/busybox.config busybox/.config
sed -i "/CONFIG_CROSS_COMPILER_PREFIX=/c\CONFIG_CROSS_COMPILER_PREFIX=\"$PWD/toolchain/i686-elf/bin/i686-elf-\"" busybox/.config
sed -i "/CONFIG_EXTRA_LDFLAGS=/c\CONFIG_EXTRA_LDFLAGS=\"-L$PWD/libsys -lc -lgcc -lsys\"" busybox/.config