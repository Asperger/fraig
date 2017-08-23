/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr {
public:
   CirMgr() {
      _PIs = new GateList;
      _POs = new GateList;
      _ANDs = new GateList;
      _UNDEFs = new GateList;
      _DFS = new GateVList;
      _CONST = new conGate(0);
   }
   ~CirMgr() { resetlist(); }

   conGate* _CONST;
   FEClist _FECgroups;

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned) const;

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist();
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&);
   void resetMark(bool);

private:
   ofstream           *_simLog;
   unsigned M, I, L, O, A;
   GateList* _PIs;
   GateList* _POs;
   GateList* _ANDs;
   GateList* _UNDEFs;
   GateVList* _DFS;

   void resetlist();
   bool buildConnect();
   void DFSopt(CirGate*);
   void DFSsim(CirGate*);
   void DFScheck(CirGate*);
   void DFSprint(CirGate*);
   void initsim();
   void collectFEC();
   void DFSinitSAT(CirGate*, SatSolver&, SatTable&);
   bool solveSAT(Var&, Var&, SatSolver&);
};

#endif // CIR_MGR_H
