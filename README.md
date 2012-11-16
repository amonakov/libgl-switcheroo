libgl-switcheroo
================

LD_AUDIT-based libGL.so.1 switching.

User choices are saved in `$XDG_CONFIG_HOME/libgl-switcheroo.conf`.


Usage
-----

Add the following lines into `~/.xprofile`:

    mkdir /tmp/libgl-switcheroo-$USER
    gtkglswitch &

If you are using KDM, GDM, or LXDM to login into an X session, the commands
from `~/.xprofile` are executed.  Otherwise, make sure that `.xprofile` is
sourced during X session startup (add `. ~/.xprofile` into your `.xinitrc`).

You can then run the following command in a shell session to make all programs
run from that session go through the libGL switcheroo (useful for Wine):

    export LD_AUDIT=/path/to/libgl-switcheroo/\$LIB/libgl-switcheroo.so
    wine steam

Building
--------

    LIBDIR=lib make && CC=gcc\ -m32 LIBDIR=lib32 make

Adjust `LIBDIR` variables above as appropriate for your distribution
(reflecting how `/usr/lib*` are named):

* Arch needs `lib` and `lib32` as above
* Gentoo needs `lib64` and `lib32`
* RPM-based may need `lib64` and `lib`
* Debian (with multiarch) needs `lib/x86_64-linux-gnu` and `lib/i386-linux-gnu`
* Ubuntu (with multiarch) needs `x86_64-linux-gnu` and `i386-linux-gnu`
