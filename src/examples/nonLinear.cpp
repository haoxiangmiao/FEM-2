#include<cstdlib>
#include"../meshing/NaiveMesh.h"
#include"../physics/MechanicalBoundaryConditions.h"
#include"../materials/VonMisesPlaneStress.h"
#include"../materials/VonMisesPlaneStrain.h"
#include"../materials/PlaneStrain.h"
#include"../plotter/gnuplot_i.hpp"
#include"../fem/ImplAssembly.h"
#include"../solver/descent/ConjugateGradientDescent.h"
#include"../solver/NLSolverCG.h"
#include <unistd.h>
#include"meshing/CustomMesh.h"
#include"tensors/Tensor.h"
#include"tensors/Stress.h"

using std::cout;

int main(int argc, char* argv[])
{
	
	VonMisesPlaneStrain mat1(210,0.25, 100, 70);
	
	NaiveMesh mesh(1,1);
	Vector<int> leftInclusive=mesh.setLeftInclusive();
	Vector<int> rightInclusive=mesh.setRightInclusive();
	mesh.print();
	mesh.setElementMaterial(&mat1);
	
	Force f(150,0);
	for(int i=0;i<rightInclusive.size();++i)
	{
		mesh.getNode(rightInclusive[i])->setPointForce(&f);
	}
	
	Discretization disc(2,mesh);
	
	MechanicalBoundaryConditions encastre(true,true,false);
	encastre.setNodes(leftInclusive);
	
	
	//mesh.print();
	disc.addBoundaryCondition(encastre);
	disc.DOFenum();
	//mesh.print();
	ImplAssembly ass(&disc);
	
	ass.printImplAssembly();
	
	ass.matrixAssemblyRoutine();
	ass.vectorAssemblyRoutine();
	
	
	
	
	NLSolverCG solver(ass,10e-10,20,50);
	solver.printNLSolver();
	solver.solve();
	
	//mesh.print();
	
 	/* ass.localSolutionVectorAssemblyRoutine(solver.getU());
	ass.globalInternalForceAssembly();
	cout<<"CALCULATED VIA Ku\n";
	cout<<ass.getGlobalVector()-ass.getGlobalMatrix()*solver.getU();
	cout<<(ass.getGlobalVector()-ass.getGlobalMatrix()*solver.getU()).norm();
	cout<<"\nASSEMBLED\n";
	cout<<ass.getGlobalVector()-ass.getGlobalInternalForce();
	cout<<(ass.getGlobalVector()-ass.getGlobalInternalForce()).norm(); */
	
	
}