/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include "cirDef.h"

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate {
public:
   CirGate() {
      _out = new GateList;
      _fanins = new IdList;
      _mark = false;
      _value_str = "";
   }
   ~CirGate() {}

   GateType _type;
   string _name;
   string _value_str;
   bool _mark;
   bool _value;

   // Basic access methods
   void setIndex(unsigned i) { _index = i; }
   void setLineNo(unsigned n) { _line = n; }
   void setFanin(unsigned id) { _fanins->push_back(id); }

   string getTypeStr() const;
   unsigned getIndex() const { return _index; }
   unsigned getLineNo() const { return _line; }
   IdList* getFanin() const { return _fanins; }
   GateList* output() const { return _out;}
   virtual CirGate* input() const { return NULL; }
   virtual bool inverted() const { return true; }
   virtual CirGate* left_input() const { return NULL; }
   virtual CirGate* right_input() const { return NULL; }
   virtual bool left_inverted() const { return true; }
   virtual bool right_inverted() const { return true; }

   virtual void resetInput(CirGate*, CirGate*, bool) {};
   virtual void resetInput(CirGate*, CirGate*) {};
   virtual void gateRegist(unsigned, CirGate*) {};
   void resetOutput(CirGate*, CirGate*);
   virtual void unregist(unsigned) {};
   void merge(CirGate*);

   // Printing functions
   void reportGate() const;
   void reportFanin(int);
   void reportFanout(int);
   void DFSin(CirGate*, int, int, bool);
   void DFSout(CirGate*, int, int, unsigned);

protected:
   unsigned _index;
   unsigned _line;
   GateList* _out;
   IdList* _fanins;   // temporarily store ID as 2n+1 to determine whether inverted or not
};

class conGate : public CirGate {
public:
   conGate(unsigned i) {
      _index = i;
      _type = CONST_GATE;
      _value = i;
   }
   ~conGate() {}
   //bool inverted() const { return false; }
   void gateRegist(unsigned, CirGate*);
   void unregist(unsigned);
};

class iGate : public CirGate {
public:
   iGate(unsigned i) {
      _type = PI_GATE;
      _index = i/2;
      _invert = i%2;
   }
   ~iGate() {}

   bool inverted() const { return _invert; }
   void gateRegist(unsigned, CirGate*);
   void unregist(unsigned);

private:
   bool _invert;
};

class oGate : public CirGate {
public:
   oGate(unsigned i) {
      _type = PO_GATE;
      _index = i/2;
      _invert = i%2;
   }
   ~oGate() {}

   CirGate* input() const { return _in; }
   bool inverted() const { return _invert; }
   void resetInput(CirGate*, CirGate*, bool);
   void resetInput(CirGate*, CirGate*);
   void gateRegist(unsigned, CirGate*);
   void unregist(unsigned);

private:
   bool _invert;
   CirGate* _in;
};

class andGate : public CirGate {
public:
   andGate(unsigned i, unsigned l, unsigned r) {
      _type = AIG_GATE;
      _index = i/2;
      _left_invert = l%2;
      _right_invert = l%2;
      _fanins->push_back(l);
      _fanins->push_back(r);
   }
   ~andGate() {}

   CirGate* left_input() const { return _left_in; }
   CirGate* right_input() const { return _right_in; }
   bool left_inverted() const { return _left_invert; }
   bool right_inverted() const { return _right_invert; }
   void resetInput(CirGate*, CirGate*, bool);
   void resetInput(CirGate*, CirGate*);
   void gateRegist(unsigned, CirGate*);
   void unregist(unsigned);

private:
   CirGate* _left_in;
   CirGate* _right_in;
   bool _left_invert;
   bool _right_invert;
};

#endif // CIR_GATE_H
