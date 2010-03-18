#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <stdlib.h>

#include "ut_simple_increase.h"

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_SimpleIncrease );

const char* KLogSpecFileName = "../testfaplogspec.txt";


class CFT_Simple: public CAE_Object
{
public:
    static CFT_Simple* NewL(const char* aInstName, CAE_Object* aMan,
                TInt aStepNumMaxValue, CAE_StateBase::StateType aStepNumMaxType);
    CFT_Simple(const char* aInstName, CAE_Object* aMan);
    TInt StepNum() {return iStepNum->Value();}
    TInt StepNumMax() {return iStepNumMax->Value();}
    TInt UpdateStepCallsNum() {return iUpdateStepCallsNum;}
private:
    void ConstructL(TInt aStepNumMaxValue, CAE_StateBase::StateType aStepNumMaxType);
protected:
    CAE_TRANS_DEF(UpdateStep, CFT_Simple);
private:
    CAE_TState<TInt>*   iStepNum;
    CAE_TState<TInt>*   iStepNumMax;
    TInt                iUpdateStepCallsNum;
};

CFT_Simple::CFT_Simple(const char* aInstName, CAE_Object* aMan):
    CAE_Object(aInstName, aMan), iUpdateStepCallsNum(0)
{
}

void CFT_Simple::ConstructL(TInt aStepNumMaxValue, CAE_StateBase::StateType aStepNumMaxType)
{
    CAE_Object::ConstructL();

    iStepNum = CAE_TState<TInt>::NewL("StepNum", this, CAE_TRANS(UpdateStep), CAE_StateBase::EType_Reg);
    iStepNumMax = CAE_TState<TInt>::NewL("StepNumMax", this, TTransInfo(), aStepNumMaxType);

    ~(*iStepNum) = 0;
    iStepNum->AddInputL(iStepNum);

    ~(*iStepNumMax) = aStepNumMaxValue;
}

CFT_Simple* CFT_Simple::NewL(const char* aInstName, CAE_Object* aMan,
        TInt aStepNumMax, CAE_StateBase::StateType aNumMaxType)
{
    CFT_Simple* self = new CFT_Simple(aInstName, aMan);
    self->ConstructL(aStepNumMax, aNumMaxType);
    return self;
}

void CFT_Simple::UpdateStep(CAE_State* aState)
{
    printf("UpdateStep = %d \n", StepNum());
    iUpdateStepCallsNum++;
    if (StepNum() < StepNumMax())
        (*iStepNum) = StepNum() + 1;
}


void UT_FAP_SimpleIncrease::setUp()
{
    iEnv = CAE_Env::NewL(1, KLogSpecFileName);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
}

void UT_FAP_SimpleIncrease::tearDown()
{
    delete iEnv;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

void UT_FAP_SimpleIncrease::test_withMaxNum(TInt aMaxNum, CAE_StateBase::StateType aNumMaxType)
{
    CFT_Simple* simple = CFT_Simple::NewL("Simple", NULL, aMaxNum, aNumMaxType);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CFT_Simple", simple != 0);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect StepNum", 0, simple->StepNum());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect StepNumMax", aMaxNum, simple->StepNumMax());
    iEnv->AddL(simple);

    for (TInt i=0; i<aMaxNum; i++)
    {
        iEnv->Step();
        char buf[256];
        sprintf(buf, "Incorrect StepNum at attempt %d", i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, i+1, simple->StepNum());
        sprintf(buf, "Incorrect StepNumMax at attempt %d", i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, aMaxNum, simple->StepNumMax());
        sprintf(buf, "Incorrect UpdateStepCallsNum at attempt %d", i);
        CPPUNIT_ASSERT_EQUAL_MESSAGE(buf, i+1, simple->UpdateStepCallsNum());
    }

    iEnv->Step();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect StepNum at post1", aMaxNum, simple->StepNum());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect StepNumMax at post1", aMaxNum, simple->StepNumMax());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect UpdateStepCallsNum at post1", aMaxNum+1, simple->UpdateStepCallsNum());
    iEnv->Step();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect StepNum at post2", aMaxNum, simple->StepNum());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect StepNumMax at post2", aMaxNum, simple->StepNumMax());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Incorrect UpdateStepCallsNum at post2", aMaxNum+1, simple->UpdateStepCallsNum());
}

void UT_FAP_SimpleIncrease::test_SimpleIncrease_Type_Reg()
{
    test_withMaxNum(5, CAE_StateBase::EType_Reg);
}

void UT_FAP_SimpleIncrease::test_SimpleIncrease_Type_Input()
{
    test_withMaxNum(10, CAE_StateBase::EType_Input);
}
