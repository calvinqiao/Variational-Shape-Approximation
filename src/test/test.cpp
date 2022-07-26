#include <stdio.h>
#include "polygonalmesh.h"
#include "proxy.h"

#define NUM_PROXIES 6
#define TOLERANCE 0.001
#define MAX_ITERS 20

int main(int argc,char *argv[])
{
	PolygonalMesh mesh;
//    MakeCube(mesh);
    if (2<=argc) {
        mesh.LoadBinStl(argv[1]);
    }
    else {
        printf("DBug: couldnt read stl\n");
        return 0;
    }
    
    // Debug stl file
//    PolygonalMesh::PolygonHandle curr_plhd = mesh.FindFirstPolygon();
//    unsigned int key;
//    YsVec3 pos;
//    while (curr_plhd != mesh.NullPolygon()) {
//        key = mesh.GetSearchKey(curr_plhd);
//        pos = mesh.GetPolygonCenter(curr_plhd);
//        printf("%d: %f, %f, %f\n", key, pos[0], pos[1], pos[2]);
//        mesh.MoveToNextPolygon(curr_plhd);
//    }
    
//    // Debug stl file
//    auto curr_vthd = mesh.FindFirstVertex();
//    unsigned int key;
//    while (curr_vthd != mesh.NullVertex()) {
//        auto plvec = mesh.FindPolygonFromVertex(curr_vthd);
//        printf("%d\n", plvec.size());
//        mesh.FindNextVertex(curr_vthd);
//    }

    // ProxyContainer proxy_obj = ProxyContainer(NUM_PROXIES);
    // proxy_obj.setMesh(&mesh);
    // proxy_obj.generateProxies(TOLERANCE, MAX_ITERS);

    
}
