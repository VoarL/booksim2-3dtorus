// Create new file: routers/dor_allocator.cpp

#include "dor_allocator.hpp"
#include "globals.hpp"
#include <algorithm>
#include <iostream>

DORAllocator::DORAllocator(Module *parent, const string& name,
                           int inputs, int outputs) :
  SparseAllocator(parent, name, inputs, outputs),
  _grants(outputs, -1),
  _gptrs(outputs, 0),  // Grant pointers for round-robin within same priority
  _debug(false)
{
  // Initialize priority mapping based on router port configuration
  _InitializePriorityMapping();
}

void DORAllocator::_InitializePriorityMapping() {
  // Map input ports to dimensional priorities
  // This assumes standard 4-port or 7-port router layout
  
  _input_priorities.resize(_inputs, 0); // Default priority
  
  if (gN == 3) { // 3D network
    // Z-first, Y-second, X-last priority (0 = highest priority)
    // 4-port mapping: 0=X, 1=Y, 2=Z, 3=PE
    if (_inputs == 4) {
      _input_priorities[0] = 2; // X dimension - lowest priority
      _input_priorities[1] = 1; // Y dimension - second priority  
      _input_priorities[2] = 0; // Z dimension - highest priority
      _input_priorities[3] = 3; // PE injection - lowest priority
    }
    // 7-port mapping: 0=East, 1=West, 2=South, 3=North, 4=Up, 5=Down, 6=PE
    else if (_inputs == 7) {
      _input_priorities[0] = 2; // East (X) - lowest priority
      _input_priorities[1] = 2; // West (X) - lowest priority
      _input_priorities[2] = 1; // South (Y) - second priority
      _input_priorities[3] = 1; // North (Y) - second priority
      _input_priorities[4] = 0; // Up (Z) - highest priority
      _input_priorities[5] = 0; // Down (Z) - highest priority
      _input_priorities[6] = 3; // PE - lowest priority
    }
  } else if (gN == 2) {
    // 2D network - Y-first, X-second priority
    // Assuming port 0=X, port 1=Y, port 2=PE
    _input_priorities[0] = 1; // X dimension - second priority
    _input_priorities[1] = 0; // Y dimension - highest priority
    _input_priorities[_inputs-1] = 2; // PE - lowest priority
  } else {
    // 1D or other - just dimension order priority
    for (int i = 0; i < _inputs - 1; ++i) {
      _input_priorities[i] = (_inputs - 2) - i; // Reverse order (higher dim = higher priority)
    }
    _input_priorities[_inputs-1] = _inputs; // PE lowest priority
  }
  
  if (_debug) {
    cout << "DORAllocator: Dimensional priority Z→Y→X, Input priorities: ";
    for (int i = 0; i < _inputs; ++i) {
      cout << "Port" << i << "=P" << _input_priorities[i] << " ";
    }
    cout << endl;
  }
}

void DORAllocator::Allocate() {
  // Clear previous grants
  fill(_grants.begin(), _grants.end(), -1);
  
  // Process all output ports
  for (int output = 0; output < _outputs; ++output) {
    
    // Skip if output already matched or no requests
    if (_outmatch[output] != -1 || _out_req[output].empty()) {
      continue;
    }
    
    // Find highest priority request for this output
    int best_input = -1;
    int best_priority = 999; // Lower number = higher priority
    
    // Check all inputs requesting this output
    for (auto& req : _out_req[output]) {
      int input = req.second.port;
      int priority = _input_priorities[input];
      
      // Skip if input already matched
      if (_inmatch[input] != -1) continue;
      
      // Choose this input if it has higher priority (lower number)
      if (priority < best_priority) {
        best_priority = priority;
        best_input = input;
      }
      // Tie-breaking with round-robin
      else if (priority == best_priority && best_input != -1) {
        // Use grant pointer for round-robin within same priority
        int current_ptr = _gptrs[output];
        if ((input >= current_ptr && best_input < current_ptr) ||
            (input >= current_ptr && input < best_input) ||
            (best_input < current_ptr && input < best_input)) {
          best_input = input;
        }
      }
    }
    
    // Make the match if we found a valid input
    if (best_input != -1) {
      _inmatch[best_input] = output;
      _outmatch[output] = best_input;
      _grants[output] = best_input;
      
      // Update grant pointer for round-robin
      _gptrs[output] = (best_input + 1) % _inputs;
    }
  }
  
  if (_debug) {
    cout << "DORAllocator grants: ";
    for (int o = 0; o < _outputs; ++o) {
      if (_grants[o] != -1) {
        cout << "Out" << o << "←In" << _grants[o] << "(P" << _input_priorities[_grants[o]] << ") ";
      }
    }
    cout << endl;
  }
}

void DORAllocator::_ProcessPriorityGroup(const vector<pair<int, int>>& requests, int priority) {
  // Within same priority level, use round-robin starting from grant pointer
  
  map<int, vector<int>> output_requests; // output -> list of input ports
  
  // Group by output port
  for (auto& req : requests) {
    int output = req.first;
    int input = req.second;
    output_requests[output].push_back(input);
  }
  
  // For each output with requests, pick winner using round-robin
  for (auto& entry : output_requests) {
    int output = entry.first;
    vector<int>& inputs = entry.second;
    
    if (_grants[output] != -1) continue; // Already granted to higher priority
    
    // Start checking from grant pointer position
    int start_pos = _gptrs[output] % inputs.size();
    
    for (int i = 0; i < (int)inputs.size(); ++i) {
      int idx = (start_pos + i) % inputs.size();
      int input = inputs[idx];
      
      // Check if this input is already matched
      bool input_free = true;
      for (int other_out = 0; other_out < _outputs; ++other_out) {
        if (_grants[other_out] == input) {
          input_free = false;
          break;
        }
      }
      
      if (input_free) {
        _grants[output] = input;
        _gptrs[output] = (idx + 1) % inputs.size(); // Update pointer for next time
        break;
      }
    }
  }
}

void DORAllocator::_UpdateGrantPointers() {
  // Advance grant pointers when successful match found
  for (int output = 0; output < _outputs; ++output) {
    if (_grants[output] != -1) {
      _gptrs[output] = (_gptrs[output] + 1) % _inputs;
    }
  }
}
