#include "proxy.h"
#include <math.h>
#include <stdexcept>
#include <queue>
#include <unordered_set>
#include <stdio.h>

struct Triangle
{
    double error;
    int i_proxy;
    PolygonHandle plhd;
};

class PQCompare
{
public:
    bool operator() (Triangle left, Triangle right){
        return left.error > right.error;
    }
};

void Proxy::clearPolygons()
{
    plhd_vec.clear(); 
}

void Proxy::addPolygon(PolygonHandle plhd)
{
    plhd_vec.push_back(plhd);
}

ProxyContainer::ProxyContainer(int N)
{
    n_proxies = N;
    proxy_arr.reserve(N);
}

void ProxyContainer::setMesh(PolygonalMesh* mesh_in)
{
    mesh = mesh_in;
}

double ProxyContainer::computeDistance(PolygonHandle plhd, int px_idx)
{
//    printf("DBug: Started compute Distance\n");
    YsVec3 plg_normal = mesh->GetPolygonNormal(plhd);
    YsVec3 proxy_normal = proxy_arr[px_idx].normal;
    YsVec3 temp = plg_normal-proxy_normal;
    double dist_normal_squared = temp.GetSquareLength();

    double area = mesh->GetPolygonArea(plhd);

//    printf("DBug: Finished compute distance\n");
    return dist_normal_squared*area;
}

double ProxyContainer::computeDistanceL2(PolygonHandle plhd, int px_idx)
{
    auto proxy_normal = proxy_arr[px_idx].normal;
    auto a = proxy_normal[0]; auto b = proxy_normal[1]; auto c = proxy_normal[2];
    auto proxy_center = proxy_arr[px_idx].center;
    auto x = proxy_center[0]; auto y = proxy_center[1]; auto z = proxy_center[2];
    auto d = -(a*x + b*y + c*z);

    auto vertex_hds = GetPolygonVertex(plhd);
    auto v1 = GetVertexPosition(vertex_hds[0]);
    auto v2 = GetVertexPosition(vertex_hds[1]);
    auto v3 = GetVertexPosition(vertex_hds[2]);

    auto sqrt_a2b2c2 = sqrt(a*a+b*b+c*c);

    auto d1 = (a*v1[0]+b*v1[1]+c*v1[2]+d)/sqrt_a2b2;
    auto d2 = (a*v2[0]+b*v2[1]+c*v2[2]+d)/sqrt_a2b2;
    auto d3 = (a*v3[0]+b*v3[1]+c*v3[2]+d)/sqrt_a2b2;

    double area = mesh->GetPolygonArea(plhd);
    
    return area*(d1*d1+d2*d2+d3*d3+d1*d2+d1*d3+d2*d3)/6;
}

void ProxyContainer::initializeProxies()
{
    // PolygonalMesh *mesh
    // std::vector<Proxy*> proxy_arr
    // get all the polygons
    int num_polygon = mesh->GetNumPolygon();
    int numProxy = n_proxies;
    int index = 0;
    std::unordered_set<int> chosen_indices;
    while (numProxy != 0) {
        int nth_polygon;
        while (1) {
            nth_polygon = std::rand() % num_polygon;
            if (chosen_indices.count(nth_polygon) == 0) {
                chosen_indices.insert(nth_polygon);
                break;
            }
        }
        
        // call GetNPolygon
        PolygonHandle plhd = mesh->GetNPolygon(nth_polygon);   
        YsVec3 p_normal = mesh->GetPolygonNormal(plhd);
        YsVec3 p_center = mesh->GetPolygonCenter(plhd);

        Proxy p;
        p.normal = p_normal;
        p.center = p_center;
        proxy_arr.push_back(p);
        ++index;
        --numProxy;
    }
}

double ProxyContainer::partitionPolygons()
{
    std::priority_queue<Triangle, std::vector<Triangle>, PQCompare> triangle_queue;
    std::unordered_set<YSHASHKEY> assigned_set;

    PolygonHandle curr_plhd;
    double curr_dist, min_dist;
    unsigned int code;
    int plhdCounter;
    Triangle top_triangle;
    std::vector <PolygonHandle> neighbor_vec;
    std::vector <Triangle> seed_vec;

    double total_error = 0.;
    
    // Find polygons closest to each proxy
    for (int i=0; i<n_proxies; i++) {
        PolygonHandle closestPlhd; 
        curr_plhd = mesh->FindFirstPolygon();

        // Clear current polygons
        proxy_arr[i].clearPolygons();

        // Find best polygon
        plhdCounter = 0;
        while (curr_plhd != mesh->NullPolygon()) {
//          printf("DBug: plhdCounter size: %d out of %lld, compute\n",plhdCounter, mesh->GetNumPolygon());
            curr_dist = computeDistance(curr_plhd, i);
            code = mesh->GetSearchKey(curr_plhd);
            if (assigned_set.find(code) == assigned_set.end() && (plhdCounter == 0 || curr_dist < min_dist)) {
                closestPlhd = curr_plhd;
                min_dist = curr_dist;
                plhdCounter += 1;
            }
            mesh->MoveToNextPolygon(curr_plhd);
        }

        // Add best polygon as seed
        proxy_arr[i].addPolygon(closestPlhd);
        total_error += min_dist;
        assigned_set.insert(mesh->GetSearchKey(closestPlhd));
        
        Triangle triangle;
        triangle.error = min_dist;
        triangle.i_proxy = i;
        triangle.plhd = closestPlhd;
        seed_vec.push_back(triangle);
    }

    // Initialize queue with neighbors of seed
    for (auto triangle : seed_vec) {
        neighbor_vec = mesh->FindNNeighbors(triangle.plhd, 3);
        for (int j=0; j<neighbor_vec.size(); j++) {
            Triangle new_triangle;
            new_triangle.plhd = neighbor_vec[j];
            new_triangle.i_proxy = triangle.i_proxy;
            new_triangle.error = computeDistance(neighbor_vec[j], new_triangle.i_proxy);
            triangle_queue.push(new_triangle);
        }
    }
    
    // Assign polygons to proxies by flooding
    while (!triangle_queue.empty()) {
        // Pop top element
//        printf("DBug: Triangle Queue size %lu\n",triangle_queue.size());
        top_triangle = triangle_queue.top();
        triangle_queue.pop();
        code = mesh->GetSearchKey(top_triangle.plhd);

        if (assigned_set.find(code) == assigned_set.end()) {
            // Assign to proxy
            proxy_arr[top_triangle.i_proxy].addPolygon(top_triangle.plhd);

            // Update stats
            total_error += top_triangle.error;
            assigned_set.insert(code);

            // Find and insert neighboring polygons
            neighbor_vec.clear();
            neighbor_vec = mesh->FindNNeighbors(top_triangle.plhd, 3);
//            printf("DBug: Neighbors size is %d",neighbor_vec.size());
            for (int j=0; j<neighbor_vec.size(); j++) {
                Triangle new_triangle;
                new_triangle.plhd = neighbor_vec[j];
                new_triangle.i_proxy = top_triangle.i_proxy;
//                printf("DBug: vertex size %lu\n", mesh->GetPolygonVertex(neighbor_vec[j]).size());
//                printf("DBug: plhd = %d, j = %d\n", mesh->GetSearchKey(neighbor_vec[j]),j);
                if (mesh->GetPolygonVertex(neighbor_vec[j]).size() == 0) {
                    continue;
                }
                new_triangle.error = computeDistance(neighbor_vec[j], new_triangle.i_proxy);
                triangle_queue.push(new_triangle);
            }
//            printf("DBug: Exited for \n");
        }
//        printf("DBug: Exited if\n");
    }
//    printf("DBug: Ending while loop in partition\n");
    return total_error;
}

void ProxyContainer::fitProxies()
{
    /* L^{2,1} method
     *    Center of proxy => barycenter of polygons
     *    Normal of proxy => area weighted average of polygon normals
     */
    YsVec3 plg_center, plg_normal;
    double plg_area, total_area;
    total_area = 0.0;
    for (int i=0; i<n_proxies; i++) {
        // Reset values
        proxy_arr[i].center = YsVec3::Origin();
        proxy_arr[i].normal = YsVec3::Origin();

        // Sum over polygons to obtain numerator
        for (int j=0; j<proxy_arr[i].plhd_vec.size(); j++) {
            // Find polygon parameters
            plg_normal = mesh->GetPolygonNormal(proxy_arr[i].plhd_vec[j]);
            plg_center = mesh->GetPolygonCenter(proxy_arr[i].plhd_vec[j]);
            plg_area = mesh->GetPolygonArea(proxy_arr[i].plhd_vec[j]);

            // Add to nominator
            proxy_arr[i].center = proxy_arr[i].center + plg_area*plg_center;
            proxy_arr[i].normal = proxy_arr[i].normal + plg_area*plg_normal;

            // Update statistics
            total_area += plg_area;
        }

        // Divide by denominator
        proxy_arr[i].center /= total_area;
        proxy_arr[i].normal.Normalize();
    }
}

void ProxyContainer::generateProxies(double convergence_thres, int max_iters)
{
    int iters = 0;
    double prev_error, error;

    initializeProxies();
    prev_error = -1;
    while (iters < max_iters) {
        error = partitionPolygons();
//        printf("DBug: On iter %d, error %f\n",iters,error);
        fitProxies();
        iters++;

        // Convergence
        if (abs(prev_error - error) < convergence_thres) {
            break;
        } else {
            prev_error = error;
        }
    }
    error = partitionPolygons();
//    printf("DBug: On iter %d, error %f\n",iters,error);
}

int ProxyContainer::getNumProxies() const
{
    return n_proxies;
}

std::vector<ProxyContainer::ProxyHandle> ProxyContainer::getProxies() const
{
    std::vector<ProxyContainer::ProxyHandle> px_vec;

    for (int i=0; i<n_proxies; i++){
        ProxyContainer::ProxyHandle pxhd;
        pxhd.idx = i;
        px_vec.push_back(pxhd);
    }

    return px_vec;
}

YsVec3 ProxyContainer::getProxyCenter(ProxyContainer::ProxyHandle pxhd) const
{
    return proxy_arr[pxhd.idx].center;
}

YsVec3 ProxyContainer::getProxyNormal(ProxyContainer::ProxyHandle pxhd) const
{
    return proxy_arr[pxhd.idx].normal;
}

YsColor ProxyContainer::getProxyColor(ProxyContainer::ProxyHandle pxhd) const
{
    return proxy_arr[pxhd.idx].color;
}

void ProxyContainer::setProxyColor(ProxyContainer::ProxyHandle pxhd, YsColor color)
{
    proxy_arr[pxhd.idx].color = color;
}

std::vector<PolygonHandle> ProxyContainer::getProxyPolygons(ProxyContainer::ProxyHandle pxhd) const
{
    std::vector<PolygonHandle> pl_vec;

    for (auto plhd : proxy_arr[pxhd.idx].plhd_vec){
        pl_vec.push_back(plhd);
    }

    return pl_vec;
}

std::vector<VertexHandle> ProxyContainer::getProxyVertices(ProxyContainer::ProxyHandle pxhd) const
{
    std::vector<VertexHandle> vvec;
    std::unordered_set<unsigned int> assigned_set;
    unsigned int key;
    
    for (auto plhd : getProxyPolygons(pxhd)){
        auto tmp_vvec = mesh->GetPolygonVertex(plhd);

        for (int j=0; j<tmp_vvec.size(); j++){
            key = mesh->GetSearchKey(tmp_vvec[j]);

            if (assigned_set.find(key) == assigned_set.end()) {
                vvec.push_back(tmp_vvec[j]);
                assigned_set.insert(key);
            }
        }
    }

    return vvec;
}

bool ProxyContainer::polygonInProxy(PolygonHandle plhd, ProxyContainer::ProxyHandle pxhd) const
{
    bool output = false;
    const std::vector<PolygonHandle>* vec_ptr = &(proxy_arr[pxhd.idx].plhd_vec);

    for (int i=0; i<vec_ptr->size(); i++) {
        if (mesh->GetSearchKey(vec_ptr->at(i)) == mesh->GetSearchKey(plhd)) {
            output = true;
            break;
        }
    }

    return output;
}
