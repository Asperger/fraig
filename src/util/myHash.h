/****************************************************************************
  FileName     [ myHash.h ]
  PackageName  [ util ]
  Synopsis     [ Define Hash and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-2013 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_H
#define MY_HASH_H

#include <vector>

using namespace std;

//--------------------
// Define Hash classes
//--------------------
// To use Hash ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.

class HashKey
{
public:
   HashKey(vector<unsigned>* f) { _list = *f; }
 
   size_t operator () () const {
      unsigned total = 0;
      for (vector<unsigned>::const_iterator it = _list.begin(); it != _list.end(); it++)
         total += *it;
      return total;
   }

   bool operator != (const HashKey& k) { return !(*this == k); }
   bool operator == (const HashKey& k) {
      if (_list.size() == k._list.size()){
         vector<unsigned> tmp1 = _list;
         vector<unsigned> tmp2 = k._list;
         for (vector<unsigned>::iterator i = tmp1.begin(); i != tmp1.end(); i++){
            for (vector<unsigned>::iterator j = tmp2.begin(); j != tmp2.end(); j++){
               if (*i == *j){
                  tmp2.erase(j);
                  break;
               }
            }
         }
         if (tmp2.size() == 0) return true;
         else return false;
      }
      else return false;
   }

private:
   vector<unsigned> _list;
};

template <class HashKey, class HashData>
class Hash
{
typedef pair<HashKey, HashData> HashNode;

public:
   Hash() : _numBuckets(0), _buckets(0) {}
   Hash(size_t b) : _numBuckets(0), _buckets(0) { init(b); }
   ~Hash() { reset(); }

   // TODO: implement the Hash<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   // (_bId, _bnId) range from (0, 0) to (_numBuckets, 0)
   //
   class iterator
   {
      friend class Hash<HashKey, HashData>;

   public:
      iterator(vector<HashNode>* c, size_t b, size_t x, size_t y){ _buckets = c; _numBuckets = b; _x = x; _y = y; }
      iterator(const iterator& i){ _buckets = i._buckets; _numBuckets = i._numBuckets; _x = i._x; _y = i._y; }
      ~iterator(){}

      const HashNode& operator * () const { return _buckets[_x][_y]; }
      HashNode& operator * () { return _buckets[_x][_y]; }

      iterator& operator ++ () {
         if (_y < _buckets[_x].size()-1) _y++;
         else {
            if (_x < _numBuckets-1) _x++;
            else _x = 0;
            _y = 0;
         }
         return *this;
      }
      iterator operator ++ (int) { iterator tmp(*this); ++(*this); return tmp; }
      iterator& operator -- () {
         if (_y > 0) _y--;
         else {
            if (_x > 0) _x--;
            else _x = _numBuckets-1;
            _y = _buckets[_x].size()-1;
         }
         return *this;
      }
      iterator operator -- (int) { iterator tmp(*this); --(*this); return tmp; }

      iterator& operator = (const iterator& i) {
         _buckets = i._buckets;
         _numBuckets = i._numBuckets;
         _x = i._x;
         _y = i._y;
         return *this;
      }

      bool operator != (const iterator& i) const { return (_buckets[_x][_y] != i._buckets[i._x][i._y])?true:false; }
      bool operator == (const iterator& i) const { return (_buckets[_x][_y] == i._buckets[i._x][i._y])?true:false; }

   private:
      vector<HashNode>* _buckets;
      size_t            _numBuckets;
      int               _x;
      int               _y;
   };

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const { return iterator(_buckets, _numBuckets, 0, 0); }
   // Pass the end
   iterator end() const { return iterator(_buckets, _numBuckets, _numBuckets-1, _buckets[_numBuckets-1].size()); }
   // return true if no valid data
   bool empty() const { return (size() == 0); }
   // number of valid data
   size_t size() const {
      int num = 0;
      for (int i = 0; i < _numBuckets; i++)
         num += _buckets[i].size();
      return num;
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [](size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   void init(size_t b) { _buckets = new vector<HashNode>[b]; _numBuckets = b; }
   void reset() {
      for (size_t i = 0; i < _numBuckets; i++) _buckets[i].clear();
      delete [] _buckets;
   }

   // check if k is in the hash...
   // if yes, update d and return true;
   // else return false;
   bool check(const HashKey& k, HashData& d) {
      size_t i = bucketNum(k);
      typename vector<HashNode>::iterator it;
      for (it = _buckets[i].begin(); it != _buckets[i].end(); it++){
         if (it->first == k){
            d = it->second;
            return true;
         }
      }
      return false;
   }

   // return true if inserted successfully (i.e. k is not in the hash)
   // return false if k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d) {
      size_t i = bucketNum(k);
      typename vector<HashNode>::iterator it;
      for (it = _buckets[i].begin(); it != _buckets[i].end(); it++)
         if (it->first == k) return false;
      _buckets[i].push_back(HashNode(k, d));
      return true;
   }

   // return true if inserted successfully (i.e. k is not in the hash)
   // return false if k is already in the hash ==> still do the insertion
   bool replaceInsert(const HashKey& k, const HashData& d) {
      size_t i = bucketNum(k);
      typename vector<HashNode>::iterator it;
      for (it = _buckets[i].begin(); it != _buckets[i].end(); it++){
         if (it->first == k){
            it->second = d;
            return false;
         }
      }
      _buckets[i].push_back(HashNode(k, d));
      return true;
   }

   // Need to be sure that k is not in the hash
   void forceInsert(const HashKey& k, const HashData& d) {
      _buckets[bucketNum(k)].push_back(HashNode(k, d));
   }

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { _cache = new CacheNode[s]; _size = s;}
   void reset() { delete _cache; }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      for (int i = 0; i < _size; i++)
         if (_cache[i].first == k && _cache[i].second == d)
            return true;
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      for (int i = 0; i < _size; i++)
         if (_cache[i].first == k)
            _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
