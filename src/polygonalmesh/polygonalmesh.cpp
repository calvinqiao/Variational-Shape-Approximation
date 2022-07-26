#include <unordered_set>
#include <algorithm>

#include "polygonalmesh.h"

PolygonalMesh::PolygonalMesh()
{
	searchKeySeed=1;
}
PolygonalMesh::~PolygonalMesh()
{
	CleanUp();
}
void PolygonalMesh::CleanUp(void)
{
	vtxList.clear();
}

void PolygonalMesh::GetBoundingBox(YsVec3 &min,YsVec3 &max) const
{
	if(0<GetNumVertex())
	{
		auto vtHd=FindFirstVertex();
		min=GetVertexPosition(vtHd);
		max=GetVertexPosition(vtHd);
		while(NullVertex()!=vtHd)
		{
			auto pos=GetVertexPosition(vtHd);
			min.SetX(std::min(pos.x(),min.x()));
			min.SetY(std::min(pos.y(),min.y()));
			min.SetZ(std::min(pos.z(),min.z()));
			max.SetX(std::max(pos.x(),max.x()));
			max.SetY(std::max(pos.y(),max.y()));
			max.SetZ(std::max(pos.z(),max.z()));
			vtHd=FindNextVertex(vtHd);
		}
	}
	else
	{
		min=YsVec3::Origin();
		max=YsVec3::Origin();
	}
}
void PolygonalMesh::GetBoundingBox(YsVec3 bbx[2]) const
{
	GetBoundingBox(bbx[0],bbx[1]);
}

long long int PolygonalMesh::GetNumVertex(void) const
{
	return vtxList.size();
}

PolygonalMesh::VertexHandle PolygonalMesh::FindFirstVertex(void) const
{
	VertexHandle vtHd;
	vtHd.iter=vtxList.begin();
	return vtHd;
}

PolygonalMesh::VertexHandle PolygonalMesh::FindNextVertex(VertexHandle vtHd) const
{
	if(vtxList.end()!=vtHd.iter)
	{
		++vtHd.iter;
		return vtHd;
	}
	return NullVertex();
}

PolygonalMesh::VertexHandle PolygonalMesh::AddVertex(const YsVec3 &pos)
{
	Vertex vtx;
	vtx.pos=pos;
	vtx.searchKey=searchKeySeed++;
	vtxList.push_back(vtx);

	VertexHandle vtHd;
	vtHd.iter=vtxList.end();
	--vtHd.iter;

	vtxSearch[vtx.searchKey]=vtHd;

	return vtHd;
}

void PolygonalMesh::DeleteVertex(VertexHandle vtHd)
{
	if(NullVertex()!=vtHd)
	{
		auto found=vtxSearch.find(GetSearchKey(vtHd));
		vtxSearch.erase(found);
		vtxList.erase(vtHd.iter);
	}
}

void PolygonalMesh::DeleteUnusedVertex(void)
{
	auto vtHd=FindFirstVertex();
	while(NullVertex()!=vtHd)
	{
		auto nextVtHd=FindNextVertex(vtHd);
		if(0==FindPolygonFromVertex(vtHd).size())
		{
			DeleteVertex(vtHd);
		}
		vtHd=nextVtHd;
	}
}

unsigned int PolygonalMesh::GetSearchKey(VertexHandle vtHd) const
{
	if(NullVertex()!=vtHd)
	{
		return vtHd.iter->searchKey;
	}
	return NullSearchKey;
}

PolygonalMesh::VertexHandle PolygonalMesh::FindVertex(unsigned int searchKey) const
{
	auto found=vtxSearch.find(searchKey);
	if(vtxSearch.end()!=found)
	{
		return found->second;
	}
	return NullVertex();
}

PolygonalMesh::VertexHandle PolygonalMesh::NullVertex(void) const
{
	VertexHandle vtHd;
	vtHd.iter=vtxList.end();
	return vtHd;
}

YsVec3 PolygonalMesh::GetVertexPosition(VertexHandle vtHd) const
{
	if(NullVertex()!=vtHd)
	{
		return vtHd.iter->pos;
	}
	else
	{
		return YsVec3::Origin();
	}
}

void PolygonalMesh::SetVertexPosition(VertexHandle vtHd,const YsVec3 &newPos)
{
	if(NullVertex()!=vtHd)
	{
		vtHd.iter->pos=newPos;
	}
}

long long int PolygonalMesh::GetNumPolygon(void) const
{
	return plgList.size();
}

PolygonalMesh::PolygonHandle PolygonalMesh::FindFirstPolygon(void) const
{
	PolygonHandle plHd;
	plHd.iter=plgList.begin();
	return plHd;
}

PolygonalMesh::PolygonHandle PolygonalMesh::FindNextPolygon(PolygonHandle plHd) const
{
	if(plgList.end()!=plHd.iter)
	{
		++plHd.iter;
		return plHd;
	}
	return NullPolygon();
}

PolygonalMesh::PolygonHandle PolygonalMesh::GetNPolygon(int n) const
{
	PolygonHandle plHd;
	plHd.iter = plgList.begin();
	while (n != 0) {
		++plHd.iter;
		--n;
	}
	return plHd;
}

bool PolygonalMesh::MoveToNextPolygon(PolygonHandle &plHd) const
{
	if(NullPolygon()==plHd)
	{
		plHd.iter=plgList.begin();
	}
	else
	{
		++plHd.iter;
	}
	if(NullPolygon()!=plHd)
	{
		return true;
	}
	return false;
}

PolygonalMesh::PolygonHandle PolygonalMesh::AddPolygon(long long int nVtx,const VertexHandle vtHdAry[])
{
	Polygon plg;
	plg.vtHdAry.resize(nVtx);
	for(int i=0; i<nVtx; ++i)
	{
		plg.vtHdAry[i]=vtHdAry[i];
	}
	plg.nom=YsVec3::Origin();
	plg.col.SetIntRGB(0,0,255);
	plg.searchKey=searchKeySeed++;

	plgList.push_back((Polygon &&)plg);

	PolygonHandle plHd;
	plHd.iter=plgList.end();
	--plHd.iter;

	plgSearch[plg.searchKey]=plHd;

	for(int i=0; i<nVtx; ++i)
	{
		vtxToPlg[GetSearchKey(vtHdAry[i])].push_back(plHd);
	}

	return plHd;
}
void PolygonalMesh::SetPolygonNormal(PolygonHandle plHd,const YsVec3 &nom)
{
	if(plHd!=NullPolygon())
	{
		plHd.iter->nom=nom;
	}
}

unsigned int PolygonalMesh::GetSearchKey(PolygonHandle plHd) const
{
	if(NullPolygon()!=plHd)
	{
		return plHd.iter->searchKey;
	}
	return NullSearchKey;
}

PolygonalMesh::PolygonHandle PolygonalMesh::FindPolygon(unsigned int searchKey) const
{
	auto found=plgSearch.find(searchKey);
	if(plgSearch.end()!=found)
	{
		return found->second;
	}
	return NullPolygon();
}

PolygonalMesh::PolygonHandle PolygonalMesh::NullPolygon(void) const
{
	PolygonHandle plHd;
	plHd.iter=plgList.end();
	return plHd;
}

void PolygonalMesh::SetPolygonVertex(PolygonHandle plHd,long long int nVtx,const VertexHandle vtHdAry[])
{
	{
		for(auto vtHd : GetPolygonVertex(plHd))
		{
			auto &plgSet=vtxToPlg[GetSearchKey(vtHd)];
			for(int i=plgSet.size()-1; 0<=i; --i)
			{
				if(plHd==plgSet[i])
				{
					plgSet[i]=plgSet.back();
					plgSet.pop_back();
				}
			}
		}
	}

	plHd.iter->vtHdAry.clear();
	for(int i=0; i<nVtx; ++i)
	{
		plHd.iter->vtHdAry.push_back(vtHdAry[i]);
		vtxToPlg[GetSearchKey(vtHdAry[i])].push_back(plHd);
	}
}

void PolygonalMesh::SetPolygonColor(PolygonHandle plHd,YsColor col)
{
	if(NullPolygon()!=plHd)
	{
		plHd.iter->col=col;
	}
}

YsColor PolygonalMesh::GetPolygonColor(PolygonHandle plHd) const
{
	if(NullPolygon()!=plHd)
	{
		return plHd.iter->col;
	}
	YsColor c;
	c.SetIntRGB(0,0,0);
	return c;
}

YsVec3 PolygonalMesh::GetPolygonCenter(PolygonHandle plHd) const
{
	YsVec3 cen=YsVec3::Origin();
	double div=0.0;
	for(auto vtHd : GetPolygonVertex(plHd))
	{
		cen+=GetVertexPosition(vtHd);
		div+=1.0;
	}
	return cen/div;
}

double PolygonalMesh::GetPolygonArea(PolygonHandle plhd) const
{
//    printf("DBug: In Get polygon area\n");
	std::vector <VertexHandle> vertex_hds = GetPolygonVertex(plhd);
    YsVec3 posA, posB, posC, AB, AC;
    if (vertex_hds.size() == 3) {
        posA = GetVertexPosition(vertex_hds[0]);
        posB = GetVertexPosition(vertex_hds[1]);
        posC = GetVertexPosition(vertex_hds[2]);
    }
    else {
        printf("DBug: Invalid number of vertices in this polygon (%lu)!",vertex_hds.size());
        throw std::invalid_argument( "Invalid number of vertices in this polygon!" );
    }

    AB[0] = posA[0] - posB[0]; AB[1] = posA[1] - posB[1]; AB[2] = posA[2] - posB[2];
    AC[0] = posA[0] - posC[0]; AC[1] = posA[1] - posC[1]; AC[2] = posA[2] - posC[2];

    double sum_part0, sum_part1, sum_part2;

    sum_part0 = AB[1]*AC[2] - AB[2]*AC[1];
    sum_part1 = AB[2]*AC[0] - AB[0]*AC[2];
    sum_part2 = AB[0]*AC[1] - AB[1]*AC[0];

    double area;
    area = 0.5 * sqrt(sum_part0*sum_part0 + sum_part1*sum_part1 + sum_part2*sum_part2);

    return area;
}

std::vector <PolygonalMesh::VertexHandle> PolygonalMesh::GetPolygonVertex(PolygonHandle plHd) const
{
	if(NullPolygon()!=plHd)
	{
		return plHd.iter->vtHdAry;
	}
	std::vector <PolygonalMesh::VertexHandle> empty;
	return empty;
}
YsVec3 PolygonalMesh::GetPolygonNormal(PolygonHandle plHd) const
{
	if(NullPolygon()!=plHd)
	{
		return plHd.iter->nom;
	}
	return YsVec3::Origin();
}

std::vector <PolygonalMesh::PolygonHandle> PolygonalMesh::FindPolygonFromVertex(VertexHandle vtHd) const
{
	auto found=vtxToPlg.find(GetSearchKey(vtHd));
	if(vtxToPlg.end()!=found)
	{
		return found->second;
	}
	std::vector <PolygonHandle> empty;
	return empty;
}

std::vector <PolygonalMesh::VertexHandle> PolygonalMesh::GetConnectedVertex(VertexHandle fromVtHd) const
{
	std::vector <VertexHandle> connVtHd;

	for(auto plHd : FindPolygonFromVertex(fromVtHd))
	{
		auto plVtHd=GetPolygonVertex(plHd);
		for(int i=0; i<plVtHd.size(); ++i)
		{
			if(plVtHd[(i+1)%plVtHd.size()]==fromVtHd ||
			   plVtHd[(i+plVtHd.size()-1)%plVtHd.size()]==fromVtHd)
			{
				if(connVtHd.end()==std::find(connVtHd.begin(),connVtHd.end(),plVtHd[i]))
				{
					connVtHd.push_back(plVtHd[i]);
				}
			}
		}
	}
	return connVtHd;
}

std::vector <PolygonalMesh::PolygonHandle> PolygonalMesh::FindPolygonFromEdgePiece(VertexHandle edVtHd0,VertexHandle edVtHd1) const
{
	std::vector <PolygonHandle> usingPlHd;

	for(auto plHd : FindPolygonFromVertex(edVtHd0))
	{
		auto plVtHd=GetPolygonVertex(plHd);
		if(plVtHd.end()!=std::find(plVtHd.begin(),plVtHd.end(),edVtHd1))
		{
			for(int i=0; i<plVtHd.size(); ++i)
			{
				if((plVtHd[i]==edVtHd0 && plVtHd[(i+1)%plVtHd.size()]==edVtHd1) ||
				   (plVtHd[i]==edVtHd1 && plVtHd[(i+1)%plVtHd.size()]==edVtHd0))
				{
					usingPlHd.push_back(plHd);
				}
			}
        } else {
        }
	}

	return usingPlHd;
}

PolygonalMesh::PolygonHandle PolygonalMesh::GetNeighborPolygon(PolygonHandle plHd,int edIdx) const
{
	auto plVtHd=GetPolygonVertex(plHd);
	if(0<=edIdx && edIdx<plVtHd.size())
	{
//        printf("DBug: edIdx is acceptable\n");
		auto usingPlHd=FindPolygonFromEdgePiece(plVtHd[edIdx],plVtHd[(edIdx+1)%plVtHd.size()]);
//        printf("DBug: size of usingPlHd is %lu\n",usingPlHd.size());
		if(2==usingPlHd.size())
		{
//            printf("DBug: 2==usingPlHd.size is acceptable\n");
			if(usingPlHd[0]==plHd)
			{
				return usingPlHd[1];
			}
			else
			{
				return usingPlHd[0];
			}
		}
	}
    else {
        printf("DBug: edIdx is unacceptable (%d, size is %lu)\n",edIdx,plVtHd.size());
    }

	return NullPolygon();
}

std::vector <PolygonalMesh::PolygonHandle> PolygonalMesh::FindNNeighbors(PolygonalMesh::PolygonHandle plHd, int NNei) const
{
	std::vector <PolygonalMesh::PolygonHandle> todo;
//    std::vector <int> todoN;
//    std::unordered_set <unsigned int> visited;
    
    auto plVtHd=GetPolygonVertex(plHd);
    for(auto edIdx=0; edIdx<plVtHd.size(); ++edIdx)
    {
        auto neiPlHd=GetNeighborPolygon(plHd,edIdx);
        if (neiPlHd == NullPolygon()) {
            continue;
        }
        todo.push_back(neiPlHd);
    }

	return todo;
}
