#ifndef __FAP_EXT_H
#define __FAP_EXT_H

//************************************************************
// Finite automata programming extensions, level 1 prototype  
// Yuri Borisov  15/07/05  FAP_CR_007  Added FAP environment 
// Yuri Borisov  18/07/05  FAP_CR_009  Added support of logging
//*************************************************************

#include <string.h>
#include "fapbase.h"

class CAE_LogCtrl;
class CAE_ChroManBase;

class CAE_Env: public CActive, public MAE_Env
{
public:
	enum
	{
		KEnvLoadDefault = 3
	};
public:
	FAPWS_API static CAE_Env* NewL(TInt aPriority, const char* aLogSpecFile, const char *aLogFileName = NULL, TInt aLoad = KEnvLoadDefault);
	FAPWS_API virtual ~CAE_Env();
	FAPWS_API void AddL(CAE_Object* aComp);
	FAPWS_API void Step();
	FAPWS_API void AddChmanXml(const char *aXmlFileName);
	FAPWS_API inline CAE_Object *Root();
public:
	// From MAE_Env
	virtual MAE_Provider *Provider() const;
	virtual MAE_ChroMan *Chman() const;
	virtual MCAE_LogRec *Logger();
protected:
	FAPWS_API CAE_Env(TInt aPriority, TInt aLoad = KEnvLoadDefault);
	FAPWS_API void ConstructL(const char* aLogSpecFile, const char *aLogFileName = NULL);
	FAPWS_API virtual void DoCancel();
	FAPWS_API virtual void RunL();
private:
	CAE_Object* iRoot;
	TInt		iLoad;	// The number of step per one activity
	CAE_LogCtrl* iLogger;
	TUint32		iStepCount;
	CAE_ChroManBase *iChroman; // Chromosome manager
	MAE_Provider *iProvider;
};


inline CAE_Object *CAE_Env::Root() { return iRoot;}

#endif // __FAP_EXT_H
