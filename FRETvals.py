from numpy import sqrt, mean, std

Forsterdist = 54

ijset = [(9,130), (33,130), (54,130), (72,130), (33,72), (9,54),
        (72,92), (54,72), (9,72), (9,33), (54,92), (92,130)]

ETeffs = {
          (9,130) : 0.36, (33,130) : 0.53,(54,130) : 0.5, (72,130) : 0.55, (92,130) : 0.65, (33,72) : 0.72,
          (9,54) : 0.7, (72,92) : 0.85, (54,72) : 0.85, (9,72) : 0.64, (9,33) : 0.86, (54,92) : 0.66}
          
ETerrs = { (9,130) : 0.0529, (33,130) : 0.0608, (54,130) : 0.0627,
            (72,130) : 0.0621, (92,130) : 0.0624, (33,72) : 0.0643,
            (9,54) : 0.0608, (72,92) : 0.0451, (54,72) : 0.0437,
            (9,72) : 0.0658, (9,33) : 0.0503, (54,92) : 0.0674 }

ijs = [(54, 72), (72, 92), (9, 33), (54, 92), (92, 130), (33, 72),
            (9, 54), (72, 130), (9, 72), (54, 130), (33, 130), (9, 130)]
ETs = [ETeffs[ij] for ij in ijs]
ETerrlst = [ETerrs[ij] for ij in ijs]

revijs = sorted(ijset, key=lambda (i,j): -abs(j-i))
expETsij = [ETeffs[ij] for ij in revijs]
expETerrs = [ETerrs[ij] for ij in revijs]

ijexp = sorted(ijset, key=lambda ij: -ETeffs[ij])
expETs = [ETeffs[ij] for ij in ijexp]

ijsRW = [(72, 92), (54, 72), (9, 33), (54, 92), (92, 130), (33, 72),
            (9, 54), (72, 130), (9, 72), (54, 130), (33, 130), (9, 130)]
expETsRW = [ETeffs[ij] for ij in ijsRW]
expETerrsRW = [ETerrs[ij] for ij in ijsRW]

FRETdists={
         (9,130) : (63.4,16.3), (33,130) : (52.8,13.2), (54,130) : (54.0,13.5), (72,130) : (51.6,12.9),
         (92,130) : (47.0,11.5), (33,72) : (44.0,10.5), (9,54) : (44.6,10.7), (72,92) : (37.9,8.5),
         (54,72) : (37.9,8.5), (9,72) : (47.7,11.5), (9,33) : (37.3,8.3), (54,92) : (46.8,11.1)}

#def ETeffrsq(ds):
#    dists = [(d - ed)**2 for d,ed in zip(ds, expETs)]
#    return sqrt(mean(dists))

def ETeffrsq(d):
    dists = [(d[ij] - ETeffs[ij])**2 for ij in ijs]
    return sqrt(mean(dists))

distribijs = [(1, 140), (12, 127), (14, 125), (21, 118), (21, 111), (36, 75), (38, 77), (43, 82), (57, 133), (59, 135), (63, 139), (9, 130), (33, 130), (54, 130), (72, 130), (92, 130), (33, 72), (9, 54), (72, 92), (54, 72), (9, 72), (9, 33), (54, 92)]
