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


	int PositionToIndex(const Elite::Vector2 pos) const
	{

		auto foundIt = std::find_if(begin(HouseGrid), end(HouseGrid), [&](Rect r) {
			return r.IsInBounds(pos);
			});


		if (foundIt == end(HouseGrid))
			return -1;

		return foundIt - begin(HouseGrid);
	}

};

struct EntityInfoExtended final : EntityInfo {

	void LoadItemInfo(EntityInfo info)
	{
		Location = info.Location;
		Type = info.Type;
		EntityHash = info.EntityHash;

		IsInit = true;
	}


	EntityInfo AsEntityInfo()
	{
		EntityInfo toReturn{};

		toReturn.Location = Location;
		toReturn.Type = Type;
		toReturn.EntityHash = EntityHash;

		return toReturn;
	}
	bool IsInit{ false };
};

struct WorldInfoExtended final : WorldInfo
{

	int NrOfCells{  };
	float CellWidth{ };
	float CellHeight{ };


	bool IsInit{ false };
	Rect Bounds;
	int CurrentCellIndex{0};
	std::vector<Rect> WorldGrid{};

	void LoadWorldInfo(WorldInfo info)
	{
		IsInit = true;
		Center = info.Center;
		Dimensions = info.Dimensions;

		Bounds = Rect{ Center.x - Dimensions.x * 0.5f, Center.y - Dimensions.y * 0.5f, Dimensions.x - 4, Dimensions.y - 4 };

		int nrOfCells{ 11 };
		CellWidth = Bounds.Width / nrOfCells;
		CellHeight = Bounds.Height / nrOfCells;
		std::vector<Rect> tempVector;
		for (int i = 0; i < nrOfCells; ++i)
		{
			for (int j = 0; j < nrOfCells; ++j)
			{
				Rect cell{
					Bounds.Left + CellWidth * j,
					Bounds.Bottom + CellHeight * i,
					CellWidth,
					CellHeight
				};
				tempVector.push_back(cell);
			}
		}

		
		int top = nrOfCells - 1, bottom = 0, left = 0, right = nrOfCells - 1;

		while (top >= bottom && left <= right)
		{
			// Add the top row to the vector
			for (int i = left; i <= right; ++i)
			{
				WorldGrid.push_back(tempVector[top * nrOfCells + i]);
			}

			top--;

			// Break if no more elements
			if (top < bottom || left > right)
				break;

			// Print the rightmost column
			for (int i = top; i >= bottom; i--)
			{
				WorldGrid.push_back(tempVector[i * nrOfCells + right]);
			}
			
			right--;

			// Break if no more elements
			if (top < bottom || left > right)
				break;

			// Print the bottom row
			for (int i = right; i >= left; i--)
			{
				WorldGrid.push_back(tempVector[bottom * nrOfCells + i]);
			}

			bottom++;

			// Break if no more elements
			if (top < bottom || left > right)
				break;



			// Print the leftmost column	
			for (int i = bottom; i <= top; ++i)
			{
				WorldGrid.push_back(tempVector[i * nrOfCells + left]);
			}

			left++;

			if (top < bottom || left > right)
				break;
		}

		std::reverse(begin(WorldGrid), end(WorldGrid));


	}


	int PositionToIndex(const Elite::Vector2 pos) const
	{

		auto foundIt = std::find_if(begin(WorldGrid), end(WorldGrid), [&](Rect r) {
			return r.IsInBounds(pos);
			});


		if (foundIt == end(WorldGrid))
			return -1;

		return foundIt - begin(WorldGrid);
	}
};

#endif // !DATASTRUCTS


