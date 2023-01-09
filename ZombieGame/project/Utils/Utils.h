#ifndef UTILS
#define UTILS
#include "stdafx.h"
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"


namespace Utils {

	//using: http://devmag.org.za/2009/04/17/basic-collision-detection-in-2d-part-2/
	bool IsInCircle(const Elite::Vector2& location, const Elite::Vector2& center, const float& radius)
	{
		Elite::Vector2 toCenter{ center - location };

		return toCenter.MagnitudeSquared() < Elite::Square(radius);
	}
}

#endif