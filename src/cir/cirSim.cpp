/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <cassert>
#include <string>
#include "rnGen.h"
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

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
	if (_simLog != NULL){
		if (!_simLog->is_open()){
			cerr << "[ERROR] Cannot open log file." << endl;
			return;
		}
	}
	RandomNumGen ranum(3345678);
	GateList::iterator i;
	unsigned sim_times = _PIs->size()*2;
	clock_t c;
	c = clock();
	initsim();
	for (unsigned t = 0; t < sim_times; t++){
		for (i = _PIs->begin(); i != _PIs->end(); i++){
			bool value = ranum(2);
			i->second->_value = value;
			i->second->_value_str += BtoS(value);
			if (_simLog != NULL) *_simLog << BtoS(value);
		}
		resetMark(false);
		for (i = _POs->begin(); i != _POs->end(); i++)
			DFSsim(i->second);
		if (_simLog != NULL){
			*_simLog << " ";
			for (i = _POs->begin(); i != _POs->end(); i++)
				*_simLog << BtoS(i->second->_value);
			*_simLog << endl;
		}
	}
	collectFEC();
	cout << sim_times << " patterns take " << float(clock()-c)/CLOCKS_PER_SEC << " seconds to simulate." << endl;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
	if (!patternFile.is_open()){
		cerr << "[ERROR] Cannot open pattern file." << endl;
		return;
	}
	if (_simLog != NULL){
		if (!_simLog->is_open()){
			cerr << "[ERROR] Cannot open log file." << endl;
			return;
		}
	}
	clock_t c;
	string line;
	int sim_times = 0;
	c = clock();
	initsim();
	while (!patternFile.eof()){
		getline(patternFile, line);
		if (line.size() == 0) continue;
		else if (line.size() != _PIs->size()){
			cerr << "[ERROR] Pattern(" << line << ") length(" << line.size() << ") does not match the number of input("
				  << _PIs->size() << ") in a circuit!!" << endl; continue;
		}
		size_t pos = line.find_first_not_of("01");
		if (pos != string::npos){
			while (pos != string::npos){
				cerr << "[ERROR] Pattern(" << line << ") contains a non-0/1 character(" << line[pos] << ")." << endl;
				pos = line.find_first_not_of("01", pos+1);
			}
			continue;
		}
		GateList::iterator i;
		int j = 0;
		for (i = _PIs->begin(); i != _PIs->end(); i++){
			i->second->_value = StoB(line[j]);
			i->second->_value_str += line[j];
			j++;
		}
		resetMark(false);
		for (i = _POs->begin(); i != _POs->end(); i++)
			DFSsim(i->second);
		if (_simLog != NULL){
			*_simLog << line << " ";
			for (i = _POs->begin(); i != _POs->end(); i++)
				*_simLog << BtoS(i->second->_value);
			*_simLog << endl;
		}
		sim_times++;
	}
	collectFEC();
	cout << sim_times << " patterns take " << float(clock()-c)/CLOCKS_PER_SEC << " seconds to simulate." << endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

void
CirMgr::initsim()
{
	GateList::iterator i;
   for (i = _ANDs->begin(); i != _ANDs->end(); i++)
   	i->second->_value_str = "";
   for (i = _POs->begin(); i != _POs->end(); i++)
   	i->second->_value_str = "";
   for (i = _PIs->begin(); i != _PIs->end(); i++)
   	i->second->_value_str = "";
}

void
CirMgr::DFSsim(CirGate* g)
{
	g->_mark = true;
	switch (g->_type){
		case PO_GATE:
			if (!(g->input()->_mark)) DFSsim(g->input());
			g->_value = XOR(g->input()->_value,!g->inverted());
			g->_value_str += BtoS(g->_value);
			break;
		case AIG_GATE:
			if (!(g->left_input()->_mark)) DFSsim(g->left_input());
			if (!(g->right_input()->_mark)) DFSsim(g->right_input());
			g->_value = XOR(g->left_input()->_value,!g->left_inverted()) && XOR(g->right_input()->_value,!g->right_inverted());
			g->_value_str += BtoS(g->_value);
			break;
		case PI_GATE: break;
		case CONST_GATE: break;
		case UNDEF_GATE: g->_value = false; g->_value_str += BtoS(g->_value); break;
		default: break;
	}
}
