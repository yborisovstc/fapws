//************************************************************
// Finite automata programming extension, level 1 prototype 
// Yuri Borisov  15/07/05  FAP_CR_007  Added FAP environment 
// Yuri Borisov  18/07/05  FAP_CR_009  Added support of logging
//*************************************************************

#include "fapbase.h"
#include "fapext.h"
#include "faplogger.h"
#include "fapfact.h"
#include "panics.h"

const char* KRootName = "Root";

CAE_Env::CAE_Env(TInt aPriority, TInt aLoad): CActive(aPriority),
	iLoad(aLoad), iLogger(NULL), iStepCount(0), iChroman(NULL)
{
}

FAPWS_API CAE_Env::~CAE_Env()
{
    delete iRoot;
    if (iChroman != NULL)
    {
	delete iChroman;
	iChroman = NULL;
    }
    delete iLogger;
}


FAPWS_API CAE_Env* CAE_Env::NewL(TInt aPriority, const char* aLogSpecFile, const char *aLogFileName, TInt aLoad)
{
	CAE_Env* self = new CAE_Env(aPriority, aLoad);
	self->ConstructL(aLogSpecFile, aLogFileName);
	return self;
}

FAPWS_API CAE_Env* CAE_Env::NewL(const TStateInfo** aSinfos, const TTransInfo** aTinfos, const char* aSpecFile, TInt aPriority, 
	const char* aLogSpecFile, const char *aLogFileName, TInt aLoad)
{
	CAE_Env* self = new CAE_Env(aPriority, aLoad);
	self->ConstructL(aSinfos, aTinfos, aSpecFile, aLogSpecFile, aLogFileName);
	return self;
}

void CAE_Env::ConstructL(const char* aLogSpecFile, const char *aLogFileName)
{
    iRoot = CAE_Object::NewL(KRootName, NULL, (const char *) NULL);
    iLogger = CAE_LogCtrl::NewL(iRoot, aLogSpecFile, aLogFileName);
    iProvider = CAE_Fact::NewL();
    SetActive();
}

void CAE_Env::ConstructL(const TStateInfo** aSinfos, const TTransInfo** aTinfos, const char* aSpec, 
	const char* aLogSpecFile, const char *aLogFileName)
{
    AddChmanXml(aSpec);
    iLogger = CAE_LogCtrl::NewL(iRoot, aLogSpecFile, aLogFileName);
    iProvider = CAE_Fact::NewL();
    if (aSinfos != NULL) {
	iProvider->RegisterStates(aSinfos);
    }
    if (aTinfos != NULL) {
	iProvider->RegisterTransfs(aTinfos);
    }
    void *rootspec = Chman()->GetChild(NULL);
    iRoot = CAE_Object::NewL(KRootName, NULL, (const char *) rootspec, this);
    SetActive();
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
}

FAPWS_API void CAE_Env::Step()
{
	iStepCount++;
	if (Logger())
	    Logger()->WriteFormat(" ============= TICK: %d ===============", iStepCount);
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



MAE_Provider *CAE_Env::Provider() const
{
    return iProvider;
}

MCAE_LogRec *CAE_Env::Logger()
{
    return iLogger->LogRec();
}

FAPWS_API MAE_ChroMan* CAE_Env::Chman() const 
{ 
    return iChroman;
}

FAPWS_API void CAE_Env::AddChmanXml(const char *aXmlFileName)
{
    // Create Chromosome manager. There is only one for now.
    // So don't provide the functionality of selecting of manager
    // TODO [YB] Consider adding selection of manager
    _FAP_ASSERT(iChroman == NULL);
    iChroman = CAE_ChroManXFact::CreateChroManX(aXmlFileName);
}

