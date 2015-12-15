#!/usr/bin/python3

import sys
import os
import clang.cindex

class Class(object):
	def __init__(self, cursor):
		self.name = cursor.spelling
		self.public = []

		for c in cursor.get_children():
			if c.access_specifier == clang.cindex.AccessSpecifier.PUBLIC:
				self.public.append(c.spelling)

	def __str__(self):
		return self.name + ": " + ",".join(self.public)

if len(sys.argv) != 2:
	print("Usage: makogen.py [filename]")
	sys.exit()

clang.cindex.Config.set_library_file('/usr/local/lib/libclang.so')
index = clang.cindex.Index.create()

files = []

if os.path.isdir(sys.argv[1]):
	files = [os.path.join(sys.argv[1], f) for f in os.listdir(sys.argv[1])]
else:
	files.append(sys.argv[1])

cls = []

for f in files:
	translation_unit = index.parse(f, ['-x', 'c++', '-std=c++1y', '-DPLAYPG_MAKO_GEN__'])

	for c in translation_unit.cursor.get_children():
		if c.kind == clang.cindex.CursorKind.NAMESPACE and c.spelling == 'PlayPG':
			for cl in c.get_children():
				if cl.kind == clang.cindex.CursorKind.CLASS_DECL or cl.kind == clang.cindex.CursorKind.STRUCT_DECL:
					cls.append(Class(cl))

for c in cls:
	print(str(c))

