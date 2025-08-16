// $Id$

#ifndef _UNITORUS_HPP_
#define _UNITORUS_HPP_

#include "network.hpp"
#include <vector>

class UniTorus : public Network {

  vector<int> _dim_sizes;  // Size of each dimension

  // Direction-specific properties
  vector<int> _dim_bandwidth;
  vector<int> _dim_latency;
  vector<float> _dim_penalty;
  vector<vector<int>> _nearest_elevator; 
  string _vertical_topology;
  
  // Debug flag
  bool _debug;

  void _ComputeSize( const Configuration &config );
  void _BuildNet( const Configuration &config );
  void _ParseDirectionConfig( const Configuration &config );
  void _ParseElevatorMapping( const string& mapping_str );

  // Unidirectional helper functions (only positive direction)
  int _NextChannel( int node, int dim );
  int _NextNode( int node, int dim );
  
  // Coordinate conversion functions
  vector<int> _NodeToCoords( int node ) const;
  int _CoordsToNode( const vector<int>& coords ) const;

public:
  UniTorus( const Configuration &config, const string & name );
  static void RegisterRoutingFunctions();

  const vector<vector<int>>& GetNearestElevatorMapping() const;

  int GetN( ) const;
  int GetDimSize( int dim ) const;
  const vector<int>& GetDimSizes( ) const;
  int GetDimLatency( int dim ) const;
  float GetDimPenalty( int dim ) const;

  double Capacity( ) const;

  void InsertRandomFaults( const Configuration &config );

};

#endif
