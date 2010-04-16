#ifndef __UT_FAP_SPEC_CREAT_H
#define __UT_FAP_SPEC_CREAT_H

#include "fapext.h"
//#include "fapbase.h"

#include <cppunit/extensions/HelperMacros.h>

/**
 * Class to perform unit test of creation environment from spec (xml)
 *
 * Test procedure is the following:
 */
class UT_FAP_SpecCreat : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_SpecCreat);
    CPPUNIT_TEST(test_SpecCreat_main);
    CPPUNIT_TEST_SUITE_END();

public:
    /**
     * Set up the test session
     */
    virtual void setUp();
    /**
     * Clean up the test session
     */
    virtual void tearDown();

private:
    void test_SpecCreat_main();
private:
    CAE_Env* iEnv;
};

#endif // __UT_FAP_SPEC_CREAT_H
