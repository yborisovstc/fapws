#ifndef __FAP_BASE_PANIC_H
#define __FAP_BASE_PANIC_H

//************************************************************
// Finite automata programming base, level 1 prototype. Panics  
// Yuri Borisov  11/07/05  FAP_CR_002  Added panics
//*************************************************************


enum
{
	EFapPan_IncorrLinksSize = 0x00000001,  // Incompatible states sizes be direct link
	EFapPan_CopyTransIncompatible = 0x00000002, // Incompatible states sizes by copy-transition
	EFapPan_IncorrSpecIndex = 0x00000003, // Incorrect log spec index within spec container 
};

// Error codes
#define KFapErr_None  0
#define KFapErr_NotFound  (-1)
#define KFapErr_Overflow  (-5)

#endif // __FAP_BASE_PANIC_H


