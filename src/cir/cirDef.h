/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2010-2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>
#include <map>
#include <string>
#include <time.h>

#include "sat.h"

using namespace std;

#define XOR(a,b) ((a && !b) || (!a && b))
#define BtoS(a) ((a)?"0":"1")
#define StoB(a) ((a == '0')?false:true)

class CirPValue;
class CirGateV;
class CirGate;
class CirPiGate;
class CirPoGate;
class CirMgr;
class SatSolver;

typedef map<unsigned, CirGate*>    GateList;
typedef vector<CirGate*>           GateVList;
typedef map<string, GateVList>     FEClist;
typedef vector<unsigned>           IdList;
typedef CirGate**                  GateArray;
typedef CirPiGate**                PiArray;
typedef CirPoGate**                PoArray;
typedef map<CirGate*, Var>         SatTable;

enum GateType
{
   UNDEF_GATE = 0,
   PI_GATE    = 1,
   PO_GATE    = 2,
   AIG_GATE   = 3,
   CONST_GATE = 4,

   TOT_GATE
};

#endif // CIR_DEF_H
