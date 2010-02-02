#ifndef __FAP_FACT_H
#define __FAP_FACT_H

//************************************************************
// Finite automata programming- factories  
// Yuri Borisov  26/08/06		  Initial version 
//*************************************************************

#include <fapbase.h>
#include <vector>

class CAE_ChroManBase;


// Base class of provider implementation
class CAE_ProviderBase: public MAE_Provider
{
    public:
	FAPWS_API CAE_ProviderBase();
	FAPWS_API virtual ~CAE_ProviderBase();
	virtual MAE_ChroMan* Chman() const { return NULL;};
};

// Factory of element of object
class CAE_Fact: public MAE_Provider
{
    public:
	FAPWS_API static CAE_Fact* NewL();
	FAPWS_API virtual ~CAE_Fact();
	FAPWS_API void AddProviderL(CAE_ProviderBase* aProv);
	FAPWS_API void AddChmanXml(const char *aXmlFileName);
	// From MAE_Provider
	FAPWS_API virtual CAE_Base* CreateStateL(TUint32 aTypeUid, const char* aInstName, CAE_Object* aMan) const;
	FAPWS_API virtual CAE_Base* CreateObjectL(TUint32 aTypeUid) const;
	FAPWS_API virtual CAE_Base* CreateObjectL(const char *aName) const;
	virtual MAE_ChroMan* Chman() const;
    protected:
	FAPWS_API CAE_Fact();
	FAPWS_API void ConstructL();
    private:
	CAE_ProviderBase* GetProviderAt(TInt aInd) const;
    private:
	vector<CAE_ProviderBase*>* iProviders;
	CAE_ChroManBase *iChroman; // Chromosome manager
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
	FAPWS_API virtual CAE_Base* CreateObjectL(const char *aName) const;
};

// Base of chromosome manager
class CAE_ChroManBase: public MAE_ChroMan
{
    public:
	FAPWS_API virtual ~CAE_ChroManBase() {};
    protected:
	FAPWS_API CAE_ChroManBase() {};
};

// Chromosome manager for XML based chromosome
class CAE_ChroManX: public CAE_ChroManBase
{
    public:
	static CAE_ChroManX *New(const char *aFileName);
	virtual ~CAE_ChroManX();
	virtual void Copy(const void *aSrc, void *aDest);
    protected:
	void Construct(const char *aFileName);
	CAE_ChroManX();
    private:
	void *iDoc;	// XML document
};


#endif // __FAP_FACT_H
