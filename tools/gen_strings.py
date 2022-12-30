#!/usr/bin/python3
#
# Copyright (c) 2020 Adrian Siekierka
#
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted.
#
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
# RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
# CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

from pathlib import Path
import glob, os, re, sys

properties = {}
property_langs = {}
property_idx = 0
property_keys = {}

for fn in glob.glob("lang//*.properties"):
	lang_key = Path(fn).stem
	with open(fn) as fp_i:
		for i in fp_i:
			property_langs[lang_key] = True
			i: str = i.rstrip("\n")
			if i.startswith("#"):
				continue
			if "=" in i:
				kv = i.split("=", maxsplit=1)
				if kv[0] not in properties:
					properties[kv[0]] = {}
				properties[kv[0]][lang_key] = kv[1]
				if lang_key == "en":
					property_keys[kv[0]] = property_idx
					property_idx += 1

with (
	open(sys.argv[2], "w") as fp_c,
	open(sys.argv[3], "w") as fp_h,
):
	hdr_define = '__%s__' % re.sub(r'[^a-zA-Z0-9]', '_', Path(sys.argv[3]).name).upper()

	print("// Auto-generated file. Please do not edit directly.\n", file = fp_c)
	print("#include <stdint.h>\n#include \"%s\"\n" % Path(sys.argv[3]).name, file = fp_c)
	print("// Auto-generated file. Please do not edit directly.\n", file = fp_h)
	print(f"#ifndef {hdr_define}\n#define {hdr_define}\n", file = fp_h)

	for k, v in sorted(property_keys.items(), key=lambda x: x[1]):
		print(f"#define LK_{k} {v}", file = fp_h)
	print(f"#define LK_TOTAL {property_idx}\n", file = fp_h)
	for k in property_langs.keys():
		print(f"extern const char __far* const __far lang_keys_{k}[{property_idx}];", file = fp_h)

	# Emit strings
	property_strings = {}
	property_string_idx = 0
	for k, vv in properties.items():
		for lang_key, v in vv.items():
			if v not in property_strings:
				print(f"const char __far lk_entry_{property_string_idx}[] = \"{v}\";", file = fp_c)
				property_strings[v] = property_string_idx
				property_string_idx += 1

	# Emit string arrays
	for lang_key in property_langs.keys():
		print(f"\nconst char __far* const __far lang_keys_{lang_key}[] = ", file = fp_c, end='')
		print("{", file = fp_c)
		for k, v in sorted(property_keys.items(), key=lambda x: x[1]):
			local_lang_key = lang_key
			if local_lang_key not in properties[k]:
				local_lang_key = "en"
			sv = properties[k][local_lang_key]
			print(f"\tlk_entry_{property_strings[sv]}, // {k}", file = fp_c)
		print("};", file = fp_c)

	print("\n#endif", file = fp_h);
