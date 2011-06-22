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

static const char* KLogFileName = "ut_modul_log.txt";
static const char* KSpecFileName = "ut_modul_base.xml";


class UT_FAP_Modul : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_Modul);
    CPPUNIT_TEST(test_Module_base);
    CPPUNIT_TEST_SUITE_END();
public:
    virtual void setUp();
    virtual void tearDown();
private:
    void test_Module_base();
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

    delete iEnv;
}


