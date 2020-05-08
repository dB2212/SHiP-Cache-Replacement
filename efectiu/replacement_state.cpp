#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>
#include <map>
#include <iostream>

using namespace std;

#include "replacement_state.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/*
** This file implements the cache replacement state. Users can enhance the code
** below to develop their cache replacement ideas.
**
*/


////////////////////////////////////////////////////////////////////////////////
// The replacement state constructor:                                         //
// Inputs: number of sets, associativity, and replacement policy to use       //
// Outputs: None                                                              //
//                                                                            //
// DO NOT CHANGE THE CONSTRUCTOR PROTOTYPE                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{

    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;

    mytimer    = 0;

    InitReplacementState();
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The function prints the statistics for the cache                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{

    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;

    // CONTESTANTS:  Insert your statistics printing here
    
    return out;

}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways

    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];

    // ensure that we were able to create replacement state

    assert(repl);

    // Create the state for the sets
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++) 
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];

        for(UINT32 way=0; way<assoc; way++) 
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
        }
    }

    if (replPolicy != CRC_REPL_CONTESTANT) return;

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE

    //initializing SHCT counters with zero value
    memset(SHCT, 0 ,sizeof(SHCT));
    //initializing the prediction intervals, signatures, outcomes and prefetched fields
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++){
	for(UINT32 way=0; way <assoc; way++){
		repl[setIndex][way].reref_interval = LONG_INTERVAL;
		repl[setIndex][way].outcome = false;
		repl[setIndex][way].is_pf = false ;
		repl[setIndex][way].signature_m = 0 ;
	}
	}
		
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// argument is the set index. The return value is the physical way            //
// index for the line being replaced.                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType ) {
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU ) 
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
	return Get_My_Victim (setIndex);
    }

    // We should never here here

    assert(0);
    return -1;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The arguments are: the set index, the physical way of the cache,           //
// the pointer to the physical line (should contestants need access           //
// to information of the line filled or hit upon), the thread id              //
// of the request, the PC of the request, the accesstype, and finall          //
// whether the line was a cachehit or not (cacheHit=true implies hit)         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState( 
    UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
	//fprintf (stderr, "ain't I a stinker? %lld\n", get_cycle_count ());
	//fflush (stderr);
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU ) 
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_CONTESTANT )
    {
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
	 UpdateMyPolicy(PC, setIndex, updateWayID, cacheHit, accessType);

    }
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//////// HELPER FUNCTIONS FOR REPLACEMENT UPDATE AND VICTIM SELECTION //////////
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the LRU victim in the cache set by returning the       //
// cache block at the bottom of the LRU stack. Top of LRU stack is '0'        //
// while bottom of LRU stack is 'assoc-1'                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
	// Get pointer to replacement state of current set

	LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];
	INT32   lruWay   = 0;

	// Search for victim whose stack position is assoc-1

	for(UINT32 way=0; way<assoc; way++) {
		if (replSet[way].LRUstackposition == (assoc-1)) {
			lruWay = way;
			break;
		}
	}

	// return lru way

	return lruWay;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);
    
    return way;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the LRU update routine for the traditional        //
// LRU replacement policy. The arguments to the function are the physical     //
// way and set index.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
	// Determine current LRU stack position
	UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;

	// Update the stack position of all lines before the current line
	// Update implies incremeting their stack positions by one

	for(UINT32 way=0; way<assoc; way++) {
		if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) {
			repl[setIndex][way].LRUstackposition++;
		}
	}

	// Set the LRU stack position of new line to be zero
	repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}

INT32 CACHE_REPLACEMENT_STATE::Get_My_Victim( UINT32 setIndex ) {
	// return first way always
	bool victim_found;
	UINT32 victim_way;
	victim_way = assoc -1;
	victim_found = false;

	//SRRIP to select victim
	do{
		for(UINT32 way =0 ; way < assoc; way++){
			if(repl[setIndex][way].reref_interval == DISTANT_INTERVAL){
				victim_found = true;
				victim_way = way;
				return victim_way;
				break;
			}
		}
			
		if(victim_found == false){
				for(UINT32 way =0; way < assoc; way++){
					if(repl[setIndex][way].reref_interval < DISTANT_INTERVAL){
						repl[setIndex][way].reref_interval++;
					}
					
				}
		}
	}while(!victim_found);

	return victim_way; 
}

void CACHE_REPLACEMENT_STATE::UpdateMyPolicy( Addr_t PC, UINT32 setIndex, INT32 updateWayID, bool cacheHit, UINT32 accessType ) {
	//create the signature based on the program counter and the accessType
	UINT32 signature =  MASK & ((PC << 4) + ((accessType == ACCESS_PREFETCH)<< 2));	
	//SHiP algorithm for case of Hit upon first re-reference ,SHiP++ enhancement
	if( cacheHit && (repl[setIndex][updateWayID].outcome == false)){
		if(repl[setIndex][updateWayID].is_pf == true){
			repl[setIndex][updateWayID].is_pf = false;
		}
		else{
			if(SHCT[repl[setIndex][updateWayID].signature_m] < MAX_COUNT){
				SHCT[repl[setIndex][updateWayID].signature_m]++;
			}
		}
		repl[setIndex][updateWayID].outcome = true;
	}
        //SHiP algorithm for cache insertion
	else{
		if(repl[setIndex][updateWayID].outcome == false){
			if(SHCT[repl[setIndex][updateWayID].signature_m] >0){
				SHCT[repl[setIndex][updateWayID].signature_m]--;
			}
		}

		//update the signature
		repl[setIndex][updateWayID].signature_m = signature;
		//update the outcome
		repl[setIndex][updateWayID].outcome = false;
		
		//SHiP++  enhancement to update the prefetch field associated with the line
		repl[setIndex][updateWayID].is_pf = (accessType == ACCESS_PREFETCH);
		
		//SHiP++ enhancements for writeback access, to predict the distant intervals on writeback accesses
		//or if the SHCT count is zero with the associated signature
		if(accessType == ACCESS_WRITEBACK || SHCT[signature] == 0){
			repl[setIndex][updateWayID].reref_interval = DISTANT_INTERVAL;
			}
	
		//SHiP++ enhancement for updating in case the SHCT has already the MAX_COUNT 
		else if(SHCT[signature] == MAX_COUNT){
			repl[setIndex][updateWayID].reref_interval = IMMEDIATE_INTERVAL;
			}
		//follows SHiP here
		else{
			repl[setIndex][updateWayID].reref_interval = LONG_INTERVAL;
		}

	}
	//RRPV update on cache hit
	if(cacheHit){
		//decrement the re-reference interval if the access is other than a prefetch upon cache hit
		if(repl[setIndex][updateWayID].is_pf == 0){
			if(repl[setIndex][updateWayID].reref_interval >0){
				repl[setIndex][updateWayID].reref_interval--;
			}
		}
	}
		
 
}

CACHE_REPLACEMENT_STATE::~CACHE_REPLACEMENT_STATE (void) {
}