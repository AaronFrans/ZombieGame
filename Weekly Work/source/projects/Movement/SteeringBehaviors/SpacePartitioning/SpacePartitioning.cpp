#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"

// --- Cell ---
// ------------
Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{

	m_CellWidth = m_SpaceWidth / m_NrOfRows;
	m_CellHeight = m_SpaceHeight / m_NrOfCols;


	for (int row{}; row < m_NrOfRows; row++)
	{
		for (int col{}; col < m_NrOfCols; col++)
		{
			float left{ m_CellWidth * row }, bottom{ m_CellHeight * col };
			Cell cell{
				left,
				bottom,
				m_CellWidth,
				m_CellHeight, };
			m_Cells.push_back(cell);
		}
	}
}

void CellSpace::AddAgent(SteeringAgent* pAgent)
{
	int index{ PositionToIndex(pAgent->GetPosition()) };

	m_Cells[index].agents.push_back(pAgent);

}

void CellSpace::UpdateAgentCell(SteeringAgent* pAgent, Elite::Vector2 oldPos)
{
	Elite::Vector2 newPos{ pAgent->GetPosition() };

	int oldIndex{ PositionToIndex(oldPos) };
	int newIndex{ PositionToIndex(newPos) };

	if (oldIndex == newIndex)
		return;


	m_Cells[oldIndex].agents.remove(pAgent);
	m_Cells[newIndex].agents.push_back(pAgent);


}

void CellSpace::RegisterNeighbors(SteeringAgent* pAgent, float queryRadius)
{
	m_NrOfNeighbors = 0;

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	Elite::Vector2 boundingBoxBL{ agentPos -
		Elite::Vector2{queryRadius, queryRadius} };

	Elite::Vector2 boundingBoxTR{ agentPos +
		Elite::Vector2{queryRadius, queryRadius} };

	int minCol{ PositionToColIndex(boundingBoxBL) };
	int maxCol{ PositionToColIndex(boundingBoxTR) };
	int minRow{ PositionToRowIndex(boundingBoxBL) };
	int maxRow{ PositionToRowIndex(boundingBoxTR) };



	for (int row{ minRow }; row <= maxRow; ++row)
	{
		for (int col{ minCol }; col <= maxCol; ++col)
		{
			for (auto& cellAgent : m_Cells[(col * m_NrOfRows) + row].agents)
			{
				if (cellAgent != pAgent)
				{
					Elite::Vector2 distance{ cellAgent->GetPosition() - pAgent->GetPosition() };
					if (distance.MagnitudeSquared() <= queryRadius * queryRadius)
					{

						m_Neighbors[m_NrOfNeighbors] = cellAgent;
						++m_NrOfNeighbors;
					}
				}

			}

		}

	}

	int deleteThis{};

}




void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells(float queryRadius, SteeringAgent* pDebugAgent) const
{
	Elite::Rect neighborhoodArea{ pDebugAgent->GetPosition() + Elite::Vector2{-queryRadius, -queryRadius},
		queryRadius * 2.f,
		queryRadius * 2.f
	};

	for (int i{}; i < m_NrOfRows; i++)
	{
		for (int j{}; j < m_NrOfCols; j++)
		{
			Cell cell{ m_Cells[i * m_NrOfRows + j] };
			Elite::Color cellColor{ 1,0,0 };

			if (Elite::IsOverlapping(neighborhoodArea, cell.boundingBox))
			{
				cellColor = { 0, 1, 0 };
			}

			DEBUGRENDERER2D->DrawPolygon(
				&cell.GetRectPoints()[0],
				4,
				cellColor,
				0.8f
			);
			Elite::Vector2 NumberPos{
				cell.boundingBox.bottomLeft.x + cell.boundingBox.width * 0.5f,
				cell.boundingBox.bottomLeft.y + cell.boundingBox.height * .75f,
			};
			DEBUGRENDERER2D->DrawString(
				NumberPos,
				std::to_string(cell.agents.size()).c_str());

		}
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{

	int cellCol{ static_cast<int>(pos.x / m_CellHeight) };
	int cellRow{ static_cast<int>(pos.y / m_CellWidth) };

	if (cellCol == m_NrOfCols)
		cellCol -= 1;
	if (cellRow == m_NrOfRows)
		cellRow -= 1;

	int index{ (cellCol * m_NrOfRows) + cellRow };
	return index;
}

int CellSpace::PositionToColIndex(const Elite::Vector2 pos) const
{
	int cellCol{ static_cast<int>(pos.x / m_CellHeight) };

	if (cellCol >= m_NrOfCols) cellCol = m_NrOfCols - 1;
	if (cellCol < 0) cellCol = 0;

	return cellCol;
}

int CellSpace::PositionToRowIndex(const Elite::Vector2 pos) const
{

	int cellRow{ static_cast<int>(pos.y / m_CellWidth) };

	if (cellRow >= m_NrOfRows) cellRow = m_NrOfRows - 1;
	if (cellRow < 0) cellRow = 0;


	return cellRow;
}
