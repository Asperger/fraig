/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/

string CirGate::getTypeStr() const {
	switch (_type){
		case PI_GATE: return "PI"; break;
		case PO_GATE: return "PO"; break;
		case CONST_GATE: return "CONST"; break;
		case AIG_GATE: return "AIG"; break;
		case UNDEF_GATE :
		default: return "UNDEF"; break;
	}
}

void CirGate::reportGate() const {
   string n;
   n = (_name.compare("") == 0)?"":("\""+_name+"\"");
   GateVList::const_iterator it;
   string values = _value_str;
	switch (_type){
		case PI_GATE:
         cout << "PI(" << _index << ")" << n << ", line " << _line << endl;
         cout << "Value: " << _value_str << endl;
         break;
		case PO_GATE:
         cout << "PO(" << _index << ")" << n << ", line " << _line << endl;
         cout << "Value: " << _value_str << endl;
         break;
      case AIG_GATE:
         cout << "AIG(" << _index << "), line " << _line << endl;
         cout << "FECs:";
         for (it = cirMgr->_FECgroups[values].begin(); it != cirMgr->_FECgroups[values].end(); it++){
            if ((*it)->getIndex() == _index) continue;
            else cout << " " << (*it)->getIndex();
         }
         for (unsigned i = 0; i < values.size(); i++)
            values[i] = (values[i] == '0')?'1':'0';
         for (it = cirMgr->_FECgroups[values].begin(); it != cirMgr->_FECgroups[values].end(); it++)
            cout << " !" << (*it)->getIndex();
         cout << endl << "Value: " << _value_str << endl;
         break;
		case CONST_GATE: cout << "CONST(" << _index << "), line " << _line << endl; break;
		case UNDEF_GATE :
		default: cout << "UNDEF(" << _index << "), line " << _line << endl; break;
	}
}

void CirGate::reportFanin(int level) {
   assert (level >= 0);
   cirMgr->resetMark(false);
   DFSin(this, level, 0, false);
}

void CirGate::reportFanout(int level) {
   assert (level >= 0);
   cirMgr->resetMark(false);
   DFSout(this, level, 0, _index);
}

void CirGate::DFSin(CirGate* g, int level, int count, bool invert) {
   string re = (g->_mark)?"(*)":"";
   string inv = (invert && g->_type != CONST_GATE)?"!":"";
   string tab(count*2, ' ');
   cout << tab << inv << g->getTypeStr() << " " << g->getIndex() << re << endl;
   if (g->_mark) return;
   g->_mark = true;
   if (level > count){
      switch(g->_type){
         case PO_GATE:
            DFSin(g->input(), level, count+1, g->inverted());
            break;
         case AIG_GATE:
            DFSin(g->left_input(), level, count+1, g->left_inverted());
            DFSin(g->right_input(), level, count+1, g->right_inverted());
            break;
         case CONST_GATE:
         case PI_GATE:
         case UNDEF_GATE:
         default: break;
      }
   }
}

void CirGate::DFSout(CirGate* g, int level, int count, unsigned index) {
   bool invert = false;
   switch(g->_type){
      case PI_GATE:
         invert = g->inverted(); break;
      case AIG_GATE:
         if (g->getFanin()->front()/2 == index) invert = g->left_inverted();
         if (g->getFanin()->back()/2 == index) invert = g->left_inverted();
         break;
      case CONST_GATE:
      case UNDEF_GATE:
      case PO_GATE:
      default: break;
   }
   string re = (g->_mark)?"(*)":"";
   string inv = (invert && g->_type != CONST_GATE)?"!":"";
   string tab(count*2, ' ');
   cout << tab << inv << g->getTypeStr() << " " << g->getIndex() << re << endl;
   if (g->_mark) return;
   g->_mark = true;
   if (level > count){
      for (GateList::iterator it = g->output()->begin(); it != g->output()->end(); it++)
         DFSout(it->second, level, count+1, it->first);
   }
}

void CirGate::resetOutput(CirGate* victim, CirGate* target) {
   _out->erase(victim->getIndex());
   _out->insert(pair<unsigned, CirGate*>(target->getIndex(), target));
}

void CirGate::merge(CirGate* g) {
   for (GateList::iterator it = g->output()->begin(); it != g->output()->end(); it++){
      it->second->resetInput(g, this);
      _out->insert(pair<unsigned, CirGate*>(it->first, it->second));
   }
}

void conGate::gateRegist(unsigned id, CirGate* g) {
   _out->insert(pair<unsigned, CirGate*>(id, g));
}

void conGate::unregist(unsigned id) {
   _out->erase(id);
}

void iGate::gateRegist(unsigned id, CirGate* g) {
   _out->insert(pair<unsigned, CirGate*>(id, g));
}

void iGate::unregist(unsigned id) {
   _out->erase(id);
}

void oGate::resetInput(CirGate* victim, CirGate* target, bool inverted) {
   if (_in == victim){
      _in = target;
      _invert = XOR(_invert,inverted);
      (*_fanins)[0] = target->getIndex()*2 + (int)_invert;
   }
}

void oGate::resetInput(CirGate* victim, CirGate* target) {
   if (_in == victim){
      _in = target;
      (*_fanins)[0] = target->getIndex()*2 + (int)_invert;
   }
}

void oGate::gateRegist(unsigned id, CirGate* g) {
   if (_fanins->size() == 0){
      _out->insert(pair<unsigned, CirGate*>(id, g));
      return;
   }
   if (_fanins->back()/2 == id){
      _in = g;
      _invert = _fanins->back()%2;
   }
   else _out->insert(pair<unsigned, CirGate*>(id, g));
}

void oGate::unregist(unsigned id) {
   if (_out->find(id) != _out->end())
      _out->erase(id);
   if (_in->getIndex() == id)
      _in = NULL;
}

void andGate::resetInput(CirGate* victim, CirGate* target, bool inverted) {
   if (_left_in == victim){
      _left_in = target;
      _left_invert = XOR(_left_invert,inverted);
      (*_fanins)[0] = target->getIndex()*2 + (int)_left_invert;
   }
   if (_right_in == victim){
      _right_in = target;
      _right_invert = XOR(_right_invert, inverted);
      (*_fanins)[1] = target->getIndex()*2 + (int)_right_invert;
   }
}

void andGate::resetInput(CirGate* victim, CirGate* target) {
   if (_left_in == victim){
      (*_fanins)[0] = target->getIndex()*2 + (int)_left_invert;
      _left_in = target;
   }
   if (_right_in == victim){
      (*_fanins)[1] = target->getIndex()*2 + (int)_right_invert;
      _right_in = target;
   }
}

void andGate::gateRegist(unsigned id, CirGate* g) {
   if (_fanins->size() == 0){
      _out->insert(pair<unsigned, CirGate*>(id, g));
      return;
   }
   if (_left_in == NULL && _fanins->front()/2 == id){
      _left_in = g;
      _left_invert = _fanins->front()%2;
   }
   else if (_right_in == NULL && _fanins->back()/2 == id){
      _right_in = g;
      _right_invert = _fanins->back()%2;
   }
   else _out->insert(pair<unsigned, CirGate*>(id, g));
}

void andGate::unregist(unsigned id) {
   if (_out->find(id) != _out->end())
      _out->erase(id);
   if (_left_in->getIndex() == id)
      _left_in = NULL;
   if (_right_in->getIndex() == id)
      _right_in = NULL;
}