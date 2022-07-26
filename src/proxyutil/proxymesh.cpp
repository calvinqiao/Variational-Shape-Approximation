#include "proxymesh.h"
#include <unordered_set>
typedef ProxyContainer::ProxyHandle ProxyHandle;
//---------------------         Anchor Vertex           -------------------

AnchorVertex::AnchorVertex(VertexHandle v, int indx, PolygonalMesh* m)
{
    //initial constructor, very poor positioning
    vhd = v;
    index = indx;
    mesh = m;
    
    pos = mesh->GetVertexPosition(vhd);
}

AnchorVertex::AnchorVertex() {
}

YsVec3 AnchorVertex::GetPos() const {
    return pos;
}
int AnchorVertex::GetColor() const {
    return index;
}

VertexHandle AnchorVertex::GetVertex() const {
    return vhd;
}

void AnchorVertex::AddProxyMesh(ProxyMesh *pm) {
    if (proxset.find(pm->GetIndex()) == proxset.end()) {
        proxlist.push_back(pm);
        proxset.insert(pm->GetIndex());
        pm->AddAnchorVertex(*this);
    }
}

std::vector<ProxyMesh*> AnchorVertex::GetProxies() const {
    return proxlist;
}


//---------------------         ProxyMeshEdge           -------------------

Edge::Edge(std::vector<VertexHandle> v, std::vector<ProxyMesh*> pm) 
{
    edge_v = v;
    pmlist = pm;
}




//---------------------         ProxyMesh           -------------------

ProxyMesh::ProxyMesh(ProxyHandle phd, const ProxyContainer* pc, PolygonalMesh* m, int indx) {
    mesh = m;
    proxHd = phd;
    index = indx;
    
    vlist = pc->getProxyVertices(proxHd);
//    printf("DBug: Creating Proxy Mesh, size of vlist is %lu compared to pc list %lu\n",vlist.size(), pc->getProxyVertices(proxHd).size());
}
ProxyMesh::ProxyMesh(ProxyHandle phd, PolygonalMesh* m) {
    mesh = m;
    proxHd = phd;
}

void ProxyMesh::AddAnchorVertex(AnchorVertex av) {
    if (avset.find(av.GetColor()) == avset.end()) {
        avset.insert(av.GetColor());
        avlist.push_back(av);
    }
}
void ProxyMesh::AddEdge(Edge* e) {
    edgelist.push_back(e);
}
ProxyHandle ProxyMesh::getProxy() const {
    return proxHd;
}
std::vector<VertexHandle> ProxyMesh::GetVertices() const {
    return vlist;
}
std::vector<AnchorVertex> ProxyMesh::GetAnchorVertices() const {
    return avlist;
}
int ProxyMesh::GetIndex() const {
    return index;
}
bool ProxyMesh::operator <(const ProxyMesh& pm) {
    return this->index < pm.index;
}
bool ProxyMesh::operator >(const ProxyMesh& pm) {
    return this->index > pm.index;
}
bool ProxyMesh::operator==(const ProxyMesh& pm) {
    return this->index == pm.index;
}

//---------------------         Container           -------------------

ProxyMeshContainer::ProxyMeshContainer() {
}

ProxyMeshContainer::ProxyMeshContainer(ProxyContainer pc, PolygonalMesh* m, PolygonalMesh* fm) {
    //creates the new mesh within the constructor with no refinements or new mesh
    mesh = m;
    finalMesh = fm;
    proxyCont = &pc;
    std::vector<ProxyHandle> plist = pc.getProxies(); //needs copy constructor in proxy
    
    for (ProxyHandle p : plist) {
        ProxyMesh pm = ProxyMesh(p, &pc, m, pm_index);
        pm_index++;
        pmlist.push_back(pm);
    }
    printf("DBug: created pmlist inside pmcontainer\n");
    
    //Creation of anchor vertices here
    //find all intersections of 2 meshes
    for (int i=0; i<pmlist.size()-1; i++) {
        for (int j=i+1; j<pmlist.size(); j++) {
            findIntersection(&pmlist[i], &pmlist[j]);
//            printf("DBug: Intersection of pm%d, pm%d is %d (1==true, 0==false)\n",i,j,b1);
        }
    }
    printf("DBug: Found all (%lu) edges for (%lu) proxies\n",elist.size(),pmlist.size());
    
    //find all anchor vertices at ends of edges
    for (auto edge : elist) {
        CreateAnchorVertex(edge);
    }
    
    for (auto pair : avmap) {
        UpdatePos(pair.second);
    }
  
    //All original anchor vertices should be created now.
    printf("DBug: Found all (%d) anchor vertices\n", color_index-1);

    
    //Add extra edges and anchor vertices for the chord separation
    for (auto e : elist) {
        //Add function for recursively checking discretization
        Discritize(e);
    }
    
    
    //color each vertex with closest anchor vertex
    printf("DBug: Starting to match v with av\n");
    for (auto p: pmlist) {
        for (auto v: p.vlist) {
//            printf("DBug: Size of p.avlist is %lu\n", p.avlist.size());
            findClosestAnchor(v, p.avlist);
        }
    }
    printf("DBug: Finished matching v with av\n");
    
    //for color finding, create temporary hash map with colored anchor vertices
    std::unordered_map<int, AnchorVertex> av_colors_map;
    for (auto av_pair : avmap) {
        av_colors_map[av_pair.second.GetColor()] = av_pair.second;
    }
    
    //use colors to find and create triangles
    //find all polygons with 3 colors and create polygon with those three anchor vertices
    printf("DBug: Creating initial polygons\n");
    for (auto p : plist) {
        for (auto poly : pc.getProxyPolygons(p)) {
            std::unordered_set<int> color_set;
            std::vector<int> colors;
            for (auto v : mesh->GetPolygonVertex(poly)) {
                color_set.insert(vertexColors[mesh->GetSearchKey(v)]);
                colors.push_back(vertexColors[mesh->GetSearchKey(v)]);
//                printf("DBug: Added color %d\n",vertexColors[mesh->GetSearchKey(v)]);
            }
//            printf("DBug: Size of colors is %lu\n",color_set.size());
            if (color_set.size() == 3) {
                auto v1 = finalMesh->AddVertex(av_colors_map[colors[0]].GetPos());
                auto v2 = finalMesh->AddVertex(av_colors_map[colors[1]].GetPos());
                auto v3 = finalMesh->AddVertex(av_colors_map[colors[2]].GetPos());
                VertexHandle vtArr[] = {v1, v2, v3};
                auto plhd = finalMesh->AddPolygon(3, vtArr);
//                printf("DBug: Added polygon to final mesh for colors %d, %d, %d\n",colors[0],colors[1],colors[2]);
                finalMesh->SetPolygonNormal(plhd, pc.getProxyNormal(p));
                finalMesh->SetPolygonColor(plhd, pc.getProxyColor(p));
            }
        }
    }
    printf("DBug: Finished creating final mesh with (%lld) polygons\n",finalMesh->GetNumPolygon());
//    YsVec3 point = finalMesh->GetVertexPosition(finalMesh->FindFirstVertex());
//    printf("DBug: Example vertex in final mesh: %f %f %f\n",point.x(),point.y(),point.z());

}

void ProxyMeshContainer::AddAnchorVertex(AnchorVertex av)
{
    avmap[mesh->GetSearchKey(av.GetVertex())] = av;
}

ProxyMesh ProxyMeshContainer::findProxyMesh(ProxyHandle pHd) const
{
    for (auto p : pmlist)
    {
        if (pHd == p.getProxy()) {
            return p;
        }
    }
    return NullProxyMesh();
}

ProxyMesh ProxyMeshContainer::NullProxyMesh() const
{
    ProxyHandle pHd = ProxyHandle();
    return ProxyMesh(pHd, mesh);
}


bool ProxyMeshContainer::findIntersection(ProxyMesh* pm1, ProxyMesh* pm2)
{
    std::vector<VertexHandle> edge_vert;
//    printf("DBug: Finding intersection of proxies %d and %d: ",pm1->GetIndex(), pm2->GetIndex());
    for (auto v1 : pm1->GetVertices())
    {
        for (auto v2 : pm2->GetVertices())
        {
            if (v1 == v2) {
                edge_vert.push_back(v1);
            }
        }
    }
    
    if(edge_vert.size() > 1)
    {
        std::vector<ProxyMesh*> pm;
        pm.push_back(pm1);
        pm.push_back(pm2);
        Edge E = Edge(edge_vert,pm);
        pm1->AddEdge(&E);
        pm2->AddEdge(&E);
        elist.push_back(E);
//        printf("True\n");
        return true;
    }
//    printf("False\n");
    return false;
}


void ProxyMeshContainer::findClosestAnchor(VertexHandle v, std::vector<AnchorVertex> a_vec) {
    //check if already found closest anchor
    if (vertexColors.find(mesh->GetSearchKey(v)) != vertexColors.end()) {
//        printf("DBug: already in vertexColors\n");
        return;
    }
    
    YsVec3 vpos = mesh->GetVertexPosition(v);
    YsVec3 min,max;
    mesh->GetBoundingBox(min, max);
    double min_dist = Distance(min,max);
    int min_indx = 0;
    
//    printf("DBug: Max distance is %f, vpos is %f, %f, %f\n",min_dist, vpos.x(),vpos.y(),vpos.z());
    for (auto av : a_vec) {
        YsVec3 avpos = av.GetPos();
        double temp_dist = Distance(vpos, avpos);
//        printf("DBug: Temp dist is %f, av pos is %f, %f, %f\n",temp_dist,avpos.x(), avpos.y(), avpos.z());
        if (temp_dist < min_dist) {
            min_dist = temp_dist;
            min_indx = av.GetColor();
        }
    }
//    printf("DBug: Min distance is %f\n",min_dist);
    
    vertexColors[mesh->GetSearchKey(v)] = min_indx;
}

double ProxyMeshContainer::Distance(YsVec3 a, YsVec3 b) const {
    return sqrt((a.x()-b.x())*(a.x()-b.x()) + (a.y()-b.y())*(a.y()-b.y()) + (a.z()-b.z())*(a.z()-b.z()));
}

void ProxyMeshContainer::CreateAnchorVertex(Edge e) {
    VertexHandle front, back;
    FindEnds(front, back, e);
    AnchorVertex av;
    
    //check if front not yet in map
    if (avmap.find(mesh->GetSearchKey(front)) == avmap.end()) {
        //create new anchor vertex
        av = AnchorVertex(front, color_index, mesh);
        AddAnchorVertex(av);
        color_index++;
    }
    //add proxies regardless
    for (auto prox : e.pmlist) {
        avmap.at(mesh->GetSearchKey(front)).AddProxyMesh(prox);
    }
    
    //repeat for back
    if (avmap.find(mesh->GetSearchKey(back)) == avmap.end()) {
        //create new anchor vertex
        av = AnchorVertex(back, color_index, mesh);
        AddAnchorVertex(av);
        color_index++;
    }
    //add proxies regardless
    for (auto prox : e.pmlist) {
        avmap.at(mesh->GetSearchKey(back)).AddProxyMesh(prox);
    }
}

std::vector<YsVec3> ProxyMeshContainer::GetAnchorVertexPositions() const {
    std::vector<YsVec3> result;
    for (auto av : avmap) {
        result.push_back(av.second.GetPos());
    }
    return result;
}

void ProxyMeshContainer::UpdatePos(AnchorVertex &av) {
    YsVec3 pos_update = YsVec3(0.0,0.0,0.0);
    for (auto prox :  av.GetProxies()) {
        pos_update += Project(mesh->GetVertexPosition(av.GetVertex()), prox->getProxy());
    }
    pos_update /= (double)(av.GetProxies().size());
    av.pos = pos_update;
//    printf("DBug: Updated %d position to %f, %f, %f\n",av.GetColor(), av.pos.x(),av.pos.y(),av.pos.z());
}

YsVec3 ProxyMeshContainer::Project(YsVec3 point, ProxyHandle plane) const {
    YsVec3 center = proxyCont->getProxyCenter(plane);
    YsVec3 norm = proxyCont->getProxyNormal(plane);
    YsVec3 v = point - center;
    norm.Normalize();
    double dist_dot = v.x()*norm.x() + v.y()*norm.y() + v.z()+norm.z();
    //    printf("DBug: Projection calculation dist %f dot %f\n",dist, dist_dot);
    return point - dist_dot*norm;
}

void ProxyMeshContainer::FindEnds(VertexHandle &front, VertexHandle &back, Edge e) const {
    double max_dist = 0;
    for (int i=0; i<e.edge_v.size()-1; i++) {
        VertexHandle v1 = e.edge_v[i];
        for (int j=i+1; j<e.edge_v.size(); j++) {
            VertexHandle v2 = e.edge_v[j];
            double temp_dist = Distance(mesh->GetVertexPosition(v1), mesh->GetVertexPosition(v2));
            if (temp_dist > max_dist) {
                max_dist = temp_dist;
                front = v1;
                back = v2;
            }
        }
    }
}

void ProxyMeshContainer::Discritize(Edge &e) {
    VertexHandle front, back;
    FindEnds(front, back, e);
    YsVec3 a,b,n1,n2;
    a = mesh->GetVertexPosition(front);
    b = mesh->GetVertexPosition(back);
    double chord_length = (a-b).GetLength();
    
    VertexHandle max_point;
    double dist_max = 0;
    
    //get longest distance to the edge
    for (auto v : e.edge_v) {
        double dist_temp = Project(a, b, mesh->GetVertexPosition(v));
        if (dist_temp > dist_max) {
            dist_max = dist_temp;
            max_point = v;
        }
    }
    
    //get critereon
    if (e.pmlist.size() != 2) {
        printf("DBug: Error, edge doesn't have 2 associated proxies\n");
        return;
    }
    n1 = proxyCont->getProxyNormal(e.pmlist.front()->getProxy());
    n2 = proxyCont->getProxyNormal(e.pmlist.back()->getProxy());
    double sin_normals = (n1^n2).GetLength() / (n1.GetLength() * n2.GetLength());
    
    double threshold = EDGE_THRESHOLD;
    if ((dist_max*sin_normals/chord_length) >= threshold) {
        //cut edge and recursively do this again with the two new edges
        //Need to know order of vertices on the edge but don't know that right now. 
    }
        
}

double ProxyMeshContainer::Project(YsVec3 a, YsVec3 b, YsVec3 p) const {
    return 0.0;
}
