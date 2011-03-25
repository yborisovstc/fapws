
// FAP transition agent plugin 

#include "tadesl.h"
#include "deslbase.h"

CAE_TaDesl::CAE_TaDesl(MCAE_LogRec* aLogger): CAE_TranExBase(aLogger), iInterpr(NULL)
{
    iInterpr = new CSL_Interpr(iLogger);
}

CAE_TaDesl::~CAE_TaDesl()
{
    if (iInterpr != NULL)
	delete iInterpr;
}

void CAE_TaDesl::EvalTrans(MAE_TransContext* aContext, CAE_StateBase* aState, const string& aTrans)
{
    iInterpr->EvalTrans(aContext, aState, aTrans);
}

const multimap<string, CSL_ExprBase*>& CAE_TaDesl::Exprs()
{
    return iInterpr->Exprs();
}

void* CAE_TaDesl::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this: NULL;
}
