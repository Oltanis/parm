#ifdef VEC2D
#ifdef LONGFLOAT
%module sim2dlong
#else
%module sim2d
#endif
#else
#define VEC3D
#ifdef LONGFLOAT
%module sim3dlong
#else
%module sim3d
#endif
#endif
#pragma SWIG nowarn=302,321,389
%feature("autodoc", "1");

%include pyabc.i
%include typemaps.i
%include exception.i
%include carrays.i
%include complex.i
%include std_vector.i
%include std_list.i
%include std_set.i
%include boost_shared_ptr.i
%include std_map.i

%apply double { long double } 

%include <pybuffer.i>
%pybuffer_mutable_binary(double *buffer, size_t sizet)

%shared_ptr(Box)
%shared_ptr(OriginBox)
%shared_ptr(InfiniteBox)
%shared_ptr(LeesEdwardsBox)
%shared_ptr(SCbox)
%ignore AtomIter;
%shared_ptr(atomgroup)
%shared_ptr(subgroup)
%shared_ptr(statetracker)
%shared_ptr(interaction)
%shared_ptr(constraint)
%shared_ptr(atomvec)
%shared_ptr(metagroup)
%shared_ptr(neighborlist)
%shared_ptr(ContactTracker)
%shared_ptr(EnergyTracker)
%shared_ptr(RsqTracker)
%shared_ptr(ISFTracker)
%shared_ptr(SmoothLocs)
%shared_ptr(RDiffs)
%shared_ptr(SCatomvec)
%shared_ptr(coordConstraint)
%shared_ptr(RandomForce)
%shared_ptr(fixedForce)
%shared_ptr(fixedForceRegion)
%shared_ptr(fixedSpring)
%shared_ptr(SoftWall)
%shared_ptr(SoftWallCylinder)
%shared_ptr(WalledBox2D)
%shared_ptr(COMSpring)
%shared_ptr(bondpairs)
%shared_ptr(angletriples)
%shared_ptr(dihedrals)
%shared_ptr(interactionpairsx)
%shared_ptr(LJsimple)
%shared_ptr(Charges)
%shared_ptr(SCSpringList)
%shared_ptr(coordCOMConstraint)
%shared_ptr(relativeConstraint)
%shared_ptr(distConstraint)
%shared_ptr(linearConstraint)
%shared_ptr(NPHGaussianConstraint)
%shared_ptr(SimpleListed< HertzianAtom,HertzianPair >)
%shared_ptr(NListed< LJatom,LJpair >)
%shared_ptr(NListed< LJatom,LJpair >)
%shared_ptr(NListed< LJatomcut,LJAttractPair >)
%shared_ptr(NListed< HydroAtom,HydroPair >)
%shared_ptr(NListed< LJAtomIndexed,LJAttractPair >)
%shared_ptr(NListed< LJAtomIndexed,LJCutPair >)
%shared_ptr(NListed< LJAttractRepulseAtom,LJAttractRepulsePair >)
%shared_ptr(NListed< LJAttractFixedRepulseAtom,LJAttractFixedRepulsePair >)
%shared_ptr(NListed< LJDoubleAtom,LJDoublePair >)
%shared_ptr(NListed< EisMclachlanAtom,EisMclachlanPair >)
%shared_ptr(NListed< LJishAtom,LJishPair >)
%shared_ptr(NListed< LoisOhernAtom,LoisOhernPair >)
%shared_ptr(NListed< LoisOhernAtom,LoisOhernPairMinCLs >)
%shared_ptr(NListed< LoisLinAtom,LoisLinPair >)
%shared_ptr(NListed< LoisLinAtom,LoisLinPairMin >)
%shared_ptr(NListed< HertzianAtom,HertzianPair >)
%shared_ptr(NListed< HertzianAtomIndexed,HertzianPair >)
%shared_ptr(NListed< HertzianDragAtom,HertzianDragPair >)
%shared_ptr(SCboxed< HertzianAtom,HertzianPair >)
%shared_ptr(NListedVirial< HertzianAtom,HertzianPair >)

%{
#include "vecrand.hpp"
#include "vecrand.cpp"
#include "box.hpp"
#include "box.cpp"
#include "trackers.hpp"
#include "trackers.cpp"
#include "interaction.hpp"
#include "interaction.cpp"
#include "constraints.hpp"
#include "constraints.cpp"
#include "collection.hpp"
#include "collection.cpp"
#include <Python.h>

// Part of the numpy initialization sequence.
// See http://wiki.scipy.org/Cookbook/SWIG_NumPy_examples#head-6c11cb03512f8fd5bfa20f8d8c6f69e7cc1ce494
#define SWIG_FILE_WITH_INIT
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

static int myErr = 0;
%}

// Part of the numpy initialization sequence.
%init %{
    import_array()
%}

%exception {
    try {
        $action
    } catch(std::range_error &e) {
        SWIG_exception(SWIG_ValueError, e.what());
    //~ } catch(DivisionByZero) {
    //~     SWIG_exception(SWIG_DivisionByZero, "Division by zero");
    //~ } catch(OutOfMemory) {
    //~     SWIG_exception(SWIG_MemoryError, "Out of memory");
    } catch(std::invalid_argument &e) {
        SWIG_exception(SWIG_ValueError, e.what());
    } catch(std::overflow_error &e) {
        SWIG_exception(SWIG_OverflowError, e.what());
    } catch(std::exception &e) {
        SWIG_exception(SWIG_RuntimeError,e.what());
    }catch(...) {
        SWIG_exception(SWIG_RuntimeError,"Unknown exception");
    }
}

// Convert Vec2, Vec3 output into numpy arrays
// Taken from
// http://stackoverflow.com/questions/24375198/error-wrapping-eigen-c-with-python-using-swig
%typemap(out) Vec2 { 
    npy_intp dims[1] = {2}; 
    PyObject* array = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    double* data = ((double *)PyArray_DATA((PyArrayObject *) array)); 
    data[0] = $1(0);
    data[1] = $1(1);
    $result = array; 
};

// Written myself, using http://www.swig.org/Doc3.0/Typemaps.html#Typemaps
%typemap(out) Vec3 { 
    npy_intp dims[1] = {3}; 
    PyObject* array = PyArray_SimpleNew(1, dims, NPY_DOUBLE);
    double* data = ((double *)PyArray_DATA((PyArrayObject *) array)); 
    data[0] = $1(0);
    data[1] = $1(1);
    data[2] = $1(2);
    $result = array; 
};

// Take any sequence as input for a Vec2 (or Vec3, below)
%typemap(in) Vec2 (Vec2 temp) {
  if (PySequence_Check($input)) {
    PyObject* tup = PySequence_Tuple($input);
    if (!PyArg_ParseTuple(tup,"dd", &temp(0), &temp(1))) {
      PyErr_SetString(PyExc_TypeError,"sequence must have 2 doubles.");
      return NULL;
    }
    $1 = temp;
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a sequence.");
    return NULL;
  }
};

%typemap(in) Vec3 (Vec3 temp) {
  if (PySequence_Check($input)) {
    PyObject* tup = PySequence_Tuple($input);
    if (!PyArg_ParseTuple(tup,"ddd", &temp(0), &temp(1), &temp(2))) {
      PyErr_SetString(PyExc_TypeError,"sequence must have 3 doubles.");
      return NULL;
    }
    $1 = temp;
  } else {
    PyErr_SetString(PyExc_TypeError,"expected a sequence.");
    return NULL;
  }
};

// Note that Vec2 has higher precedence than Vec3
%typecheck(SWIG_TYPECHECK_FLOAT_ARRAY) Vec2 {
    $1 = (PySequence_Check($input) && (PySequence_Size($input) == 2)) ? 1 : 0;
};

%typecheck(SWIG_TYPECHECK_DOUBLE_ARRAY) Vec3 {
    $1 = (PySequence_Check($input) && (PySequence_Size($input) == 3)) ? 1 : 0;
};

#ifdef VEC2D
%typemap(in) Vec = Vec2;
%typemap(out) Vec = Vec2;
%typemap(typecheck) Vec = Vec2;
#else 
%typemap(in) Vec = Vec3;
%typemap(out) Vec = Vec3;
%typemap(typecheck) Vec = Vec3;
#endif 

#ifdef VEC2D
namespace std {
    %template(_jamminglist) list<jamminglist>;
    %template(_jamminglistrot) list<jamminglistrot>;
}
#endif

namespace std {
    %template(fvector) vector<float>;
    %template(_ffvector) vector<vector<float> >;
    %template(dvector) vector<double>;
    %template(_ddvector) vector<vector<double> >;
    %template(_dddvector) vector<vector<vector<double> > >;
    %template(cvector) vector<std::complex<double> >;
    %template(_ccvector) vector<vector<std::complex<double> > >;
    %template(_cccvector) vector<vector<vector<std::complex<double> > > >;
    %template(ldvector) vector<long double>;
    //%template(avector) vector<shared_ptr<atomgroup> >;
    //%template(aptrvector) vector<shared_ptr<atom> >;
    %template(ivector) vector<shared_ptr<interaction> >;
    %template(ifxvector) vector<shared_ptr<interactionpairsx> >;
    %template(tvector) vector<shared_ptr<statetracker> >;
    %template(constraintvector) vector<shared_ptr<constraint> >;
    #ifdef VEC2D
    %template(wallvector) vector<shared_ptr<SoftWall> >;
    #endif
    %template(idvector) vector<atomid>;
    %template(idpairvector) vector<idpair>;
    %template(intvector) vector<int>;
    %template(uintvector) vector<unsigned int>;
    %template(ulongvector) vector<unsigned long>;
    %template(_eventset) set<event>;
    %template(pair_uint_CNodePath) pair<unsigned int, CNodePath>;
    %template(map_uint_CNodePath) map<unsigned int, CNodePath>;
    %template(vector_CNode) vector<CNode>;
    %template(pair_int_CNode) pair<int, vector<CNode> >;
    %template(map_int_CNode) map<int, vector<CNode> >;
}

%extend atom {
    %insert("python") %{
        def __getstate__(self):
            return (tuple(self.x),tuple(self.v),tuple(self.f), tuple(self.a))
        
        def __setstate__(self, lst):
            self.x, self.v, self.f, self.a = [Vec(*r) for r in lst]
        
        def __str__(self):
            if hasattr(self, 'name'):
                return "<atom %s>" % self.name
            return "<atom>"
        
        def __repr__(self):
            #ifdef VEC2D
            x,y = tuple(self.x)
            if hasattr(self, 'name'):
                return "<atom %s at (%.2f,%.2f)>" % (self.name,x,y)
            return "<atom at (%.2f,%.2f)>" % (x,y)
            #else
            x,y,z = tuple(self.x)
            if hasattr(self, 'name'):
                return "<atom %s at (%.2f,%.2f,%.2f)>" % (self.name,x,y,z)
            return "<atom at (%.2f,%.2f,%.2f)>" % (x,y,z)
            #endif
    %};
}

%extend atomid {
    %insert("python") %{
        def __str__(self):
            return "<atomid %s>" % self.n()
        
        def __repr__(self):
            #ifdef VEC2D
            x,y = tuple(self.x)
            return "<atomid %s at (%.2f,%.2f)>" % (self.n(),x,y)
            #else
            x,y,z = tuple(self.x)
            return "<atomid %s at (%.2f,%.2f,%.2f)>" % (self.n(),x,y,z)
            #endif
    %};
}

%extend atomgroup {
    %insert("python") %{
        def __iter__(self):
            for i in range(self.size()):
                yield self.get_id(i)
    %};
};


%extend atomvec {
    %insert("python") %{
        def __iter__(self):
            for i in range(self.size()):
                yield self[i]
        
        def __len__(self):
            return self.size()
        
        def __getitem__(self, obj):
            return self.get_id(obj)
        
        #def __setitem__(self, obj, val):
        #    return self.set(obj, val)
        
        def __getstate__(self):
            return ([self.getmass(i) for i in range(self.N())],
                        [a.__getstate__() for a in self])
        
        def __setstate__(self, lst):
            masses, atomstates = lst
            self.__init__(fvector(masses))
            for i, atomstate in enumerate(atomstates):
                #~ print "i:",i
                a = self.get(i)
                a.__setstate__(atomstate)
    %};
};

%extend SCatomvec {
    %insert("python") %{
        def __iter__(self):
            for i in range(self.size()):
                yield self[i]
        
        def all_pairs(self):
            for i in range(self.pairs()):
                yield self.pair(i)
        
        def __len__(self):
            return self.size()
        
        def __getitem__(self, obj):
            return self.get_id(obj)
    %};
};

%extend idpair {
    %insert("python") %{
        def __iter__(self):
            return iter((self.first(), self.last()))
    %};
};

%exception neighborlist::__getitem__ {
  assert(!myErr);
  $action
  if (myErr) {
    myErr = 0; // clear flag for next time
    // You could also check the value in $result, but it's a PyObject here
    SWIG_exception(SWIG_IndexError, "Index out of bounds");
  }
};

%include "vecrand.hpp"
%include "box.hpp"
%include "trackers.hpp"
%include "interaction.hpp"
%include "constraints.hpp"
%template(LJgroup) NListed<LJatom, LJpair>; // Pure repulsive
%template(LJattract) NListed<LJatomcut, LJAttractPair>;  // Both repulsive and attractive
%template(Hydrophobicity) NListed<HydroAtom, HydroPair>;  // Pure attractive
%template(LJattractix) NListed<LJAtomIndexed, LJAttractPair>;  // Pure attractive, with indices for sigma and epsilon
%template(LJfullix) NListed<LJAtomIndexed, LJCutPair>;  // Both repulsive and attractive, with indices for sigma and epsilon

// Both repulsive and attractive, with indices for epsilon, sigma fixed per atom.
// Positive epsilon -> Attractive + repulsive, negative for pure repulsive.
%template(LJAttractRepulse) NListed<LJAttractRepulseAtom, LJAttractRepulsePair>;
%template(LJAttractFixedRepulse) NListed<LJAttractFixedRepulseAtom, LJAttractFixedRepulsePair>;
%template(LJDouble) NListed<LJDoubleAtom, LJDoublePair>;
%template(EisMclachlan) NListed<EisMclachlanAtom, EisMclachlanPair>;
%template(LJish) NListed<LJishAtom, LJishPair>;
%template(HertzianSimple) SimpleListed<HertzianAtom, HertzianPair>;
%template(Hertzian) NListed<HertzianAtom, HertzianPair>;
%template(HertzianIdx) NListed<HertzianAtomIndexed, HertzianPair>;
%template(HertzianDrag) NListed<HertzianDragAtom, HertzianDragPair>;
%template(HertzianSC) SCboxed<HertzianAtom, HertzianPair>;
%template(HertzianVirial) NListedVirial<HertzianAtom, HertzianPair>;
%template(LoisOhern) NListed<LoisOhernAtom, LoisOhernPair>;
%template(LoisLin) NListed<LoisLinAtom, LoisLinPair>;
%template(LoisLinMin) NListed<LoisLinAtom, LoisLinPairMin>;
%template(LoisOhernMin) NListed<LoisOhernAtom, LoisOhernPairMinCLs>;
//%rename(__lt__) jamminglist::operator<;

//%{
//    shared_ptr<NListedVirial<HertzianAtom, HertzianPair> > Hertzian(neighborlist *neighbors){
//        NListedVirial<HertzianAtom, HertzianPair> *h = new NListedVirial<HertzianAtom, HertzianPair>(neighbors);
//        return shared_ptr<NListedVirial<HertzianAtom, HertzianPair> >(h);
//    };
//%}

%extend neighborlist {
  idpair __getitem__(size_t i) {
    if (i >= $self->numpairs()) {
      myErr = 1;
      return idpair(atomid(), atomid());
    }
    return $self->get((unsigned int) i);
  }
  
  %insert("python") %{
    def __iter__(self):
        for i in range(self.numpairs()):
            yield self[i]
  %}
};

// %extend collection {
//     %insert("python") %{
//         def __init__(self, box, groups, interactions=None, trackers=None, constraints=None):
//             self.box = box
//             self.groups  = groups
//             self.interactions = interactions
//             self.trackers = trackers
//             self.constraints = constraints
//     %};
// }

%extend NListed<LJatom, LJpair> {
    %insert("python") %{
        def add_atom(self, epsilon, sigma, a):
            self.add(LJatom(epsilon, sigma, a))
    %};
}

%extend NListed<LJatomcut, LJAttractPair> {
    %insert("python") %{
        def add_atom(self, epsilon, sigma, a, cut):
            self.add(LJatomcut(epsilon, sigma, a, cut))
    %};
}

%extend RsqTracker {
    %insert("python") %{
        def mean_array(self):
            import numpy as np
            l = self.means()
            l = [[list(v) for v in innerl] for innerl in l]
            return np.array(l)
        
        def var_array(self):
            import numpy as np
            l = self.vars()
            l = [[list(v) for v in innerl] for innerl in l]
            return np.array(l)
    %};
}

%include "collection.hpp"
%include "collection.cpp"
