#include <vector>
#include <iostream>
#include <ysclass.h>
#include <math.h>

#include <fslazywindow.h>

#include "polygonalmesh.h"
#include "proxy.h"
#include "proxymesh.h"


#define NUM_PROXIES 10
#define TOLERANCE 0.001
#define MAX_ITERS 20
#define PI 3.14159265

class FsLazyWindowApplication : public FsLazyWindowApplicationBase
{
protected:
	bool needRedraw;

	YsMatrix4x4 Rc;
	double d;
	YsVec3 t;

	PolygonalMesh mesh;
    PolygonalMesh finalMesh;
    PolygonalMesh initialMesh;
    ProxyContainer px_container = ProxyContainer(NUM_PROXIES);
    ProxyMeshContainer pm_container;
    bool created_pm = false;
	std::vector <float> vtx,nom,col;
	YsVec3 bbx[2];

	static void AddColor(std::vector <float> &col,float r,float g,float b,float a);
	static void AddVertex(std::vector <float> &vtx,float x,float y,float z);
	static void AddNormal(std::vector <float> &nom,float x,float y,float z);

	void RemakeVertexArray(void);

public:
	FsLazyWindowApplication();
	virtual void BeforeEverything(int argc,char *argv[]);
	virtual void GetOpenWindowOption(FsOpenWindowOption &OPT) const;
	virtual void Initialize(int argc,char *argv[]);
	virtual void Interval(void);
	virtual void BeforeTerminate(void);
	virtual void Draw(void);
	virtual bool UserWantToCloseProgram(void);
	virtual bool MustTerminate(void) const;
	virtual long long int GetMinimumSleepPerInterval(void) const;
	virtual bool NeedRedraw(void) const;
};

/* static */ void FsLazyWindowApplication::AddColor(std::vector <float> &col,float r,float g,float b,float a)
{
	col.push_back(r);
	col.push_back(g);
	col.push_back(b);
	col.push_back(a);
}
/* static */ void FsLazyWindowApplication::AddVertex(std::vector <float> &vtx,float x,float y,float z)
{
	vtx.push_back(x);
	vtx.push_back(y);
	vtx.push_back(z);
}
/* static */ void FsLazyWindowApplication::AddNormal(std::vector <float> &nom,float x,float y,float z)
{
	nom.push_back(x);
	nom.push_back(y);
	nom.push_back(z);
}

void FsLazyWindowApplication::RemakeVertexArray(void)
{
	vtx.clear();
	col.clear();
	nom.clear();

	for(auto plHd=mesh.NullPolygon(); true==mesh.MoveToNextPolygon(plHd); )
	{
		auto plVtHd=mesh.GetPolygonVertex(plHd);
		auto plCol=mesh.GetPolygonColor(plHd);
		auto plNom=mesh.GetPolygonNormal(plHd);

		// Let's assume every polygon is a triangle for now.
		if(3==plVtHd.size())
		{
			for(int i=0; i<3; ++i)
			{
				auto vtPos=mesh.GetVertexPosition(plVtHd[i]);
				vtx.push_back(vtPos.xf());
				vtx.push_back(vtPos.yf());
				vtx.push_back(vtPos.zf());
				nom.push_back(plNom.xf());
				nom.push_back(plNom.yf());
				nom.push_back(plNom.zf());
				col.push_back(plCol.Rf());
				col.push_back(plCol.Gf());
				col.push_back(plCol.Bf());
				col.push_back(plCol.Af());
			}
		}
	}
}



FsLazyWindowApplication::FsLazyWindowApplication()
{
	needRedraw=false;
	d=10.0;
	t=YsVec3::Origin();
}

/* virtual */ void FsLazyWindowApplication::BeforeEverything(int argc,char *argv[])
{
    if (3<=argc) {
        int size = std::max(atoi(argv[2]),4);
        px_container = ProxyContainer(size);
        printf("DBug: Size is %d\n",size);
    }
    else {
        px_container = ProxyContainer(NUM_PROXIES);
    }
}
/* virtual */ void FsLazyWindowApplication::GetOpenWindowOption(FsOpenWindowOption &opt) const
{
	opt.x0=0;
	opt.y0=0;
	opt.wid=1200;
	opt.hei=800;
}
/* virtual */ void FsLazyWindowApplication::Initialize(int argc,char *argv[])
{
	if(2<=argc && true==mesh.LoadBinStl(argv[1]))
	{
		RemakeVertexArray();
		mesh.GetBoundingBox(bbx[0],bbx[1]);

		t=(bbx[0]+bbx[1])/2.0;
		d=(bbx[1]-bbx[0]).GetLength()*1.2;

		printf("Target %s\n",t.Txt());
		printf("Diagonal %lf\n",d);

    
        px_container.setMesh(&mesh);
        px_container.generateProxies(TOLERANCE, MAX_ITERS);

        std::vector<ProxyContainer::ProxyHandle> px_vec = px_container.getProxies();
        // std::cout << " proxt num = " << px_vec[0].size() << std::endl;
        YsColor color;
        double dtheta = 2.0*PI / px_vec.size();
        for (int i=0; i<px_vec.size(); i++) {
//            color = YsColor(\
//                    std::max(sin(dtheta*i) + 0.3, 0.0),\
//                    std::max(sin(dtheta*i + 2.0*PI/3.0), 0.0),\
//                    std::max(sin(dtheta*i + 4.0*PI/3.0) + 0.1, 0.0));
            color = YsColor((double)rand()/(double)RAND_MAX, (double)rand()/(double)RAND_MAX, (double)rand()/(double)RAND_MAX);
            
        	std::vector<PolygonHandle> px_polygons = \
        	px_container.getProxyPolygons(px_vec[i]);
            px_container.setProxyColor(px_vec[i], color);

	        for (int j=0; j<px_polygons.size(); j++) {
	        	mesh.SetPolygonColor(px_polygons[j], color);
	        } 
        }
        RemakeVertexArray();
        initialMesh = mesh;
	}
}
/* virtual */ void FsLazyWindowApplication::Interval(void)
{
	auto key=FsInkey();
	if(FSKEY_ESC==key)
	{
		SetMustTerminate(true);
	}

	if(FsGetKeyState(FSKEY_LEFT))
	{
		Rc.RotateXZ(YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_RIGHT))
	{
		Rc.RotateXZ(-YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_UP))
	{
		Rc.RotateYZ(YsPi/60.0);
	}
	if(FsGetKeyState(FSKEY_DOWN))
	{
		Rc.RotateYZ(-YsPi/60.0);
	}
    
    if(FSKEY_M == key)
    {
        if (!created_pm) {
            pm_container = ProxyMeshContainer(px_container, &mesh, &finalMesh);
            printf("DBug: Created pm_container\n");
            created_pm = true;
        }
        mesh = finalMesh;
        RemakeVertexArray();
    }
    
    if(FSKEY_O == key)
    {
        mesh = initialMesh;
        RemakeVertexArray();
    }


	needRedraw=true;
}
/* virtual */ void FsLazyWindowApplication::Draw(void)
{
	needRedraw=false;

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	int wid,hei;
	FsGetWindowSize(wid,hei);
	auto aspect=(double)wid/(double)hei;
	glViewport(0,0,wid,hei);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0,aspect,d/10.0,d*2.0);

	YsMatrix4x4 globalToCamera=Rc;
	globalToCamera.Invert();

	YsMatrix4x4 modelView;  // need #include ysclass.h
	modelView.Translate(0,0,-d);
	modelView*=globalToCamera;
	modelView.Translate(-t);

	GLfloat modelViewGl[16];
	modelView.GetOpenGlCompatibleMatrix(modelViewGl);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLfloat lightDir[]={0.0f,1.0f/(float)sqrt(2.0f),1.0f/(float)sqrt(2.0f),0.0f};
	glLightfv(GL_LIGHT0,GL_POSITION,lightDir);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glMultMatrixf(modelViewGl);


    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4,GL_FLOAT,0,col.data());
    glNormalPointer(GL_FLOAT,0,nom.data());
    glVertexPointer(3,GL_FLOAT,0,vtx.data());
    glDrawArrays(GL_TRIANGLES,0,vtx.size()/3);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    
    
	FsSwapBuffers();
}
/* virtual */ bool FsLazyWindowApplication::UserWantToCloseProgram(void)
{
	return true; // Returning true will just close the program.
}
/* virtual */ bool FsLazyWindowApplication::MustTerminate(void) const
{
	return FsLazyWindowApplicationBase::MustTerminate();
}
/* virtual */ long long int FsLazyWindowApplication::GetMinimumSleepPerInterval(void) const
{
	return 10;
}
/* virtual */ void FsLazyWindowApplication::BeforeTerminate(void)
{
}
/* virtual */ bool FsLazyWindowApplication::NeedRedraw(void) const
{
	return needRedraw;
}


static FsLazyWindowApplication *appPtr=nullptr;

/* static */ FsLazyWindowApplicationBase *FsLazyWindowApplicationBase::GetApplication(void)
{
	if(nullptr==appPtr)
	{
		appPtr=new FsLazyWindowApplication;
	}
	return appPtr;
}
