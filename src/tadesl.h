#ifndef __FAP_TADESL_H
#define __FAP_TADESL_H

// FAP transition agent plugin 
//
#include "faptran.h"

// Executable agent for FAP transtions
class CAE_TaDesl: public CAE_TranExBase
{
    public:
	CAE_TaDesl();
	virtual ~CAE_TaDesl();
	// From MAE_TranEx
	virtual void EvalTrans(CAE_StateBase* aState, const string& aTrans);
	static const char *Type() {return "TaDesl";}; 
    private:
	virtual void *DoGetFbObj(const char *aName);
};



#endif // __FAP_TADESL_H
