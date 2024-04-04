all: Logitech_G110_change_color

Logitech_G110_change_color::
	gcc -o Logitech_G110_change_color Logitech_G110_change_color.c -lusb-1.0

debug::
	gcc -g -o Logitech_G110_change_color_debug Logitech_G110_change_color.c -lusb-1.0



