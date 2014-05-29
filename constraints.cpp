#include "constraints.hpp"


ContactTracker::ContactTracker(sptr<Box> box, sptr<atomgroup> atoms, vector<flt> dists) :
    atoms(atoms), dists(dists), contacts(), breaks(0), formations(0),
        incontact(0){
    //~ cout << "Making contact tracker." << endl;
    uint N = atoms->size();
    dists.resize(N);
    contacts.resize(N);
    for(uint i=0; i<N; i++){
        contacts[i].resize(i, false);
    }
    update(*box);
    breaks = 0;
    formations = 0;
};

void ContactTracker::update(Box &box){
    //~ cout << "Contact tracker update." << endl;
    uint N = atoms->size();
    incontact = 0;
    for(uint i=0; i<N; i++){
        Vec ri = atoms->get(i)->x;
        for(uint j=0; j<i; j++){
            Vec rj = atoms->get(j)->x;
            Vec dr = box.diff(ri,rj);
            bool curcontact = (dr.mag() <= ((dists[i] + dists[j])/2));
            if(curcontact && (!contacts[i][j])) formations++;
            else if(!curcontact && contacts[i][j]) breaks++;
            if(curcontact) incontact++;
            
            contacts[i][j] = curcontact;
        }
    }
    //~ cout << "Contact tracker update done." << endl;
};

void EnergyTracker::update(Box &box){
    if(nskipped + 1 < nskip){
        nskipped += 1;
        return;
    }
    
    nskipped = 0;
    uint Natoms = atoms->size();
    flt curU = 0, curK = 0;
    for(uint i=0; i<Natoms; i++){
        atom &curatom = *(atoms->get(i));
        curK += curatom.v.sq() * curatom.m / 2;
    }
    
    vector<sptr<interaction> >::iterator it;
    for(it = interactions.begin(); it != interactions.end(); it++){
        curU += (*it)->energy(box);
    }
    
    curU -= U0;
    Ks += curK;
    Us += curU;
    Es += curK + curU;
    Ksq += curK*curK;
    Usq += curU*curU;
    Esq += (curK + curU)*(curK + curU);
    N++;
};

void EnergyTracker::setU0(Box &box){
    flt curU = 0;
    vector<sptr<interaction> >::iterator it;
    for(it = interactions.begin(); it != interactions.end(); it++){
        curU += (*it)->energy(box);
    }
    setU0(curU);
};


RsqTracker1::RsqTracker1(atomgroup& atoms, uint skip, Vec com) :
            pastlocs(atoms.size(), Vec()), rsqsums(atoms.size(), 0.0),
            rsqsqsums(atoms.size(), 0.0), skip(skip), count(0){
    for(uint i = 0; i<atoms.size(); i++){
        pastlocs[i] = atoms[i].x - com;
    };
};
        
void RsqTracker1::reset(atomgroup& atoms, Vec com){
    pastlocs.resize(atoms.size());
    rsqsums.assign(atoms.size(), 0);
    rsqsqsums.assign(atoms.size(), 0);
    for(uint i = 0; i<atoms.size(); i++){
        pastlocs[i] = atoms[i].x - com;
    };
    count = 0;
};

bool RsqTracker1::update(Box& box, atomgroup& atoms, uint t, Vec com){
    if(t % skip != 0) return false;
    
    for(uint i = 0; i<atoms.size(); i++){
        //flt dist = box.diff(atoms[i].x, pastlocs[i]).sq();
        Vec r = atoms[i].x - com;
        flt dist = (r - pastlocs[i]).sq();
        // We don't want the boxed distance - we want the actual distance moved!
        rsqsums[i] += dist;
        rsqsqsums[i] += dist*dist;
        pastlocs[i] = r;
    };
    count += 1;
    return true;
};

vector<flt> RsqTracker1::rsq_mean(){
    vector<flt> means(rsqsums.size(), 0.0);
    for(uint i=0; i<rsqsums.size(); i++){
        means[i] = rsqsums[i] / count;
    }
    return means;
};

vector<flt> RsqTracker1::rsq_var(){
    vector<flt> vars(rsqsums.size(), 0.0);
    for(uint i=0; i<rsqsums.size(); i++){
        flt mn = (rsqsums[i])/count;
        flt sqmn = (rsqsqsums[i])/count;
        vars[i] = sqmn - mn*mn;
    }
    return vars;
};
    

RsqTracker::RsqTracker(sptr<atomgroup> atoms, vector<uint> ns, bool usecom) : 
            atoms(atoms), curt(0), usecom(usecom){
    Vec com = usecom ? atoms->com() : Vec();
    for(vector<uint>::iterator n=ns.begin(); n!=ns.end(); n++){
        singles.push_back(RsqTracker1(*atoms, *n, com));
    }
};

void RsqTracker::update(Box &box){
    curt++;
    Vec com = usecom ? atoms->com() : Vec();
    for(vector<RsqTracker1>::iterator it=singles.begin(); it!=singles.end(); it++){
        it->update(box, *atoms, curt, com);
    }
};

void RsqTracker::reset(){
    curt = 0;
    Vec com = usecom ? atoms->com() : Vec();
    for(vector<RsqTracker1>::iterator it=singles.begin(); it!=singles.end(); it++){
        it->reset(*atoms, com);
    }
};

vector<vector<flt> > RsqTracker::means(){
    vector<vector<flt> > vals;
    vals.reserve(singles.size());
    for(vector<RsqTracker1>::iterator it=singles.begin(); it!=singles.end(); it++){
        vals.push_back(it->rsq_mean());
    }
    return vals;
};

vector<vector<flt> > RsqTracker::vars(){
    vector<vector<flt> > vals;
    vals.reserve(singles.size());
    for(vector<RsqTracker1>::iterator it=singles.begin(); it!=singles.end(); it++){
        vals.push_back(it->rsq_var());
    }
    return vals;
};

vector<flt> RsqTracker::counts(){
    vector<flt> vals;
    vals.reserve(singles.size());
    for(vector<RsqTracker1>::iterator it=singles.begin(); it!=singles.end(); it++){
        vals.push_back(it->get_count());
    }
    return vals;
};
//Replicate for R^4
RfrTracker1::RfrTracker1(atomgroup& atoms, uint skip, Vec com) :
            pastlocs(atoms.size(), Vec()), rfrsums(atoms.size(), 0.0),
            rfrfrsums(atoms.size(), 0.0), skip(skip), count(0){
    for(uint i = 0; i<atoms.size(); i++){
        pastlocs[i] = atoms[i].x - com;
    };
};
        
void RfrTracker1::reset(atomgroup& atoms, Vec com){
    pastlocs.resize(atoms.size());
    rfrsums.assign(atoms.size(), 0);
    rfrfrsums.assign(atoms.size(), 0);
    for(uint i = 0; i<atoms.size(); i++){
        pastlocs[i] = atoms[i].x - com;
    };
    count = 0;
};

bool RfrTracker1::update(Box& box, atomgroup& atoms, uint t, Vec com){
    if(t % skip != 0) return false;
    
    for(uint i = 0; i<atoms.size(); i++){
        //flt dist = box.diff(atoms[i].x, pastlocs[i]).sq();
        Vec r = atoms[i].x - com;
        flt dist = (r - pastlocs[i]).sq();
        // We don't want the boxed distance - we want the actual distance moved!
        rfrsums[i] += dist;
        rfrfrsums[i] += dist*dist*dist*dist;
        pastlocs[i] = r;
    };
    count += 1;
    return true;
};

vector<flt> RfrTracker1::rfr_mean(){
    vector<flt> means(rfrsums.size(), 0.0);
    for(uint i=0; i<rfrsums.size(); i++){
        means[i] = rfrsums[i] / count;
    }
    return means;
};

vector<flt> RfrTracker1::rfr_var(){
    vector<flt> vars(rfrsums.size(), 0.0);
    for(uint i=0; i<rfrsums.size(); i++){
        flt mn = (rfrsums[i])/count;
        flt sqmn = (rfrfrsums[i])/count;
        vars[i] = sqmn - mn*mn;
    }
    return vars;
};
    

RfrTracker::RfrTracker(sptr<atomgroup> atoms, vector<uint> ns, bool usecom) : 
            atoms(atoms), curt(0), usecom(usecom){
    Vec com = usecom ? atoms->com() : Vec();
    for(vector<uint>::iterator n=ns.begin(); n!=ns.end(); n++){
        singles.push_back(RfrTracker1(*atoms, *n, com));
    }
};

void RfrTracker::update(Box &box){
    curt++;
    Vec com = usecom ? atoms->com() : Vec();
    for(vector<RfrTracker1>::iterator it=singles.begin(); it!=singles.end(); it++){
        it->update(box, *atoms, curt, com);
    }
};

void RfrTracker::reset(){
    curt = 0;
    Vec com = usecom ? atoms->com() : Vec();
    for(vector<RfrTracker1>::iterator it=singles.begin(); it!=singles.end(); it++){
        it->reset(*atoms, com);
    }
};

vector<vector<flt> > RfrTracker::means(){
    vector<vector<flt> > vals;
    vals.reserve(singles.size());
    for(vector<RfrTracker1>::iterator it=singles.begin(); it!=singles.end(); it++){
        vals.push_back(it->rfr_mean());
    }
    return vals;
};

vector<vector<flt> > RfrTracker::vars(){
    vector<vector<flt> > vals;
    vals.reserve(singles.size());
    for(vector<RfrTracker1>::iterator it=singles.begin(); it!=singles.end(); it++){
        vals.push_back(it->rfr_var());
    }
    return vals;
};

vector<flt> RfrTracker::counts(){
    vector<flt> vals;
    vals.reserve(singles.size());
    for(vector<RfrTracker1>::iterator it=singles.begin(); it!=singles.end(); it++){
        vals.push_back(it->get_count());
    }
    return vals;
};
// End of Replication

bool jamminglist::operator<(const jamminglist& other ){
    //return distsq < other.distsq;
    if(other.distsq  - distsq > 1e-8) return true;
    if(distsq  - other.distsq > 1e-8) return false;
    //~ cout << "\nWithin 1e-8\n";
    
    uint sz = size();
    uint osz = other.size();
    if(sz < osz) return true;
    if(sz > osz) return false;
    //~ cout << sz << ' ' << osz << ' ' << N << " Indices:";
    for(uint i=0; i<sz; i++){
        if (assigned[i] < other.assigned[i]) return true;
        if (assigned[i] > other.assigned[i]) return false;
        //~ cout << " " << i;
    }
    //~ cout << " Done. Comparing sizes...\n";
    //~ cout << "Equal!\n";
    return false; // consider them equal
};

#ifdef VEC2D
/* There are two ways of looking at the different arrangements.
 * In both cases, we leave A the same as it was, and rotate / flip / translate B.
 * Also in both cases, we wrap A, then subtract off its COM (in an infinite box).
 * 
 * Method 1:
   * "unwrap" the box into 9 boxes (2D), and then choose a box for each
   * of the particles.
   * (N-1)⁹, right? (×8×N!, with rotations / flips / permutations)
 * Method 2:
   * Pick a particle (in B), move the box so that's on the left.
   * Pick another particle (maybe the same), move the box so its on the bottom.
   * Calculate COM of B without PBC, subtract that off.
   * Compare A and B, but using the PBC to calculate distances.
   * N² (×8×N!)
 * Method 3:
   * Only try different rotations, but measure distances as Σ (\vec r_{ij} - \vec s_{ij})²
 
 * We'll go with method 3.
*/

bool jamminglistrot::operator<(const jamminglistrot& other ){
    //return distsq < other.distsq;
    if(other.distsq  - distsq > 1e-8) return true;
    if(distsq  - other.distsq > 1e-8) return false;
    
    uint sz = size();
    uint osz = other.size();
    if(sz < osz) return true;
    if(sz > osz) return false;
    //~ cout << "\nWithin 1e-8. ";
    
    //~ cout << sz << ' ' << osz << ' ' << N << " Indices:";
    for(uint i=0; i<sz; i++){
        if (assigned[i] < other.assigned[i]) return true;
        if (assigned[i] > other.assigned[i]) return false;
        //~ cout << " " << i;
    }
    //~ cout << " Done. Comparing rotations...";
    if(rotation < other.rotation) return true;
    if(rotation > other.rotation) return false;
    
    //~ cout << " Comparing sizes...";
    if(sz < osz) return true;
    if(sz > osz) return false;
    //~ cout << "Equal!\n";
    return false; // consider them equal
};

jammingtree2::jammingtree2(sptr<Box>box, vector<Vec>& A0, vector<Vec>& B0)
            : box(box), jlists(), A(A0), Bs(8, B0){
    for(uint rot=0; rot < 8; rot++){
        for(uint i=0; i<B0.size(); i++){
                        Bs[rot][i] = B0[i].rotate_flip(rot); }
        if(A0.size() <= B0.size()) jlists.push_back(jamminglistrot(rot));
        //~ cout << "Created, now size " << jlists.size() << endl;
    }
    
};

flt jammingtree2::distance(jamminglistrot& jlist){
    flt dist = 0;
    uint rot = jlist.rotation;
    for(uint i=1; i<jlist.size(); i++){
        uint si = jlist.assigned[i];
        for(uint j=0; j<i; j++){
            uint sj = jlist.assigned[j];
            Vec rij = box->diff(A[i], A[j]);
            Vec sij = box->diff(Bs[rot][si], Bs[rot][sj]);
            dist += box->diff(rij, sij).sq();
        }
    }
    return dist / ((flt) jlist.assigned.size());
};

list<jamminglistrot> jammingtree2::expand(jamminglistrot curjlist){
    vector<uint>& curlist = curjlist.assigned;
    list<jamminglistrot> newlists = list<jamminglistrot>();
    if(curlist.size() >= A.size()){
        return newlists;
    }
    
    uint N = (uint) Bs[curjlist.rotation].size();
    for(uint i=0; i < N; i++){
        vector<uint>::iterator found = find(curlist.begin(), curlist.end(), i);
        //if (find(curlist.begin(), curlist.end(), i) != curlist.end()){
        if (found != curlist.end()) continue;
        
        jamminglistrot newjlist = jamminglistrot(curjlist, i, 0);
        newjlist.distsq = distance(newjlist);
        newlists.push_back(newjlist);
    }
    return newlists;
};

bool jammingtree2::expand(){
    jamminglistrot curjlist = jlists.front();
    list<jamminglistrot> newlists = expand(curjlist);
    
    if(newlists.size() <= 0){
        //~ cout << "No lists made\n";
        return false;
    }
    //~ cout << "Have " << newlists.size() << "\n";
    newlists.sort();
    //~ cout << "Sorted.\n";
    jlists.pop_front();
    //~ cout << "Popped.\n";
    jlists.merge(newlists);
    //~ cout << "Merged to size " << jlists.size() << "best dist now " << jlists.front().distsq << "\n";
    return true;
};


jammingtreeBD::jammingtreeBD(sptr<Box> box, vector<Vec>& A, vector<Vec>& B, 
                    uint cutoffA, uint cutoffB) :
            jammingtree2(box, A, B), cutoff1(cutoffA), cutoff2(cutoffB){
    if(cutoffA > cutoffB){jlists.clear();}
    if(A.size() - cutoffA > B.size() - cutoffB){jlists.clear();}
};

list<jamminglistrot> jammingtreeBD::expand(jamminglistrot curjlist){
    vector<uint>& curlist = curjlist.assigned;
    list<jamminglistrot> newlists = list<jamminglistrot>();
    if(curlist.size() >= A.size()){
        return newlists;
    }
    
    uint N = (uint) Bs[curjlist.rotation].size();
    uint start = 0, end=cutoff2;
    if(curlist.size() >= cutoff1){
        start=cutoff2;
        end = N;
    }
    for(uint i=start; i < end; i++){
        vector<uint>::iterator found = find(curlist.begin(), curlist.end(), i);
        //if (find(curlist.begin(), curlist.end(), i) != curlist.end()){
        if (found != curlist.end()) continue;
        
        jamminglistrot newjlist = jamminglistrot(curjlist, i, 0);
        newjlist.distsq = distance(newjlist);
        newlists.push_back(newjlist);
    }
    return newlists;
};

bool jammingtreeBD::expand(){
    jamminglistrot curjlist = jlists.front();
    list<jamminglistrot> newlists = expand(curjlist);
    
    if(newlists.size() <= 0) return false;
    newlists.sort();
    jlists.pop_front();
    jlists.merge(newlists);
    return true;
};


vector<Vec> jammingtree2::locationsB(jamminglistrot jlist){
    uint rot = jlist.rotation;
    vector<Vec> locs = vector<Vec>(jlist.size());
    
    uint N = jlist.size();
    for(uint i=0; i<N; i++){
        uint si = jlist.assigned[i];
        locs[i] = A[i];
        for(uint j=0; j<N; j++){
            uint sj = jlist.assigned[j];
            Vec rij = box->diff(A[i], A[j]);
            Vec sij = box->diff(Bs[rot][si], Bs[rot][sj]);
            locs[i] -= box->diff(rij, sij)/N;
        }
    }
    return locs;
};


vector<Vec> jammingtree2::locationsA(jamminglistrot jlist){
    uint rot = jlist.rotation;
    vector<Vec> locs = vector<Vec>(Bs[rot].size(), Vec(NAN,NAN));
    
    uint N = jlist.size();
    for(uint i=0; i<N; i++){
        uint si = jlist.assigned[i];
        locs[si] = Bs[rot][si];
        for(uint j=0; j<N; j++){
            uint sj = jlist.assigned[j];
            Vec rij = box->diff(A[i], A[j]);
            Vec sij = box->diff(Bs[rot][si], Bs[rot][sj]);
            locs[si] += box->diff(rij, sij)/N;
        }
        
        // this is an inverse rotateflip
        locs[si] = locs[si].rotate_flip_inv(rot);
    }
    return locs;
};

Vec jammingtree2::straight_diff(Box &bx, vector<Vec>& As, vector<Vec>& Bs){
    uint N = (uint) As.size();
    if(Bs.size() != N) return Vec(NAN,NAN);
    
    Vec loc = Vec();
    for(uint i=0; i<N; i++){
        for(uint j=0; j<N; j++){
            Vec rij = bx.diff(As[i], As[j]);
            Vec sij = bx.diff(Bs[i], Bs[j]);
            loc += bx.diff(rij, sij);
        }
    }
    return loc / N;
};

flt jammingtree2::straight_distsq(Box &bx, vector<Vec>& As, vector<Vec>& Bs){
    uint N = (uint) As.size();
    if(Bs.size() != N) return NAN;
    
    flt dist = 0;
    for(uint i=0; i<N; i++){
        for(uint j=0; j<N; j++){
            Vec rij = bx.diff(As[i], As[j]);
            Vec sij = bx.diff(Bs[i], Bs[j]);
            dist += bx.diff(rij, sij).sq();
        }
    }
    return dist / N;
};

#endif
