#ifndef PROXY_IS_INCLUDED
#define PROXY_IS_INCLUDED

#include <vector>
#include <polygonalmesh.h>
#include <ysclass.h>

typedef PolygonalMesh::PolygonHandle PolygonHandle;
typedef PolygonalMesh::VertexHandle VertexHandle;

class Proxy
{
friend class ProxyContainer;
private:
    std::vector<PolygonHandle> plhd_vec; // Associated polygons
    YsVec3 center;
    YsVec3 normal;
    YsColor color;

private:
    void clearPolygons(); // Clear associated polygons
    void addPolygon(PolygonHandle); // Add polygon to associated polygons
};

class ProxyContainer
{
private:
    std::vector<Proxy> proxy_arr;
    PolygonalMesh *mesh;
    int n_proxies;

public:
    class ProxyHandle
    {
        friend class ProxyContainer;
        private:
            int idx;
        public:
            ProxyHandle(void) {};
            ProxyHandle(const ProxyHandle &incoming) {idx = incoming.idx;};
            bool operator==(ProxyHandle incoming) {return idx == incoming.idx;};
            bool operator!=(ProxyHandle incoming) {return idx != incoming.idx;};
    };

public:
    ProxyContainer(int);

    void setMesh(PolygonalMesh*); // Assign mesh
    void generateProxies(double, int); // Perform Lloyd's algorithm

    int getNumProxies(void) const; // Returns number of proxies
    std::vector<ProxyHandle> getProxies() const; // Returns vector of proxies
    YsVec3 getProxyCenter(ProxyHandle) const; // Returns proxy center
    YsVec3 getProxyNormal(ProxyHandle) const; // Returns proxy normal
    YsColor getProxyColor(ProxyHandle) const; // Returns proxy color
    void setProxyColor(ProxyHandle, YsColor); // Sets proxy color
    std::vector<PolygonHandle> getProxyPolygons(ProxyHandle) const; // List of polygons associated with proxy
    std::vector<VertexHandle> getProxyVertices(ProxyHandle) const; // List of vertices associated with proxy
    bool polygonInProxy(PolygonHandle, ProxyHandle) const; // Check if polygon is in proxy

private:
    double computeDistance(PolygonHandle, int); // Distance measure between polygon and proxy
    void initializeProxies(void); // Generates initial set of proxies from mesh
    double partitionPolygons(void); // Maximization step: Assigns polygons to proxies using flooding
    void fitProxies(void); // Expectation step: Fits new proxies to the polygon clusters
};


#endif
