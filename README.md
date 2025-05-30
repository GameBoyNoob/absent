# absent
Simple and flexible X tiling window manager

[Video example](https://youtu.be/6glkO97ToIY)

# Installation

Dependencies

```console
gcc, make, xcb, xcb-util, xcb-proto, xcb-util-keysyms, xcb-util-cursor, xcb-util-wm (xcb-util-icccm), xkbcommon, xcb-cursors, xcb-randr
```

Install window manager

- clone the repository 

```console
git clone https://github.com/speckitor/absent
```

- compile wm with make

```console
make
```

- copy `absent`, `autostartabsent` and `absent.desktop` files to `/usr/local/bin` and `/usr/share/xsessions/`

```console
sudo make copy
```

Removing window manager

- to remove all binaries and session file run

```console
sudo make clean
```

# Configuration
The `config.h` file contains all configuration. The `autostartabsent` file contains commands that are executed when the window manager starts (can be disabled in `config.h` or modified). After editing these files you need to recompile wm.
