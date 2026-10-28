#include "LoopSubdivision.h"
#include <unordered_map>

using namespace std;
using namespace glm;

void LoopSubdivision::subdivide(PolygonMesh& mesh, int nSubdiv)
{
	if (mesh.getVertices().empty() || mesh.getFaceIndices().empty())
	{
		std::cerr << __FUNCTION__ << ": mesh not ready" << std::endl;
		return;
	}

	mesh.triangulate();

	HalfEdge::Mesh _mesh;
	_mesh.build(mesh);

	for (int iter = 0; iter < nSubdiv; ++iter)
		_mesh = apply(_mesh);

	_mesh.restore(mesh);
	mesh.calcVertexNormals();
}

HalfEdge::Mesh LoopSubdivision::apply(HalfEdge::Mesh& mesh)
{
	return mesh;	// TODO: delete this line

	HalfEdge::Mesh newMesh;

	const int nOldVertices = (int)mesh.vertices.size();
	const int nOldFaces = (int)mesh.faces.size();
	const int nOldHalfEdges = (int)mesh.halfEdges.size();

	// Step 0: allocate memory for even (i.e., old) vertices

	newMesh.vertices.reserve(nOldVertices + nOldHalfEdges);

	for (int vi = 0; vi < nOldVertices; ++vi)
		newMesh.addVertex();

	// Step 1: create odd (i.e., new) vertices by splitting half edges
	// NOTE: one odd vertex is added for each edge; except on the boundary, it is NOT at the midpoint of the edge
	//       (in general, it does not even lie on the edge)

	unordered_map<long, HalfEdge::Vertex*> newEdgeVertexDict;	// key: id of an old half-edge, value: odd vertex for its edge
	unordered_map<long, pair<HalfEdge::HalfEdge*, HalfEdge::HalfEdge*>> newHalfEdgeDict;

	for (int hi = 0; hi < nOldHalfEdges; ++hi)
	{
		auto oldHE = mesh.halfEdges[hi];

		vec3 startVertexPosition = mesh.vertices[oldHE->pStartVertex->id]->position;
		vec3 endVertexPosition = mesh.vertices[oldHE->getEndVertex()->id]->position;

		HalfEdge::Vertex* edgeVertex = nullptr;

		if (oldHE->pPair == nullptr)	// on boundary
		{
			edgeVertex = newMesh.addVertex();
			// TODO: calculate the position of the odd vertex for this boundary edge
			// HINT: on the boundary, it is the midpoint of the edge (weights 1/2 and 1/2)
		}
		else
		{
			auto edgeVertexIter = newEdgeVertexDict.find(oldHE->pPair->id); // check if pair has been already registered

			if (edgeVertexIter == newEdgeVertexDict.end())
			{
				edgeVertex = newMesh.addVertex();
				// TODO: calculate the position of the odd vertex for this edge
				// HINT: the weights are 3/8 for each end vertex of this edge and 1/8 for each of the two vertices opposite to this edge
				//       (one in each of the two triangles sharing this edge)
			}
			else
			{
				edgeVertex = edgeVertexIter->second;
			}
		}

		newEdgeVertexDict[oldHE->id] = edgeVertex; // used in Step 3

		auto formerHE = newMesh.addHalfEdge();
		auto latterHE = newMesh.addHalfEdge();

		auto evenStartVertex = newMesh.vertices[oldHE->pStartVertex->id];
		auto evenEndVertex = newMesh.vertices[oldHE->getEndVertex()->id];

		formerHE->pStartVertex = evenStartVertex;
		if (evenStartVertex->pHalfEdge == nullptr)
			evenStartVertex->pHalfEdge = formerHE;

		latterHE->pStartVertex = edgeVertex;
		if (edgeVertex->pHalfEdge == nullptr)
			edgeVertex->pHalfEdge = latterHE;

		newHalfEdgeDict[hi] = make_pair(formerHE, latterHE);

		// register pairs

		if (oldHE->pPair != nullptr)
		{
			auto iter = newHalfEdgeDict.find(oldHE->pPair->id);

			if (iter != newHalfEdgeDict.end())
			{
				HalfEdge::HalfEdge* pairFormerHE = iter->second.first;
				HalfEdge::HalfEdge* pairLatterHE = iter->second.second;

				HalfEdge::Helper::SetPair(pairFormerHE, latterHE);
				HalfEdge::Helper::SetPair(pairLatterHE, formerHE);
			}
		}
	}

	// Step 2: update even (i.e., old) vertices

	for (int vi = 0; vi < nOldVertices; ++vi)
	{
		auto newVertex = newMesh.vertices[vi];
		const auto oldVertex = mesh.vertices[vi];
		const auto oldVertexPosition = oldVertex->position;

		// TODO: calculate the new vertex position
		// c.f., HalfEdge::Vertex::countValence() in HalfEdgeDataStructure.cpp
		// HINT: if HalfEdge::Vertex::onBoundary() is true, use only the vertex itself (weight 3/4)
		//       and its two neighbors on the boundary (weight 1/8 each)
	}

	// Step 3: create new faces

	for (int fi = 0; fi < nOldFaces; ++fi)
	{
		auto oldFace = mesh.faces[fi];

		// TODO: update the half-edge data structure within each old face
		// HINT: the number of new faces within each old face is always 4 in the case of Loop subdivision,
		//       so you can write down all the steps without using a "for" or "while" loop 
	}

	cerr << __FUNCTION__ << ": check data consistency" << endl;
	newMesh.checkDataConsistency();

	return newMesh;
}
