#include <fapext.h>
#include <fapplat.h>
#include <fapbase.h>
#include <fapstext.h>
#include <stdlib.h>

#include <cppunit/extensions/HelperMacros.h>

/*
 * This test is for checking the functionality of embedded transitions
 */

static const char* KLogSpecFileName = "../testfaplogspec.txt";
static const char* KLogFileName = "ut_emtran_log.txt";
static const char* KSpecFileName = "ut_emtran_spec_2.xml";
// static const char* KSpecFileName = "ut_emtran_spec.xml";


static const TTransInfo* tinfos[] = { NULL};



class UT_FAP_Emtran : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_Emtran);
    CPPUNIT_TEST(test_Emtran_main);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_Emtran_main();
private:
    CAE_Env* iEnv;
};

CPPUNIT_TEST_SUITE_REGISTRATION( UT_FAP_Emtran );


void UT_FAP_Emtran::setUp()
{
    iEnv = CAE_Env::NewL(NULL, tinfos, KSpecFileName, 1, NULL, KLogFileName);
    CPPUNIT_ASSERT_MESSAGE("Fail to create CAE_Env", iEnv != 0);
}

void UT_FAP_Emtran::tearDown()
{
    delete iEnv;
    CPPUNIT_ASSERT_EQUAL_MESSAGE("tearDown", 0, 0);
}

void UT_FAP_Emtran::test_Emtran_main()
{
    printf("\n === Test of embedded trans\n");

    for (TInt i=0; i<40; i++)
    {
        iEnv->Step();
    }

    CAE_ConnPointBase* c_snail_1 = iEnv->Root()->GetOutpN("snail_1");
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_1] output", c_snail_1 != 0);
    CAE_StateBase* sb_snail_1_coord = c_snail_1->GetSrcPin("coord")->GetFbObj(sb_snail_1_coord);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail_1.coord] src pin state", sb_snail_1_coord != 0);
    CAE_TState<TInt>& s_snail_1_coord = *sb_snail_1_coord;
    CPPUNIT_ASSERT_MESSAGE("Incorrect [snail_1.coord] value", ~s_snail_1_coord == 504);

    CAE_ConnPointExtC* ext = c_snail_1->GetFbObj(ext);
    CPPUNIT_ASSERT_MESSAGE("Fail to get [snail] extender", ext != 0);
    for (vector<CAE_ConnPointExtC::Slot>::iterator it = ext->Slots().begin(); it != ext->Slots().end(); it++) {
	CAE_ConnPointBase* cp = it->Srcs()["coord"].first;
	CPPUNIT_ASSERT_MESSAGE("Fail to get [coord] conn point in [snail]extender", cp != 0);
	CAE_StateBase* sb_snail_coord = cp->GetSrcPin("_1")->GetFbObj(sb_snail_coord);
	CPPUNIT_ASSERT_MESSAGE("Fail to get [coord] src pin state in [snail] extender", sb_snail_coord != 0);
    };
}


