#ifndef POLYGONALMESH_H_IS_INCLUDED
#define POLYGONALMESH_H_IS_INCLUDED


#include <list>
#include <unordered_map>
#include <ysclass.h>


class PolygonalMesh
{
public:
	enum
	{
		NullSearchKey=0x7fffffff
	};

private:
	unsigned int searchKeySeed;

protected:
	class Vertex
	{
	public:
		unsigned int searchKey;
		YsVec3 pos;
	};
private:
	mutable std::list <Vertex> vtxList;
public:
	class VertexHandle
	{
	friend class PolygonalMesh;
	private:
		std::list <Vertex>::iterator iter;
	public:
		bool operator==(VertexHandle incoming)
		{
			return iter==incoming.iter;
		}
		bool operator!=(VertexHandle incoming)
		{
			return iter!=incoming.iter;
		}
	};

private:
	std::unordered_map <unsigned int,VertexHandle> vtxSearch;

protected:
	class Polygon
	{
	public:
		std::vector <VertexHandle> vtHdAry;
		YsVec3 nom;
		YsColor col;
		unsigned int searchKey;
	};
private:
	mutable std::list <Polygon> plgList;
public:
	class PolygonHandle
	{
	friend PolygonalMesh;
	private:
		std::list <Polygon>::iterator iter;
	public:
		bool operator==(PolygonHandle incoming)
		{
			return iter==incoming.iter;
		}
		bool operator!=(PolygonHandle incoming)
		{
			return iter!=incoming.iter;
		}
	};

private:
	std::unordered_map <unsigned int,PolygonHandle> plgSearch;
	std::unordered_map <unsigned int,std::vector <PolygonHandle> > vtxToPlg;

public:
	PolygonalMesh();
	~PolygonalMesh();
	void CleanUp(void);

private:
	unsigned int GetInteter(const unsigned char dat[4])
	{
		unsigned int b0=dat[0];
		unsigned int b1=dat[1];
		unsigned int b2=dat[2];
		unsigned int b3=dat[3];
		return b0+b1*0x100+b2*0x10000+b3*0x1000000;
	}

	bool CPUIsLittleEndian(void)
	{
		int a=1;
		unsigned char *b=((unsigned char *)&a);
		if(1==b[0])
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	float GetFloat(const unsigned char dat[4])
	{
		if(true==CPUIsLittleEndian())
		{
			const float *fPtr=(const float *)dat;
			return *fPtr;
		}
		else
		{
			unsigned char flip[4]={dat[3],dat[2],dat[1],dat[0]};
			const float *fPtr=(const float *)flip;
			return *fPtr;
		}
	}

public:
	void GetBoundingBox(YsVec3 &min,YsVec3 &max) const;
	void GetBoundingBox(YsVec3 bbx[2]) const;

	long long int GetNumVertex(void) const;
	VertexHandle FindFirstVertex(void) const;
	VertexHandle FindNextVertex(VertexHandle vtHd) const;

	VertexHandle AddVertex(const YsVec3 &pos);
	void DeleteVertex(VertexHandle vtHd);
	void DeleteUnusedVertex(void);
	unsigned int GetSearchKey(VertexHandle vtHd) const;
	VertexHandle FindVertex(unsigned int searchKey) const;
	VertexHandle NullVertex(void) const;
	YsVec3 GetVertexPosition(VertexHandle vtHd) const;
	void SetVertexPosition(VertexHandle vtHd,const YsVec3 &newPos);


	long long int GetNumPolygon(void) const;
	PolygonHandle FindFirstPolygon(void) const;
	PolygonHandle GetNPolygon(int n) const;
	PolygonHandle FindNextPolygon(PolygonHandle plHd) const;
	bool MoveToNextPolygon(PolygonHandle &plHd) const;

	PolygonHandle AddPolygon(long long int nVtx,const VertexHandle vtHdAry[]);
	void SetPolygonNormal(PolygonHandle plHd,const YsVec3 &nom);
	unsigned int GetSearchKey(PolygonHandle plHd) const;
	PolygonHandle FindPolygon(unsigned int searchKey) const;
	PolygonHandle NullPolygon(void) const;
	void SetPolygonVertex(PolygonHandle plHd,long long int nVtx,const VertexHandle vtHdAry[]);

	void SetPolygonColor(PolygonHandle plHd,YsColor col);
	YsColor GetPolygonColor(PolygonHandle plHd) const;
	YsVec3 GetPolygonCenter(PolygonHandle plHd) const;
	double GetPolygonArea(PolygonHandle plhd) const;
	std::vector <VertexHandle> GetPolygonVertex(PolygonHandle plHd) const;
	YsVec3 GetPolygonNormal(PolygonHandle plHd) const;
	double GetPolygonHashValue(PolygonHandle plhd);

	std::vector <PolygonHandle> FindPolygonFromVertex(VertexHandle vtHd) const;
	std::vector <VertexHandle> GetConnectedVertex(VertexHandle fromVtHd) const;
	std::vector <PolygonHandle> FindPolygonFromEdgePiece(VertexHandle edVtHd0,VertexHandle edVtHd1) const;
	PolygonHandle GetNeighborPolygon(PolygonHandle plHd,int edIdx) const;
	std::vector <PolygonHandle> FindNNeighbors(PolygonHandle plHd0,int nNei) const;

	bool SaveBinStl(const char fn[]) const;
	bool LoadBinStl(const char fn[]);

private:
	bool CPUisLittleEndian(void) const;
	int BinaryToInt(const unsigned char dw[4]);
	float BinaryToFloat(const unsigned char dw[4]);
    void IntToBinary(unsigned char dw[4], const int value) const;
    void FloatToBinary(unsigned char dw[4], const float value) const;
	void AddBinaryStlTriangle(const unsigned char buf[50]);
    void GetBinaryStlTriangle(unsigned char buf[50], const PolygonHandle plHd) const;
    void StitchVertexN2(void);
    void StitchVertex(void);
};


#endif




