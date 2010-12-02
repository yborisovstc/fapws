#ifndef __FAP_STEXT_H
#define __FAP_STEXT_H

// FAP States extentions

#include "fapbase.h"
#include <stdlib.h>

struct CF_TdPoint
{
    CF_TdPoint() {}
    CF_TdPoint(TInt aX, TInt aY): iX(aX), iY(aY) {}
    TInt iX;
    TInt iY;
};

struct CF_TdVectF;

struct CF_TdPointF
{
    CF_TdPointF() {}
    CF_TdPointF(const CF_TdPointF& aSrc): iX(aSrc.iX), iY(aSrc.iY) {}
    inline CF_TdPointF(const CF_TdVectF& aSrc);
    CF_TdPointF(const CF_TdPoint& aSrc): iX(aSrc.iX), iY(aSrc.iY)  {}
    CF_TdPointF(float aX, float aY): iX(aX), iY(aY) {}
    CF_TdPointF& operator=(const CF_TdPointF& aOpd) { iX=aOpd.iX; iY=aOpd.iY; return *this;}
    CF_TdPointF operator+(const CF_TdPointF& aOpd) { return CF_TdPointF(iX+aOpd.iX, iY+aOpd.iY);}
    CF_TdPointF operator-(const CF_TdPointF& aOpd) { return CF_TdPointF(iX-aOpd.iX, iY-aOpd.iY);}
    float M2() {return (iX*iX + iY*iY);}
    float iX;
    float iY;
};

struct CF_TdVect
{
    TInt iX;
    TInt iY;
};

struct CF_TdVectF
{
    CF_TdVectF() {}
    CF_TdVectF(const CF_TdVectF& aSrc): iX(aSrc.iX), iY(aSrc.iY) {}
    CF_TdVectF(const CF_TdPointF& aEnd): iX(aEnd.iX), iY(aEnd.iY) {}
    CF_TdVectF(const CF_TdPointF& aBeg, const CF_TdPointF& aEnd): iX(aEnd.iX-aBeg.iX), iY(aEnd.iY-aBeg.iY) {}
    CF_TdVectF(float aX, float aY): iX(aX), iY(aY) {}
    CF_TdVectF operator+(const CF_TdVectF& aOpd) { return CF_TdVectF(iX+aOpd.iX, iY+aOpd.iY);} 
    CF_TdVectF operator-(const CF_TdVectF& aOpd) { return CF_TdVectF(iX-aOpd.iX, iY-aOpd.iY);} 
    CF_TdVectF operator*(float aOpd) { return CF_TdVectF(iX*aOpd, iY*aOpd); }
    CF_TdVectF operator/(float aOpd) { return CF_TdVectF(iX/aOpd, iY/aOpd); }
    CF_TdVectF& operator=(const CF_TdVectF& aOpd) { iX=aOpd.iX; iY=aOpd.iY; return *this;}
    CF_TdVectF& operator+=(const CF_TdVectF& aOpd) { iX+=aOpd.iX; iY+=aOpd.iY; return *this;}
    CF_TdVectF& operator-=(const CF_TdVectF& aOpd) { iX-=aOpd.iX; iY-=aOpd.iY; return *this;}
    float M2() {return (iX*iX + iY*iY);}
 
    float iX;
    float iY;
};

inline CF_TdPointF::CF_TdPointF(const CF_TdVectF& aSrc): iX(aSrc.iX), iY(aSrc.iY) {}

struct CF_Rect
{
    CF_Rect(float aLeft, float aTop, float aRight, float aBottom) {
	iLeftUpper.iX=aLeft; iLeftUpper.iY=aTop; iRightLower.iX=aRight; iRightLower.iY=aBottom;}
    CF_Rect() {}
    CF_TdPoint iLeftUpper;
    CF_TdPoint iRightLower;
};


/*
   template<>inline const char *CAE_TState<TUint8>::Type() {return "StUint8"; }

template<>inline const char *CAE_TState<TInt>::Type() {return "StInt"; }

template<>inline const char *CAE_TState<TUint32>::Type() {return "StUint32"; }

template<>inline const char *CAE_TState<TBool>::Type() {return "StBool"; }
*/

#endif // __FAP_STEXT_H
