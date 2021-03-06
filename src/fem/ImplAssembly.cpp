#include"ImplAssembly.h"

ImplAssembly::ImplAssembly(Discretization* d):disc(d)
{
	E=disc->getMesh().getNumElements();
	
	setN();
	verbose=false;
}
std::string ImplAssembly::status()
{
	for(int i=0;i<E;++i)
	{
		if(disc->getMesh().getElements()[i]->getMaterial()->isPlastic())
			return "PLASTIC";
	}
	return "ELASTIC";
}
void ImplAssembly::matrixAssemblyRoutine()
{
	
	globalK.matrixResize(disc->getTotalDOF(),disc->getTotalDOF());
	globalK=0;
	if(verbose)
	{
		std::cout<<"\nBEGIN: MATRIX ASSEMBLY ROUTINE****\n";
		std::cout<<"   THE GLOBAL K HAS BEEN INITILIZED:";
	}
		
	//std::cout<<globalK;
	//std::cout<<"   THE NUMBER OF ELEMENTS IS: "<<E<<"\n\n";
	//iterate through every element
	for(int e=0;e<E;++e)
	{
		//std::cout<<"THE NUMBER OF ELEMENT IS: "<<e<<"\n\n";
		//generate every element's stiffness matrix
		Element* el=disc->getMesh().getElements()[e];
		el->calculateMatrix();
		//disc->getMesh().getElements()[e]->calculateMatrix();
		
		//disc.getMesh().getElements()[e].calculateMatrix();
		//iterate through the matrix of each element Kij
		for(int i=0;i<n;++i)
		{
			for(int j=0;j<n;++j)
			{
				
				int index1=connectivityArray(el,i);
				int index2=connectivityArray(el,j);
				if(index1>=0&&index2>=0)
				{
					this->globalK(index1,index2)+=el->getMatrix()(i,j);
				}
				

			}
		}
		
	}
	if(verbose)
	{
		std::cout<<"RESULT GLOBAL MATRIX IS: \n";
		std::cout<<globalK;
		std::cout<<"END: MATRIX ASSEMBLY ROUTINE****\n";
	}
	
}
void ImplAssembly::vectorAssemblyRoutine()
{
	if(verbose)
		std::cout<<"\nBEGIN: VECTOR ASSEMBLY ROUTINE******\n";
	globalF.resize(disc->getTotalDOF());
	int totalNodes=disc->getMesh().getNumNodes();
	int dpn=disc->getDofPerNode();
	int index;
	/*
	 1.-First we access the nodes
	 2.-Check if the node actually has an assigned point force
	 3.-Enter the dofNumbers vector, this gives you the global number
	 4.-Enter the forcce vector and remove the same position as in dofNumbers
	 5.-assign the value of force vector to the value of globalF a given by the value of dofNumbers-1
	*/
	for(int i=0;i<totalNodes;++i)
	{
		if(disc->getMesh().getNodes()[i].getPointForce()!=nullptr)
		{
			for(int j=0;j<dpn;++j)
			{
				index=disc->getMesh().getNodes()[i].getDOFnumbers()[j];
				this->globalF[index-1]=disc->getMesh().getNodes()[i].getPointForce()->getComponent(j);
			}
		}
		
	}
	if(verbose)
	{
		std::cout<<globalF;
		std::cout<<"END: VECTOR ASSEMBLY ROUTINE******\n";
	}
	
}
void ImplAssembly::localSolutionVectorAssemblyRoutine(Vector<double>& globalSolution)
{
	if(verbose)
		std::cout<<"\nBEGIN: LOCAL SOLUTION VECTOR ASSEMBLY ROUTINE******\n";
	//globalF.resize(disc->getTotalDOF());
	int totalNodes=disc->getMesh().getNumNodes();
	int dpn=disc->getDofPerNode();
	int index;
	for(int e=0;e<E;++e)
	{
		Element* el=disc->getMesh().getElements()[e];
		//********HUGE PROBLEM: THIS MUST BE DONE SOMEWHRE ELSE; THIS CAN MODIFY OUR LOCAL VALUES,AND ITS STUPID WORK
		el->resizeElementSolutionVector(n);
		//here we can bring the internal vector to zero to assemble it later
		el->getInternalForce()=0;
		for(int i=0;i<n;++i)
		{
			int index=connectivityArray(el,i);
			if(index>=0)
			{
				el->getElementSolutionVector()[i]=globalSolution[index];
			}
		}
		el->computeTensorialResults();
	}
	if(verbose)
		std::cout<<"END: LOCAL SOLUTION VECTOR ASSEMBLY ROUTINE******\n";
}
void ImplAssembly::globalInternalForceAssembly()
{
	//globalInternalForce=0;
	if(verbose)
		std::cout<<"\nBEGIN: INTERNAL FORCE VECTOR ASSEMBLY ROUTINE******\n";
	globalInternalForce.resize(disc->getTotalDOF());
	int totalNodes=disc->getMesh().getNumNodes();
	int dpn=disc->getDofPerNode();
	int index;
	/*
	 1.-First we access the nodes
	 2.-Check if the node actually has an assigned point force
	 3.-Enter the dofNumbers vector, this gives you the global number
	 4.-Enter the forcce vector and remove the same position as in dofNumbers
	 5.-assign the value of force vector to the value of globalF a given by the value of dofNumbers-1
	*/
	for(int i=0;i<totalNodes;++i)
	{
		/* if(disc->getMesh().getNodes()[i].getPointForce()!=nullptr)
		{ */
			for(int j=0;j<dpn;++j)
			{
				index=disc->getMesh().getNodes()[i].getDOFnumbers()[j];
				if(index>=0)
					this->globalInternalForce[index-1]=disc->getMesh().getNodes()[i].getInternalForce()[j];
			}
		//}
		
	}
	if(verbose)
	{
		std::cout<<globalInternalForce;
		std::cout<<globalInternalForce.norm()<<"\n";
		std::cout<<"END: INTERNAL FORCE VECTOR ASSEMBLY ROUTINE******\n";
	}

	
}
int ImplAssembly::connectivityArray(Element* el, int i)
{
	//to know which node
	int dofs=disc->getDofPerNode();
	return el->getElementNodes()[i/dofs]->getDOFnumbers()[i%dofs]-1;
	//return 0;
}
void ImplAssembly::setN()
{
	int nodesPerElement=disc->getMesh().getElements()[0]->getElementNodes().size();
	this->n=disc->getDofPerNode()*nodesPerElement;
}
void ImplAssembly::printImplAssembly()
{
	std::cout<<"\nBEGIN: ASSEMBLY PROCEDURE****\n";
	std::cout<<"   ASSEMBLES SYSTEM FOR: "<<E<<" ELEMENTS\n";
	std::cout<<"   WITH A TOTAL OF: "<<disc->getTotalDOF()<<" DOFs\n";
	std::cout<<"   EVERY ELEMENT HAS: "<<n<<" DOFS\n";
	std::cout<<"   EVERY NODE HAS: "<<disc->getDofPerNode()<<" DOFS\n";
	std::cout<<"END: ASSEMBLY PROCEDURE****\n";
}
void ImplAssembly::printMesh()
{
	disc->getMesh().print();
}
Matrix<double>& ImplAssembly::getGlobalMatrix()
{
	return this->globalK;
}
Vector<double>& ImplAssembly::getGlobalVector()
{
	return this->globalF;
}
Vector<double>& ImplAssembly::getGlobalInternalForce()
{
	/* for(int i=0;i<globalInternalForce.size();++i)
	{
		if(abs(globalInternalForce[i])<10e-15)
			globalInternalForce[i]=0;
	} */ 
	return this->globalInternalForce;
}
int ImplAssembly::getTotalDOF()
{
	return disc->getTotalDOF();
}
Discretization* ImplAssembly::getDiscretization()
{
	return disc;
}
void ImplAssembly::zeroNodalInternalForce()
{
	int s=disc->getMesh().getNodes().size();
	for(int i=0;i<s;++i)
	{
		disc->getMesh().getNodes()[i].getInternalForce()=0;
	}
}