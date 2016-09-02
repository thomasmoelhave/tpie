#!/usr/bin/env python3

# Copyright 2016, The TPIE development team
#
# This file is part of TPIE.
#
# TPIE is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the
# Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
# License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with TPIE.  If not, see <http://www.gnu.org/licenses/>

import os
import os.path
import sys
import subprocess
from glob import glob
from cmakeast import ast as cmakeast

os.chdir(os.path.dirname(sys.argv[0]))
os.chdir('..')

ctests = []
test_decs = ['add_unittest', 'add_fulltest']

with open('test/unit/CMakeLists.txt', 'r') as f:
	ast = cmakeast.parse(f.read())

for s in ast.statements:
	if isinstance(s, cmakeast.FunctionCall):
		if s.name in test_decs:
			args = list(map(lambda x: x.contents, s.arguments))
			suit = args[0]
			tests = args[1:]
			for t in tests:
				ctests.append(suit + '_' + t)

os.chdir('build')

uttests = []
for suit in glob('test/unit/ut-*'):
	suitname = suit.split('/ut-')[1]
	out = subprocess.check_output([suit, '--help'])
	out = out.split(b'Available tests:')[1]
	out = out.split(b'\n\n')[0]
	for test in out.strip().split(b'\n'):
		testname = str(test.strip().split()[0], 'utf-8')
		uttests.append(suitname + '_' + testname)

sc = set(ctests)
sut = set(uttests)
missing = sorted(sut - sc)

if missing:
	print('Tests in ut-*, but not in ctest:')
	for t in missing:
		print(' ' * 4 + t)

	sys.exit(1)
else:
	print('All tests in ut-* found in ctest.')
