// Compute default gain/attenuration depending on the scale
// G1a/B is driven by SENSEL3

const float G1a=10./11.;
const float G1b=10./1100.;
const float G2=2.;
const float G4=1.+1000./130.; // 130 for new board, 150 for old boards, R14 and R15

// G3 is driven by SENSEL0/1/2
const float G3[6]={1.,2.,4.,10.,20.,40.};

