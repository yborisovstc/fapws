//************************************************************
// Finite automata programming extension, level 1 prototype 
// Yuri Borisov  15/07/05  FAP_CR_007  Added FAP environment 
// Yuri Borisov  18/07/05  FAP_CR_009  Added support of logging
//*************************************************************

#include "fapbase.h"
#include "fapext.h"
#include "faplogger.h"
#include "panics.h"

const char* KRootName = "Root";

CAE_Env::CAE_Env(TInt aPriority, TInt aLoad): CActive(aPriority),
	iLoad(aLoad), iLogger(NULL), iStepCount(0)
{
}

FAPWS_API CAE_Env::~CAE_Env()
{
	delete iRoot;
	delete iLogger;
}


FAPWS_API CAE_Env* CAE_Env::NewL(TInt aPriority, const char* aLogSpecFile, TInt aLoad)
{
	CAE_Env* self = new CAE_Env(aPriority, aLoad);
	self->ConstructL(aLogSpecFile);
	return self;
}

void CAE_Env::ConstructL(const char* aLogSpecFile)
{
	iRoot = CAE_Object::NewL(KRootName, NULL);
	iLogger = CAE_LogCtrl::NewL(iRoot, aLogSpecFile);
	SetActive();
//	TRequestStatus* pst = &iStatus;
//	User::RequestComplete(pst, KErrNone);
}

FAPWS_API void CAE_Env::RunL()
{
	for (TInt i = 0; i <iLoad; i++)
	{
		iStepCount++;
		iRoot->Update();
		if (iLogger != NULL)
		{
			iLogger->DoLogL(iStepCount);
		}
		iRoot->Confirm();
	}
	SetActive();
//	TRequestStatus* pst = &iStatus;
//	User::RequestComplete(pst, KErrNone);
}

FAPWS_API void CAE_Env::Step()
{
	iStepCount++;
	iRoot->Update();
	if (iLogger != NULL)
	{
		iLogger->DoLogL(iStepCount);
	}
	iRoot->Confirm();
}


FAPWS_API void CAE_Env::DoCancel()
{
}

FAPWS_API void CAE_Env::AddL(CAE_Object* aComp)
{
	iRoot->RegisterCompL(aComp);
}
