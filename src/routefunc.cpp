// $Id$

/*
 Copyright (c) 2007-2015, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*routefunc.cpp
 *
 *This is where most of the routing functions reside. Some of the topologies
 *has their own "register routing functions" which must be called to access
 *those routing functions. 
 *
 *After writing a routing function, don't forget to register it. The reg 
 *format is rfname_topologyname. 
 *
 */

#include <map>
#include <cstdlib>
#include <cassert>

#include "booksim.hpp"
#include "routefunc.hpp"
#include "kncube.hpp"
#include "random_utils.hpp"
#include "misc_utils.hpp"
#include "fattree.hpp"
#include "tree4.hpp"
#include "qtree.hpp"
#include "cmesh.hpp"



map<string, tRoutingFunction> gRoutingFunctionMap;
/* Global information used by routing functions */

int gNumVCs;

/* Add more functions here
 *
 */

// ============================================================
//  Balfour-Schultz
int gReadReqBeginVC, gReadReqEndVC;
int gWriteReqBeginVC, gWriteReqEndVC;
int gReadReplyBeginVC, gReadReplyEndVC;
int gWriteReplyBeginVC, gWriteReplyEndVC;

// ============================================================
//  QTree: Nearest Common Ancestor
// ===
void qtree_nca( const Router *r, const Flit *f,
		int in_channel, OutputSet* outputs, bool inject)
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else {

    int height = QTree::HeightFromID( r->GetID() );
    int pos    = QTree::PosFromID( r->GetID() );
    
    int dest   = f->dest;
    
    for (int i = height+1; i < gN; i++) 
      dest /= gK;
    if ( pos == dest / gK ) 
      // Route down to child
      out_port = dest % gK ; 
    else
      // Route up to parent
      out_port = gK;        

  }

  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

// ============================================================
//  Tree4: Nearest Common Ancestor w/ Adaptive Routing Up
// ===
void tree4_anca( const Router *r, const Flit *f,
		 int in_channel, OutputSet* outputs, bool inject)
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int range = 1;
  
  int out_port;

  if(inject) {

    out_port = -1;

  } else {

    int dest = f->dest;
    
    const int NPOS = 16;
    
    int rH = r->GetID( ) / NPOS;
    int rP = r->GetID( ) % NPOS;
    
    if ( rH == 0 ) {
      dest /= 16;
      out_port = 2 * dest + RandomInt(1);
    } else if ( rH == 1 ) {
      dest /= 4;
      if ( dest / 4 == rP / 2 )
	out_port = dest % 4;
      else {
	out_port = gK;
	range = gK;
      }
    } else {
      if ( dest/4 == rP )
	out_port = dest % 4;
      else {
	out_port = gK;
	range = 2;
      }
    }
    
    //  cout << "Router("<<rH<<","<<rP<<"): id= " << f->id << " dest= " << f->dest << " out_port = "
    //       << out_port << endl;

  }

  outputs->Clear( );

  for (int i = 0; i < range; ++i) 
    outputs->AddRange( out_port + i, vcBegin, vcEnd );
}

// ============================================================
//  Tree4: Nearest Common Ancestor w/ Random Routing Up
// ===
void tree4_nca( const Router *r, const Flit *f,
		int in_channel, OutputSet* outputs, bool inject)
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else {

    int dest = f->dest;
    
    const int NPOS = 16;
    
    int rH = r->GetID( ) / NPOS;
    int rP = r->GetID( ) % NPOS;
    
    if ( rH == 0 ) {
      dest /= 16;
      out_port = 2 * dest + RandomInt(1);
    } else if ( rH == 1 ) {
      dest /= 4;
      if ( dest / 4 == rP / 2 )
	out_port = dest % 4;
      else
	out_port = gK + RandomInt(gK-1);
    } else {
      if ( dest/4 == rP )
	out_port = dest % 4;
      else
	out_port = gK + RandomInt(1);
    }
    
    //  cout << "Router("<<rH<<","<<rP<<"): id= " << f->id << " dest= " << f->dest << " out_port = "
    //       << out_port << endl;

  }

  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

// ============================================================
//  FATTREE: Nearest Common Ancestor w/ Random  Routing Up
// ===
void fattree_nca( const Router *r, const Flit *f,
               int in_channel, OutputSet* outputs, bool inject)
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else {
    
    int dest = f->dest;
    int router_id = r->GetID(); //routers are numbered with smallest at the top level
    int routers_per_level = powi(gK, gN-1);
    int pos = router_id%routers_per_level;
    int router_depth  = router_id/ routers_per_level; //which level
    int routers_per_neighborhood = powi(gK,gN-router_depth-1);
    int router_neighborhood = pos/routers_per_neighborhood; //coverage of this tree
    int router_coverage = powi(gK, gN-router_depth);  //span of the tree from this router
    

    //NCA reached going down
    if(dest <(router_neighborhood+1)* router_coverage && 
       dest >=router_neighborhood* router_coverage){
      //down ports are numbered first

      //ejection
      if(router_depth == gN-1){
	out_port = dest%gK;
      } else {	
	//find the down port for the destination
	int router_branch_coverage = powi(gK, gN-(router_depth+1)); 
	out_port = (dest-router_neighborhood* router_coverage)/router_branch_coverage;
      }
    } else {
      //up ports are numbered last
      assert(in_channel<gK);//came from a up channel
      out_port = gK+RandomInt(gK-1);
    }
  }  
  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

// ============================================================
//  FATTREE: Nearest Common Ancestor w/ Adaptive Routing Up
// ===
void fattree_anca( const Router *r, const Flit *f,
                int in_channel, OutputSet* outputs, bool inject)
{

  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));


  int out_port;

  if(inject) {

    out_port = -1;

  } else {


    int dest = f->dest;
    int router_id = r->GetID(); //routers are numbered with smallest at the top level
    int routers_per_level = powi(gK, gN-1);
    int pos = router_id%routers_per_level;
    int router_depth  = router_id/ routers_per_level; //which level
    int routers_per_neighborhood = powi(gK,gN-router_depth-1);
    int router_neighborhood = pos/routers_per_neighborhood; //coverage of this tree
    int router_coverage = powi(gK, gN-router_depth);  //span of the tree from this router
    

    //NCA reached going down
    if(dest <(router_neighborhood+1)* router_coverage && 
       dest >=router_neighborhood* router_coverage){
      //down ports are numbered first

      //ejection
      if(router_depth == gN-1){
	out_port = dest%gK;
      } else {	
	//find the down port for the destination
	int router_branch_coverage = powi(gK, gN-(router_depth+1)); 
	out_port = (dest-router_neighborhood* router_coverage)/router_branch_coverage;
      }
    } else {
      //up ports are numbered last
      assert(in_channel<gK);//came from a up channel
      out_port = gK;
      int random1 = RandomInt(gK-1); // Chose two ports out of the possible at random, compare loads, choose one.
      int random2 = RandomInt(gK-1);
      if (r->GetUsedCredit(out_port + random1) > r->GetUsedCredit(out_port + random2)){
	out_port = out_port + random2;
      }else{
	out_port =  out_port + random1;
      }
    }
  }  
  outputs->Clear( );
  
  outputs->AddRange( out_port, vcBegin, vcEnd );
}




// ============================================================
//  Mesh - adatpive XY,YX Routing 
//         pick xy or yx min routing adaptively at the source router
// ===

int dor_next_mesh( int cur, int dest, bool descending = false );

void adaptive_xy_yx_mesh( const Router *r, const Flit *f, 
		 int in_channel, OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else if(r->GetID() == f->dest) {

    // at destination router, we don't need to separate VCs by dim order
    out_port = 2*gN;

  } else {

    //each class must have at least 2 vcs assigned or else xy_yx will deadlock
    int const available_vcs = (vcEnd - vcBegin + 1) / 2;
    assert(available_vcs > 0);
    
    int out_port_xy = dor_next_mesh( r->GetID(), f->dest, false );
    int out_port_yx = dor_next_mesh( r->GetID(), f->dest, true );

    // Route order (XY or YX) determined when packet is injected
    //  into the network, adaptively
    bool x_then_y;
    if(in_channel < 2*gN){
      x_then_y =  (f->vc < (vcBegin + available_vcs));
    } else {
      int credit_xy = r->GetUsedCredit(out_port_xy);
      int credit_yx = r->GetUsedCredit(out_port_yx);
      if(credit_xy > credit_yx) {
	x_then_y = false;
      } else if(credit_xy < credit_yx) {
	x_then_y = true;
      } else {
	x_then_y = (RandomInt(1) > 0);
      }
    }
    
    if(x_then_y) {
      out_port = out_port_xy;
      vcEnd -= available_vcs;
    } else {
      out_port = out_port_yx;
      vcBegin += available_vcs;
    }

  }

  outputs->Clear();

  outputs->AddRange( out_port , vcBegin, vcEnd );
  
}

void xy_yx_mesh( const Router *r, const Flit *f, 
		 int in_channel, OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else if(r->GetID() == f->dest) {

    // at destination router, we don't need to separate VCs by dim order
    out_port = 2*gN;

  } else {

    //each class must have at least 2 vcs assigned or else xy_yx will deadlock
    int const available_vcs = (vcEnd - vcBegin + 1) / 2;
    assert(available_vcs > 0);

    // Route order (XY or YX) determined when packet is injected
    //  into the network
    bool x_then_y = ((in_channel < 2*gN) ?
		     (f->vc < (vcBegin + available_vcs)) :
		     (RandomInt(1) > 0));

    if(x_then_y) {
      out_port = dor_next_mesh( r->GetID(), f->dest, false );
      vcEnd -= available_vcs;
    } else {
      out_port = dor_next_mesh( r->GetID(), f->dest, true );
      vcBegin += available_vcs;
    }

  }

  outputs->Clear();

  outputs->AddRange( out_port , vcBegin, vcEnd );
  
}

//
// End Balfour-Schultz
//=============================================================

//=============================================================

int dor_next_mesh( int cur, int dest, bool descending )
{
  if ( cur == dest ) {
    return 2*gN;  // Eject
  }

  int dim_left;

  if(descending) {
    for ( dim_left = ( gN - 1 ); dim_left > 0; --dim_left ) {
      if ( ( cur * gK / gNodes ) != ( dest * gK / gNodes ) ) { break; }
      cur = (cur * gK) % gNodes; dest = (dest * gK) % gNodes;
    }
    cur = (cur * gK) / gNodes;
    dest = (dest * gK) / gNodes;
  } else {
    for ( dim_left = 0; dim_left < ( gN - 1 ); ++dim_left ) {
      if ( ( cur % gK ) != ( dest % gK ) ) { break; }
      cur /= gK; dest /= gK;
    }
    cur %= gK;
    dest %= gK;
  }

  if ( cur < dest ) {
    return 2*dim_left;     // Right
  } else {
    return 2*dim_left + 1; // Left
  }
}

//=============================================================

void dor_next_torus( int cur, int dest, int in_port,
		     int *out_port, int *partition,
		     bool balance = false )
{
  int dim_left;
  int dir;
  int dist2;

  for ( dim_left = 0; dim_left < gN; ++dim_left ) {
    if ( ( cur % gK ) != ( dest % gK ) ) { break; }
    cur /= gK; dest /= gK;
  }
  
  if ( dim_left < gN ) {

    if ( (in_port/2) != dim_left ) {
      // Turning into a new dimension

      cur %= gK; dest %= gK;
      dist2 = gK - 2 * ( ( dest - cur + gK ) % gK );
      
      if ( ( dist2 > 0 ) || 
	   ( ( dist2 == 0 ) && ( RandomInt( 1 ) ) ) ) {
	*out_port = 2*dim_left;     // Right
	dir = 0;
      } else {
	*out_port = 2*dim_left + 1; // Left
	dir = 1;
      }
      
      if ( partition ) {
	if ( balance ) {
	  // Cray's "Partition" allocation
	  // Two datelines: one between k-1 and 0 which forces VC 1
	  //                another between ((k-1)/2) and ((k-1)/2 + 1) which 
	  //                forces VC 0 otherwise any VC can be used
	  
	  if ( ( ( dir == 0 ) && ( cur > dest ) ) ||
	       ( ( dir == 1 ) && ( cur < dest ) ) ) {
	    *partition = 1;
	  } else if ( ( ( dir == 0 ) && ( cur <= (gK-1)/2 ) && ( dest >  (gK-1)/2 ) ) ||
		      ( ( dir == 1 ) && ( cur >  (gK-1)/2 ) && ( dest <= (gK-1)/2 ) ) ) {
	    *partition = 0;
	  } else {
	    *partition = RandomInt( 1 ); // use either VC set
	  }
	} else {
	  // Deterministic, fixed dateline between nodes k-1 and 0
	  
	  if ( ( ( dir == 0 ) && ( cur > dest ) ) ||
	       ( ( dir == 1 ) && ( dest < cur ) ) ) {
	    *partition = 1;
	  } else {
	    *partition = 0;
	  }
	}
      }
    } else {
      // Inverting the least significant bit keeps
      // the packet moving in the same direction
      *out_port = in_port ^ 0x1;
    }    

  } else {
    *out_port = 2*gN;  // Eject
  }
}

//=============================================================

void dim_order_mesh( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  int out_port = inject ? -1 : dor_next_mesh( r->GetID( ), f->dest );
  
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  if ( !inject && f->watch ) {
    *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
	       << "Adding VC range [" 
	       << vcBegin << "," 
	       << vcEnd << "]"
	       << " at output port " << out_port
	       << " for flit " << f->id
	       << " (input port " << in_channel
	       << ", destination " << f->dest << ")"
	       << "." << endl;
  }
  
  outputs->Clear();

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

void dim_order_ni_mesh( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  int out_port = inject ? -1 : dor_next_mesh( r->GetID( ), f->dest );
  
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  // at the destination router, we don't need to separate VCs by destination
  if(inject || (r->GetID() != f->dest)) {

    int const vcs_per_dest = (vcEnd - vcBegin + 1) / gNodes;
    assert(vcs_per_dest > 0);

    vcBegin += f->dest * vcs_per_dest;
    vcEnd = vcBegin + vcs_per_dest - 1;

  }
  
  if( !inject && f->watch ) {
    *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
	       << "Adding VC range [" 
	       << vcBegin << "," 
	       << vcEnd << "]"
	       << " at output port " << out_port
	       << " for flit " << f->id
	       << " (input port " << in_channel
	       << ", destination " << f->dest << ")"
	       << "." << endl;
  }
  
  outputs->Clear( );
  
  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

void dim_order_pni_mesh( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  int out_port = inject ? -1 : dor_next_mesh( r->GetID(), f->dest );
  
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  if(inject || (r->GetID() != f->dest)) {
    int next_coord = f->dest;
    if(!inject) {
      int out_dim = out_port / 2;
      for(int d = 0; d < out_dim; ++d) {
	next_coord /= gK;
      }
    }
    next_coord %= gK;
    assert(next_coord >= 0 && next_coord < gK);
    int vcs_per_dest = (vcEnd - vcBegin + 1) / gK;
    assert(vcs_per_dest > 0);
    vcBegin += next_coord * vcs_per_dest;
    vcEnd = vcBegin + vcs_per_dest - 1;
  }

  if( !inject && f->watch ) {
    *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
	       << "Adding VC range [" 
	       << vcBegin << "," 
	       << vcEnd << "]"
	       << " at output port " << out_port
	       << " for flit " << f->id
	       << " (input port " << in_channel
	       << ", destination " << f->dest << ")"
	       << "." << endl;
  }
  
  outputs->Clear( );
  
  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

// Random intermediate in the minimal quadrant defined
// by the source and destination
int rand_min_intr_mesh( int src, int dest )
{
  int dist;

  int intm = 0;
  int offset = 1;

  for ( int n = 0; n < gN; ++n ) {
    dist = ( dest % gK ) - ( src % gK );

    if ( dist > 0 ) {
      intm += offset * ( ( src % gK ) + RandomInt( dist ) );
    } else {
      intm += offset * ( ( dest % gK ) + RandomInt( -dist ) );
    }

    offset *= gK;
    dest /= gK; src /= gK;
  }

  return intm;
}

//=============================================================

void romm_mesh( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else {

    if ( in_channel == 2*gN ) {
      f->ph   = 0;  // Phase 0
      f->intm = rand_min_intr_mesh( f->src, f->dest );
    } 

    if ( ( f->ph == 0 ) && ( r->GetID( ) == f->intm ) ) {
      f->ph = 1; // Go to phase 1
    }

    out_port = dor_next_mesh( r->GetID( ), (f->ph == 0) ? f->intm : f->dest );

    // at the destination router, we don't need to separate VCs by phase
    if(r->GetID() != f->dest) {

      //each class must have at least 2 vcs assigned or else valiant valiant will deadlock
      int available_vcs = (vcEnd - vcBegin + 1) / 2;
      assert(available_vcs > 0);

      if(f->ph == 0) {
	vcEnd -= available_vcs;
      } else {
	assert(f->ph == 1);
	vcBegin += available_vcs;
      }
    }

  }

  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

void romm_ni_mesh( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  // at the destination router, we don't need to separate VCs by destination
  if(inject || (r->GetID() != f->dest)) {

    int const vcs_per_dest = (vcEnd - vcBegin + 1) / gNodes;
    assert(vcs_per_dest > 0);

    vcBegin += f->dest * vcs_per_dest;
    vcEnd = vcBegin + vcs_per_dest - 1;

  }

  int out_port;

  if(inject) {

    out_port = -1;

  } else {

    if ( in_channel == 2*gN ) {
      f->ph   = 0;  // Phase 0
      f->intm = rand_min_intr_mesh( f->src, f->dest );
    } 

    if ( ( f->ph == 0 ) && ( r->GetID( ) == f->intm ) ) {
      f->ph = 1; // Go to phase 1
    }

    out_port = dor_next_mesh( r->GetID( ), (f->ph == 0) ? f->intm : f->dest );

  }

  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

void min_adapt_mesh( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  outputs->Clear( );
  
  if(inject) {
    // injection can use all VCs
    outputs->AddRange(-1, vcBegin, vcEnd);
    return;
  } else if(r->GetID() == f->dest) {
    // ejection can also use all VCs
    outputs->AddRange(2*gN, vcBegin, vcEnd);
    return;
  }

  int in_vc;

  if ( in_channel == 2*gN ) {
    in_vc = vcEnd; // ignore the injection VC
  } else {
    in_vc = f->vc;
  }
  
  // DOR for the escape channel (VC 0), low priority 
  int out_port = dor_next_mesh( r->GetID( ), f->dest );    
  outputs->AddRange( out_port, 0, vcBegin, vcBegin );
  
  if ( f->watch ) {
      *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
		  << "Adding VC range [" 
		  << vcBegin << "," 
		  << vcBegin << "]"
		  << " at output port " << out_port
		  << " for flit " << f->id
		  << " (input port " << in_channel
		  << ", destination " << f->dest << ")"
		  << "." << endl;
   }
  
  if ( in_vc != vcBegin ) { // If not in the escape VC
    // Minimal adaptive for all other channels
    int cur = r->GetID( );
    int dest = f->dest;
    
    for ( int n = 0; n < gN; ++n ) {
      if ( ( cur % gK ) != ( dest % gK ) ) { 
	// Add minimal direction in dimension 'n'
	if ( ( cur % gK ) < ( dest % gK ) ) { // Right
	  if ( f->watch ) {
	    *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
			<< "Adding VC range [" 
		       << (vcBegin+1) << "," 
			<< vcEnd << "]"
			<< " at output port " << 2*n
			<< " with priority " << 1
			<< " for flit " << f->id
			<< " (input port " << in_channel
			<< ", destination " << f->dest << ")"
			<< "." << endl;
	  }
	  outputs->AddRange( 2*n, vcBegin+1, vcEnd, 1 ); 
	} else { // Left
	  if ( f->watch ) {
	    *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
			<< "Adding VC range [" 
		       << (vcBegin+1) << "," 
			<< vcEnd << "]"
			<< " at output port " << 2*n+1
			<< " with priority " << 1
			<< " for flit " << f->id
			<< " (input port " << in_channel
			<< ", destination " << f->dest << ")"
			<< "." << endl;
	  }
	  outputs->AddRange( 2*n + 1, vcBegin+1, vcEnd, 1 ); 
	}
      }
      cur  /= gK;
      dest /= gK;
    }
  } 
}

//=============================================================

void planar_adapt_mesh( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  outputs->Clear( );
  
  if(inject) {
    // injection can use all VCs
    outputs->AddRange(-1, vcBegin, vcEnd);
    return;
  }

  int cur     = r->GetID( ); 
  int dest    = f->dest;

  if ( cur != dest ) {
   
    int in_vc   = f->vc;
    int vc_mult = (vcEnd - vcBegin + 1) / 3;

    // Find the first unmatched dimension -- except
    // for when we're in the first dimension because
    // of misrouting in the last adaptive plane.
    // In this case, go to the last dimension instead.

    int n;
    for ( n = 0; n < gN; ++n ) {
      if ( ( ( cur % gK ) != ( dest % gK ) ) &&
	   !( ( in_channel/2 == 0 ) &&
	      ( n == 0 ) &&
	      ( in_vc < vcBegin+2*vc_mult ) ) ) {
	break;
      }

      cur  /= gK;
      dest /= gK;
    }

    assert( n < gN );

    if ( f->watch ) {
      *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
		  << "PLANAR ADAPTIVE: flit " << f->id 
		  << " in adaptive plane " << n << "." << endl;
    }

    // We're in adaptive plane n

    // Can route productively in d_{i,2}
    bool increase;
    bool fault;
    if ( ( cur % gK ) < ( dest % gK ) ) { // Increasing
      increase = true;
      if ( !r->IsFaultyOutput( 2*n ) ) {
	outputs->AddRange( 2*n, vcBegin+2*vc_mult, vcEnd );
	fault = false;

	if ( f->watch ) {
	  *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
		      << "PLANAR ADAPTIVE: increasing in dimension " << n
		      << "." << endl;
	}
      } else {
	fault = true;
      }
    } else { // Decreasing
      increase = false;
      if ( !r->IsFaultyOutput( 2*n + 1 ) ) {
	outputs->AddRange( 2*n + 1, vcBegin+2*vc_mult, vcEnd ); 
	fault = false;

	if ( f->watch ) {
	  *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
		      << "PLANAR ADAPTIVE: decreasing in dimension " << n
		      << "." << endl;
	}
      } else {
	fault = true;
      }
    }
      
    n = ( n + 1 ) % gN;
    cur  /= gK;
    dest /= gK;
      
    if ( !increase ) {
      vcBegin += vc_mult;
    }
    vcEnd = vcBegin + vc_mult - 1;
      
    int d1_min_c;
    if ( ( cur % gK ) < ( dest % gK ) ) { // Increasing in d_{i+1}
      d1_min_c = 2*n;
    } else if ( ( cur % gK ) != ( dest % gK ) ) {  // Decreasing in d_{i+1}
      d1_min_c = 2*n + 1;
    } else {
      d1_min_c = -1;
    }
      
    // do we want to 180?  if so, the last
    // route was a misroute in this dimension,
    // if there is no fault in d_i, just ignore
    // this dimension, otherwise continue to misroute
    if ( d1_min_c == in_channel ) { 
      if ( fault ) {
	d1_min_c = in_channel ^ 1;
      } else {
	d1_min_c = -1;
      }

      if ( f->watch ) {
	*gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
		    << "PLANAR ADAPTIVE: avoiding 180 in dimension " << n
		    << "." << endl;
      }
    }
      
    if ( d1_min_c != -1 ) {
      if ( !r->IsFaultyOutput( d1_min_c ) ) {
	outputs->AddRange( d1_min_c, vcBegin, vcEnd );
      } else if ( fault ) {
	// major problem ... fault in d_i and d_{i+1}
	r->Error( "There seem to be faults in d_i and d_{i+1}" );
      }
    } else if ( fault ) { // need to misroute!
      bool atedge;
      if ( cur % gK == 0 ) {
	d1_min_c = 2*n;
	atedge = true;
      } else if ( cur % gK == gK - 1 ) {
	d1_min_c = 2*n + 1;
	atedge = true;
      } else {
	d1_min_c = 2*n + RandomInt( 1 ); // random misroute

	if ( d1_min_c  == in_channel ) { // don't 180
	  d1_min_c = in_channel ^ 1;
	}
	atedge = false;
      }
      
      if ( !r->IsFaultyOutput( d1_min_c ) ) {
	outputs->AddRange( d1_min_c, vcBegin, vcEnd );
      } else if ( !atedge && !r->IsFaultyOutput( d1_min_c ^ 1 ) ) {
	outputs->AddRange( d1_min_c ^ 1, vcBegin, vcEnd );
      } else {
	// major problem ... fault in d_i and d_{i+1}
	r->Error( "There seem to be faults in d_i and d_{i+1}" );
      }
    }
  } else {
    outputs->AddRange( 2*gN, vcBegin, vcEnd ); 
  }
}

//=============================================================
/*
  FIXME: This is broken (note that f->dr is never actually modified).
  Even if it were, this should really use f->ph instead of introducing a single-
  use field.

void limited_adapt_mesh( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  outputs->Clear( );

  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  if ( inject ) {
    outputs->AddRange( -1, vcBegin, vcEnd - 1 );
    f->dr = 0; // zero dimension reversals
    return;
  }

  int cur = r->GetID( );
  int dest = f->dest;
  
  if ( cur != dest ) {
    if ( ( f->vc != vcEnd ) && 
	 ( f->dr != vcEnd - 1 ) ) {
      
      for ( int n = 0; n < gN; ++n ) {
	if ( ( cur % gK ) != ( dest % gK ) ) { 
	  int min_port;
	  if ( ( cur % gK ) < ( dest % gK ) ) { 
	    min_port = 2*n; // Right
	  } else {
	    min_port = 2*n + 1; // Left
	  }
	  
	  // Go in a productive direction with high priority
	  outputs->AddRange( min_port, vcBegin, vcEnd - 1, 2 );
	  
	  // Go in the non-productive direction with low priority
	  outputs->AddRange( min_port ^ 0x1, vcBegin, vcEnd - 1, 1 );
	} else {
	  // Both directions are non-productive
	  outputs->AddRange( 2*n, vcBegin, vcEnd - 1, 1 );
	  outputs->AddRange( 2*n+1, vcBegin, vcEnd - 1, 1 );
	}
	
	cur  /= gK;
	dest /= gK;
      }
      
    } else {
      outputs->AddRange( dor_next_mesh( cur, dest ),
			 vcEnd, vcEnd, 0 );
    }
    
  } else { // at destination
    outputs->AddRange( 2*gN, vcBegin, vcEnd ); 
  }
}
*/
//=============================================================

void valiant_mesh( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else {

    if ( in_channel == 2*gN ) {
      f->ph   = 0;  // Phase 0
      f->intm = RandomInt( gNodes - 1 );
    }

    if ( ( f->ph == 0 ) && ( r->GetID( ) == f->intm ) ) {
      f->ph = 1; // Go to phase 1
    }

    out_port = dor_next_mesh( r->GetID( ), (f->ph == 0) ? f->intm : f->dest );

    // at the destination router, we don't need to separate VCs by phase
    if(r->GetID() != f->dest) {

      //each class must have at least 2 vcs assigned or else valiant valiant will deadlock
      int const available_vcs = (vcEnd - vcBegin + 1) / 2;
      assert(available_vcs > 0);

      if(f->ph == 0) {
	vcEnd -= available_vcs;
      } else {
	assert(f->ph == 1);
	vcBegin += available_vcs;
      }
    }

  }

  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

void valiant_torus( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else {

    int phase;
    if ( in_channel == 2*gN ) {
      phase   = 0;  // Phase 0
      f->intm = RandomInt( gNodes - 1 );
    } else {
      phase = f->ph / 2;
    }

    if ( ( phase == 0 ) && ( r->GetID( ) == f->intm ) ) {
      phase = 1; // Go to phase 1
      in_channel = 2*gN; // ensures correct vc selection at the beginning of phase 2
    }
  
    int ring_part;
    dor_next_torus( r->GetID( ), (phase == 0) ? f->intm : f->dest, in_channel,
		    &out_port, &ring_part, false );

    f->ph = 2 * phase + ring_part;

    // at the destination router, we don't need to separate VCs by phase, etc.
    if(r->GetID() != f->dest) {

      int const ring_available_vcs = (vcEnd - vcBegin + 1) / 2;
      assert(ring_available_vcs > 0);

      if(ring_part == 0) {
	vcEnd -= ring_available_vcs;
      } else {
	assert(ring_part == 1);
	vcBegin += ring_available_vcs;
      }

      int const ph_available_vcs = ring_available_vcs / 2;
      assert(ph_available_vcs > 0);

      if(phase == 0) {
	vcEnd -= ph_available_vcs;
      } else {
	assert(phase == 1);
	vcBegin += ph_available_vcs;
      }
    }

  }

  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

void valiant_ni_torus( const Router *r, const Flit *f, int in_channel, 
		       OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  // at the destination router, we don't need to separate VCs by destination
  if(inject || (r->GetID() != f->dest)) {

    int const vcs_per_dest = (vcEnd - vcBegin + 1) / gNodes;
    assert(vcs_per_dest > 0);

    vcBegin += f->dest * vcs_per_dest;
    vcEnd = vcBegin + vcs_per_dest - 1;

  }

  int out_port;

  if(inject) {

    out_port = -1;

  } else {

    int phase;
    if ( in_channel == 2*gN ) {
      phase   = 0;  // Phase 0
      f->intm = RandomInt( gNodes - 1 );
    } else {
      phase = f->ph / 2;
    }

    if ( ( f->ph == 0 ) && ( r->GetID( ) == f->intm ) ) {
      f->ph = 1; // Go to phase 1
      in_channel = 2*gN; // ensures correct vc selection at the beginning of phase 2
    }
  
    int ring_part;
    dor_next_torus( r->GetID( ), (f->ph == 0) ? f->intm : f->dest, in_channel,
		    &out_port, &ring_part, false );

    f->ph = 2 * phase + ring_part;

    // at the destination router, we don't need to separate VCs by phase, etc.
    if(r->GetID() != f->dest) {

      int const ring_available_vcs = (vcEnd - vcBegin + 1) / 2;
      assert(ring_available_vcs > 0);

      if(ring_part == 0) {
	vcEnd -= ring_available_vcs;
      } else {
	assert(ring_part == 1);
	vcBegin += ring_available_vcs;
      }

      int const ph_available_vcs = ring_available_vcs / 2;
      assert(ph_available_vcs > 0);

      if(phase == 0) {
	vcEnd -= ph_available_vcs;
      } else {
	assert(phase == 1);
	vcBegin += ph_available_vcs;
      }
    }

    if (f->watch) {
      *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
		 << "Adding VC range [" 
		 << vcBegin << "," 
		 << vcEnd << "]"
		 << " at output port " << out_port
		 << " for flit " << f->id
		 << " (input port " << in_channel
		 << ", destination " << f->dest << ")"
		 << "." << endl;
    }

  }
  
  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

void dim_order_torus( const Router *r, const Flit *f, int in_channel, 
		      OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else {
    
    int cur  = r->GetID( );
    int dest = f->dest;

    dor_next_torus( cur, dest, in_channel,
		    &out_port, &f->ph, false );


    // at the destination router, we don't need to separate VCs by ring partition
    if(cur != dest) {

      int const available_vcs = (vcEnd - vcBegin + 1) / 2;
      assert(available_vcs > 0);

      if ( f->ph == 0 ) {
	vcEnd -= available_vcs;
      } else {
	vcBegin += available_vcs;
      } 
    }

    if ( f->watch ) {
      *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
		 << "Adding VC range [" 
		 << vcBegin << "," 
		 << vcEnd << "]"
		 << " at output port " << out_port
		 << " for flit " << f->id
		 << " (input port " << in_channel
		 << ", destination " << f->dest << ")"
		 << "." << endl;
    }

  }
 
  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

void dim_order_ni_torus( const Router *r, const Flit *f, int in_channel, 
			 OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else {
    
    int cur  = r->GetID( );
    int dest = f->dest;

    dor_next_torus( cur, dest, in_channel,
		    &out_port, NULL, false );

    // at the destination router, we don't need to separate VCs by destination
    if(cur != dest) {

      int const vcs_per_dest = (vcEnd - vcBegin + 1) / gNodes;
      assert(vcs_per_dest);

      vcBegin += f->dest * vcs_per_dest;
      vcEnd = vcBegin + vcs_per_dest - 1;

    }

    if ( f->watch ) {
      *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
		 << "Adding VC range [" 
		 << vcBegin << "," 
		 << vcEnd << "]"
		 << " at output port " << out_port
		 << " for flit " << f->id
		 << " (input port " << in_channel
		 << ", destination " << f->dest << ")"
		 << "." << endl;
    }

  }
  
  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

void dim_order_bal_torus( const Router *r, const Flit *f, int in_channel, 
			  OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else {

    int cur  = r->GetID( );
    int dest = f->dest;

    dor_next_torus( cur, dest, in_channel,
		    &out_port, &f->ph, true );

    // at the destination router, we don't need to separate VCs by ring partition
    if(cur != dest) {

      int const available_vcs = (vcEnd - vcBegin + 1) / 2;
      assert(available_vcs > 0);

      if ( f->ph == 0 ) {
	vcEnd -= available_vcs;
      } else {
	assert(f->ph == 1);
	vcBegin += available_vcs;
      } 
    }

    if ( f->watch ) {
      *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
		 << "Adding VC range [" 
		 << vcBegin << "," 
		 << vcEnd << "]"
		 << " at output port " << out_port
		 << " for flit " << f->id
		 << " (input port " << in_channel
		 << ", destination " << f->dest << ")"
		 << "." << endl;
    }

  }
  
  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}

//=============================================================

void min_adapt_torus( const Router *r, const Flit *f, int in_channel, OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  outputs->Clear( );

  if(inject) {
    // injection can use all VCs
    outputs->AddRange(-1, vcBegin, vcEnd);
    return;
  } else if(r->GetID() == f->dest) {
    // ejection can also use all VCs
    outputs->AddRange(2*gN, vcBegin, vcEnd);
  }

  int in_vc;
  if ( in_channel == 2*gN ) {
    in_vc = vcEnd; // ignore the injection VC
  } else {
    in_vc = f->vc;
  }
  
  int cur = r->GetID( );
  int dest = f->dest;

  int out_port;

  if ( in_vc > ( vcBegin + 1 ) ) { // If not in the escape VCs
    // Minimal adaptive for all other channels
    
    for ( int n = 0; n < gN; ++n ) {
      if ( ( cur % gK ) != ( dest % gK ) ) {
	int dist2 = gK - 2 * ( ( ( dest % gK ) - ( cur % gK ) + gK ) % gK );
	
	if ( dist2 > 0 ) { /*) || 
			     ( ( dist2 == 0 ) && ( RandomInt( 1 ) ) ) ) {*/
	  outputs->AddRange( 2*n, vcBegin+3, vcBegin+3, 1 ); // Right
	} else {
	  outputs->AddRange( 2*n + 1, vcBegin+3, vcBegin+3, 1 ); // Left
	}
      }

      cur  /= gK;
      dest /= gK;
    }
    
    // DOR for the escape channel (VCs 0-1), low priority --- 
    // trick the algorithm with the in channel.  want VC assignment
    // as if we had injected at this node
    dor_next_torus( r->GetID( ), f->dest, 2*gN,
		    &out_port, &f->ph, false );
  } else {
    // DOR for the escape channel (VCs 0-1), low priority 
    dor_next_torus( cur, dest, in_channel,
		    &out_port, &f->ph, false );
  }

  if ( f->ph == 0 ) {
    outputs->AddRange( out_port, vcBegin, vcBegin, 0 );
  } else  {
    outputs->AddRange( out_port, vcBegin+1, vcBegin+1, 0 );
  } 
}

//=============================================================

void dest_tag_fly( const Router *r, const Flit *f, int in_channel, 
		   OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {

    out_port = -1;

  } else {

    int stage = ( r->GetID( ) * gK ) / gNodes;
    int dest  = f->dest;

    while( stage < ( gN - 1 ) ) {
      dest /= gK;
      ++stage;
    }

    out_port = dest % gK;
  }

  outputs->Clear( );

  outputs->AddRange( out_port, vcBegin, vcEnd );
}



//=============================================================

void chaos_torus( const Router *r, const Flit *f, 
		  int in_channel, OutputSet *outputs, bool inject )
{
  outputs->Clear( );

  if(inject) {
    outputs->AddRange(-1, 0, 0);
    return;
  }

  int cur = r->GetID( );
  int dest = f->dest;
  
  if ( cur != dest ) {
    for ( int n = 0; n < gN; ++n ) {

      if ( ( cur % gK ) != ( dest % gK ) ) { 
	int dist2 = gK - 2 * ( ( ( dest % gK ) - ( cur % gK ) + gK ) % gK );
      
	if ( dist2 >= 0 ) {
	  outputs->AddRange( 2*n, 0, 0 ); // Right
	} 
	
	if ( dist2 <= 0 ) {
	  outputs->AddRange( 2*n + 1, 0, 0 ); // Left
	}
      }

      cur  /= gK;
      dest /= gK;
    }
  } else {
    outputs->AddRange( 2*gN, 0, 0 ); 
  }
}


//=============================================================

void chaos_mesh( const Router *r, const Flit *f, 
		  int in_channel, OutputSet *outputs, bool inject )
{
  outputs->Clear( );

  if(inject) {
    outputs->AddRange(-1, 0, 0);
    return;
  }

  int cur = r->GetID( );
  int dest = f->dest;
  
  if ( cur != dest ) {
    for ( int n = 0; n < gN; ++n ) {
      if ( ( cur % gK ) != ( dest % gK ) ) { 
	// Add minimal direction in dimension 'n'
	if ( ( cur % gK ) < ( dest % gK ) ) { // Right
	  outputs->AddRange( 2*n, 0, 0 ); 
	} else { // Left
	  outputs->AddRange( 2*n + 1, 0, 0 ); 
	}
      }
      cur  /= gK;
      dest /= gK;
    }
  } else {
    outputs->AddRange( 2*gN, 0, 0 ); 
  }
}

//=============================================================

// 3D routing for unidirectional 2D torus + vertical mesh with elevators
// Z-first priority: route to elevator if Z doesn't match, otherwise normal 2D routing

void dim_order_3d_elevator_unitorus(const Router *r, const Flit *f, int in_channel, 
                                   OutputSet *outputs, bool inject)
{
  if (gElevatorMapping.empty()) {
      cerr << "ERROR: gElevatorMapping is empty!" << endl;
      // Fallback behavior
  }
  
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if (f->type == Flit::READ_REQUEST) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if (f->type == Flit::WRITE_REQUEST) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if (f->type == Flit::READ_REPLY) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if (f->type == Flit::WRITE_REPLY) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));
  int out_port;

  if(inject) {
    out_port = -1;
  } else {
    int cur = r->GetID();
    int dest = f->dest;
    bool is_vertical_mesh = (gVerticalTopology == "mesh");
    bool use_single_z_port = (!is_vertical_mesh) || (gDimSizes[2] <= 2);
    if (cur == dest) {
      // Check if we're using 5-port (mesh) or 4-port (torus) system
      out_port = r->NumOutputs() - 1; // PE is always the last port
    } else {
      vector<int> cur_coords = NodeToCoords3D(cur);
      vector<int> dest_coords = NodeToCoords3D(dest);
      
      if (cur_coords[2] != dest_coords[2]) {
        vector<int> elevator_coords = GetNearestElevator(cur);
        //cout << "Node " << cur << " coords(" << cur_coords[0] << "," << cur_coords[1] << "," << cur_coords[2] << ") maps to elevator (" << elevator_coords[0] << "," << elevator_coords[1] << ")" << endl;
        if (cur_coords[0] == elevator_coords[0] && cur_coords[1] == elevator_coords[1]) { // At elevator - choose up or down port
          if (r->NumOutputs() == 5) { // This router has separate Z-up and Z-down ports (middle layer)
            if (cur_coords[2] < dest_coords[2]) {
              out_port = 2; // Z-up
            } else {
              out_port = 3; // Z-down
            }
          } else {
            out_port = 2; // This router has only one Z port (top/bottom layer)
          }
        } else {
          // Not at elevator - route to elevator using X,Y
          out_port = Route2D_ToElevator(cur_coords, elevator_coords, vcBegin, vcEnd);
        }
      } else {
        // Z matches - do 2D X,Y routing
        out_port = Route2D_ToDestination(cur_coords, dest_coords, vcBegin, vcEnd);
      }
    }
  }

  outputs->Clear();
  outputs->AddRange(out_port, vcBegin, vcEnd);
}

// Convert node ID to 3D coordinates using consistent method
vector<int> NodeToCoords3D(int node)
{
  vector<int> coords(3); // X, Y, Z
  
  // Assuming gDimSizes[0] = X_size, gDimSizes[1] = Y_size, gDimSizes[2] = Z_size
  coords[0] = node % gDimSizes[0];                    // X coordinate
  coords[1] = (node / gDimSizes[0]) % gDimSizes[1];   // Y coordinate  
  coords[2] = node / (gDimSizes[0] * gDimSizes[1]);   // Z coordinate
  
  return coords;
}

// Get nearest elevator coordinates for current node
// This will be populated from config file parser
vector<int> GetNearestElevator(int node)
{
    vector<int> coords = NodeToCoords3D(node);
    int grid_pos = coords[1] * gDimSizes[0] + coords[0]; // y * width + x
    
    if (grid_pos < (int)gElevatorMapping.size()) {
        return gElevatorMapping[grid_pos];
    }
    cout << "going to fallback" << endl;
    // Fallback: return node's own X,Y coordinates
    return {coords[0], coords[1]};
}

// 2D dimension-order routing to elevator coordinates
int Route2D_ToElevator(const vector<int>& cur_coords, const vector<int>& elevator_coords,
                      int& vcBegin, int& vcEnd)
{
  // X-first dimension order (unidirectional torus)
  if (cur_coords[0] != elevator_coords[0]) {
    // Route in X dimension
    return Route_X_Dimension(cur_coords[0], elevator_coords[0], vcBegin, vcEnd);
  } else if (cur_coords[1] != elevator_coords[1]) {
    // Route in Y dimension  
    return Route_Y_Dimension(cur_coords[1], elevator_coords[1], vcBegin, vcEnd);
  }
  
  // Should not reach here if elevator coords are different
  return -1;
}

// 2D dimension-order routing to final destination
int Route2D_ToDestination(const vector<int>& cur_coords, const vector<int>& dest_coords,
                         int& vcBegin, int& vcEnd)
{
  // X-first dimension order (unidirectional torus)
  if (cur_coords[0] != dest_coords[0]) {
    // Route in X dimension
    return Route_X_Dimension(cur_coords[0], dest_coords[0], vcBegin, vcEnd);
  } else if (cur_coords[1] != dest_coords[1]) {
    // Route in Y dimension
    return Route_Y_Dimension(cur_coords[1], dest_coords[1], vcBegin, vcEnd);
  }
  
  // Should not reach here if destination is different
  return -1;
}

// Route in X dimension with VC partitioning for wraparound
int Route_X_Dimension(int cur_x, int dest_x, int& vcBegin, int& vcEnd)
{
  int out_port = 0; // East port
  
  // Calculate distance with modulo for torus wraparound  
  int direct_dist = (dest_x - cur_x + gDimSizes[0]) % gDimSizes[0];
  int wrap_dist = gDimSizes[0] - direct_dist;
  
  // For DOR torus, VC partitioning is optional (deadlock-free without it)
  /*int total_vcs = vcEnd - vcBegin + 1;
  if (total_vcs >= 2) {
    // Use VC partitioning if multiple VCs available
    if (direct_dist <= wrap_dist) {
      vcEnd = vcBegin + total_vcs / 2 - 1;
    } else {
      vcBegin = vcBegin + (total_vcs + 1) / 2;
    }
  }
  // If only 1 VC, leave vcBegin/vcEnd unchanged (use all available VCs)
  */
  return out_port;
}

// Route in Y dimension with VC partitioning for wraparound  
int Route_Y_Dimension(int cur_y, int dest_y, int& vcBegin, int& vcEnd)
{
  int out_port = 1; // South port
  
  // Calculate distance with modulo for torus wraparound  
  int direct_dist = (dest_y - cur_y + gDimSizes[1]) % gDimSizes[1];
  int wrap_dist = gDimSizes[1] - direct_dist;
  /*// For DOR torus, VC partitioning is optional (deadlock-free without it)
  int total_vcs = vcEnd - vcBegin + 1;
  if (total_vcs >= 2) {
    // Use VC partitioning if multiple VCs available
    if (direct_dist <= wrap_dist) {
      vcEnd = vcBegin + total_vcs / 2 - 1;
    } else {
      vcBegin = vcBegin + (total_vcs + 1) / 2;
    }
  }*/
  // If only 1 VC, leave vcBegin/vcEnd unchanged (use all available VCs)
  
  return out_port;
}




//=============================================================


void dim_order_unitorus( const Router *r, const Flit *f, int in_channel, 
                         OutputSet *outputs, bool inject )
{
  int vcBegin = 0, vcEnd = gNumVCs-1;
  if ( f->type == Flit::READ_REQUEST ) {
    vcBegin = gReadReqBeginVC;
    vcEnd = gReadReqEndVC;
  } else if ( f->type == Flit::WRITE_REQUEST ) {
    vcBegin = gWriteReqBeginVC;
    vcEnd = gWriteReqEndVC;
  } else if ( f->type ==  Flit::READ_REPLY ) {
    vcBegin = gReadReplyBeginVC;
    vcEnd = gReadReplyEndVC;
  } else if ( f->type ==  Flit::WRITE_REPLY ) {
    vcBegin = gWriteReplyBeginVC;
    vcEnd = gWriteReplyEndVC;
  }
  assert(((f->vc >= vcBegin) && (f->vc <= vcEnd)) || (inject && (f->vc < 0)));

  int out_port;

  if(inject) {
    out_port = -1;
  } else {
    int cur = r->GetID();
    int dest = f->dest;

    // Find dimension with lowest cost that needs routing (penalty-aware routing)
    int dim_to_route = -1;
    float min_cost = -1.0f;
    
    for (int dim = 0; dim < gN; ++dim) {
      // Calculate current and destination coordinates for this dimension
      int divisor = 1;
      for (int d = 0; d < dim; ++d) {
        divisor *= gDimSizes[d];
      }
      
      int cur_coord = (cur / divisor) % gDimSizes[dim];
      int dest_coord = (dest / divisor) % gDimSizes[dim];
      
      if (cur_coord != dest_coord) {
        // This dimension needs routing - calculate routing cost
        
        // Calculate distance in this dimension
        int distance;
        if (cur_coord < dest_coord) {
          distance = dest_coord - cur_coord; // Direct path
        } else {
          distance = gDimSizes[dim] - cur_coord + dest_coord; // Wraparound path
        }
        
        // Total cost = base distance + penalty - bandwidth bonus
        // Higher bandwidth makes dimension more attractive (lower cost)
        float penalty = (dim < (int)gDimPenalties.size()) ? gDimPenalties[dim] : 0.0f;
        float bandwidth_bonus = (dim < (int)gDimBandwidths.size()) ? (float)gDimBandwidths[dim] - 1.0f : 0.0f;
        float cost = (float)distance + penalty - bandwidth_bonus;
        
        // Choose dimension with lowest cost (or first one if costs are equal)
        if (dim_to_route < 0 || cost < min_cost) {
          dim_to_route = dim;
          min_cost = cost;
        }
      }
    }

    if (dim_to_route >= 0) {
      // In unidirectional torus, we can only move in positive direction
      // If destination is "behind" us, we need to go the long way around
      int divisor = 1;
      for (int d = 0; d < dim_to_route; ++d) {
        divisor *= gDimSizes[d];
      }
      
      int cur_coord = (cur / divisor) % gDimSizes[dim_to_route];
      int dest_coord = (dest / divisor) % gDimSizes[dim_to_route];
      
      // Always route in positive direction (dimension number is the port)
      out_port = dim_to_route;
      
      // Apply virtual channel partitioning for deadlock avoidance
      // Use different VC sets based on whether we're wrapping around
      if (cur_coord < dest_coord) {
        // Direct path (no wraparound)
        // Use first half of VCs
        vcEnd = vcBegin + (vcEnd - vcBegin) / 2;
      } else {
        // Wraparound path (cur > dest, going the long way)
        // Use second half of VCs
        vcBegin = vcBegin + (vcEnd - vcBegin + 1) / 2;
      }
    } else {
      // At destination
      out_port = gN; // Eject port
    }

    if (f->watch) {
      *gWatchOut << GetSimTime() << " | " << r->FullName() << " | "
                 << "Unidirectional DOR: Adding VC range [" 
                 << vcBegin << "," 
                 << vcEnd << "]"
                 << " at output port " << out_port
                 << " for flit " << f->id
                 << " (input port " << in_channel
                 << ", destination " << f->dest << ")"
                 << "." << endl;
    }
  }
 
  outputs->Clear();
  outputs->AddRange(out_port, vcBegin, vcEnd);
}

//=============================================================

void InitializeRoutingMap( const Configuration & config )
{

  gNumVCs = config.GetInt( "num_vcs" );

  //
  // traffic class partitions
  //
  gReadReqBeginVC    = config.GetInt("read_request_begin_vc");
  if(gReadReqBeginVC < 0) {
    gReadReqBeginVC = 0;
  }
  gReadReqEndVC      = config.GetInt("read_request_end_vc");
  if(gReadReqEndVC < 0) {
    gReadReqEndVC = gNumVCs / 2 - 1;
  }
  gWriteReqBeginVC   = config.GetInt("write_request_begin_vc");
  if(gWriteReqBeginVC < 0) {
    gWriteReqBeginVC = 0;
  }
  gWriteReqEndVC     = config.GetInt("write_request_end_vc");
  if(gWriteReqEndVC < 0) {
    gWriteReqEndVC = gNumVCs / 2 - 1;
  }
  gReadReplyBeginVC  = config.GetInt("read_reply_begin_vc");
  if(gReadReplyBeginVC < 0) {
    gReadReplyBeginVC = gNumVCs / 2;
  }
  gReadReplyEndVC    = config.GetInt("read_reply_end_vc");
  if(gReadReplyEndVC < 0) {
    gReadReplyEndVC = gNumVCs - 1;
  }
  gWriteReplyBeginVC = config.GetInt("write_reply_begin_vc");
  if(gWriteReplyBeginVC < 0) {
    gWriteReplyBeginVC = gNumVCs / 2;
  }
  gWriteReplyEndVC   = config.GetInt("write_reply_end_vc");
  if(gWriteReplyEndVC < 0) {
    gWriteReplyEndVC = gNumVCs - 1;
  }

  /* Register routing functions here */

  // ===================================================
  // Balfour-Schultz
  gRoutingFunctionMap["nca_fattree"]         = &fattree_nca;
  gRoutingFunctionMap["anca_fattree"]        = &fattree_anca;
  gRoutingFunctionMap["nca_qtree"]           = &qtree_nca;
  gRoutingFunctionMap["nca_tree4"]           = &tree4_nca;
  gRoutingFunctionMap["anca_tree4"]          = &tree4_anca;
  gRoutingFunctionMap["dor_mesh"]            = &dim_order_mesh;
  gRoutingFunctionMap["xy_yx_mesh"]          = &xy_yx_mesh;
  gRoutingFunctionMap["adaptive_xy_yx_mesh"]          = &adaptive_xy_yx_mesh;
  // End Balfour-Schultz
  // ===================================================

  gRoutingFunctionMap["dim_order_3d_elevator_unitorus"] = &dim_order_3d_elevator_unitorus;
  gRoutingFunctionMap["dim_order_unitorus"] = &dim_order_unitorus;

  gRoutingFunctionMap["dim_order_mesh"]  = &dim_order_mesh;
  gRoutingFunctionMap["dim_order_ni_mesh"]  = &dim_order_ni_mesh;
  gRoutingFunctionMap["dim_order_pni_mesh"]  = &dim_order_pni_mesh;
  gRoutingFunctionMap["dim_order_torus"] = &dim_order_torus;
  gRoutingFunctionMap["dim_order_ni_torus"] = &dim_order_ni_torus;
  gRoutingFunctionMap["dim_order_bal_torus"] = &dim_order_bal_torus;

  gRoutingFunctionMap["romm_mesh"]       = &romm_mesh; 
  gRoutingFunctionMap["romm_ni_mesh"]    = &romm_ni_mesh;

  gRoutingFunctionMap["min_adapt_mesh"]   = &min_adapt_mesh;
  gRoutingFunctionMap["min_adapt_torus"]  = &min_adapt_torus;

  gRoutingFunctionMap["planar_adapt_mesh"] = &planar_adapt_mesh;

  // FIXME: This is broken.
  //  gRoutingFunctionMap["limited_adapt_mesh"] = &limited_adapt_mesh;

  gRoutingFunctionMap["valiant_mesh"]  = &valiant_mesh;
  gRoutingFunctionMap["valiant_torus"] = &valiant_torus;
  gRoutingFunctionMap["valiant_ni_torus"] = &valiant_ni_torus;

  gRoutingFunctionMap["dest_tag_fly"] = &dest_tag_fly;

  gRoutingFunctionMap["chaos_mesh"]  = &chaos_mesh;
  gRoutingFunctionMap["chaos_torus"] = &chaos_torus;
}
