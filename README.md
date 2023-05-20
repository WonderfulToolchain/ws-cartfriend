# CartFriend

WonderSwan flashcart launcher/utility. Work in progress.

## Supported cartridges

  * WS Flash Masta (currently in first slot only) - target `flash_masta`
  * Generic (no flashcart-specific features) - target `generic`

## Build instructions

Requirements:

* [The Wonderful toolchain](https://wonderful.asie.pl/doc/general/getting-started/),
* Recent version of Python 3 and the Pillow library.

1. Install the WSwan target tools: `wf-pacman -S target-wswan`.
2. Build the assets: `./build_assets.sh`.
3. Build the ROM: `make TARGET=target` - see "Supported cartridges" for valid target names.

## Licensing

The source code as a whole is available under GPLv3 or later; however, some files (in particular, flashcart platform drivers and XMODEM transfer logic) are available under the zlib license to faciliate reuse in other homebrew projects - check the source file header to make sure!

**Warning:** The primary purpose of this tool is to aid homebrew development with Wonderful and other toolchains, as well as allow smooth execution of WonderSwan homebrew by end users. While the utility is provided "as-is" and without warranty, the development team reserves the right to deny support to users where doing so would unavoidably aid in copyright infringement.

