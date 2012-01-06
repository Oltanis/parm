from __future__ import print_function
import re, math, numpy as np
from itertools import izip
import hashlib, logging
#~ import pyparsing as parse

class XYZwriter:
    def __init__(self, f, usevels = True):
        self.file = f
        self.usevels = usevels
    
    def writeframe(self, reslist, comment='', com=None):
        Natoms = sum(len(r) for r in reslist)
        print(Natoms, file=self.file)
        print(comment, file=self.file)
        
        lines = []
        for r in reslist:
            for atom in r:
                elem = atom.element
                if com is None:
                    x,y,z = tuple(atom.x)
                else:
                    x,y,z = tuple(atom.x - com)
                if not self.usevels:
                    print("%s %.3f %.3f %.3f" % (elem,x,y,z), file=self.file)
                else:
                    vx,vy,vz = tuple(atom.v)
                    lines.append(("%s" + (" %.3f"*3)+ (" %.3g"*3)) % (elem,x,y,z,vx,vy,vz))
        
        # do all the writing at once
        print('\n'.join(lines), file=self.file)
        self.file.flush()
    
    def size(self):
        location = self.file.tell()
        self.file.seek(0,2)
        size = self.file.tell()
        self.file.seek(location)
        return size
    
    def close(self):
        self.file.close()

class XYZError(Exception):
    pass

def md5_file(f, block_size = None):
    loc = f.tell()
    f.seek(0)
    md5 = hashlib.md5()
    if block_size is None:
        block_size = md5.block_size
    while True:
        data = f.read(block_size)
        if not data:
            break
        md5.update(data)
    f.seek(loc)
    return md5.hexdigest()

#~ parse.ParserElement.setDefaultWhitespaceChars(" \t")
#~ real = parse.Word(parse.nums) + parse.Optional("." + parse.Word(parse.nums))
#~ real = real.setParseAction(lambda toks: float("".join(toks)))
#~ intparse = parse.Word(parse.nums).setName('int').setParseAction(lambda toks: int(toks[0]))
#~ space = parse.Suppress(' ')
#~ endln = parse.Suppress(parse.LineEnd())
#~ line = parse.Group((parse.Word(parse.alphas) + real + real + real + endln))
#~ manylines = parse.Group(parse.OneOrMore(line))
#~ 
#~ firstline = intparse + endln
#~ secondline = parse.Suppress('time ') + parse.restOfLine + endln
#~ fullframe = firstline + secondline + manylines
#~ fileparse = parse.OneOrMore(fullframe)

class XYZreader:
    firstline = re.compile("([0-9]*)")
    commentline = re.compile("time[= ]([0-9.]*)(.*)")
    regline = re.compile("(\w{1,2})\s+([\-0-9.]+)\s+([\-0-9.]+)\s+([\-0-9.]+)")
    
    def __init__(self, f):
        self.file = f
        self._size = None
        self._md5 = None
    
    def __match__(self, regex, *types):
        line = self.file.readline()
        if line == '':
            raise StopIteration, "Reached end of file."
        line = line.strip()
        match = regex.match(line)
        if match is None and types:
            raise XYZError("Failed to parse at line " + repr(line))
        groups = match.groups()
        if len(groups) != len(types):
            raise XYZError("Wrong number of types given.")
        return tuple(t(val) for t,val in zip(types,groups))
    
    def readframe(self):
        num, = self.__match__(self.firstline, int)
        t, = self.__match__(self.commentline, float, str)

        lines = [self.file.readline().strip().split(' ') for i in range(num)]
        badlines = [l for l in lines if len(l) != 4 and len(l) != 7]
        if len(badlines) > 0:
            raise ValueError, "bad line: %s" % str(badlines[0])
        
        try:
            lines = [(e,(float(x),float(y),float(z))) for e,x,y,z in lines]
            return Frame(lines, t)
        except ValueError:
            pass
        
        lines = [(e,(float(x),float(y),float(z)),(float(vx),float(vy),float(vz))) 
                    for e,x,y,z,vx,vy,vz in lines]
        return Frame(lines, t)
        
    def __iter__(self):
        location = self.file.tell()
        while True:
            yield self.readframe()
        self.file.seek(location)
    
    def close(self):
        self.file.close()
    
    def all(self):
        frames = Frames(iter(self))
        for n,f in enumerate(frames):
            f.indx = n
        return frames
    
    def md5(self):
        if self._md5 is None:
            logging.debug('calculating md5...')
            self._md5 = md5_file(self.file, 2**16)
            logging.debug('md5: %s' % self._md5)
        return self._md5
    
    def size(self):
        if self._size is None:
            location = self.file.tell()
            self.file.seek(0,2)
            self._size = self.file.tell()
            self.file.seek(location)
        return self._size
        
class Frame:
    dtype = np.float16
    
    def __init__(self, locs, t=None): #, indx=None):
        self.t = t
        #~ self.indx = indx
        resized = zip(*locs)
        if len(resized) == 2:
            self.elems, self.locs = resized
            self.vels = None
        elif len(resized) == 3:
            self.elems, self.locs, self.vels = resized
        else:
            raise ValueError, "Resized wrong length (%d)" % len(resized)
        
        #~ self.__locarray = None
        #~ self.__velarray = None   
        if self.vels is not None:
            self.__velarray = self.vels = np.array(self.vels, dtype=self.dtype)
        self.__locarray = self.locs = np.array(self.locs, dtype=self.dtype)
    
    def __iter__(self):
        return iter(self.locs)
    
    def _setx(self, atom, loc):
        x,y,z = loc
        atom.x.set(float(x), float(y),float(z))
    def _setv(self, atom, vel):
        x,y,z = vel
        atom.v.set(float(x), float(y),float(z))
    
    def into(self, atoms, check=True):
        if self.vels is not None:
            for a, elem, loc, vel in izip(atoms, self.elems, self.locs, self.vels):
                if check and a.element != elem:
                    raise TypeError, ("Element mismatch for atom %d (%s) and line %s" 
                                        % (num, atom.element, repr(line)))
                self._setx(a, loc)
                self._setv(a, vel)
        else:
            for a, elem, loc in izip(atoms, self.elems, self.locs):
                if check and a.element != elem:
                    raise TypeError, ("Element mismatch for atom %d (%s) and line %s" 
                                        % (num, atom.element, repr(line)))
                self._setx(a, loc)
    
    @property
    def time(self):
        return self.t
    
    @property
    def locarray(self):
        if self.__locarray is None:
            #~ self.__constructed += 1
            #~ print("constructing", self.time, self.__constructed)
            self.__locarray = np.array(list(self.locs), dtype=self.dtype)
        #~ else:
            #~ print("FOUND")
        return self.__locarray
    
    @property
    def velarray(self):
        if self.__velarray is None and self.vels is not None:
            self.__velarray = np.array(list(self.vels), dtype=self.dtype)
        return self.__velarray

class Frames(list):
    def into(self, atoms, check=True):
        for f in self:
            f.into(atoms, check)
            yield f.time

import re
refirst = '(\d*)\n'
resecond = 'time (\d*\.?\d+)(.*)\n'
reline = '(\w+) (\d*\.?\d+) (\d*\.?\d+) (\d*\.?\d+)\n'
remany = '(?:(\w+) (\d*\.?\d+) (\d*\.?\d+) (\d*\.?\d+)\n)+'


if __name__ == '__main__':
    s = open('test/T1-1K.xyz','r').read()
    print('got s')
    #~ v1 = re.match(refirst, s)
    #~ v12 = re.match(refirst + resecond, s)
    #~ v13 = re.match(refirst + resecond + reline + reline, s, re.MULTILINE)
    #~ 
    #~ print('found', len(v13.groups()))
    
    firstv = firstline.parseString(s)
    firstsecv = (firstline + secondline).parseString(s)
    v13 = (firstline + secondline + line).parseString(s)
    vs = fullframe.parseString(s)
    
    if False:
        import cProfile as profile
        from simw import atomvec
        f=open('/home/wendell/idp/data/T3-200K.xyz','r')
        x=XYZreader(f)
        #~ print(x.readframe())
        #~ profile.runctx('for l in x: pass', globals(), locals())
        atoms = atomvec([1]*1013)
        #~ profile.runctx('for t,l in x: Frame(l,t).into(atoms,False)', globals(), locals())
        frames = [Frame(l,t) for t,l in x]
        profile.runctx('for f in frames: f.into(atoms,False)', globals(), locals())