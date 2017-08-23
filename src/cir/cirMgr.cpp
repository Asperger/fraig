/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool CirMgr::readCircuit(const string& fileName) {
   fstream f;
   f.open(fileName.c_str(), fstream::in);
   if (!f.is_open()){ cerr << "[ERROR] Cannot open file: " << fileName << endl; return false; }
   string line;

   getline(f, line);
   char head[3];
   sscanf(line.c_str(), "%s%u%u%u%u%u", head, &M, &I, &L, &O, &A);
   int i = 0;
   while (head[i]) { head[i] = tolower(head[i]); i++; }
   string spec(head);
   if (spec.compare("aag") != 0){ cerr << "[ERROR] Not an AAG file: " << fileName << endl; return false; }
   if (L != 0){ cerr << "[ERROR] I don\'t know how to deal with latches." << endl; return false; }

   resetlist();
   for (unsigned index, i = 1; i <= I; i++){
      getline(f, line);
      sscanf(line.c_str(), "%u", &index);
      iGate* newgate = new iGate(index);
      newgate->setLineNo(i);
      _PIs->insert(pair<unsigned, iGate*>(index/2, newgate));
   }
   for (unsigned index, i = 1; i <= O; i++){
      getline(f, line);
      sscanf(line.c_str(), "%u", &index);
      oGate* newgate = new oGate(index);
      newgate->setLineNo(i+I);
      _POs->insert(pair<unsigned, oGate*>(index/2, newgate));
   }
   for (unsigned index, l, r, i = 1; i <= A; i++){
      getline(f, line);
      sscanf(line.c_str(), "%u%u%u", &index, &l, &r);
      andGate* newgate = new andGate(index, l, r);
      newgate->setLineNo(i+I+O);
      _ANDs->insert(pair<unsigned, andGate*>(index/2, newgate));
   }

   while (!f.eof()){
      getline(f, line);
      if (line[0] == 'c' || line == "") break;
      GateList* li;
      if (line[0] == 'i') li = _PIs;
      if (line[0] == 'o') li = _POs;
      int count = line[1]-48;
      string name = line.substr(line.find_first_of(' '), line.find_first_of('\n'));
      GateList::iterator it = li->begin();
      while (count != 0) { it++; count--;}
      it->second->_name = name;
   }

   f.close();
   return buildConnect();
}

bool CirMgr::buildConnect() {
   GateList::iterator i, k;
   IdList::iterator j;
   for (i = _ANDs->begin(); i != _ANDs->end(); i++){
      for (j = i->second->getFanin()->begin(); j != i->second->getFanin()->end(); j++){
         unsigned id = (*j)/2;
         if (id == 0){
            _CONST->setLineNo(i->second->getLineNo());
            _CONST->gateRegist(i->first, i->second);
            i->second->gateRegist(0, _CONST);
            continue;
         }
         k = _ANDs->find(id);
         if (k != _ANDs->end()){
            k->second->gateRegist(i->first, i->second);
            i->second->gateRegist(id, k->second);
            continue;
         }
         k = _PIs->find(id);
         if (k != _PIs->end()){
            k->second->gateRegist(i->first, i->second);
            i->second->gateRegist(id, k->second);
            continue;
         }
         iGate* undef = new iGate(*j);
         undef->_type = UNDEF_GATE;
         undef->setLineNo(i->second->getLineNo());
         _UNDEFs->insert(pair<unsigned, CirGate*>(id, undef));
         i->second->gateRegist(id, undef);
         undef->gateRegist(i->first, i->second);
      }
   }
   i = _POs->begin();
   while (i != _POs->end()){
      unsigned id = i->first;
      k = _ANDs->find(id);
      if (k != _ANDs->end()){
         unsigned newindex = _POs->rbegin()->second->getIndex()+1;
         i->second->setIndex(newindex);
         _POs->insert(pair<unsigned, CirGate*>(newindex, i->second));
         k->second->gateRegist(id, i->second);
         i->second->setFanin(id*2);
         i->second->gateRegist(k->first, k->second);
         _POs->erase(i++);
         continue;
      }
      k = _PIs->find(id);
      if (k != _PIs->end()){
         unsigned newindex = _POs->rbegin()->second->getIndex()+1;
         i->second->setIndex(newindex);
         _POs->insert(pair<unsigned, CirGate*>(newindex, i->second));
         k->second->gateRegist(id, i->second);
         i->second->setFanin(id*2);
         i->second->gateRegist(k->first, k->second);
         _POs->erase(i++);
         continue;
      }
      i++;
   }
   return true;
}

void CirMgr::resetlist() {
   _ANDs->clear();
   _PIs->clear();
   _POs->clear();
   _UNDEFs->clear();
   _DFS->clear();
}

CirGate* CirMgr::getGate(unsigned id) const {
   GateList::iterator k = _ANDs->find(id);
   if (k != _ANDs->end()) return k->second;
   k = _PIs->find(id);
   if (k != _PIs->end()) return k->second;
   k = _POs->find(id);
   if (k != _POs->end()) return k->second;
   return NULL;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void CirMgr::printSummary() const {
   size_t num_pi = _PIs->size();
   size_t num_po = _POs->size();
   size_t num_aig = _ANDs->size();
   cout << "Circuit Statistics" << endl
        << "==================" << endl
        << "  PI" << setw(12) << num_pi << endl
        << "  PO" << setw(12) << num_po << endl
        << "  AIG" << setw(11) << num_aig << endl
        << "------------------" << endl
        << "  Total" << setw(9) << num_pi+num_po+num_aig << endl;
}

void CirMgr::printNetlist(){
   lineNo = 0;
   resetMark(false);
   GateList::iterator it = _POs->begin();
   for (; it != _POs->end(); it++)
      DFSprint(it->second);
}

void CirMgr::printPIs() const {
   cout << "PIs of the circuit:";
   for (GateList::iterator it = _PIs->begin(); it != _PIs->end(); it++)
      cout << " " << it->first;
   cout << endl;
}

void CirMgr::printPOs() const {
   cout << "POs of the circuit:";
   for (GateList::iterator it = _POs->begin(); it != _POs->end(); it++)
      cout << " " << it->first;
   cout << endl;
}

void CirMgr::printFloatGates() const {
   cout << "Gates with floating fanin(s):";
   for (GateList::iterator it = _POs->begin(); it != _POs->end(); it++)
      if (it->second->input() == NULL) cout << " " << it->first;
   for (GateList::iterator it = _ANDs->begin(); it != _ANDs->end(); it++)
      if (it->second->left_input() == NULL || it->second->right_input() == NULL)
         cout << " " << it->first;
   cout << endl << "Gates defined but not used :";
   for (GateList::iterator it = _PIs->begin(); it != _PIs->end(); it++)
      if (it->second->output()->size() == 0) cout << " " << it->first;
   for (GateList::iterator it = _ANDs->begin(); it != _ANDs->end(); it++)
      if (it->second->output()->size() == 0) cout << " " << it->first;
   cout << endl;
}

void CirMgr::writeAag(ostream& outfile) {
   size_t num_pi = _PIs->size();
   size_t num_po = _POs->size();
   size_t num_aig = _ANDs->size();
   outfile << "aag " << num_pi+num_po+num_aig << " " << num_pi << " 0 " << num_po << " " << num_aig << endl;
   GateList::iterator it;
   for (it = _PIs->begin(); it != _PIs->end(); it++)
      outfile << it->first*2 << endl;
   bool empty = false;
   if (_DFS->empty()){
      resetMark(false);
      empty = true;
   }
   for (it = _POs->begin(); it != _POs->end(); it++)
      if (empty) DFScheck(it->second);
      outfile << it->first*2+it->second->inverted() << endl;
   for (it = _ANDs->begin(); it != _ANDs->end(); it++)
      if (!(it->second->_mark)) continue;
      outfile << it->first*2
              << it->second->getFanin()->front()
              << it->second->getFanin()->back() << endl;
}

void CirMgr::printFECPairs() const {
   lineNo = 0;
   FEClist::const_iterator i;
   GateVList::const_iterator j;
   for (i = _FECgroups.begin(); i != _FECgroups.end(); i++){
      cout << "[" << lineNo++ << "]";
      for (j = i->second.begin(); j != i->second.end(); j++)
         cout << " " << (*j)->getIndex();
      cout << endl;
   }
}

/**********************************************************/
/*   class CirMgr member functions for DFS traversal		 */
/**********************************************************/
void CirMgr::resetMark(bool m){
   GateList::iterator it;
   _CONST->_mark = m;
   for (it = _POs->begin(); it != _POs->end(); it++) it->second->_mark = m;
   for (it = _PIs->begin(); it != _PIs->end(); it++) it->second->_mark = m;
   for (it = _ANDs->begin(); it != _ANDs->end(); it++) it->second->_mark = m;
   for (it = _UNDEFs->begin(); it != _UNDEFs->end(); it++) it->second->_mark = m;
}

void CirMgr::DFScheck(CirGate* g) {
   g->_mark = true;
   if (g->_type == PO_GATE){
      if (!(g->input()->_mark)) DFScheck(g->input());
   }
   if (g->_type == AIG_GATE){
      if (!(g->left_input()->_mark)) DFScheck(g->left_input());
      if (!(g->right_input()->_mark)) DFScheck(g->right_input());
   }
   _DFS->push_back(g);
}

void CirMgr::DFSprint(CirGate* g) {
   g->_mark = true;
   unsigned lid, rid;
   string n, inv, lt, linv, rt, rinv;
   CirGate *left_gate, *right_gate;
   switch(g->_type){
      case CONST_GATE:
         cout << "[" << lineNo++ << "] CONST " << g->getIndex() << endl;
         break;
      case PI_GATE:
         n = (g->_name.compare("") == 0)?"":("("+g->_name+")");
         cout << "[" << lineNo++ << "] PI " << g->getIndex() << " " << n << endl;
         break;
      case PO_GATE:
         if (!(g->input()->_mark)) DFSprint(g->input());
         n = (g->_name.compare("") == 0)?"":("("+g->_name+")");
         inv = (g->inverted())?"!":"";
         cout << "[" << lineNo++ << "] PO " << g->getIndex() << " " << inv << g->input()->getIndex() << " " << n << endl;
         break;
      case AIG_GATE:
         left_gate = g->left_input();
         right_gate = g->right_input();
         if (!(left_gate->_mark)) DFSprint(left_gate);
         if (!(right_gate->_mark)) DFSprint(right_gate);
         if (left_gate->_type == CONST_GATE){
            lid = left_gate->getIndex();
            lt = "";
            linv = "";
         }
         else {
            lid = left_gate->getIndex();
            lt = (left_gate->_type == UNDEF_GATE)?"*":"";
            linv = (g->left_inverted())?"!":"";
         }
         if (right_gate->_type == CONST_GATE){
            rid = right_gate->getIndex();
            rt = "";
            rinv = "";
         }
         else {
            rid = right_gate->getIndex();
            rt = (right_gate->_type == UNDEF_GATE)?"*":"";
            rinv = (g->right_inverted())?"!":"";
         }
         cout << "[" << lineNo++ << "] AIG " << g->getIndex() << " " << lt << linv << lid << " " << rt << rinv << rid << endl;
         break;
      case UNDEF_GATE:
      default: cout << "[" << lineNo++ << "] UNDEF " << g->getIndex() << endl;
   }
}

