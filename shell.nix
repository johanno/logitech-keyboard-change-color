with import <nixpkgs> {};
pkgs.mkShell {
  buildInputs = with pkgs; [
    g15daemon
    libusb1.dev
    # c build tools
    gnumake42
    gdb
    pkg-config
    gnumake
    gcc
    libiconv
    autoconf
    automake
    libtool
  ];
}
