#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <fapstext.h>
#include <stdlib.h>

#include "ut_spec_creat.h"

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_SpecCreat );

static const char* KLogSpecFileName = "../testfaplogspec.txt";
static const char* KLogFileName = "ut_creat_log.txt";
const char* KSpecFileName = "ut_spec_creat_1.xml";

void update_event(CAE_Object* aObject, CAE_State* aState);
void update_count(CAE_Object* aObject, CAE_State* aState);

const TTransInfo KTinfo_Update_event = TTransInfo(update_event, "trans_event");
const TTransInfo KTinfo_Update_timer = TTransInfo(update_count, "trans_count");
const TTransInfo* tinfos[] = {&KTinfo_Update_event, &KTinfo_Update_timer, NULL};

void update_event(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TBool> *pself = (CAE_TState<TBool>*)aState;
    CPPUNIT_ASSERT_MESSAGE("Fail to interpret state [event]", pself != 0);
    CAE_TState<TUint32> *pcount = CAE_TState<TUint32>::Interpret((CAE_State *)aState->Input("count"));
    CPPUNIT_ASSERT_MESSAGE("Fail getting timer", pcount != 0);
    CAE_TState<TBool> *pevent = CAE_TState<TBool>::Interpret((CAE_State *)aState->Input("self"));
    CPPUNIT_ASSERT_MESSAGE("Fail getting event", pevent != 0);
    TUint32 vcount = ~*pcount;
    TBool vevent = ~*pevent;
//    *pself = vcount && vevent || !vcount && !vevent;
    *pself = (vcount == 0);
}

void update_count(CAE_Object* aObject, CAE_State* aState)
{
    CAE_TState<TUint32> *pself = CAE_TState<TUint32>::Interpret(aState);
    CPPUNIT_ASSERT_MESSAGE("Fail to interpret state [count]", pself != 0);
    CAE_TState<TUint32> *period = CAE_TState<TUint32>::Interpret((CAE_State *)aState->Input("period"));
    CPPUNIT_ASSERT_MESSAGE("Fail getting period", period != 0);
    CAE_TState<TBool> *penable = CAE_TState<TBool>::Interpret((CAE_State *)aState->Input("enable"));
    CPPUNIT_ASSERT_MESSAGE("Fail getting enable", penable != 0);
    TUint32 vperiod = ~*period;
    TUint32 vself = ~*pself;
    TBool venable = ~*penable;
    *pself = (vself > vperiod) ? 0: (venable ? vself + 1: vself);
}


void UT_FAP_SpecCreat::setUp()
{
    iEnv = CAE_Env::NewL(NULL, tinfos, KSpecFileName, 1, NULL, KLogFileName);
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
    CAE_Object *gen = iEnv->Root()->GetComp("gen_2");
    CPPUNIT_ASSERT_MESSAGE("Fail to get component [gen_2]", gen != 0);
    CAE_State *event = gen->GetOutpState("event");
    CPPUNIT_ASSERT_MESSAGE("Fail to get output [event]", event != 0);
    CAE_TState<TBool> *tevent = CAE_TState<TBool>::Interpret(event);
    CPPUNIT_ASSERT_MESSAGE("Fail to interpret event", tevent != 0);

    for (TInt i=0; i<20; i++)
    {
        iEnv->Step();
    }
}


