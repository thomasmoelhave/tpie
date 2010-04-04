#!/usr/bin/python
import re
from sys import argv
import time
from sys import exit

def fixCopyright(d):
    o=re.search("// Copyright ([0-9]{2,4}([ \t,]+[0-9]{2,4})*), The TPIE development team",d)
    if o:
        x=set([x.strip() for x in o.group(1).split(",")])
    else:
        x=set()
    x.add(time.strftime("%Y"))
    
    m = """// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; c-file-style: "stroustrup"; -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright %s, The TPIE development team
//
// This file is part of TPIE.
//
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
//
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>\n"""%(', '.join(sorted(list(x))))

    mo = re.match("(//.*\n)*// along with TPIE.  If not, see <http://www.gnu.org/licenses/>[ ]*\n",d)
    if mo:
        return m + d[mo.end(0):]
    else:
        return m + d

def fixWhiteSpace(d):
    while True:
        d2 = re.sub(r"    ",r"\t", d)
        if d2 == d: break
        d = d2
    d = re.sub(r"[ \t]+\n",r"\n",d)
    d = re.sub(r"([^a-zA-Z])if\(",r"\1if (",d);
    d = re.sub(r"([^a-zA-Z])while\(",r"\1while (",d);
    d = re.sub(r"([^a-zA-Z])for\(",r"\1for (",d);
    d = re.sub(r"\)[\t \n]*{",r") {",d)
    return d

def sortIncludes(d):
    b=0
    r=""
    for x in re.finditer("((\n#include .*)+)",d):
        r+=d[b:x.start()]
        r+='\n'+'\n'.join(sorted(x.group(0).split("\n")[1:]))
        b=x.end()
    return r + d[b:]
    
def fixHeader(d):
    global path
    p = ('_'+ path.replace('/','_').replace('.','_') ).upper()

    mo = re.match("((//.*\n)*)(([ \t]*\n)*(#ifndef[ \t]+([a-zA-Z_][a-zA-Z_0-9]*)\n#define[ \t]+([a-zA-Z_][a-zA-Z_0-9]*)\n((.*\n)*)#endif.*\n([\t ]*\n)*$)|((.*\n)*))",d)
    c = mo.group(8)
    if not c:
        c = mo.group(11)
    return mo.group(1) + "#ifndef %s\n#define %s\n%s#endif //%s\n"%(p,p,c,p)

changed=False
import os
for path in argv[1:]:
    if os.path.exists(path):
        ext = path.split('.')[-1]
        if ext in ['h','hh', 'c', 'cc', 'cpp', 'inc']:
            ne = d=open(path).read()
            ne = fixCopyright(ne)
            ne = sortIncludes(ne)
            if ext in ['h','hh']:
                ne = fixHeader(ne)
            ne = fixWhiteSpace(ne)
            if ne != d:
                changed=True
                print "Updated style in "+path
                open(path,'w').write(ne)
if changed:
    exit(1)
exit(0)

