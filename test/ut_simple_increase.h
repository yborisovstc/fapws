#ifndef __UT_FAP_SIMPLE_INCREASE_H
#define __UT_FAP_SIMPLE_INCREASE_H

#include "fapext.h"
//#include "fapbase.h"

#include <cppunit/extensions/HelperMacros.h>

/**
 * Class to perform unit test of CAE_Env, CAE_Object, CAE_TState classes
 *
 * Test procedure is the following:
 * Create simple CAE_Object (increment itself at each step, till max num).
 * Object has two CAE_TState: current number (iStepNum) and max number (iStepNumMax).
 */
class UT_FAP_SimpleIncrease : public CPPUNIT_NS::TestFixture
{
    CPPUNIT_TEST_SUITE(UT_FAP_SimpleIncrease);
    CPPUNIT_TEST(test_SimpleIncrease_Type_Reg);
    CPPUNIT_TEST(test_SimpleIncrease_Type_Input);
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
    void test_withMaxNum(TInt aMaxNum, CAE_State::StateType aNumMaxType);
    /**
     * Test when iStepNumMax is of CAE_State::EType_Reg type
     */
    void test_SimpleIncrease_Type_Reg();
    /**
     * Test when iStepNumMax is of CAE_State::EType_Input type
     */
    void test_SimpleIncrease_Type_Input();

private:
    CAE_Env* iEnv;
};

#endif // __UT_FAP_SIMPLE_INCREASE_H
