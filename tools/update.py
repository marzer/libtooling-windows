#!/usr/bin/env python3

import shutil
import os
from argparse import ArgumentParser
from pathlib import Path
from misk import *



def main():
	args = ArgumentParser(description='Makes a self-contained clang libtooling distribution for windows builds.')
	args.add_argument('--llvm', type=Path, default=None)
	args.add_argument('--build32', type=Path, default=None)
	args.add_argument('--build64', type=Path, default=None)
	args = args.parse_args()
	if args.llvm is None:
		args.llvm = Path(__file__).parent.parent.parent / r'llvm-project'  # this repo's neighbour
	if args.build32 is None:
		args.build32 = args.llvm / r'build32'
	if args.build64 is None:
		args.build64 = args.llvm / r'build64'

	llvm = args.llvm
	llvm: Path
	build32 = args.build32
	build32: Path
	build64 = args.build64
	build64: Path
	out = Path(__file__).parent.parent  # this repo's root
	configurations = (
		(32, build32, r'Debug'),
		(32, build32, r'Release'),
		(64, build64, r'Debug'),
		(64, build64, r'Release'),
	)

	assert_existing_directory(llvm)
	assert_existing_directory(llvm / r'llvm/include')
	assert_existing_directory(llvm / r'clang/include')
	for _, build, mode in configurations:
		assert_existing_directory(build / mode / r'lib')

	print(r'Creating output directories')
	delete_directory(out / r'include')
	delete_directory(out / r'lib')
	os.makedirs(out / r'include/llvm', exist_ok=True)
	os.makedirs(out / r'include/clang', exist_ok=True)
	for bits, _, mode in configurations:
		os.makedirs(out / rf'lib/win{bits}-{mode}', exist_ok=True)

	print(r'Copying includes')
	includes = [
		(llvm / r'llvm/include/llvm/ADT', out / r'include/llvm/ADT'),  #
		(llvm / r'clang/include/clang/Tooling', out / r'include/clang/Tooling')
	]
	for src, dest in includes:
		shutil.copytree(src, dest, dirs_exist_ok=True)
	copy_file(llvm / r'LICENSE.txt', out / r'include')

	print(r'Copying libs')
	libs = [
		r'clangTooling',  #
	]
	for bits, build, mode in configurations:
		for lib in libs:
			copy_file(build / mode / rf'lib/{lib}.lib', out / rf'lib/win{bits}-{mode}')

	print(r'Writing build scripts')

	script_preamble = rf'''
		#{"-"*119}
		#              *** this file was automatically generated by {Path(__file__).name}; do not edit it directly ***
		#{"-"*119}'''.strip().replace("\t\t", "")

	with open(out / r'lib/meson.build', r'w', encoding=r'utf-8', newline='\n') as f:
		f.write(
			rf'''
			{script_preamble}

			if is_x64
				if is_debug
					subdir('win64-Debug')
				else
					subdir('win64-Release')
				endif
			else
				if is_debug
					subdir('win32-Debug')
				else
					subdir('win32-Release')
				endif
			endif
			'''.strip().replace("\t\t\t", "") + '\n'
		)

	for bits, build, mode in configurations:
		with open(out / rf'lib/win{bits}-{mode}/meson.build', r'w', encoding=r'utf-8', newline='\n') as f:
			find_library = r''
			for lib in libs:
				find_library += f'''\tcpp.find_library('{lib}', dirs : [meson.current_source_dir()], required : true, static : true)'''
			f.write(
				rf'''
				{script_preamble}

				libs += [
				{find_library}
				]
				'''.strip().replace("\t\t\t\t", "") + '\n'
			)



if __name__ == '__main__':
	main()
