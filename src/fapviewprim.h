
#ifndef __FAP_VIEWPRIM_H
#define __FAP_VIEWPRIM_H

// View interface

#include "fapplat.h"
#include <string.h>
#include <string>

using namespace std;

class MAE_Window;
class MAE_ViewObserver;

class CAV_Point
{
    public:
	CAV_Point(): iX(0), iY(0) {};
	CAV_Point(TInt aX, TInt aY): iX(aX), iY(aY) {};
	CAV_Point(const CAV_Point& aPt): iX(aPt.iX), iY(aPt.iY) {};
	CAV_Point operator+(const CAV_Point& aArg) { return CAV_Point(iX + aArg.iX, iY + aArg.iY);};
	CAV_Point operator-(const CAV_Point& aArg) { return CAV_Point(iX - aArg.iX, iY - aArg.iY);};
	CAV_Point& operator+=(const CAV_Point& aArg) { iX += aArg.iX, iY += aArg.iY; return *this;};
    public:
	TInt iX, iY;
};

class CAV_Rect
{
    public:
	CAV_Rect(): iTl(), iBr() {};
	CAV_Rect(CAV_Point aTl, CAV_Point aBr): iTl(aTl), iBr(aBr) {};
	TInt Height() { return iBr.iY - iTl.iY; };
	TInt Width() { return iBr.iX - iTl.iX; };
	void Move(CAV_Point aShift) { iTl += aShift; iBr += aShift; };
	TBool Includes(CAV_Point aPt) { return aPt.iX >= iTl.iX && aPt.iX <= iBr.iX && aPt.iY >= iTl.iY && aPt.iY <= iBr.iY;};
    public:
	CAV_Point iTl, iBr; 
};


#endif // __FAP_VIEWPRIM_H
