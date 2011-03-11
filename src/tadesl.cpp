
// FAP transition agent plugin 

#include "tadesl.h"
#include "deslbase.h"

CAE_TaDesl::CAE_TaDesl(MCAE_LogRec* aLogger): CAE_TranExBase(aLogger)
{
}

CAE_TaDesl::~CAE_TaDesl()
{
}

void CAE_TaDesl::EvalTrans(CAE_StateBase* aState, const string& aTrans)
{
    CSL_Interpr *interpr = new CSL_Interpr(iLogger);
    interpr->Interpret(aTrans, aState);
    delete interpr;
}

void* CAE_TaDesl::DoGetFbObj(const char *aName)
{
    return (strcmp(aName, Type()) == 0) ? this: NULL;
}
