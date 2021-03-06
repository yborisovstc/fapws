
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

void CAE_TaDesl::EvalTrans(MAE_TransContext* aContext, CAE_EBase* aExpContext, const string& aTrans)
{
    iInterpr->EvalTrans(aContext, aExpContext, aTrans);
}

const multimap<string, CSL_ExprBase*>& CAE_TaDesl::Exprs()
{
    return iInterpr->Exprs();
}

void* CAE_TaDesl::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this: NULL;
}

CSL_ExprBase* CAE_TaDesl::GetExpr(const string& aTerm, const string& aRtype)
{
    return iInterpr->GetExpr(aTerm, aRtype);
}

void CAE_TaDesl::GetConstructors(vector<string>& aRes)
{
    iInterpr->GetConstructors(aRes);
}

