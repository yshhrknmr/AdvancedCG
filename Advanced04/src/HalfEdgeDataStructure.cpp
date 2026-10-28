#include "HalfEdgeDataStructure.h"
#include <glm/gtx/string_cast.hpp>
#include <iostream>
#include <unordered_map>
#include "fast_obj.h"

namespace HalfEdge {

	using namespace std;
	using namespace glm;

	int Vertex::countValence() const
	{
		int valence = 0;
		bool onBoundary = false;
		auto he = pHalfEdge;

		do
		{
			++valence;

			if (he->pPair == nullptr)
			{
				onBoundary = true;
				break;
			}

			he = he->pPair->pNext;
		} while (he != pHalfEdge);

		if (onBoundary)
		{
			he = pHalfEdge->pPrev;

			do
			{
				++valence;

				if (he->pPair == nullptr)
				{
					break;
				}

				he = he->pPair->pPrev;

			} while (he != pHalfEdge);
		}

		return valence;
	}

	bool Vertex::onBoundary() const
	{
		auto he = pHalfEdge;

		do
		{
			if (he->pPair == nullptr)
			{
				return true;
			}

			he = he->pPair->pNext;
		} while (he != pHalfEdge);

		return false;
	}

	vec3 Face::calcCentroidPosition() const
	{
		int nFaceVertices = 0;
		vec3 positionSum(0.f);
		auto he = pHalfEdge;

		do
		{
			positionSum += he->pStartVertex->position;
			nFaceVertices++;
			he = he->pNext;
		} while (he != pHalfEdge);

		return positionSum / (float)nFaceVertices;
	}

	void Mesh::deepCopy(const Mesh& rhs)
	{
		if (this == &rhs)
			return;

		clear();

		const int nVertices = (int)rhs.vertices.size();
		vertices.resize(nVertices);
		for (int vi = 0; vi < nVertices; ++vi)
			vertices[vi] = new Vertex(vi);

		const int nHalfEdges = (int)rhs.halfEdges.size();
		halfEdges.resize(nHalfEdges);
		for (int hi = 0; hi < nHalfEdges; ++hi)
			halfEdges[hi] = new HalfEdge(hi);

		const int nFaces = (int)rhs.faces.size();
		faces.resize(nFaces);
		for (int fi = 0; fi < nFaces; ++fi)
			faces[fi] = new Face(fi);

		for (int vi = 0; vi < nVertices; ++vi)
		{
			vertices[vi]->pHalfEdge = halfEdges[rhs.vertices[vi]->pHalfEdge->id];
			vertices[vi]->position = rhs.vertices[vi]->position;
		}

		for (int fi = 0; fi < nFaces; ++fi)
			faces[fi]->pHalfEdge = halfEdges[rhs.faces[fi]->pHalfEdge->id];

		for (int hi = 0; hi < nHalfEdges; ++hi)
		{
			HalfEdge* he = halfEdges[hi];
			he->pStartVertex = vertices[rhs.halfEdges[hi]->pStartVertex->id];
			if (rhs.halfEdges[hi]->pPair != nullptr)
				he->pPair = halfEdges[rhs.halfEdges[hi]->pPair->id];
			he->pNext = halfEdges[rhs.halfEdges[hi]->pNext->id];
			he->pPrev = halfEdges[rhs.halfEdges[hi]->pPrev->id];
			if (rhs.halfEdges[hi]->pFace != nullptr)
				he->pFace = faces[rhs.halfEdges[hi]->pFace->id];
		}
	}

	void Mesh::build(const PolygonMesh& mesh, bool checkConsistency /*= false*/)
	{
		if (mesh.getVertices().empty() || mesh.getFaceIndices().empty())
		{
			cerr << __FUNCTION__ << ": geometry not defined" << endl;
			return;
		}

		// prepare vertices

		const int nVertices = mesh.getNumVertices();
		clearVertices();
		vertices.resize(nVertices);

		for (int vi = 0; vi < nVertices; ++vi)
		{
			vertices[vi] = new Vertex(vi);
			vertices[vi]->position = mesh.getVertices()[vi];
		}

		// prepare faces and half-edges

		const int nFaces = (int)mesh.getFaceIndices().size();
		clearFaces();
		clearHalfEdges();
		faces.resize(nFaces);

		unordered_map<long long, HalfEdge*> halfEdgeDict;

		for (int fi = 0; fi < nFaces; ++fi)
		{
			Face* newFace = new Face(fi);
			faces[fi] = newFace;

			vector<HalfEdge*> sameFaceHalfEdges;

			for (const auto t : mesh.getFaceIndices()[fi].indices)
			{
				HalfEdge* he = new HalfEdge(halfEdges.size());
				halfEdges.push_back(he);
				sameFaceHalfEdges.push_back(he);

				Vertex* pStartVertex = vertices[t.vertexIdx];
				he->pStartVertex = pStartVertex;
				if (pStartVertex->pHalfEdge == nullptr)
					pStartVertex->pHalfEdge = he;

				he->pFace = newFace;
			}

			newFace->pHalfEdge = sameFaceHalfEdges.front();

			const int nSameFaceHE = (int)sameFaceHalfEdges.size();
			for (int si = 0; si < nSameFaceHE; ++si)
			{
				HalfEdge* curr = sameFaceHalfEdges[si];
				curr->pPrev = sameFaceHalfEdges[(si - 1 + nSameFaceHE) % nSameFaceHE];
				curr->pNext = sameFaceHalfEdges[(si + 1) % nSameFaceHE];

				const long long startVertexID = curr->pStartVertex->id;
				const long long endVertexID = curr->pNext->pStartVertex->id;
				const long long hashKey = startVertexID * (long long)nVertices + endVertexID;
				halfEdgeDict[hashKey] = curr;
			}
		}

		// setting pairs of half-edges

		for (auto pHalfEdge : halfEdges)
		{
			const long long startVertexID = pHalfEdge->pStartVertex->id;
			const long long endVertexID = pHalfEdge->pNext->pStartVertex->id;
			const long long pairHashKey = endVertexID * (long long)nVertices + startVertexID;
			auto itr = halfEdgeDict.find(pairHashKey);

			if (itr != halfEdgeDict.end())
			{
				HalfEdge* pPair = itr->second;
				pHalfEdge->pPair = pPair;
			}
		}

		if (checkConsistency)
			checkDataConsistency();
	}

	void Mesh::restore(PolygonMesh& mesh) const
	{
		// restore positions

		const int nPositions = (int)vertices.size();
		vector<glm::vec3> tmpVertices(nPositions);
		for (int pi = 0; pi < nPositions; ++pi)
			tmpVertices[pi] = vertices[pi]->position;

		mesh.setVertices(tmpVertices);

		// restore faces

		const int nFaces = (int)faces.size();
		vector<FaceIndices> tmpFaces(nFaces);
		for (int fi = 0; fi < nFaces; ++fi)
		{
			auto& targetIndices = tmpFaces[fi].indices;
			targetIndices.clear();

			HalfEdge* startHE = faces[fi]->pHalfEdge;
			HalfEdge* he = startHE;
			do
			{
				const int vertexIdx = he->pStartVertex->id;
				targetIndices.emplace_back(vertexIdx, -1, vertexIdx);

				he = he->pNext;
			} while (he != startHE);
		}

		mesh.setFaces(tmpFaces);
	}

	namespace
	{
		// Mesh::checkDataConsistency() prints at most this number of warnings so that a broken mesh does not flood the console
		const int MaxPrintedWarnings = 20;

		// true if p is stored in the array at the index given by its id, i.e., p is an element of this mesh
		template <typename T>
		bool isElementOf(const T* p, const vector<T*>& array)
		{
			return p->id >= 0 && p->id < (long)array.size() && array[p->id] == p;
		}
	}

	void Mesh::checkDataConsistency() const
	{
		int nWarnings = 0;
		auto printWarning = [&nWarnings]() { return nWarnings++ < MaxPrintedWarnings; };

		const int nVertices = (int)vertices.size();
		const int nHalfEdges = (int)halfEdges.size();
		const int nFaces = (int)faces.size();

		vector<bool> isInconsistentVertex(nVertices, false);
		vector<bool> isInconsistentHalfEdge(nHalfEdges, false);
		vector<bool> isInconsistentFace(nFaces, false);

		// Part 1: each pointer must point to an element of this mesh (only pPair may be null, on the boundary)

		for (int vi = 0; vi < nVertices; ++vi)
		{
			auto v = vertices[vi];
			bool isInconsistent = false;

			if (v == nullptr)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: Vertex[" << vi << "] is null" << endl;
				isInconsistent = true;
			}
			else if (v->id != vi)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: Vertex[" << vi << "] has a wrong id (" << v->id << ")" << endl;
				isInconsistent = true;
			}
			else if (v->pHalfEdge == nullptr)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*v) << "'s pHalfEdge is null" << endl;
				isInconsistent = true;
			}
			else if (!isElementOf(v->pHalfEdge, halfEdges))
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*v) << "'s pHalfEdge has an invalid address" << endl;
				isInconsistent = true;
			}

			isInconsistentVertex[vi] = isInconsistent;
		}

		for (int fi = 0; fi < nFaces; ++fi)
		{
			auto f = faces[fi];
			bool isInconsistent = false;

			if (f == nullptr)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: Face[" << fi << "] is null" << endl;
				isInconsistent = true;
			}
			else if (f->id != fi)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: Face[" << fi << "] has a wrong id (" << f->id << ")" << endl;
				isInconsistent = true;
			}
			else if (f->pHalfEdge == nullptr)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*f) << "'s pHalfEdge is null" << endl;
				isInconsistent = true;
			}
			else if (!isElementOf(f->pHalfEdge, halfEdges))
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*f) << "'s pHalfEdge has an invalid address" << endl;
				isInconsistent = true;
			}

			isInconsistentFace[fi] = isInconsistent;
		}

		for (int hi = 0; hi < nHalfEdges; ++hi)
		{
			auto he = halfEdges[hi];
			bool isInconsistent = false;

			if (he == nullptr)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: HalfEdge[" << hi << "] is null" << endl;
				isInconsistentHalfEdge[hi] = true;
				continue;
			}

			if (he->id != hi)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: HalfEdge[" << hi << "] has a wrong id (" << he->id << ")" << endl;
				isInconsistent = true;
			}

			if (he->pStartVertex == nullptr)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pStartVertex is null" << endl;
				isInconsistent = true;
			}
			else if (!isElementOf(he->pStartVertex, vertices))
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pStartVertex has an invalid address" << endl;
				isInconsistent = true;
			}

			if (he->pNext == nullptr)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pNext is null" << endl;
				isInconsistent = true;
			}
			else if (!isElementOf(he->pNext, halfEdges))
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pNext has an invalid address" << endl;
				isInconsistent = true;
			}

			if (he->pPrev == nullptr)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pPrev is null" << endl;
				isInconsistent = true;
			}
			else if (!isElementOf(he->pPrev, halfEdges))
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pPrev has an invalid address" << endl;
				isInconsistent = true;
			}

			if (he->pPair != nullptr && !isElementOf(he->pPair, halfEdges))
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pPair has an invalid address" << endl;
				isInconsistent = true;
			}

			if (he->pFace == nullptr)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pFace is null" << endl;
				isInconsistent = true;
			}
			else if (!isElementOf(he->pFace, faces))
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pFace has an invalid address" << endl;
				isInconsistent = true;
			}

			isInconsistentHalfEdge[hi] = isInconsistent;
		}

		// Part 2: the pointers must be consistent with each other
		// (only the elements that passed Part 1 are checked, so that broken pointers are never followed)

		vector<bool> hasValidPointers(nHalfEdges);
		for (int hi = 0; hi < nHalfEdges; ++hi)
			hasValidPointers[hi] = !isInconsistentHalfEdge[hi];

		// a vertex's pHalfEdge must start from the vertex

		for (int vi = 0; vi < nVertices; ++vi)
		{
			if (isInconsistentVertex[vi])
				continue;

			auto v = vertices[vi];

			if (v->pHalfEdge->pStartVertex != v)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*v) << "'s pHalfEdge does not start from the vertex" << endl;
				isInconsistentVertex[vi] = true;
			}
		}

		// pNext, pPrev, and pPair of a half-edge must point back to it, and the pair must connect the same two vertices in the opposite direction

		auto edgeKey = [nVertices](long long startVertexID, long long endVertexID) { return startVertexID * (long long)nVertices + endVertexID; };
		unordered_map<long long, int> halfEdgeDict;	// key: edgeKey(start vertex id, end vertex id), value: half-edge id
		halfEdgeDict.reserve(nHalfEdges);

		for (int hi = 0; hi < nHalfEdges; ++hi)
		{
			if (!hasValidPointers[hi])
				continue;

			auto he = halfEdges[hi];
			bool isInconsistent = false;

			if (he->pNext->pPrev != he)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pNext->pPrev is not the half-edge itself" << endl;
				isInconsistent = true;
			}

			if (he->pPrev->pNext != he)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pPrev->pNext is not the half-edge itself" << endl;
				isInconsistent = true;
			}

			if (he->pNext->pFace != he->pFace)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << " and its pNext belong to different faces" << endl;
				isInconsistent = true;
			}

			if (he->pPair != nullptr)
			{
				if (he->pPair->pPair != he)
				{
					if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pPair->pPair is not the half-edge itself" << endl;
					isInconsistent = true;
				}
				else if (hasValidPointers[he->pPair->id] &&
					(he->pPair->pStartVertex != he->pNext->pStartVertex || he->pPair->pNext->pStartVertex != he->pStartVertex))
				{
					if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << " and its pPair " << (*he->pPair) << " do not connect the same two vertices in opposite directions" << endl;
					isInconsistent = true;
				}
			}

			if (hasValidPointers[he->pNext->id])
			{
				if (he->pNext->pStartVertex == he->pStartVertex)
				{
					if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << " starts and ends at the same vertex" << endl;
					isInconsistent = true;
				}

				auto result = halfEdgeDict.emplace(edgeKey(he->pStartVertex->id, he->pNext->pStartVertex->id), hi);
				if (!result.second)
				{
					if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << " and " << (*halfEdges[result.first->second]) << " have the same start and end vertices" << endl;
					isInconsistent = true;
				}
			}

			if (isInconsistent)
				isInconsistentHalfEdge[hi] = true;
		}

		// a half-edge must have a pair if there is a half-edge in the opposite direction

		for (int hi = 0; hi < nHalfEdges; ++hi)
		{
			auto he = halfEdges[hi];

			if (!hasValidPointers[hi] || !hasValidPointers[he->pNext->id] || he->pPair != nullptr)
				continue;

			auto iter = halfEdgeDict.find(edgeKey(he->pNext->pStartVertex->id, he->pStartVertex->id));
			if (iter != halfEdgeDict.end())
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << "'s pPair is null although " << (*halfEdges[iter->second]) << " runs in the opposite direction" << endl;
				isInconsistentHalfEdge[hi] = true;
			}
		}

		// following pNext from a face's pHalfEdge must come back to it, and each half-edge must be in the loop of exactly one face

		vector<int> loopFaceID(nHalfEdges, -1);	// id of the face whose loop contains each half-edge

		for (int fi = 0; fi < nFaces; ++fi)
		{
			if (isInconsistentFace[fi])
				continue;

			auto f = faces[fi];
			auto he = f->pHalfEdge;

			do
			{
				if (!hasValidPointers[he->id])	// already reported in Part 1
				{
					isInconsistentFace[fi] = true;
					break;
				}

				if (loopFaceID[he->id] != -1)
				{
					if (loopFaceID[he->id] == fi)
					{
						if (printWarning()) cerr << __FUNCTION__ << ": Warning: following pNext from " << (*f) << "'s pHalfEdge does not come back to it" << endl;
					}
					else
					{
						if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << " is in the loops of both " << (*faces[loopFaceID[he->id]]) << " and " << (*f) << endl;
					}
					isInconsistentFace[fi] = true;
					isInconsistentHalfEdge[he->id] = true;
					break;
				}

				loopFaceID[he->id] = fi;

				if (he->pFace != f)
				{
					if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*he) << " is in the loop of " << (*f) << " but its pFace is " << (*he->pFace) << endl;
					isInconsistentFace[fi] = true;
					isInconsistentHalfEdge[he->id] = true;
				}

				he = he->pNext;
			} while (he != f->pHalfEdge);
		}

		for (int hi = 0; hi < nHalfEdges; ++hi)
		{
			if (hasValidPointers[hi] && loopFaceID[hi] == -1)
			{
				if (printWarning()) cerr << __FUNCTION__ << ": Warning: " << (*halfEdges[hi]) << " is not in the loop of any face" << endl;
				isInconsistentHalfEdge[hi] = true;
			}
		}

		// summary

		int nInconsistentVertices = 0, nInconsistentHalfEdges = 0, nInconsistentFaces = 0;
		for (bool b : isInconsistentVertex) nInconsistentVertices += b;
		for (bool b : isInconsistentHalfEdge) nInconsistentHalfEdges += b;
		for (bool b : isInconsistentFace) nInconsistentFaces += b;

		if (nWarnings > MaxPrintedWarnings)
			cerr << __FUNCTION__ << ": " << (nWarnings - MaxPrintedWarnings) << " more warnings are not printed" << endl;

		cerr << __FUNCTION__ << ": inconsistent # verts = " << nInconsistentVertices << "/" << vertices.size()
			<< ", # half edges = " << nInconsistentHalfEdges << "/" << halfEdges.size()
			<< ", # faces = " << nInconsistentFaces << "/" << faces.size() << endl;
	}

	ostream& operator<<(ostream& stream, const Vertex& v)
	{
		stream << "Vertex[id=" << v.id << ",pos=" << to_string(v.position) << "]";
		return stream;
	}

	ostream& operator<<(ostream& stream, const Face& f)
	{
		stream << "Face[id=" << f.id << "]";
		return stream;
	}

	ostream& operator<<(ostream& stream, const HalfEdge& he)
	{
		// -1 for a missing vertex, so that a broken half-edge can also be printed
		const long startVertexID = (he.pStartVertex != nullptr) ? he.pStartVertex->id : -1;
		const long endVertexID = (he.pNext != nullptr && he.pNext->pStartVertex != nullptr) ? he.pNext->pStartVertex->id : -1;
		stream << "HalfEdge[id=" << he.id << ",startVertex=" << startVertexID << ",endVertex=" << endVertexID << "]";
		return stream;
	}

}

void HalfEdge::Helper::SetPair(HalfEdge* he1, HalfEdge* he2)
{
	he1->pPair = he2;
	he2->pPair = he1;
}

void HalfEdge::Helper::SetPrevNext(HalfEdge* prevHE, HalfEdge* nextHE)
{
	prevHE->pNext = nextHE;
	nextHE->pPrev = prevHE;
}
