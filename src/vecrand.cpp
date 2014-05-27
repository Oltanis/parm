#include "vecrand.hpp"

engine randengine;
normdistribution mynormaldistribution(0,1);
normgenerator gauss(randengine, mynormaldistribution);
lindistribution mylineardistribution;
lingenerator uniformrand(randengine, mylineardistribution);
// Note: there also exists the 'uniform_on_sphere' distribution
// See http://www.boost.org/doc/libs/1_53_0/doc/html/boost_random/reference.html

// for export, esp. to python, in case you want to be using the same seeds
flt rand01(){return uniformrand();};

#ifdef VEC2D
Vec randVec(){
    return Vec(gauss(), gauss());
}

Vec randVecBoxed(){
    return Vec(uniformrand(), uniformrand());
}
#else
Vec randVec(){
    return Vec(gauss(), gauss(), gauss());
}

Vec randVecBoxed(){
    return Vec(uniformrand(), uniformrand(), uniformrand());
}
#endif

unsigned int seed(unsigned int n){
    randengine.seed(n);
    return n;
    
}

unsigned int seed(){
    unsigned int n = static_cast<unsigned int>(time(0));
    randengine.seed(n);
    return n;
}

gaussVec::gaussVec(flt sigma) : distro(0,sigma), gauss(randengine,distro){};


bivariateGauss::bivariateGauss(const flt s1, const flt s2,
        const flt corr) : distro(0,1), gauss(randengine,distro){
            set(s1, s2, corr);
};

void bivariateGauss::set(const flt s1, const flt s2, const flt corr){
    // Taken from Allen and Tildesley, 348
    assert(s1 >= 0);
    assert(s2 >= 0);
    assert(corr >= 0);
    assert(corr <= 1);
    x11 = s1;
    x21 = s2 * corr;
    x22 = s2 * sqrtflt(1 - corr*corr);
}

Pair bivariateGauss::generate(){
    flt x1 = gauss();
    flt x2 = gauss();
    // Taken from Allen and Tildesley, 348
    Pair p;
    p[0] = x11*x1;
    p[1] = x21*x1 + x22*x2;
    return p;
}

VecPair bivariateGauss::genVecs(){
#ifdef VEC2D
    Vec x1 = Vec(gauss(), gauss());
    Vec x2 = Vec(gauss(), gauss());
#else
    Vec x1 = Vec(gauss(), gauss(), gauss());
    Vec x2 = Vec(gauss(), gauss(), gauss());
#endif
    // Taken from Allen and Tildesley, 348
    VecPair p;
    p[0] = x1*x11;
    p[1] = x1*x21 + x2*x22;
    return p;
}