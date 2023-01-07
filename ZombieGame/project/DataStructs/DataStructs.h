#ifndef DATASTRUCTS
#define DATASTRUCTS
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"



struct Rect final
{
	Rect(float left, float bottom, float width, float height)
	{
		Bottom = bottom;
		Left = left;
		Width = width;
		Height = height;
	}

	Rect() = default;

	float Bottom{}, Left{}, Width{}, Height{};

	bool IsVisited{ false };


	bool IsInBounds(Elite::Vector2 point)
	{
		return (Left <= point.x && point.x <= Left + Width) && (Bottom <= point.y && point.y <= Bottom + Height);
	}
};


struct HouseInfoExtended final : HouseInfo
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

struct ItemInfoExtended final: ItemInfo{

	void LoadItemInfo(ItemInfo info)
	{
		Location = info.Location;
		Type = info.Type;
		ItemHash = info.ItemHash;

		IsInit = true;
	}


	ItemInfo AsItemInfo()
	{
		ItemInfo toReturn{};

		toReturn.Location = Location;
		toReturn.Type = Type;
		toReturn.ItemHash = ItemHash;

		return toReturn;
	}
	bool IsInit{ false };
};

struct WorldInfoExtended final : WorldInfo 
{
	Rect Bounds;

	int CurrentCellIndex{};
	std::vector<Rect> WorldGrid{};

	void LoadWorldInfo(WorldInfo info)
	{
		IsInit = true;
		Center = info.Center;
		Dimensions = info.Dimensions;

		Bounds = Rect{ Center.x - Dimensions.x * 0.5f, Center.y - Dimensions.y * 0.5f, Dimensions.x - 4, Dimensions.y - 4 };

		int nrOfCells{ 75 };
		float gridCellWidth{ Bounds.Width / nrOfCells };
		float gridCellHeight{ Bounds.Height / nrOfCells };
		for (int i = 0; i <= nrOfCells; ++i)
		{
			for (int j = 0; j <= nrOfCells; ++j)
			{
				Rect cell{
					Bounds.Left + gridCellWidth * i,
					Bounds.Bottom + gridCellHeight * j,
					gridCellWidth,
					gridCellHeight
				};
				WorldGrid.push_back(cell);
			}
		}

	}



	bool IsInit{ false };
};

#endif // !DATASTRUCTS


