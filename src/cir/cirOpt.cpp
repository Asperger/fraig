/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
void
CirMgr::sweep()
{
	GateList::iterator i, j;
	clock_t c;
	c = clock();
	resetMark(false);
	_DFS->clear();
	for (i = _POs->begin(); i != _POs->end(); i++)
		DFScheck(i->second);
	for (i = _ANDs->begin(); i != _ANDs->end(); i++){
		if (!i->second->_mark){
			cout << "Clearing #" << i->first << endl;
			_ANDs->erase(i);
			i->second->left_input()->unregist(i->first);
			i->second->right_input()->unregist(i->first);
			for (j = i->second->output()->begin(); j != i->second->output()->end(); j++)
				j->second->unregist(i->first);
		}
	}
	_UNDEFs->clear();
	cout << "Sweeping takes " << float(clock()-c)/CLOCKS_PER_SEC << " seconds.\n";
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
void
CirMgr::optimize()
{
	GateList::iterator it;
	clock_t c;
	c = clock();
	resetMark(false);
	for (it = _POs->begin(); it != _POs->end(); it++)
		DFSopt(it->second);
	cout << "Optimization takes " << float(clock()-c)/CLOCKS_PER_SEC << " seconds.\n";
	resetMark(false);
	for (it = _POs->begin(); it != _POs->end(); it++)
		DFScheck(it->second);
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::DFSopt(CirGate* g)
{
	g->_mark = true;
	if (g->_type == AIG_GATE){
		if (!(g->left_input()->_mark)) DFSopt(g->left_input());
		if (!(g->right_input()->_mark)) DFSopt(g->right_input());
		CirGate* left = g->left_input();
		bool left_inverted = g->left_inverted();
		CirGate* right = g->right_input();
		bool right_inverted = g->right_inverted();
		CirGate* rep_gate;
		bool rep_invt;
		bool replace = true;
		if (left->_type == CONST_GATE && right->_type == CONST_GATE){
			rep_invt = (left_inverted && right_inverted);
			rep_gate = _CONST;
		}
		else if (left->_type == CONST_GATE){
			if (left_inverted){
				rep_gate = _CONST;
				rep_invt = false;
			}
			else {
				rep_gate = right;
				rep_invt = right_inverted;
			}
		}
		else if (right->_type == CONST_GATE){
			if (right_inverted){
				rep_gate = _CONST;
				rep_invt = false;
			}
			else {
				rep_gate = left;
				rep_invt = left_inverted;
			}
		}
		else if (left == right){
			if (left_inverted == right_inverted){
				rep_gate = left;
				rep_invt = left_inverted;
			}
			else {
				rep_gate = _CONST;
				rep_invt = false;
			}
		}
		else replace = false;
		if (replace){
			cout << "Replacing " << g->getIndex() << " with " << ((rep_invt)?"!":"") << rep_gate->getIndex() << endl;
			GateList::iterator it;
			for (it = g->output()->begin(); it != g->output()->end(); it++){
				it->second->resetInput(g, rep_gate, rep_invt);
				rep_gate->resetOutput(g, it->second);
			}
		}
	}
	if (g->_type == PO_GATE)
		if (!(g->input()->_mark)) DFSopt(g->input());
}