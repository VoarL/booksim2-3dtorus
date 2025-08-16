#ifndef _DOR_ALLOCATOR_HPP_
#define _DOR_ALLOCATOR_HPP_

#include "allocator.hpp"
#include <vector>
#include <map>

class DORAllocator : public SparseAllocator {
private:
  vector<int> _grants;           // Grant results: output -> input
  vector<int> _gptrs;            // Grant pointers for round-robin
  vector<int> _input_priorities; // Priority level for each input port
  
  bool _debug;

  void _InitializePriorityMapping();
  void _ProcessPriorityGroup(const vector<pair<int, int>>& requests, int priority);
  void _UpdateGrantPointers();

public:
  DORAllocator(Module *parent, const string& name, int inputs, int outputs);
  virtual ~DORAllocator() {}
  
  virtual void Allocate();
};

#endif

