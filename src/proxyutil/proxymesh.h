#ifndef PROXYMESH_IS_INCLUDED
#define PROXYMESH_IS_INCLUDED

#include <vector>
#include <polygonalmesh.h>
#include <ysclass.h>
#include "proxy.h"
#include <unordered_set>

#define EDGE_THRESHOLD 1.0;

typedef ProxyContainer::ProxyHandle ProxyHandle;
class ProxyMesh;

//mesh is in main program, or can be added to container class

class AnchorVertex
{
friend class ProxyMeshContainer;
private:
    PolygonalMesh* mesh;
    VertexHandle vhd; //original vertex position
    int index; //"Color" assignment
    std::vector<ProxyMesh*> proxlist;
    std::unordered_set<int> proxset;
    YsVec3 pos;
    
public:
    AnchorVertex(VertexHandle vertex, int index, PolygonalMesh* mesh); //creates Anchor
    AnchorVertex();
    
    //accessors
    YsVec3 GetPos() const;
    int GetColor() const;
    std::vector<ProxyMesh*> GetProxies() const;
    VertexHandle GetVertex() const;
    
    //not implemented yet
    void AddProxyMesh(ProxyMesh* pm);
    
};

class Edge
//Edge vertices of proxies
{
friend class ProxyMeshContainer;
protected:
    std::vector<VertexHandle> edge_v; //tracks vertex in edge
    std::vector<ProxyMesh*> pmlist; //tracks proxies attatched to edge
    
public:
    Edge(std::vector<VertexHandle> v, std::vector<ProxyMesh*> pm);
};


class ProxyMesh
//Mesh of Single Proxy
{
friend class ProxyMeshContainer;
private:
    PolygonalMesh* mesh; //pointer to original mesh
    ProxyHandle proxHd; //pointer to proxy
    std::unordered_set<int> avset; //set of associated anchor vertecies
    std::vector<AnchorVertex> avlist; //list of associated anchor vertecies
    std::vector<VertexHandle> vlist; //list of all vertices on proxy
    std::vector<Edge*> edgelist; //list of all accociated edges
    int index; //for sorting or set
    
protected:
    ProxyMesh(ProxyHandle phd, PolygonalMesh* m); //null mesh
    
public:
    ProxyMesh(ProxyHandle phd, const ProxyContainer* pc, PolygonalMesh* mesh, int indx); //constructor
    void AddAnchorVertex(AnchorVertex av); //adds to avlist
    void AddEdge(Edge* e); //adds to elist
    ProxyHandle getProxy() const; //accessor
    int GetIndex() const; //accesor
    std::vector<VertexHandle> GetVertices() const; //accessor
    std::vector<AnchorVertex> GetAnchorVertices() const; //accessor
    bool operator <(const ProxyMesh& pm); //unused
    bool operator >(const ProxyMesh& pm); //unused
    bool operator ==(const ProxyMesh& pm); //unused
};

class ProxyMeshContainer
{
private:
    PolygonalMesh* mesh; //original mesh
    PolygonalMesh* finalMesh; //final mesh
    ProxyContainer* proxyCont; //proxy container
    std::vector<ProxyMesh> pmlist; //list of proxy meshes
    std::unordered_map<YSHASHKEY, AnchorVertex> avmap; //hash map of anchors with vertex as hashkey
    std::vector<Edge> elist; //list of all edges
    int color_index = 1; //"color" of anchor vertices, increases by 1 for each av
    int pm_index = 0; //index coutner of proxymeshes
    
    std::unordered_map<YSHASHKEY,int> vertexColors;
    
    
protected:
    bool findIntersection(ProxyMesh* pm1, ProxyMesh* pm2); //creates an edge if true
    void findClosestAnchor(VertexHandle v, std::vector<AnchorVertex> avList); //used in flooding triangluation
    double Distance(YsVec3 a, YsVec3 b) const; //helpful function
    void CreateAnchorVertex(Edge e);
    
public:
    ProxyMeshContainer(); //null container for default construction
    ProxyMeshContainer(ProxyContainer container, PolygonalMesh* mesh, PolygonalMesh* fMesh);
    void AddAnchorVertex(AnchorVertex av); //adds to avmap
    ProxyMesh NullProxyMesh() const;
    
    ProxyMesh findProxyMesh(ProxyHandle pHd) const; //finds mesh by proxy handle
    void findIntersection(ProxyMesh pm1, ProxyMesh pm2, ProxyMesh pm3); //finds anchor vertex at intersection
    //above function might be replaced
    
    std::vector<YsVec3> GetAnchorVertexPositions() const; //accessor
    
    //anchor vertex helper
    void UpdatePos(AnchorVertex &av);
    YsVec3 Project(YsVec3 point, const ProxyHandle plane) const;
    
    //Edge helper
    void FindEnds(VertexHandle &front, VertexHandle &back, Edge e) const;
    void Discritize(Edge &e);
    double Project(YsVec3 a, YsVec3 b, YsVec3 p) const;
};


#endif
