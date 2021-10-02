/*
*
*	LICENSE:
*	just give me credits if u use part of my work!
*
*/

#include <Windows.h>
#include <stdio.h>
#include <math.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

#define LUMP_PLANES 1
#define LUMP_FACES 7
#define LUMP_MODELS 14
#define LUMP_BRUSHSIDES 19
#define LUMP_ORIGINALFACES 27
#define HEADER_LUMPS 64

#define FLOAT_EPSILON 0.01f

using std::string;



struct Vector
{
	float x;
	float y;
	float z;
};

struct dmodel_t
{
	Vector	mins, maxs;		// bounding box
	Vector	origin;			// for sounds or lights
	int	headnode;		// index into node array
	int	firstface, numfaces;	// index into face array
};

struct dface_t
{
	unsigned short	planenum;		// the plane number
	byte		side;			// faces opposite to the node's plane direction
	byte		onNode;			// 1 of on node, 0 if in leaf
	int		firstedge;		// index into surfedges
	short		numedges;		// number of surfedges
	short		texinfo;		// texture info
	short		dispinfo;		// displacement info
	short		surfaceFogVolumeID;	// ?
	byte		styles[4];		// switchable lighting info
	int		lightofs;		// offset into lightmap lump
	float		area;			// face area in units^2
	int		LightmapTextureMinsInLuxels[2];	// texture lighting info
	int		LightmapTextureSizeInLuxels[2];	// texture lighting info
	int		origFace;		// original face this was split from
	unsigned short	numPrims;		// primitives
	unsigned short	firstPrimID;
	unsigned int	smoothingGroups;	// lightmap smoothing group
};

struct lump_t
{
	int	fileofs;	// offset into file (bytes)
	int	filelen;	// length of lump (bytes)
	int	version;	// lump format version
	char	fourCC[4];	// lump ident code
};

struct dheader_t
{
	int	ident;                // BSP file identifier
	int	version;              // BSP file version
	lump_t	lumps[HEADER_LUMPS];  // lump directory array
	int	mapRevision;          // the map's revision (iteration, version) number
};

struct dbrushside_t
{
	unsigned short	planenum;	// facing out of the leaf
	short		texinfo;	// texture info
	short		dispinfo;	// displacement info
	short		bevel;		// is the side a bevel plane?
};

struct dplane_t
{
	Vector	normal;	// normal vector
	float	dist;	// distance from origin
	int	type;	// plane axis identifier
};

// functinos
void ProcessFaceLump(dheader_t* dfile);
void ProcessBsLump(dheader_t* dfile);
void ProcessPlaneLump(dheader_t* dfile);

std::vector<dface_t*> lumpFaces;
std::vector<dbrushside_t*> lumpBrushSides;
std::map<unsigned int, dplane_t*> lumpPlanes;


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "deobf.exe arguments: <file>" << std::endl << "You should drag file onto me!" << std::endl;

		Sleep(30000);
		return 0;
	}

	std::cout << "arguments: " << argv[1] << std::endl;

	std::ifstream file(string(argv[1]), std::ifstream::in | std::ifstream::binary);
	if (!file.good())
	{
		std::cout << "Unable to find inputed file. You should drag file onto me!" << std::endl;
		file.close();

		Sleep(30000);
		return 0;
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	string st = buffer.str();
	dheader_t* dfile = (dheader_t*)st.c_str();
	
	if (dfile->ident != 0x50534256)
	{
		std::cout << "Invalid file format. Please feed us valid .bsp file!" << std::endl;
		file.close();

		Sleep(30000);
		return 0;
	}


	ProcessFaceLump(dfile);
	ProcessBsLump(dfile);
	ProcessPlaneLump(dfile);
			
			
	for (unsigned int b = 0; b < lumpBrushSides.size(); b++)
	{
		unsigned short iBrushPlane = lumpBrushSides[b]->planenum;

		if (lumpBrushSides[b]->texinfo != 0x0000)
			continue;


		for (unsigned int f = 0; f < lumpFaces.size(); f++)
		{
			unsigned short iFacePlane = lumpFaces[f]->planenum;

			if  (	(fabs(lumpPlanes[iBrushPlane]->dist - lumpPlanes[iFacePlane]->dist) < FLOAT_EPSILON)
				&&  (fabs(lumpPlanes[iBrushPlane]->normal.x - lumpPlanes[iFacePlane]->normal.x) < FLOAT_EPSILON)
				&&  (fabs(lumpPlanes[iBrushPlane]->normal.y - lumpPlanes[iFacePlane]->normal.y) < FLOAT_EPSILON)
				&&  (fabs(lumpPlanes[iBrushPlane]->normal.z - lumpPlanes[iFacePlane]->normal.z) < FLOAT_EPSILON)
				)
			{
				lumpBrushSides[b]->texinfo = lumpFaces[f]->texinfo;
			}
		}
	}

	CreateDirectory("finished", NULL);
	GetLastError();

	string path(argv[1]);
	string base_filename = path.substr(path.find_last_of("/\\") + 1);
//	std::cout << base_filename;

	std::ofstream output(string(".\\finished\\") + base_filename, std::ifstream::out | std::ifstream::binary);
	if (output.good())
	{
		output << st;
		output.close();
		std::cout << "finished " << base_filename << " exported successfully" << std::endl;
	}
	else
		std::cout << "Failed to export finished bsp. Reason: UNKNOWN DES!" << std::endl;

	file.close();
	Sleep(30000);
	return 0;
}

void ProcessFaceLump(dheader_t* dfile)
{
	lump_t faceLump = dfile->lumps[LUMP_FACES];
	
	dface_t* faces = reinterpret_cast<dface_t *>(reinterpret_cast<unsigned int>(dfile) + static_cast<unsigned int>(faceLump.fileofs));

	while (true)
	{
		lumpFaces.push_back(faces);
		faces++;
		if (reinterpret_cast<unsigned int>(faces) >= reinterpret_cast<unsigned int>(dfile) + static_cast<unsigned int>(faceLump.fileofs) + static_cast<unsigned int>(faceLump.filelen))
			break;
	}
}

void ProcessBsLump(dheader_t* dfile)
{
	lump_t bsLump = dfile->lumps[LUMP_BRUSHSIDES];

	dbrushside_t* brushes = reinterpret_cast<dbrushside_t *>(reinterpret_cast<unsigned int>(dfile) + static_cast<unsigned int>(bsLump.fileofs));

	while (true)
	{
		lumpBrushSides.push_back(brushes);
		brushes++;

	//	std::cout << brushes->texinfo << std::endl;
		if (reinterpret_cast<unsigned int>(brushes) >= reinterpret_cast<unsigned int>(dfile) + static_cast<unsigned int>(bsLump.fileofs) + static_cast<unsigned int>(bsLump.filelen))
			break;
	}
}

void ProcessPlaneLump(dheader_t* dfile)
{
	lump_t plaLump = dfile->lumps[LUMP_PLANES];

	dplane_t* planes = reinterpret_cast<dplane_t *>(reinterpret_cast<unsigned int>(dfile) + static_cast<unsigned int>(plaLump.fileofs));
	int iPlaneNum = 0;
	while (true)
	{
		lumpPlanes.insert(std::pair<unsigned int, dplane_t*>(iPlaneNum, planes));
		iPlaneNum++;
		planes++;
		

		if (reinterpret_cast<unsigned int>(planes) >= reinterpret_cast<unsigned int>(dfile) + static_cast<unsigned int>(plaLump.fileofs) + static_cast<unsigned int>(plaLump.filelen))
			break;
	}
}