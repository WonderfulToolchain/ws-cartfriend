# CartFriend

WonderSwan flashcart launcher/utility. Work in progress.

## Supported cartridges

  * WS Flash Masta (currently in first slot only) - target `flash_masta`
  * Generic (no flashcart-specific features) - target `generic`

## Usage

* Y4/Y2 - change visible tab
* X1/X3 - navigate menu
* X2/X4 - change option values
* A - select
* B - go back (inside options), menu (outside of options)

### Browse

The "Browse" tab allows:

* Launching installed software (A),
* Verifying basic software information (B -> Info),
* Renaming software slots (B -> Rename).

### Tools

The "Tools" tab provides small tools useful for development:

* [WSMonitor](https://bitbucket.org/trap15/wsmonitor) - use the EXT port as a serial port to access a rudimentary monitor.

### Settings

The "Settings" tab allows configuring CartFriend.

* Hide slot IDs in menu - by default, CartFriend will label unnamed slots using their ID (publisher ID, game ID, game version, checksum). If this option is enabled, nothing will be shown for unnamed slots.
* Slot type config - change the type of a slot.
  * Launcher - the slot CartFriend was launched from; cannot be changed.
  * Soft - one program (takes up entire slot).
  * Multi(Linear)Soft - multiple <=1MB programs (in 1MB increments). Such a slot cannot make use of save data.
  * Unused - the slot is not used.
* Save block mapping - map available save blocks to Soft slots. This allows mapping mutliple blocks to one slot, allowing multiple
  distinct saves for one piece of software.
* Save data management - allows unloading save data from SRAM to Flash, as well as clearing save data for a given block.
* Advanced - advanced settings:
  * Buffered flash writes - enable faster flash writing.
  * Serial I/O rate - toggle the EXT serial port speed between 9600 and 38400 bps.
  * Force SRAM on next run - for the next software launched, ignore data in Flash - assume data in SRAM is this software's save data. 
  * Unlock IEEP next boot - enable to unlock the internal EEPROM on the next boot. This is useful for installing BootFriend and/or custom splashes.

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

