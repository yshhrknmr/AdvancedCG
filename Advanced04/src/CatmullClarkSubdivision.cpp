#include "CatmullClarkSubdivision.h"
#include <unordered_map>

using namespace std;
using namespace glm;

void CatmullClarkSubdivision::subdivide(PolygonMesh& mesh, int nSubdiv)
{
	if (mesh.getVertices().empty() || mesh.getFaceIndices().empty())
	{
		std::cerr << __FUNCTION__ << ": mesh not ready" << std::endl;
		return;
	}

	HalfEdge::Mesh _mesh;
	_mesh.build(mesh);

	for (int iter = 0; iter < nSubdiv; ++iter)
		_mesh = apply(_mesh);

	_mesh.restore(mesh);
	mesh.calcVertexNormals();
}

HalfEdge::Mesh CatmullClarkSubdivision::apply(HalfEdge::Mesh& mesh)
{
	return mesh;	// TODO: delete this line

	HalfEdge::Mesh newMesh;

	const int nOldVertices = (int)mesh.vertices.size();
	const int nOldFaces = (int)mesh.faces.size();
	const int nOldHalfEdges = (int)mesh.halfEdges.size();

	// Step 0: allocate memory for even (i.e., old) vertices

	newMesh.vertices.reserve(nOldFaces + nOldVertices + nOldHalfEdges);

	for (int vi = 0; vi < nOldVertices; ++vi)
		newMesh.addVertex();

	// Step 1: generate face centroids

	for (int fi = 0; fi < nOldFaces; ++fi)
	{
		auto oldFace = mesh.faces[fi];
		auto newFaceCentroid = newMesh.addVertex();
		// TODO: calculate the positions of face centroids
	}

	// Step 2: create odd (i.e., new) vertices by splitting half edges
	// NOTE: one odd vertex is added for each edge; except on the boundary, it is NOT at the midpoint of the edge
	//       (in general, it does not even lie on the edge)

	unordered_map<long, HalfEdge::Vertex*> newEdgeVertexDict;	// key: id of an old half-edge, value: odd vertex for its edge
	unordered_map<long, pair<HalfEdge::HalfEdge*, HalfEdge::HalfEdge*>> newHalfEdgeDict;

	for (int hi = 0; hi < nOldHalfEdges; ++hi)
	{
		auto oldHE = mesh.halfEdges[hi];

		vec3 startVertexPosition = oldHE->getStartVertex()->position;
		vec3 endVertexPosition = oldHE->getEndVertex()->position;

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
				// HINT: it is the average of the two end vertices of this edge and the centroids of the two faces sharing this edge;
				//       the centroid of an old face f was created in Step 1 as newMesh.vertices[nOldVertices + f->id]
			}
			else
			{
				edgeVertex = edgeVertexIter->second;
			}
		}

		newEdgeVertexDict[oldHE->id] = edgeVertex;

		auto formerHE = newMesh.addHalfEdge();
		auto latterHE = newMesh.addHalfEdge();

		auto evenStartVertex = newMesh.vertices[oldHE->pStartVertex->id];
		auto evenEndVertex = newMesh.vertices[oldHE->pNext->pStartVertex->id];

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

	// Step 3: update even (i.e., old) vertex positions

	for (int vi = 0; vi < nOldVertices; ++vi)
	{
		auto newVertex = newMesh.vertices[vi];
		const auto oldVertex = mesh.vertices[vi];
		const auto oldVertexPosition = oldVertex->position;

		// TODO: calculate the new vertex position
		// c.f., HalfEdge::Vertex::countValence() in HalfEdgeDataStructure.cpp
		// HINT: in the formula for polygon meshes on the lecture slides, R is the average of the odd vertices
		//       (Step 2) for the edges connected to the vertex, and S is the average of the centroids (Step 1)
		//       of the faces around the vertex; if HalfEdge::Vertex::onBoundary() is true, use only the vertex
		//       itself (weight 3/4) and its two neighbors on the boundary (weight 1/8 each)
	}

	// Step 4: set up new faces

	for (int fi = 0; fi < nOldFaces; ++fi)
	{
		auto oldFace = mesh.faces[fi];
		auto centroidVertex = newMesh.vertices[oldFace->id + nOldVertices];

		// TODO: update the half-edge data structure within each old face
		// HINT: use the following std::vector to store temporary data and process step by step
		//vector<HalfEdge::HalfEdge*> tmpToCentroidHalfEdges;
		//vector<HalfEdge::Face*> tmpNewFaces;
	}

	cerr << __FUNCTION__ << ": check data consistency" << endl;
	newMesh.checkDataConsistency();

	return newMesh;
}
