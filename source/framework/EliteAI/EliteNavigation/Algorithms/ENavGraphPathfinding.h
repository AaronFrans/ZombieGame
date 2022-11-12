#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

#define USE_PORTALS

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(Vector2 startPos, Vector2 endPos, NavGraph* pNavGraph, std::vector<Vector2>& debugNodePositions, std::vector<Portal>& debugPortals)
		{
			//Create the path to return
			std::vector<Vector2> finalPath{};
			debugNodePositions = finalPath;

			//Get the start and endTriangle
			auto start{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos) };
			auto end{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos) };

			if (!start || !end)
				return finalPath;

			if (start == end)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}


			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph
			auto graphCopy = std::static_pointer_cast<NavGraph>(pNavGraph->Clone());


			//Create extra node for the Start Node (Agent's position
			auto startNode{ new NavGraphNode(graphCopy->GetNextFreeNodeIndex(), -1, startPos) };
			graphCopy->AddNode(startNode);
			for (auto& lineIndex : start->metaData.IndexLines)
			{
				int nodeIndex = graphCopy->GetNodeIdxFromLineIdx(lineIndex);
				if (nodeIndex == invalid_node_index) continue;
				auto lineNode = graphCopy->GetNode(nodeIndex);
				graphCopy->AddConnection(new GraphConnection2D{ nodeIndex, startNode->GetIndex(), Distance(lineNode->GetPosition(), startNode->GetPosition()) });
			}

			//Create extra node for the endNode
			auto endNode{ new NavGraphNode(graphCopy->GetNextFreeNodeIndex(), -1, endPos) };
			graphCopy->AddNode(endNode);
			for (auto& lineIndex : end->metaData.IndexLines)
			{
				int nodeIndex = graphCopy->GetNodeIdxFromLineIdx(lineIndex);
				if (nodeIndex == invalid_node_index) continue;
				auto lineNode = graphCopy->GetNode(nodeIndex);
				graphCopy->AddConnection(new GraphConnection2D{ endNode->GetIndex(), nodeIndex, Distance(lineNode->GetPosition(), endNode->GetPosition()) });
			}
			//Run A star on new graph
			auto pathfinder = AStar<NavGraphNode, GraphConnection2D>(graphCopy.get(), Elite::HeuristicFunctions::Chebyshev);

			auto nodes = pathfinder.FindPath(startNode, endNode);

			//OPTIONAL BUT ADVICED: Debug Visualisation

			for (auto& node : nodes)
				debugNodePositions.push_back(node->GetPosition());
#ifdef USE_PORTALS
			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			debugPortals = SSFA::FindPortals(nodes, pNavGraph->GetNavMeshPolygon());

			

			finalPath = SSFA::OptimizePortals(debugPortals);
			//debugNodePositions = finalPath;
#else

			for (auto& node : nodes)
			finalPath.push_back(node->GetPosition());
#endif // USE_PORTALS




			return finalPath;
		}
	};
}
