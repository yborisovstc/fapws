#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <stdlib.h>

#include "ut_spec_creat.h"

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_SpecCreat );

static const char* KLogSpecFileName = "../testfaplogspec.txt";
static const char* KLogFileName = "ut_creat_log.txt";
const char* KSpecFileName = "ut_spec_creat_1.xml";

void update_event(CAE_Object* aObject, CAE_State* aState);
void update_timer(CAE_Object* aObject, CAE_State* aState);

const TTransInfo KTinfo_Update_event = TTransInfo(update_event, "trans_event");
const TTransInfo KTinfo_Update_timer = TTransInfo(update_timer, "trans_timer");

void update_event(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TBool> *pself = CAE_TState<TBool>::Interpret(aState);
    CPPUNIT_ASSERT_MESSAGE("Fail to interpret state [event]", pself != 0);
    CAE_TState<TUint32> *ptimer = CAE_TState<TUint32>::Interpret((CAE_State *)aState->Input("timer"));
    CPPUNIT_ASSERT_MESSAGE("Fail getting timer", ptimer != 0);
    CAE_TState<TBool> *pevent = CAE_TState<TBool>::Interpret((CAE_State *)aState->Input("event"));
    CPPUNIT_ASSERT_MESSAGE("Fail getting event", pevent != 0);
    TUint32 vtimer = (*ptimer)();
    TBool vevent = (*pevent)();
    *pself = vtimer && vevent || !vtimer && !vevent;
}

void update_timer(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TUint32> *pself = CAE_TState<TUint32>::Interpret(aState);
    CPPUNIT_ASSERT_MESSAGE("Fail to interpret state [timer]", pself != 0);
    CAE_TState<TUint32> *period = CAE_TState<TUint32>::Interpret((CAE_State *)aState->Input("period"));
    CPPUNIT_ASSERT_MESSAGE("Fail getting period", period != 0);
    CAE_TState<TUint32> *ptimer = CAE_TState<TUint32>::Interpret((CAE_State *)aState->Input("timer"));
    CPPUNIT_ASSERT_MESSAGE("Fail getting timer", ptimer != 0);
    TUint32 vperiod = (*period)();
    TUint32 vtimer = (*ptimer)();
    *pself = (vtimer > vperiod) ? 0: vtimer + 1;
}


void UT_FAP_SpecCreat::setUp()
{
    iEnv = CAE_Env::NewL(1, KLogSpecFileName, KLogFileName);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
}

void UT_FAP_SpecCreat::tearDown()
{
    delete iEnv;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

void UT_FAP_SpecCreat::test_SpecCreat_main()
{
    printf("\n === Test of creation object from spec");
    iEnv->AddChmanXml(KSpecFileName);
    iEnv->Provider()->RegisterTransf(&KTinfo_Update_event);
    iEnv->Provider()->RegisterTransf(&KTinfo_Update_timer);
    void *spec = iEnv->Chman()->GetChild(NULL);
    CPPUNIT_ASSERT_MESSAGE("Fail to get spec", spec != 0);
    CAE_Object *test  = CAE_Object::NewL(NULL, NULL, spec, iEnv);
    CPPUNIT_ASSERT_MESSAGE("Fail to create test object", test != 0);
    iEnv->AddL(test);
    CAE_Object *gen = test->GetComp("generator");
    CPPUNIT_ASSERT_MESSAGE("Fail to get generator", gen != 0);
    CAE_State *period = gen->GetInput("period");
    CPPUNIT_ASSERT_MESSAGE("Fail to get period", period != 0);
    CAE_TState<TUint32> *tper = CAE_TState<TUint32>::Interpret(period);
    CPPUNIT_ASSERT_MESSAGE("Fail to interpret period", tper != 0);
//    (*tper) = 5;// Implemented state init from spec
    


    for (TInt i=0; i<20; i++)
    {
        iEnv->Step();
    }
}

