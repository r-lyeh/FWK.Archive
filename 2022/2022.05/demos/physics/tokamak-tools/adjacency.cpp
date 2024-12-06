// adjacency.cpp : Defines the entry point for the console application.
//

/*
 *	For convex hull processing
 */

#include "tokamak.h"
#include <stdio.h>
#include <windows.h>

extern "C"
{
#include "qhull_a.h"
}

const int INIT_ARRAY_SIZE = 100;

struct ConvexFace
{
	int neighbourFaces;
	int neighbourVerts;
	int neighbourEdges;
};
struct ConvexVert
{
	int neighbourEdges;
};
struct ConvexEdge
{
	neByte f1;
	neByte f2;
	neByte v1;
	neByte v2;
};

template <class T> class FArray
{
public:
	FArray() : nextFree(0), size(INIT_ARRAY_SIZE) 
	{
		dataArray =  new T[INIT_ARRAY_SIZE];

		keyArray = new u32[INIT_ARRAY_SIZE];
	}

	T & operator [] (int i)
	{ 
		assert(i < nextFree); return dataArray[i];
	}

	void Add(u32 key, const T & item) 
	{
		for (int i = 0; i < nextFree; i++)
		{
			if (keyArray[i] == key)
			{
				return;
			}
		}
		if (nextFree == size)
			Grow();
		
		keyArray[nextFree] = key;

		dataArray[nextFree++] = item;
	}
	int GetIndex(u32 key)
	{
		for (int i = 0; i < nextFree; i++)
		{
			if (keyArray[i] == key)
			{
				return i;
			}
		}
		return -1;
	}

	int Get(u32 key, T & item)
	{
		for (int i = 0; i < nextFree; i++)
		{
			if (keyArray[i] == key)
			{
				item = dataArray[i];

				return i;
			}
		}
		return -1;
	}
	int GetCount() {
		return nextFree;
	}

public:
	T * dataArray;
	u32 * keyArray;
	int nextFree;
	int size;

private:
	void Grow() {
		T * newData = new T[size * 2];

		u32 * newKey = new u32[size * 2];

		for (int i = 0; i < nextFree; i++)
		{
			newData[i] = dataArray[i];

			newKey[i] = keyArray[i];
		}
		size = size * 2;

		delete dataArray;

		delete keyArray;

		dataArray = newData;

		keyArray = newKey;
	}
};

class neighbourItem
{
public:
	s32 neighbour;
	s32 nextNeighbour;

	neighbourItem() : neighbour (-1), nextNeighbour(-1) {}
};

#define BUF_SIZE 1024

u32 MakeEdgeKey(neByte v1, neByte v2)
{
	if (v1 < v2)
	{
		return (v1 << 16 | v2);
	}
	else
	{
		return (v2 << 16 | v1);
	}
}

int main(int argc, char* argv[])
{
	if (argc < 3)
	{
		fprintf(stdout, "ADJACENCY input_file output_file [qhull_options]\n");

		return 1;
	}
	FILE * ff = fopen(argv[1], "r");

	fseek( ff, 0L, SEEK_SET );

	if (ff == NULL)
	{
		fprintf(stdout, "cannot open file %s.\n", argv[1]);

		return 1;
	}

	int vertexCount = 0;
	
	fscanf(ff, "%ld", &vertexCount);

	if (vertexCount <= 0)
	{
		return 1;
	}

	double * vertArray = new double[vertexCount * 3];

	s32 i;

	double * pdouble = vertArray;

	for (i = 0; i < vertexCount; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			double p;

			if (fscanf(ff, "%Lf", &p))
			{
				*(pdouble++) = p;;
			}
			else
			{
				return 1;
			}
		}
	}
	qh_init_A (stdin, stdout, stderr, 0, NULL);
	
	int exitcode= setjmp (qh errexit);

	char options [200];

	if (argc == 4)
		sprintf (options, "qhull %s", argv[3]);
	else
		sprintf (options, "qhull");

	qh_initflags (options);
	qh_init_B (vertArray, vertexCount, 3, true);
	qh_qhull();
	qh_vertexneighbors();
	qh_check_output();
	qh_triangulate();
	qh_findgood_all (qh facet_list); 

	FArray<facetT *> faceRecord;
	FArray<ridgeT *> edgeRecord;
	FArray<ConvexEdge> edgeRecord2;
	FArray<vertexT *> vertexRecord;
{
	vertexT *vertex, **vertexp;

	FORALLvertex_(qh vertex_list)
	{
		vertexRecord.Add(vertex->id, vertex);
	}

	facetT *facet;

	FORALLfacet_(qh facet_list)
	{
		faceRecord.Add(facet->id, facet);
	}
	FORALLfacet_(qh facet_list)
	{
		if (0)//facet->ridges)
		{
			ridgeT * ridge, ** ridgep;

			FOREACHridge_(facet->ridges)
			{
				edgeRecord.Add(ridge->id, ridge);
			}
		}
		else
		{
			int vertexIndex[3], c = 0;

			FOREACHvertex_(facet->vertices)
			{
				neByte h = vertexRecord.GetIndex(vertex->id);

				ASSERT(h < qh num_vertices);

				vertexIndex[c++] = h;
			}
			c = 0;

			facetT * neighbor, ** neighborp;

			FOREACHneighbor_(facet)
			{
				ConvexEdge e;

				e.f1 = faceRecord.GetIndex(facet->id);
				e.f2 = faceRecord.GetIndex(neighbor->id);

				ASSERT(e.f1 < qh num_facets);
				ASSERT(e.f2 < qh num_facets);

				e.v1 = vertexIndex[(c+1)%3];
				e.v2 = vertexIndex[(c+2)%3];

				u32 key = MakeEdgeKey(e.v1, e.v2);

				edgeRecord2.Add(key, e);

				c++;
			}
		}
	}
}
	if (faceRecord.GetCount() > 255)
	{
		fprintf(stderr, "Error: Object has more than 255 faces (Number of faces = %d).\n", faceRecord.GetCount());
		exit(0);
	}
	if (vertexRecord.GetCount() > 255)
	{
		fprintf(stderr, "Error: Object has more than 255 vertices (Number of vertices = %d).\n", vertexRecord.GetCount());
		exit(0);
	}
	if (edgeRecord2.GetCount() > 255)
	{
		fprintf(stderr, "Error: Object has more than 255 edges (Number of edges = %d).\n", edgeRecord2.GetCount());
		exit(0);
	}
{
	int edgeCount = edgeRecord2.GetCount();
	int zero = 0;
	
	FILE * ff = fopen(argv[2], "wb+");

	fwrite(&(qh num_facets), sizeof(int), 1, ff);
	fwrite(&(qh num_vertices), sizeof(int), 1, ff);
	fwrite(&edgeCount, sizeof(int), 1, ff);
	fwrite(&zero, sizeof(int), 1, ff);
	
	TOKAMAK_OUTPUT_3("%d, %d, %d \n", qh num_facets, qh num_vertices, edgeCount);

	int i;
	f32 f;

	for (i = 0; i < faceRecord.GetCount(); i++)
	{
		facetT * facet = faceRecord[i];
		
		for (int j = 0; j < 3; j++)
		{
			f = facet->normal[j];

			fwrite(&f, sizeof(f), 1, ff);

			TOKAMAK_OUTPUT_1("%f ", f);
		}
		f = facet->offset;

		fwrite(&f, sizeof(f), 1, ff);

		TOKAMAK_OUTPUT_1("%f \n", f);
	}
	for (i = 0; i < vertexRecord.GetCount(); i++)
	{
		vertexT *vertex	= vertexRecord[i];

		for (int j = 0; j < 3; j++)
		{
			f = vertex->point[j];

			fwrite(&f, sizeof(f), 1, ff);

			TOKAMAK_OUTPUT_1("%f ", f);
		}
		fwrite(&zero, sizeof(f), 1, ff);

		TOKAMAK_OUTPUT("\n");
	}

	int offset = sizeof(int) * 4;
	offset += sizeof(float) * 4 * (faceRecord.GetCount() + qh num_vertices);
	offset += sizeof(ConvexFace) * faceRecord.GetCount();
	offset += sizeof(ConvexVert) * vertexRecord.GetCount();

	if (edgeRecord.GetCount() != 0)
		offset += sizeof(ConvexEdge) * edgeRecord.GetCount();
	else
		offset += sizeof(ConvexEdge) * edgeRecord2.GetCount();

	ConvexFace cface;

	TOKAMAK_OUTPUT("Face Records \n");

	for (i = 0; i < faceRecord.GetCount(); i++)
	{
		facetT * facet = faceRecord[i];

		cface.neighbourFaces = offset;	offset += sizeof(neByte) * 3;//cface.numberFaceNeighbour;
		cface.neighbourVerts = offset;	offset += sizeof(neByte) * 3;//cface.numberVertNeighbour;
		cface.neighbourEdges = offset;	offset += sizeof(neByte) * 3;//cface.numberFaceNeighbour;

		int ss = sizeof(cface);

		fwrite(&cface, sizeof(cface), 1, ff);
		
		TOKAMAK_OUTPUT_3("Face %d has %d face neighbour, %d vertics\n", i, 3, 3);
	}

	TOKAMAK_OUTPUT("Vertex Records \n");

	ConvexVert cvert;

	for (i = 0; i < vertexRecord.GetCount(); i++)
	{
		vertexT * vertex = vertexRecord[i];

		int numberEdgeNeighbour = 0;

		facetT * neighbor, ** neighborp;
		
		FOREACHneighbor_(vertex)
		{
			numberEdgeNeighbour++;
		}
		cvert.neighbourEdges = offset;	offset += sizeof(neByte) * (numberEdgeNeighbour + 1);

		int ss = sizeof(cvert);

		fwrite(&cvert, ss, 1, ff);

		TOKAMAK_OUTPUT_2("Vertex %d has %d edges\n", i, numberEdgeNeighbour);
	}
	ConvexEdge cedge;

	if (edgeRecord.GetCount() != 0)
	{
		for (i = 0; i < edgeRecord.GetCount(); i++)
		{
			ridgeT * ridge = edgeRecord[i];

			cedge.f1 = faceRecord.GetIndex(ridge->top->id);
			
			cedge.f2 = faceRecord.GetIndex(ridge->bottom->id);

			vertexT * vertex, **vertexp;
			int v[2], c = 0;
			FOREACHvertex_(ridge->vertices)
			{
				ASSERT(c < 2);

				v[c] = vertex->id;

				c++;
			}
			cedge.v1 = vertexRecord.GetIndex(v[0]);

			cedge.v2 = vertexRecord.GetIndex(v[1]);

			fwrite(&cedge, sizeof(ConvexEdge), 1, ff);

			TOKAMAK_OUTPUT_1("Edge %d = ", i);
			
			TOKAMAK_OUTPUT_4(" faces %d, %d, verts %d, %d\n", cedge.f1, cedge.f2, cedge.v1, cedge.v2);
		}
	}
	else
	{
		for (i = 0; i < edgeRecord2.GetCount(); i++)
		{
			cedge = edgeRecord2[i];

			fwrite(&cedge, sizeof(ConvexEdge), 1, ff);

			TOKAMAK_OUTPUT_1("Edge %d = ", i);
			
			TOKAMAK_OUTPUT_4(" faces %d, %d, verts %d, %d\n", cedge.f1, cedge.f2, cedge.v1, cedge.v2);
		}
	}
	for (i = 0; i < faceRecord.GetCount(); i++)
	{
		facetT * facet = faceRecord[i];

		facetT * neighbor, ** neighborp;

		TOKAMAK_OUTPUT_1("Face %d = {", i);
		
		FOREACHneighbor_(facet)
		{
			neByte bb = faceRecord.GetIndex(neighbor->id);

			fwrite(&bb, sizeof(neByte), 1, ff);

			TOKAMAK_OUTPUT_1("%d,", bb);
		}
		TOKAMAK_OUTPUT("}, {");

		vertexT * vertex, ** vertexp;
		
		FOREACHvertex_(facet->vertices)
		{
			neByte bb = vertexRecord.GetIndex(vertex->id);

			fwrite(&bb, sizeof(neByte), 1, ff);

			TOKAMAK_OUTPUT_1("%d,", bb);
		}
		TOKAMAK_OUTPUT("}, {");

		if (0)//facet->ridges)
		{
			ridgeT * ridge, ** ridgep;

			FOREACHridge_(facet->ridges)
			{
				neByte bb = edgeRecord.GetIndex(ridge->id);

				fwrite(&bb, sizeof(neByte), 1, ff);

				TOKAMAK_OUTPUT_1("%d,", bb);
			}
		}
		else
		{
			int vertexIndex[3], c = 0;

			FOREACHvertex_(facet->vertices)
			{
				vertexIndex[c++] = vertexRecord.GetIndex(vertex->id);
			}
			c = 0;

			facetT * neighbor, ** neighborp;

			FOREACHneighbor_(facet)
			{
				neByte v1 = vertexIndex[(c+1)%3];
				neByte v2 = vertexIndex[(c+2)%3];
				u32 key = MakeEdgeKey(v1, v2);

				neByte bb = edgeRecord2.GetIndex(key);

				fwrite(&bb, sizeof(neByte), 1, ff);

				TOKAMAK_OUTPUT_1("%d,", bb);
				c++;
			}
		}
		TOKAMAK_OUTPUT("}\n");
	}

	// vertex adjacent edges

	for (i = 0; i < vertexRecord.GetCount(); i++)
	{
		neByte * ridgeNeigbour = new neByte[100];

		vertexT * vert = vertexRecord[i];

		facetT * neighbor, ** neighborp;

		int numEdge = 0;

		FOREACHneighbor_(vert)
		{
			if (0)//neighbor->ridges)
			{
				ridgeT * ridge, ** ridgep;

				FOREACHridge_(neighbor->ridges)
				{
					vertexT * vertex, ** vertexp;

					FOREACHvertex_(ridge->vertices)
					{
						if (vertexRecord.GetIndex(vertex->id) == i)
						{
							//add this ridge
							neByte eindex = edgeRecord.GetIndex(ridge->id);

							bool found = false;

							for (int k = 0; k < numEdge; k++)
							{
								if (ridgeNeigbour[k] == eindex)
								{	
									found = true;
									break;
								}
							}
							if (!found)
							{
								ridgeNeigbour[numEdge++] = eindex;
							}
						}
					}
				}
			}
			else
			{
				vertexT * vertex, ** vertexp;

				neByte vs[3]; int c = 0;

				FOREACHvertex_(neighbor->vertices)
				{
					vs[c++] = vertexRecord.GetIndex(vertex->id);
				}
				for (int ii = 0; ii < 3; ii++)
				{
					neByte v1 = vs[(ii+1)%3];

					neByte v2 = vs[(ii+2)%3];

					if (v1 != i && v2 != i)
						continue;

					u32 key = MakeEdgeKey(v1, v2);

					neByte eindex = edgeRecord2.GetIndex(key);

					bool found = false;

					for (int k = 0; k < numEdge; k++)
					{
						if (ridgeNeigbour[k] == eindex)
						{
							found = true;

							break;
						}
					}
					if (!found)
					{
						ridgeNeigbour[numEdge++] = eindex;
					}
				}
			}
		}
		TOKAMAK_OUTPUT_1("Vertex %d = {", i);

		for (int j = 0; j < numEdge; j++)
		{
			neByte bb = ridgeNeigbour[j];

			fwrite(&bb, sizeof(neByte), 1, ff);

			TOKAMAK_OUTPUT_1("%d,", bb);
		}
		neByte f = 0xff;

		fwrite(&f, sizeof(neByte), 1, ff);

		TOKAMAK_OUTPUT("}\n");
		
		delete ridgeNeigbour;
	}
	fclose(ff);

	TOKAMAK_OUTPUT("\n");
}
	qh NOerrexit= True;
	
	//qh_freeqhull (!qh_ALL);

//	int curlong, totlong;

	//qh_memfreeshort (&curlong, &totlong);

	delete vertArray;

	return 0;
}
