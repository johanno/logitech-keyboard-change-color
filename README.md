# Logitech G110 keyboard light changer for linux
A shell program for a logitech G110 keyboard light.

Usage:

    sudo ./Logitech_G110_change_color [color 0-255]
    Sets the color. If no color is given it loops them through.
may work without sudo. Didn't work for me.

build:

    make [debug]

dependencies can be seen in shell.nix
