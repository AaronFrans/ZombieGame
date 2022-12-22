#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon;
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	//1. Go over all the edges of the navigationmesh and create nodes
	for (auto& line : m_pNavMeshPolygon->GetLines())
	{
		auto triangles{ m_pNavMeshPolygon->GetTrianglesFromLineIndex(line->index) };
		if (triangles.size() > 1)
		{
			Vector2 center{ (line->p1.x + line->p2.x) * 0.5f, (line->p1.y + line->p2.y) * 0.5f };
			NavGraphNode* node{ new NavGraphNode(GetNextFreeNodeIndex(), line->index, center)};
			AddNode(node);
		}
	}



	//2. Create connections now that every node is created
	for (auto& triangle : m_pNavMeshPolygon->GetTriangles())
	{
		std::vector<int> indexes{};


		for (auto& lineIndex : triangle->metaData.IndexLines)
		{
			int i{ GetNodeIdxFromLineIdx(lineIndex) };
			if (i != invalid_node_index)
				indexes.push_back(i);
		}

		switch (indexes.size())
		{
		case 2:
			AddConnection(new GraphConnection2D(indexes[0], indexes[1]));
			break;
		case 3:
			AddConnection(new GraphConnection2D(indexes[0], indexes[1]));
			AddConnection(new GraphConnection2D(indexes[1], indexes[2]));
			AddConnection(new GraphConnection2D(indexes[2], indexes[0]));
			break;
		}
	}

	//3. Set the connections cost to the actual distance
	SetConnectionCostsToDistance();

}

