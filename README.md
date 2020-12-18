# PSX-Disc-FS - a PS1/PS2 Disc Filesystem Library

# Overview

The goal of this library is to provide high-level filesystem emulation for emulators that
rely on CD or DVD media, with a particular focus on PS1 and PS2 emulators. The project
provides an API to parse and enumerate the contents of PS1 and PS2 disc images, in a
similar manner to the Playstation BIOS itself.

CDROM and DVD media filesystems are typically following the ECMA-119 standard, though
some console BIOSes may have only supported a subset of this standard needed to ship
games cheaply and reduce BIOS code complexity.

### what the project is intended for

 - for BIOS HLE implementations in PS1 and PS2 emulators
 - for standalone utilities that perform static analysis and batch operations across
   a large collection of disc images

### what this project is not

This project is not intended to be any of the following:
 - a fully ECMA-119 compliant library

# Future Plans

 - Add support for modifying and re-writing filesystems