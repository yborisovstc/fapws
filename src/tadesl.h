#ifndef __FAP_TADESL_H
#define __FAP_TADESL_H

// FAP transition agent plugin 
//
#include "faptran.h"

class CSL_Interpr;
// Executable agent for FAP transtions
class CAE_TaDesl: public CAE_TranExBase
{
    public:
	CAE_TaDesl(MCAE_LogRec* aLogger);
	virtual ~CAE_TaDesl();
	// From MAE_TranEx
	virtual void EvalTrans(MAE_TransContext* aContext, CAE_StateBase* aState, const string& aTrans);
	virtual const multimap<string, CSL_ExprBase*>& Exprs();
	static const char *Type() {return "TaDesl";}; 
    private:
	virtual void *DoGetFbObj(const char *aName);
    private:
	CSL_Interpr *iInterpr;
};



#endif // __FAP_TADESL_H
