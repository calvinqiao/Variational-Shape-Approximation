#include <vector>
#include "polygonalmesh.h"
#include <string>
#include <stdio.h>

bool PolygonalMesh::LoadBinStl(const char fn[])
{
    FILE *fp=fopen(fn,"rb");
    if(nullptr!=fp)
    {
        unsigned char buf[80];
        
        fread(buf,1,80,fp);
        
        
        fread(buf,4,1,fp);
        auto nTri=GetInteter(buf);
        printf("%d\n",nTri);
        
        
        int nActual=0;
        while(50==fread(buf,1,50,fp))
        {
            auto nx=GetFloat(buf+0);
            auto ny=GetFloat(buf+4);
            auto nz=GetFloat(buf+8);
            
            auto x0=GetFloat(buf+12);
            auto y0=GetFloat(buf+16);
            auto z0=GetFloat(buf+20);
            
            auto x1=GetFloat(buf+24);
            auto y1=GetFloat(buf+28);
            auto z1=GetFloat(buf+32);
            
            auto x2=GetFloat(buf+36);
            auto y2=GetFloat(buf+40);
            auto z2=GetFloat(buf+44);
            
            YsVec3 vtxPos[3]=
            {
                YsVec3(x0,y0,z0),
                YsVec3(x1,y1,z1),
                YsVec3(x2,y2,z2),
            };
            VertexHandle vtHd[3]=
            {
                AddVertex(vtxPos[0]),
                AddVertex(vtxPos[1]),
                AddVertex(vtxPos[2]),
            };
            auto plHd=AddPolygon(3,vtHd);
            
            SetPolygonNormal(plHd,YsVec3(nx,ny,nz));
            
            ++nActual;
        }
        
        
        printf("%d\n",nActual);
        
        StitchVertex();
        DeleteUnusedVertex();
        
        fclose(fp);
        return true;
    }
    return false;
}

void PolygonalMesh::StitchVertexN2(void)
{
    for(auto vtHd0=FindFirstVertex(); NullVertex()!=vtHd0; vtHd0=FindNextVertex(vtHd0))
    {
        for(auto vtHd1=FindNextVertex(vtHd0); NullVertex()!=vtHd1; vtHd1=FindNextVertex(vtHd1))
        {
            if(GetVertexPosition(vtHd0)==GetVertexPosition(vtHd1))
            {
                auto plHdSet=FindPolygonFromVertex(vtHd1);
                for(auto plHd : plHdSet)
                {
                    auto plVtHd=GetPolygonVertex(plHd);
                    for(auto &v : plVtHd)
                    {
                        if(v==vtHd1)
                        {
                            v=vtHd0;
                        }
                    }
                    SetPolygonVertex(plHd,plVtHd.size(),plVtHd.data());
                }
            }
        }
    }
}

void PolygonalMesh::StitchVertex(void)
{
    YsLattice3d <std::vector <PolygonalMesh::VertexHandle> > ltc;
    YsVec3 bbx[2];
    
    GetBoundingBox(bbx);
    auto dgn=(bbx[1]-bbx[0]);
    auto lDgn=dgn.GetLength();
    bbx[0].SubX(lDgn/100.0);
    bbx[0].SubY(lDgn/100.0);
    bbx[0].SubZ(lDgn/100.0);
    bbx[1].AddX(lDgn/100.0);
    bbx[1].AddY(lDgn/100.0);
    bbx[1].AddZ(lDgn/100.0);
    ltc.Create(200,200,200,bbx[0],bbx[1]);
    
    for(auto vtHd=FindFirstVertex(); NullVertex()!=vtHd; vtHd=FindNextVertex(vtHd))
    {
        auto vtPos=GetVertexPosition(vtHd);
        auto blkIdx=ltc.GetBlockIndex(vtPos);
        if(ltc.IsInRange(blkIdx)==true)
        {
            ltc[blkIdx].push_back(vtHd);
        }
        else
        {
            printf("%s %d\n",__FUNCTION__,__LINE__);
        }
    }
    
    for(auto vtHd=FindFirstVertex(); NullVertex()!=vtHd; vtHd=FindNextVertex(vtHd))
    {
        if(0==FindPolygonFromVertex(vtHd).size())
        {
            continue;
        }
        
        auto vtPos=GetVertexPosition(vtHd);
        auto min=vtPos,max=vtPos;
        min.SubX(1e-6);
        min.SubY(1e-6);
        min.SubZ(1e-6);
        max.AddX(1e-6);
        max.AddY(1e-6);
        max.AddZ(1e-6);
        
        std::vector <PolygonalMesh::VertexHandle> matchingVtHd;
        
        auto minIdx=ltc.GetBlockIndex(min);
        auto maxIdx=ltc.GetBlockIndex(max);
        for(auto x=minIdx.x(); x<=maxIdx.x(); ++x)
        {
            for(auto y=minIdx.y(); y<=maxIdx.y(); ++y)
            {
                for(auto z=minIdx.z(); z<=maxIdx.z(); ++z)
                {
                    YsVec3i blkIdx(x,y,z);
                    if(true==ltc.IsInRange(blkIdx))
                    {
                        for(auto v : ltc[blkIdx])
                        {
                            if(GetVertexPosition(v)==GetVertexPosition(vtHd))
                            {
                                matchingVtHd.push_back(v);
                            }
                        }
                        // matchingVtHd.insert(matchingVtHd.end(),ltc[blkIdx].begin(),ltc[blkIdx].end());
                    }
                }
            }
        }
        
        PolygonalMesh::VertexHandle vtHd0=vtHd;
        
        for(auto vtHd1 : matchingVtHd)
        {
            if(vtHd0!=vtHd1)
            {
                auto plHdSet=FindPolygonFromVertex(vtHd1);
                for(auto plHd : plHdSet)
                {
                    auto plVtHd=GetPolygonVertex(plHd);
                    for(auto &v : plVtHd)
                    {
                        if(v==vtHd1)
                        {
                            v=vtHd0;
                        }
                    }
                    SetPolygonVertex(plHd,plVtHd.size(),plVtHd.data());
                }
            }
        }
    }
}

bool PolygonalMesh::CPUisLittleEndian(void) const
{
    unsigned int one=1;
    auto *dat=(const unsigned char *)&one;
    if(1==dat[0])
    {
        return true;
    }
    return false;
}
int PolygonalMesh::BinaryToInt(const unsigned char dw[4])
{
    int b0=(int)dw[0];
    int b1=(int)dw[1];
    int b2=(int)dw[2];
    int b3=(int)dw[3];
    return b0+b1*0x100+b2*0x10000+b3*0x1000000;
}
float PolygonalMesh::BinaryToFloat(const unsigned char dw[4])
{
    if(true==CPUisLittleEndian())
    {
        const float *fPtr=(const float *)dw;
        return *fPtr;
    }
    else
    {
        float value;
        auto *valuePtr=(unsigned char *)&value;
        valuePtr[0]=dw[3];
        valuePtr[1]=dw[2];
        valuePtr[2]=dw[1];
        valuePtr[3]=dw[0];
        return value;
    }
}


void PolygonalMesh::IntToBinary(unsigned char dw[4],const int b) const
{
    int b0 = b%0x100;
    int b1 = b/0x100%0x100;
    int b2 = b/0x10000%0x100;
    int b3 = b/0x1000000%0x100;
    
    dw[0] = b0;
    dw[1] = b1;
    dw[2] = b2;
    dw[3] = b3;
}
void PolygonalMesh::FloatToBinary(unsigned char dw[4], const float value) const
{
    if(true==CPUisLittleEndian())
    {
        auto *valuePtr=(unsigned char *)&value;
        dw[0] = valuePtr[0];
        dw[1] = valuePtr[1];
        dw[2] = valuePtr[2];
        dw[3] = valuePtr[3];
    }
    else
    {
        auto *valuePtr=(unsigned char *)&value;
        dw[3] = valuePtr[0];
        dw[2] = valuePtr[1];
        dw[1] = valuePtr[2];
        dw[0] = valuePtr[3];
    }
}

void PolygonalMesh::AddBinaryStlTriangle(const unsigned char buf[50])
{
    float nx=BinaryToFloat(buf),ny=BinaryToFloat(buf+4),nz=BinaryToFloat(buf+8);
    const YsVec3 nom(nx,ny,nz);

	const YsVec3 tri[3]=
	{
	    YsVec3(BinaryToFloat(buf+12),BinaryToFloat(buf+16),BinaryToFloat(buf+20)),
	    YsVec3(BinaryToFloat(buf+24),BinaryToFloat(buf+28),BinaryToFloat(buf+32)),
	    YsVec3(BinaryToFloat(buf+36),BinaryToFloat(buf+40),BinaryToFloat(buf+44)),
	};

//    printf("DBug: Added vertex x = %f y = %f z = %f\n",BinaryToFloat(buf+12),BinaryToFloat(buf+16),BinaryToFloat(buf+20));
//    printf("DBug: Added vertex x = %f y = %f z = %f\n",BinaryToFloat(buf+24),BinaryToFloat(buf+28),BinaryToFloat(buf+32));
//    printf("DBug: Added vertez x = %f y = %f z = %f\n",BinaryToFloat(buf+36),BinaryToFloat(buf+40),BinaryToFloat(buf+44));
    
	const VertexHandle triVtHd[3]=
	{
		AddVertex(tri[0]),
		AddVertex(tri[1]),
		AddVertex(tri[2]),
	};
//    printf("DBug: Added normal x = %f y = %f z = %f\n",nx,ny,nz);
	auto plHd=AddPolygon(3,triVtHd);
	SetPolygonColor(plHd,YsBlue());
	SetPolygonNormal(plHd,nom);
}

void PolygonalMesh::GetBinaryStlTriangle(unsigned char buf[50], const PolygonHandle plHd) const
{
    double x,y,z;
    GetPolygonNormal(plHd).Get(x, y, z);
    float nx=x,ny=y,nz=z;
//    printf("DBug: Getting normal %f %f %f\n",x,y,z);
    
    FloatToBinary(buf, nx);
    FloatToBinary(buf+4, ny);
    FloatToBinary(buf+8, nz);
    
    auto triangle = GetPolygonVertex(plHd);
    
    for (int i=0; i<3; i++) {
        GetVertexPosition(triangle[i]).Get(x, y, z);
        
        FloatToBinary(buf+12+12*i, (float)x);
        FloatToBinary(buf+16+12*i, (float)y);
        FloatToBinary(buf+20+12*i, (float)z);
    }
}

bool PolygonalMesh::SaveBinStl(const char fn[]) const
{
    FILE *fp = fopen(fn, "wb");
    
    if(nullptr!=fp)
    {
        unsigned char title[80] = "wing.stl from NACA information, ps6_2 Chris Cortez";
        fwrite(title,1,80,fp); // Write title
        
        unsigned char n_tri[4];
        IntToBinary(n_tri, (int)GetNumPolygon());
        fwrite(n_tri,4,1,fp);  // Write 4 bytes of position
        int i = 0;
        double x,y,z;
        
        for(auto plHd=NullPolygon(); true==PolygonalMesh::MoveToNextPolygon(plHd); )
        {
            unsigned char buf[50];
            GetBinaryStlTriangle(buf,plHd);
            fwrite(buf, 1, 50, fp);
            
            i++;
            GetVertexPosition(GetPolygonVertex(plHd)[0]).Get(x, y, z);
//            printf("DBug: Printing Polygon %d, x = %f y = %f z = %f\n", i,x,y,z);
        }
        
//        printf("DBug: Finished Printing, Num of Tri: %d\n",i);
        fclose(fp);
        return true;
    }
    
	return false;
}
