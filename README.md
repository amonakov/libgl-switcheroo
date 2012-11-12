libgl-switcheroo
================

FUSE filesystem that pretends to contain `$LIB/libGL.so.1` symlinks. On
access, redirect to system libGL or primus' libGL based on user's response.
User choices are saved in `$XDG_CONFIG_HOME/libgl-switcheroo.conf`.


Usage
-----

Add the following lines into `~/.xprofile`:

    mkdir -p /tmp/libgl-switcheroo-$USER/fs
    gtkglswitch &
    libgl-switcheroo /tmp/libgl-switcheroo-$USER/fs &

If you are using KDM, GDM, or LXDM to login into an X session, the commands
from `~/.xprofile` are executed.  Otherwise, make sure that `.xprofile` is
sourced during X session startup (add `. ~/.xprofile` into your `.xinitrc`).

You can then run the following command in a shell session to make all programs
run from that session go through the libGL switcheroo (useful for Steam):

    export LD_LIBRARY_PATH=/tmp/libgl-switcheroo-$USER/fs/\$LIB${LD_LIBRARY_PATH+:}$LD_LIBRARY_PATH

Building
--------

    make LIB64PATH=\"lib\" LIB32PATH=\"lib32\"

Adjust `LIBxxPATH` variables above as appropriate for your distribution
(reflecting how `/usr/lib*` are named):

* Arch needs `lib` and `lib32` as above
* Gentoo needs `lib64` and `lib32`
* RPM-based may need `lib64` and `lib`
* Debian (with multiarch) needs `lib/x86_64-linux-gnu` and `lib/i386-linux-gnu`
* Ubuntu (with multiarch) needs `x86_64-linux-gnu` and `i386-linux-gnu`, and
  additionally `LIBPATH=\"/usr/lib\"`
