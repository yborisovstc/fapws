#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <fapstext.h>
#include <deslbase.h>
#include <stdlib.h>

#include <cppunit/extensions/HelperMacros.h>

/*
 * This test is for checking the functionality of modularity
 */

static const char* KLogFileName = "ut_modul_base.txt";
static const char* KSpecFileName = "ut_modul_base.xml";


class UT_FAP_Modul : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_Modul);
    CPPUNIT_TEST(test_Module_base);
    CPPUNIT_TEST(test_Module_loc);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_Module_base();
    void test_Module_loc();
private:
    CAE_Env* iEnv;
};

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_Modul );


void UT_FAP_Modul::setUp()
{
}

void UT_FAP_Modul::tearDown()
{
//    delete iEnv;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

void UT_FAP_Modul::test_Module_base()
{
    printf("\n === Test of modularity - Base\n");

    iEnv = CAE_Env::NewL(NULL, NULL, KSpecFileName, 1, NULL, KLogFileName);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
    iEnv->ConstructSystem();

    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }

    CAE_ConnPointBase* c_incr = iEnv->Root()->GetOutpN("my_incr");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [my_incr] output", c_incr != 0);
    CAE_StateBase* sb_incr = c_incr->GetSrcPin("_1")->GetFbObj(sb_incr);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [c_incr._1] src pin state", sb_incr != 0);
    CAE_TState<TInt>& s_incr = *sb_incr;
    CPPUNIT_ASSERT_MESSAGE("Incorrect [c_incr._1] value", ~s_incr == 45);

    delete iEnv;
}

static const char* KSpecFileName_Loc = "ut_modul_loc.xml";
static const char* KLogFileName_Loc = "ut_modul_loc.txt";

void UT_FAP_Modul::test_Module_loc()
{
    printf("\n === Test of modularity - Local\n");

    iEnv = CAE_Env::NewL(NULL, NULL, KSpecFileName_Loc, 1, NULL, KLogFileName_Loc);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
    iEnv->ConstructSystem();

    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }

    CAE_ConnPointBase* c_incr = iEnv->Root()->GetOutpN("output");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [output] output", c_incr != 0);
    CAE_StateBase* sb_incr = c_incr->GetSrcPin("_1")->GetFbObj(sb_incr);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [c_incr._1] src pin state", sb_incr != 0);
    CAE_TState<TInt>& s_incr = *sb_incr;
    CPPUNIT_ASSERT_MESSAGE("Incorrect [c_incr._1] value", ~s_incr == 45);

    delete iEnv;
}
