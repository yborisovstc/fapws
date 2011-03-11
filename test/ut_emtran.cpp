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
}


