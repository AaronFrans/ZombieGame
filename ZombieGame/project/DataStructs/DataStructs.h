#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"



struct Rect
{
	Rect(float left, float bottom, float width, float height)
	{
		Bottom = bottom;
		Left = left;
		Width = width;
		Height = height;
	}
	Rect() = default;

	float Bottom, Left, Width, Height;

	bool visited{ false };


	bool IsInBounds(Elite::Vector2 point)
	{
		return (Left <= point.x && point.x <= Left + Width) && (Bottom <= point.y && point.y <= Bottom + Height);
	}
};


struct HouseInfoExtended : HouseInfo
{
	Rect Bounds;

	int CurrentCellIndex{};
	std::vector<Rect> HouseGrid;

	bool IsInit{ false };

	void LoadHouseInfo(HouseInfo info, float playerSize)
	{
		Center = info.Center;
		Size = info.Size;


		Bounds = Rect{ Center.x - Size.x * 0.5f + 2, Center.y - Size.y * 0.5f + 2, Size.x - 4, Size.y - 4 };



		//Split house into rectangles

		float gridXCells{ Bounds.Width / playerSize };
		float gridYCells{ Bounds.Height / playerSize };
		for (int i = 0; i <= std::floor(gridXCells) - 1; ++i)
		{
			for (int j = 0; j <= std::floor(gridYCells) - 1; ++j)
			{
				Rect cell{
					Bounds.Left + playerSize * i,
					Bounds.Bottom + playerSize * j,
					playerSize,
					playerSize
				};
				HouseGrid.push_back(cell);
			}
		}

		IsInit = true;
	}


};