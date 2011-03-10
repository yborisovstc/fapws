#ifndef __FAP_TADESL_H
#define __FAP_TADESL_H

// FAP transition agent plugin 
//
#include "../../src/faptran.h"

// Executable agent for FAP transtions
class CAE_TaDesl: public CAE_TranExBase
{
    public:
	CAE_TaDesl();
	virtual ~CAE_TaDesl();
	// From MAE_TranEx
	virtual void EvalTrans();
};



#endif // __FAP_TADESL_H
