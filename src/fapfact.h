#ifndef __FAP_EXT_H
#define __FAP_EXT_H

//************************************************************
// Finite automata programming- factories  
// Yuri Borisov  26/08/06		  Initial version 
//*************************************************************

#include "fapbase.h"
#include <vector>

// Base class of provider implementation
class CAE_ProviderBase: public MAE_Provider
{
public:
	FAPWS_API CAE_ProviderBase();
	FAPWS_API virtual ~CAE_ProviderBase();
};

// Factory of element of object
class CAE_Fact: public MAE_Provider
{
public:
	FAPWS_API static CAE_Fact* NewL();
	FAPWS_API virtual ~CAE_Fact();
	FAPWS_API void AddProviderL(CAE_ProviderBase* aProv);
	// From MAE_Provider
	FAPWS_API virtual CAE_Base* CreateStateL(TUint32 aTypeUid, const char* aInstName, CAE_Object* aMan) const;
	FAPWS_API virtual CAE_Base* CreateObjectL(TUint32 aTypeUid) const;
protected:
	FAPWS_API CAE_Fact();
	FAPWS_API void ConstructL();
private:
	CAE_ProviderBase* GetProviderAt(TInt aInd) const;
private:
	vector<CAE_ProviderBase*>* iProviders;
};


// General provider
class CAE_ProviderGen: public CAE_ProviderBase
{
public:
	FAPWS_API CAE_ProviderGen();
	FAPWS_API virtual ~CAE_ProviderGen();
	// From MAE_Provider
	FAPWS_API virtual CAE_Base* CreateStateL(TUint32 aTypeUid, const char* aInstName, CAE_Object* aMan) const;
	FAPWS_API virtual CAE_Base* CreateObjectL(TUint32 aTypeUid) const;
};


#endif // __FAP_EXT_H
