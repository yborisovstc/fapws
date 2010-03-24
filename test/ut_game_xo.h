#ifndef __UT_FAP_GAME_XO_H
#define __UT_FAP_GAME_XO_H

#include "fapext.h"

#include <cppunit/extensions/HelperMacros.h>

/**
 * Class to perform unit test of CAE_Env, CAE_Object, CAE_TState classes
 *
 * Test procedure is the following:
 * Create CAE_Object which represents game XO.
 * Object has several states.
 */
class UT_FAP_GameXO : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_GameXO);
    CPPUNIT_TEST(test_gameXO);
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
    /**
     * Check gameXO
     */
    void test_gameXO();

private:

};

#endif // __UT_FAP_GAME_XO_H