#!/bin/sh
echo "[ Compiling 8x8 font ]"
python3 tools/font2raw.py res/font_default.png 8 8 a res/font_default.bin
python3 tools/bin2c.py --bank 3 res/font_default.c res/font_default.h res/font_default.bin
echo "[ Generating strings ]"
python3 tools/gen_strings.py lang res/lang.c res/lang.h
echo "[ Generating binary blobs ]"
python3 tools/bin2c.py res/wsmonitor.c res/wsmonitor.h thirdparty/wsmonitor.bin
