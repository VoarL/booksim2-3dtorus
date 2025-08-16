// $Id$

/*unitorus.cpp
 *
 * Unidirectional Torus with dimension-ordered routing
 * Each link direction has configurable bandwidth and penalties
 *
 */

#include "booksim.hpp"
#include <vector>
#include <sstream>
#include <ctime>
#include <cassert>
#include <iostream>
#include <cstdlib>
#include <string>
#include "unitorus.hpp"
#include "random_utils.hpp"
#include "misc_utils.hpp"
#include "routefunc.hpp"

UniTorus::UniTorus( const Configuration &config, const string & name ) :
Network( config, name )
{
  _debug = config.GetInt("unitorus_debug");
  _ComputeSize( config );
  _ParseDirectionConfig( config );
  _Alloc( );
  
  // Verify allocation worked
  if (_debug) {
    cout << "Verifying channel allocation:" << endl;
    for (int c = 0; c < _channels; ++c) {
      if (_chan[c] == nullptr || _chan_cred[c] == nullptr) {
        cerr << "ERROR: Channel " << c << " not allocated properly" << endl;
        exit(-1);
      }
    }
    cout << "All " << _channels << " channels allocated successfully" << endl;
  }
  
  _BuildNet( config );
}

void UniTorus::_ComputeSize( const Configuration &config )
{
  // Parse dimension sizes from comma-separated string
  string dim_sizes_str = config.GetStr("dim_sizes");
  
  if (dim_sizes_str.empty() || dim_sizes_str == "0") {
    cerr << "Error: dim_sizes must be specified as comma-separated values (e.g., dim_sizes = 4,6,8)" << endl;
    exit(-1);
  }
  
  // Parse comma-separated dimension sizes
  _dim_sizes.clear();
  
  // Remove braces if present (config parser format: {val1,val2,val3})
  string clean_str = dim_sizes_str;
  if (!clean_str.empty() && clean_str.front() == '{') {
    clean_str = clean_str.substr(1);
  }
  if (!clean_str.empty() && clean_str.back() == '}') {
    clean_str = clean_str.substr(0, clean_str.length() - 1);
  }
  
  vector<string> tokens;
  size_t start = 0, end = 0;
  while ((end = clean_str.find(',', start)) != string::npos) {
    tokens.push_back(clean_str.substr(start, end - start));
    start = end + 1;
  }
  tokens.push_back(clean_str.substr(start));
  
  for (const string& token : tokens) {
    int dim_size = atoi(token.c_str());
    if (dim_size <= 0) {
      cerr << "Error: All dimension sizes must be positive integers. Found: " << token << endl;
      cerr << "Expected format: dim_sizes = {size1,size2,...,sizeN} (e.g., dim_sizes = {4,6,8})" << endl;
      exit(-1);
    }
    _dim_sizes.push_back(dim_size);
  }
  
  // Calculate total network size
  _size = 1;
  for (int i = 0; i < (int)_dim_sizes.size(); ++i) {
    _size *= _dim_sizes[i];
  }
  
  // For global compatibility (some routing functions may still use these)
  gN = _dim_sizes.size();
  gK = _dim_sizes[0]; // Use first dimension as default for legacy compatibility
  gDimSizes = _dim_sizes; // Set global dimension sizes for routing functions
  gDimPenalties = _dim_penalty; // Set global dimension penalties for routing functions
  gDimBandwidths = _dim_bandwidth; // Set global dimension bandwidths for routing functions
  gVerticalTopology = _vertical_topology;
  // Parse elevator mapping
  string elevator_mapping_str = config.GetStr("elevator_mapping_coords");
  if (!elevator_mapping_str.empty()) {
      _ParseElevatorMapping(elevator_mapping_str);
      gElevatorMapping = _nearest_elevator;
  }
  // Calculate channels after bandwidth is parsed
  // We'll update this in _ParseDirectionConfig()
  _channels = 0; // Temporary, will be calculated later

  _nodes = _size;
  
  if (_debug) {
    cout << "UniTorus dimensions: ";
    for (int i = 0; i < (int)_dim_sizes.size(); ++i) {
      cout << _dim_sizes[i];
      if (i < (int)_dim_sizes.size() - 1) cout << "x";
    }
    cout << " = " << _size << " nodes" << endl;
  }
}

void UniTorus::_ParseDirectionConfig( const Configuration &config )
{
  int num_dims = _dim_sizes.size();
  
  // Initialize vectors for each dimension
  _dim_bandwidth.resize(num_dims, 1);  // Default bandwidth = 1
  _dim_latency.resize(num_dims, 1);    // Default latency = 1
  _dim_penalty.resize(num_dims, 0.0);    // Default penalty = 0

  // Helper function to parse and validate comma-separated values
  auto parseAndValidate = [num_dims](const string& param_str, const string& param_name) -> vector<int> {
    vector<int> values;
    if (param_str.empty() || param_str == "0") {
      return values; // Return empty vector for default handling
    }
    
    // Remove braces if present (config parser format: {val1,val2,val3})
    string clean_str = param_str;
    if (!clean_str.empty() && clean_str.front() == '{') {
      clean_str = clean_str.substr(1);
    }
    if (!clean_str.empty() && clean_str.back() == '}') {
      clean_str = clean_str.substr(0, clean_str.length() - 1);
    }
    
    vector<string> tokens;
    size_t start = 0, end = 0;
    while ((end = clean_str.find(',', start)) != string::npos) {
      string token = clean_str.substr(start, end - start);
      // Trim whitespace
      token.erase(0, token.find_first_not_of(" \t"));
      token.erase(token.find_last_not_of(" \t") + 1);
      if (!token.empty()) {
        tokens.push_back(token);
      }
      start = end + 1;
    }
    string last_token = clean_str.substr(start);
    last_token.erase(0, last_token.find_first_not_of(" \t"));
    last_token.erase(last_token.find_last_not_of(" \t") + 1);
    if (!last_token.empty()) {
      tokens.push_back(last_token);
    }
    
    // Validate count matches number of dimensions
    if ((int)tokens.size() != num_dims) {
      cerr << "Error: " << param_name << " has " << tokens.size() 
           << " values but topology has " << num_dims << " dimensions." << endl;
      cerr << "Expected format: " << param_name << " = {val1,val2,...,val" << num_dims << "}" << endl;
      exit(-1);
    }
    
    // Convert to integers and validate
    for (const string& token : tokens) {
      int val = atoi(token.c_str());
      if (val <= 0) {
        cerr << "Error: All values in " << param_name << " must be positive integers. Found: " << token << endl;
        exit(-1);
      }
      values.push_back(val);
    }
    
    return values;
  };

  // Helper function for penalty parsing (allows zero)
  auto parseAndValidatePenalty = [num_dims](const string& param_str, const string& param_name) -> vector<int> {
    vector<int> values;
    if (param_str.empty() || param_str == "0") {
      return values; // Return empty vector for default handling
    }
    
    // Remove braces if present (config parser format: {val1,val2,val3})
    string clean_str = param_str;
    if (!clean_str.empty() && clean_str.front() == '{') {
      clean_str = clean_str.substr(1);
    }
    if (!clean_str.empty() && clean_str.back() == '}') {
      clean_str = clean_str.substr(0, clean_str.length() - 1);
    }
    
    vector<string> tokens;
    size_t start = 0, end = 0;
    while ((end = clean_str.find(',', start)) != string::npos) {
      string token = clean_str.substr(start, end - start);
      // Trim whitespace
      token.erase(0, token.find_first_not_of(" \t"));
      token.erase(token.find_last_not_of(" \t") + 1);
      if (!token.empty()) {
        tokens.push_back(token);
      }
      start = end + 1;
    }
    string last_token = clean_str.substr(start);
    last_token.erase(0, last_token.find_first_not_of(" \t"));
    last_token.erase(last_token.find_last_not_of(" \t") + 1);
    if (!last_token.empty()) {
      tokens.push_back(last_token);
    }
    
    // Validate count matches number of dimensions
    if ((int)tokens.size() != num_dims) {
      cerr << "Error: " << param_name << " has " << tokens.size() 
           << " values but topology has " << num_dims << " dimensions." << endl;
      cerr << "Expected format: " << param_name << " = {val1,val2,...,val" << num_dims << "}" << endl;
      exit(-1);
    }
    
    // Convert to integers and validate (allow zero for penalties)
    for (const string& token : tokens) {
      int val = atoi(token.c_str());
      if (val < 0) {
        cerr << "Error: All values in " << param_name << " must be non-negative integers. Found: " << token << endl;
        exit(-1);
      }
      values.push_back(val);
    }
    
    return values;
  };

  // Helper function for penalty parsing (allows zero, returns float)
  auto parseAndValidatePenaltyFloat = [num_dims](const string& param_str, const string& param_name) -> vector<float> {
    vector<float> values;
    if (param_str.empty() || param_str == "0") {
      return values; // Return empty vector for default handling
    }
    
    // Remove braces if present (config parser format: {val1,val2,val3})
    string clean_str = param_str;
    if (!clean_str.empty() && clean_str.front() == '{') {
      clean_str = clean_str.substr(1);
    }
    if (!clean_str.empty() && clean_str.back() == '}') {
      clean_str = clean_str.substr(0, clean_str.length() - 1);
    }
    
    vector<string> tokens;
    size_t start = 0, end = 0;
    while ((end = clean_str.find(',', start)) != string::npos) {
      string token = clean_str.substr(start, end - start);
      // Trim whitespace
      token.erase(0, token.find_first_not_of(" \t"));
      token.erase(token.find_last_not_of(" \t") + 1);
      if (!token.empty()) {
        tokens.push_back(token);
      }
      start = end + 1;
    }
    string last_token = clean_str.substr(start);
    last_token.erase(0, last_token.find_first_not_of(" \t"));
    last_token.erase(last_token.find_last_not_of(" \t") + 1);
    if (!last_token.empty()) {
      tokens.push_back(last_token);
    }
    
    // Validate count matches number of dimensions
    if ((int)tokens.size() != num_dims) {
      cerr << "Error: " << param_name << " has " << tokens.size() 
           << " values but topology has " << num_dims << " dimensions." << endl;
      cerr << "Expected format: " << param_name << " = {val1,val2,...,val" << num_dims << "}" << endl;
      exit(-1);
    }
    
    // Convert to floats and validate (allow zero for penalties)
    for (const string& token : tokens) {
      float val = atof(token.c_str());
      if (val < 0.0f) {
        cerr << "Error: All values in " << param_name << " must be non-negative numbers. Found: " << token << endl;
        exit(-1);
      }
      values.push_back(val);
    }
    
    return values;
  };

  // Parse and validate bandwidth
  string bandwidth_str = config.GetStr("dim_bandwidth");
  vector<int> bandwidth_values = parseAndValidate(bandwidth_str, "dim_bandwidth");
  if (!bandwidth_values.empty()) {
    _dim_bandwidth = bandwidth_values;
    gDimBandwidths = _dim_bandwidth; // Update global for routing functions
  }

  // Parse and validate latency
  string latency_str = config.GetStr("dim_latency");
  vector<int> latency_values = parseAndValidate(latency_str, "dim_latency");
  if (!latency_values.empty()) {
    _dim_latency = latency_values;
  }

  // Parse and validate penalty (allows zero)
  string penalty_str = config.GetStr("dim_penalty");
  vector<float> penalty_values = parseAndValidatePenaltyFloat(penalty_str, "dim_penalty");
  if (!penalty_values.empty()) {
    _dim_penalty = penalty_values;
    gDimPenalties = _dim_penalty; // Update global for routing functions
  }

  // Parse and validate _vertical_topology
  _vertical_topology = config.GetStr("vertical_topology");
  gVerticalTopology = _vertical_topology; // Update global for routing functions
  bool is_vertical_mesh = (gVerticalTopology == "mesh");

  // Calculate total channels - one per dimension per node
  // Bandwidth will affect channel capacity, not number of channels
  if (is_vertical_mesh && _dim_sizes.size() > 2) {
    // For mesh: X + Y + Z-up + Z-down
    int xy_channels = 2 * _size;  // X and Y dimensions
    int nodes_per_layer = _dim_sizes[0] * _dim_sizes[1];
    int z_layers = _dim_sizes[2];
    int z_up_channels = (z_layers - 1) * nodes_per_layer;
    int z_down_channels = (z_layers - 1) * nodes_per_layer;
    _channels = xy_channels + z_up_channels + z_down_channels;
    // For 3×3×2: 36 + 9 + 9 = 54 channels
  } else {
    // For torus: original calculation
    _channels = _dim_sizes.size() * _size;  // 3 × 18 = 54
  }

  if (_debug) {
    cout << "DEBUG: Total channels allocated for " << (is_vertical_mesh ? "mesh" : "torus") 
        << " mode: " << _channels << endl;
  }

  // Print configuration
  if (_debug) {
    cout << "vertical topology" << gVerticalTopology << endl;
    cout << "UniTorus Direction Configuration:" << endl;
    for (int i = 0; i < num_dims; ++i) {
      cout << "  Dimension " << i << ": size=" << _dim_sizes[i]
           << ", bandwidth=" << _dim_bandwidth[i] 
           << ", latency=" << _dim_latency[i] 
           << ", penalty=" << _dim_penalty[i] << endl;
    }
    cout << "Total channels: " << _channels << endl;
  }
}

void UniTorus::RegisterRoutingFunctions() {

}

void UniTorus::_BuildNet( const Configuration &config )
{
  ostringstream router_name;

  if (_debug) {
    cout << "Building Unidirectional " << _dim_sizes.size() << "-D Torus" << endl;
    cout << "Dimensions: ";
    for (int i = 0; i < (int)_dim_sizes.size(); ++i) {
      cout << _dim_sizes[i];
      if (i < (int)_dim_sizes.size() - 1) cout << "x";
    }
    cout << " = " << _size << " nodes, " << _channels << " channels" << endl;
  }

  // Validate network configuration
  int expected_size = 1;
  for (int dim : _dim_sizes) {
    expected_size *= dim;
  }
  
  if (_size != expected_size) {
    cerr << "ERROR: Network size mismatch!" << endl;
    cerr << "  Calculated size: " << expected_size << endl;
    cerr << "  Stored _size: " << _size << endl;
    exit(-1);
  }
  
  if (_debug) {
    cout << "DEBUG: Network validation passed - " << _size << " nodes" << endl;
  }

  bool is_vertical_mesh = (gVerticalTopology == "mesh");
  // Create routers
  for ( int node = 0; node < _size; ++node ) {
    if (_debug) cout << "Creating router for node " << node << endl;

    router_name << "router";
    
    // Generate router name based on coordinates
    vector<int> coords = _NodeToCoords(node);
    for (int i = 0; i < (int)_dim_sizes.size(); ++i) {
      router_name << "_" << coords[i];
    }

    if (_debug) {
      cout << "Router name: " << router_name.str() << endl;
      //cout << "Node " << node << " inputs=" << (_dim_sizes.size() + 1) << " outputs=" << (_dim_sizes.size() + 1) << endl;
    }

    // Each router has n output ports (one per dimension) + 1 injection + 1 ejection
    int net_ports = 2; // X, Y always
    if (is_vertical_mesh) {
      if (coords[2] < _dim_sizes[2] - 1) net_ports++; // Z-up
      if (coords[2] > 0) net_ports++;                 // Z-down
    } else {
      net_ports++; // Z torus
    }
    int total_ports = net_ports + 1; // + PE
    if (_debug) cout << "DEBUG: Node " << node << " coords(" << coords[0] << "," << coords[1] << "," << coords[2] << ") gets " << total_ports << " ports" << endl;

    _routers[node] = Router::NewRouter(config, this, router_name.str(), 
                                    node, total_ports, total_ports);
    //_routers[node] = Router::NewRouter( config, this, router_name.str( ), 
    //                                    node, _dim_sizes.size() + 1, _dim_sizes.size() + 1 );
    if (_routers[node] == nullptr) {
      cerr << "ERROR: Failed to create router for node " << node << endl;
      exit(-1);
    }
    
    _timed_modules.push_back(_routers[node]);

    if (_debug) cout << "Router created successfully" << endl;

    router_name.str("");
  }

  // Connect all the channels after all routers are created
  int channel_counter = 0;

  if (_debug) {
    cout << "DEBUG: Starting channel connections, is_vertical_mesh = " << is_vertical_mesh << endl;
    cout << "DEBUG: _channels allocated = " << _channels << endl;
  }

  for ( int node = 0; node < _size; ++node ) {
    for ( int dim = 0; dim < (int)_dim_sizes.size(); ++dim ) {
      
      if (dim == 2 && is_vertical_mesh) {
        if (_debug) {
          cout << "DEBUG: Processing Z-dimension for node " << node << " in mesh mode" << endl;
        }
        vector<int> coords = _NodeToCoords(node);
        
        // Z-up connection (if not at top layer)
        if (coords[2] < _dim_sizes[2] - 1) {
          int up_node = node + (_dim_sizes[0] * _dim_sizes[1]);
          int up_channel = channel_counter;
          
          if (up_channel >= _channels) {
            cout << "ERROR: Z-up channel " << up_channel << " exceeds allocated channels " << _channels << endl;
            exit(-1);
          }
          
          _routers[node]->AddOutputChannel(_chan[up_channel], _chan_cred[up_channel]);
          _routers[up_node]->AddInputChannel(_chan[up_channel], _chan_cred[up_channel]);
          
          _chan[up_channel]->SetLatency(_dim_latency[dim]);
          _chan_cred[up_channel]->SetLatency(_dim_latency[dim]);
          channel_counter++;
        }
        
        // Z-down connection (if not at bottom layer)
        if (coords[2] > 0) {
          int down_node = node - (_dim_sizes[0] * _dim_sizes[1]);
          int down_channel = channel_counter;
          
          if (down_channel >= _channels) {
            cout << "ERROR: Z-down channel " << down_channel << " exceeds allocated channels " << _channels << endl;
            exit(-1);
          }
          
          _routers[node]->AddOutputChannel(_chan[down_channel], _chan_cred[down_channel]);
          _routers[down_node]->AddInputChannel(_chan[down_channel], _chan_cred[down_channel]);
          
          _chan[down_channel]->SetLatency(_dim_latency[dim]);
          _chan_cred[down_channel]->SetLatency(_dim_latency[dim]);
          channel_counter++;
        }
        
      } else {
        // Normal X,Y dimension connections
        int next_node = _NextNode( node, dim );
        int channel = channel_counter;

        if (_debug) {
          cout << "DEBUG: Normal connection dim " << dim << " - node " << node 
              << " -> node " << next_node << " via channel " << channel << endl;
        }

        _routers[node]->AddOutputChannel(_chan[channel], _chan_cred[channel]);
        _routers[next_node]->AddInputChannel(_chan[channel], _chan_cred[channel]);

        _chan[channel]->SetLatency( _dim_latency[dim] );
        _chan_cred[channel]->SetLatency( _dim_latency[dim] );
        channel_counter++;
      }
    }
  }

  
  // Add injection and ejection channels for all routers
  for ( int node = 0; node < _size; ++node ) {
    // Add injection and ejection channels
    _routers[node]->AddInputChannel( _inject[node], _inject_cred[node] );
    _routers[node]->AddOutputChannel( _eject[node], _eject_cred[node] );
    _inject[node]->SetLatency( 1 );
    _inject_cred[node]->SetLatency( 1 );
    _eject[node]->SetLatency( 1 );
    _eject_cred[node]->SetLatency( 1 );
  }

  // After ALL channel connections (including injection/ejection)
  if (_debug) {
    cout << "DEBUG: Final port usage validation:" << endl;
    for (int node = 0; node < _size; ++node) {
      vector<int> coords = _NodeToCoords(node);
      
      // Count expected connections for this router
      int expected_inputs = 2;  // X, Y inputs
      int expected_outputs = 2; // X, Y outputs
      
      if (is_vertical_mesh) {
        if (coords[2] < _dim_sizes[2] - 1) expected_outputs++; // Z-up output
        if (coords[2] > 0) expected_outputs++;                 // Z-down output
        if (coords[2] > 0) expected_inputs++;                  // Z-down input 
        if (coords[2] < _dim_sizes[2] - 1) expected_inputs++;  // Z-up input
      } else {
        expected_inputs++;  // Z input
        expected_outputs++; // Z output  
      }
      
      expected_inputs++;  // PE injection
      expected_outputs++; // PE ejection
      
      cout << "Router " << node << " coords(" << coords[0] << "," << coords[1] 
          << "," << coords[2] << "): expected " << expected_inputs << "/" 
          << expected_outputs << " connections, allocated " 
          << _routers[node]->NumInputs() << "/" << _routers[node]->NumOutputs() 
          << " ports" << endl;
          
      if (expected_inputs > _routers[node]->NumInputs() || 
          expected_outputs > _routers[node]->NumOutputs()) {
        cout << "ERROR: Port overflow on router " << node << "!" << endl;
      }
    }
  }
}

void UniTorus::_ParseElevatorMapping(const string& mapping_str) {
    cout << "DEBUG: Starting _ParseElevatorMapping with input: " << mapping_str << endl;
    // Calculate expected number of coordinates
    int expected_coords = _dim_sizes[0] * _dim_sizes[1] * 2; // 2 coords per (x,y) position
    
    if (_debug) {
      cout << "DEBUG: Network size " << _dim_sizes[0] << "x" << _dim_sizes[1] 
          << " expects " << expected_coords << " elevator coordinates" << endl;
    }
    // Parse format: [1,1,6,0,3,7,1,1,3,1,0,8,1,9,2,2,4,7]
    // Each pair (x,y) represents nearest elevator for that grid position
    
    _nearest_elevator.clear();
    int grid_size = _dim_sizes[0] * _dim_sizes[1]; // X*Y grid size
    cout << "DEBUG: Grid size (X*Y): " << _dim_sizes[0] << "*" << _dim_sizes[1] << " = " << grid_size << endl;
    
    _nearest_elevator.resize(grid_size);
    cout << "DEBUG: Resized _nearest_elevator to " << grid_size << " elements" << endl;
    
    // Remove brackets and parse comma-separated values
    string clean_str = mapping_str;
    if (_debug) cout << "DEBUG: Original string: " << clean_str << endl;
    
    if (!clean_str.empty() && clean_str.front() == '{') {
        clean_str = clean_str.substr(1);
        if (_debug) cout << "DEBUG: Removed opening brace" << endl;
    }
    if (!clean_str.empty() && clean_str.back() == '}') {
        clean_str = clean_str.substr(0, clean_str.length() - 1);
        if (_debug) cout << "DEBUG: Removed closing brace" << endl;
    }
    
    if (_debug) {
      cout << "DEBUG: Clean string: " << clean_str << endl;
    }
    
    vector<int> coords;
    size_t start = 0, end = 0;
    while ((end = clean_str.find(',', start)) != string::npos) {
        string token = clean_str.substr(start, end - start);
        int val = atoi(token.c_str());
        coords.push_back(val);
        start = end + 1;
    }
    string last_token = clean_str.substr(start);
    int last_val = atoi(last_token.c_str());
    coords.push_back(last_val);
    if (_debug) {
      cout << "DEBUG: Parsed last coordinate: " << last_val << endl;
      
      cout << "DEBUG: Total coordinates parsed: " << coords.size() << endl;
      cout << "DEBUG: Expected pairs: " << grid_size << " (need " << grid_size * 2 << " coordinates)" << endl;
    }
    if ((int) coords.size() != grid_size * 2) {
      cerr << "ERROR: Coordinate count mismatch! Got " << coords.size() 
          << " but need " << grid_size * 2 << endl;
      cerr << "Expected: " << (_dim_sizes[0]) << "x" << (_dim_sizes[1]) 
          << " = " << grid_size << " positions × 2 coordinates each" << endl;
      cerr << "Provided: " << coords.size() << " coordinates" << endl;
      exit(-1);  // ← This stops the entire program
    }

    // VALIDATION: Check coordinate count
    if ((int)coords.size() != expected_coords) {
      cerr << "ERROR: Elevator mapping coordinate mismatch!" << endl;
      cerr << "  Network size: " << _dim_sizes[0] << "x" << _dim_sizes[1] 
          << " (" << (_dim_sizes[0] * _dim_sizes[1]) << " positions)" << endl;
      cerr << "  Expected coordinates: " << expected_coords 
          << " (2 per position)" << endl;
      cerr << "  Provided coordinates: " << coords.size() << endl;
      cerr << "  Mapping string: " << mapping_str << endl;
      exit(-1);
    }
    
    // Convert pairs to elevator coordinates
    for (int i = 0; i < (int)coords.size(); i += 2) {
        int grid_pos = i / 2;
        
        if (grid_pos >= grid_size) {
            cerr << "ERROR: grid_pos " << grid_pos << " exceeds grid_size " << grid_size << endl;
            return;
        }
        if (_debug) {
          cout << "DEBUG: Setting grid_pos " << grid_pos << " to elevator (" 
              << coords[i] << "," << coords[i+1] << ")" << endl;
        }
             
        _nearest_elevator[grid_pos] = {coords[i], coords[i+1]};
    }
    
    gElevatorMapping = _nearest_elevator;
}

const vector<vector<int>>& UniTorus::GetNearestElevatorMapping() const {
    return _nearest_elevator;
}


int UniTorus::_NextChannel( int node, int dim )
{
  // Calculate the channel index for a given node and dimension
  // Each node has one channel per dimension, bandwidth affects capacity
  return node * _dim_sizes.size() + dim;
}

int UniTorus::_NextNode( int node, int dim )
{
  vector<int> coords = _NodeToCoords(node);
  
  // Move to next coordinate in this dimension (with wraparound)
  coords[dim] = (coords[dim] + 1) % _dim_sizes[dim];
  
  return _CoordsToNode(coords);
}

vector<int> UniTorus::_NodeToCoords( int node ) const
{
  vector<int> coords(_dim_sizes.size());
  int temp = node;
  
  for (int dim = 0; dim < (int)_dim_sizes.size(); ++dim) {
    coords[dim] = temp % _dim_sizes[dim];
    temp /= _dim_sizes[dim];
  }
  
  return coords;
}

int UniTorus::_CoordsToNode( const vector<int>& coords ) const
{
  int node = 0;
  int multiplier = 1;
  
  for (int dim = 0; dim < (int)_dim_sizes.size(); ++dim) {
    node += coords[dim] * multiplier;
    multiplier *= _dim_sizes[dim];
  }
  
  return node;
}

int UniTorus::GetN( ) const
{
  return _dim_sizes.size();
}

int UniTorus::GetDimSize( int dim ) const
{
  return _dim_sizes[dim];
}

const vector<int>& UniTorus::GetDimSizes( ) const
{
  return _dim_sizes;
}

int UniTorus::GetDimLatency( int dim ) const
{
  return _dim_latency[dim];
}

float UniTorus::GetDimPenalty( int dim ) const
{
  return _dim_penalty[dim];
}

double UniTorus::Capacity( ) const
{
  // Calculate total capacity considering per-dimension bandwidths
  double total_capacity = 0.0;
  for ( int dim = 0; dim < (int)_dim_sizes.size(); ++dim ) {
    total_capacity += (double)_dim_bandwidth[dim];
  }
  return total_capacity;
}

void UniTorus::InsertRandomFaults( const Configuration &config )
{
  // TODO: Implement random fault insertion if needed
}
