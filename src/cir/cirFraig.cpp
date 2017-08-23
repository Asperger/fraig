/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2010-2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHash.h"
#include "util.h"

using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
void
CirMgr::strash()
{
	if (_DFS->empty())
		for (GateList::iterator i = _POs->begin(); i != _POs->end(); i++)
			DFScheck(i->second);
	clock_t c;
	c = clock();
	Hash<HashKey, CirGate*> hashTable(32);
	for (GateVList::iterator j = _DFS->begin(); j != _DFS->end(); j++){
		if ((*j)->_type == PO_GATE || (*j)->_type == PI_GATE) continue;
		HashKey key((*j)->getFanin());
		CirGate* mergeGate;
		if (hashTable.check(key, mergeGate)){
			if (mergeGate != *j){
				cout << "Merging " << (*j)->getIndex() << " and " << mergeGate->getIndex() << endl;
				mergeGate->merge(*j);
			}
		}
		else hashTable.forceInsert(key, *j);
	}
	cout << "Strash takes " << float(clock()-c)/CLOCKS_PER_SEC << " seconds.\n";
}

void
CirMgr::fraig()
{
	clock_t c;
	GateList::iterator i;
	FEClist::iterator j;
	GateVList::iterator k, l;
	SatSolver solver;
	SatTable table;
	c = clock();
	solver.initialize();
	resetMark(false);
	for (i = _POs->begin(); i != _POs->end(); i++)
		DFSinitSAT(i->second, solver, table);
	for (j = _FECgroups.begin(); j != _FECgroups.end(); j++){
		if (j->second.size() <= 1) continue;
		k = j->second.begin();
		while (k != j->second.end()){
			l = k+1;
			while (l != j->second.end()){
				if (j->second.size() <= 1) break;
				if (solveSAT(table[*k], table[*l], solver)){
					cout << (*k)->getIndex() << " and " << (*l)->getIndex() << " are equivalent pair.\n";
					(*k)->merge(*l);
					l = j->second.erase(l);
				}
				else l++;
			}
			k = j->second.erase(k);
			if (j->second.size() <= 1) break;
		}
	}
	cout << "FRAIG takes " << float(clock()-c)/CLOCKS_PER_SEC << " seconds.\n";
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/

void
CirMgr::collectFEC()
{
	_FECgroups.clear();
	for (GateList::iterator i = _ANDs->begin(); i != _ANDs->end(); i++)
   	_FECgroups[i->second->_value_str].push_back(i->second);
}

void
CirMgr::DFSinitSAT(CirGate* g, SatSolver& s, SatTable& t)
{
	g->_mark = true;
	switch (g->_type){
		case PO_GATE:
			if (!g->input()->_mark) DFSinitSAT(g->input(), s, t);
			break;
		case AIG_GATE:
			t.insert(pair<CirGate*, Var>(g, s.newVar()));
			if (!g->left_input()->_mark) DFSinitSAT(g->left_input(), s, t);
			if (!g->right_input()->_mark) DFSinitSAT(g->right_input(), s, t);
			s.addAigCNF(t[g], t[g->left_input()], g->left_inverted(), t[g->right_input()], g->right_inverted());
			break;
		case PI_GATE:
			t.insert(pair<CirGate*, Var>(g, s.newVar()));
			break;
		case CONST_GATE:{
			t.insert(pair<CirGate*, Var>(g, s.newVar()));
			Var vir = s.newVar();
			s.addAigCNF(t[g], vir, true, vir, false);
			break;}
		case UNDEF_GATE:
		default: break;
	}
}

bool
CirMgr::solveSAT(Var& a, Var& b, SatSolver& s)
{
	Var f = s.newVar();
	s.addXorCNF(f, a, false, b, false);
	s.assumeRelease();
	s.assumeProperty(f, true);
	return !s.assumpSolve();
}