#include"VonMises.h"

using std::cout;

VonMises::VonMises(double E, double mu, double yS, double pM):ElastoPlasticMaterial(E,mu,yS,pM)
{
	
	initializeModel();

}
void VonMises::rr(Tensor& strain)
{
	radialReturn(strain);
}
Tensor VonMises::radialReturn(Tensor& strains)
{	
	//initializeModel();
	bool verbose=true;
	if(verbose)
		cout<<"BEFORE CALCULATION: "<<C;
	double tolerance=10e-12;
	double yielding=0;
	//Matrix<double> Ctemp;
	//Matrix<double> Num;
	//double den;
	dLambda=0;
	dK=0;
	
	//Trial stress
	
	trialStress=Cel*strains;
	if(verbose)
	{
		cout<<"THE TRIAL STRESS IN THIS LOAD STEP IS: \n";
		cout<<trialStress;
	}
	
	//Take the trial Stress a our initial guess for the stress state...TAKE ZERO...since we come from there
	stressIncrements=trialStress;
	
	yielding=yieldFunction(trialStress);
	//cout<<"THE YIELD FUNCTIONS VALUE IS: \n";
	//cout<<yielding<<"\n";
	
	int count=10;
	
	int i=0;
	int a;
	if(yielding>0 )
	{	
		plastic=true;
		linearProblemUpdate(stressIncrements);
		updateSolution();
		//cout<<"THE SOLUTION IS: "<<solution;
		if(verbose)
		{
			cout<<"STEP		"<<"NORM RESIDUUM		"<<"VALUE OF YIELD FUNCTION    "<<"DLAMBDA		"<<"DK\n"
			<<i<<"			"<<residual.norm()<<"			"<<yieldFunction(stressIncrements)
				<<"  	  "<<dLambda<<"		"<<dK<<"\n"; 
		}
		
		while(i<count && residual.norm()>tolerance)
		{
			
			ASym=(~A)*A;
			residualSym=(~A)*residual;
			cg.solve();
			solution-=cg.getU();		
			dissasembleSolution();
			linearProblemUpdate(stressIncrements);
			
			//updateSolution();
	
			i++;
			//cout<<"STEP		"<<"NORM RESIDUUM		"<<"VALUE OF YIELD FUNCTION    "<<"DLAMBDA		"<<"DK\n"
			if(verbose)
			{
				cout<<i<<"			"<<residual.norm()<<"			"<<yieldFunction(stressIncrements)
				<<"  	  "<<dLambda<<"		"<<dK<<"\n"; 
			}
			
			//cout<<"THE SYMMETRIC PROBLEM IS: "<<ASym;
			//cout<<residualSym;
			
			if(verbose)
			{
				cout<<"THE MATRIX IS: "<<A;
				cout<<"THE RESIDUAL IS: "<<residual;
				cout<<"THE STRESS STATE IS: "<<stressIncrements;
				cout<<"THE EQUIVALENT STRESS IS:"<<equivalentStress(stressIncrements)<<"\n";
				cout<<"THE TRIAL STRESS STATE: "<<trialStress;
				cout<<"THE FLOW DIRECTION IS: "<<dF_dSigma; 
			}		
		
		}
		if(verbose)
		{
			cout<<"STEP		"<<"NORM RESIDUUM		"<<"VALUE OF YIELD FUNCTION    "<<"DLAMBDA		"<<"DK\n"
			<<i<<"			"<<residual.norm()<<"			"<<yieldFunction(stressIncrements)
			<<"  	  "<<dLambda<<"		"<<dK<<"\n";
			cout<<"THE STRESS STATE IS: "<<stressIncrements;
			cout<<"DLAMBDA: "<<dLambda<<"\n";
			cout<<"NORMAL IS: "<<dF_dSigma;
			cout<<"THE EQUIVALENT STRESS IS:"<<equivalentStress(stressIncrements)<<"\n";
			cout<<"TESTING THE YIELD FUNCTION: "<<equivalentStress(stressIncrements)-yieldStress-plasticModulus*dK<<"\n";	
		}
		if(abs(equivalentStress(stressIncrements)-yieldStress-plasticModulus*dK)>tolerance)
			cout<<"PLASTIC CORRECTOR DID NOT CONVERGE: "<<abs(equivalentStress(stressIncrements)-yieldStress-plasticModulus*dK)<<"\n";
		
		calculatePlasticStrains();
		updateYieldStress();
		if(verbose)
			cout<<"THE YIELD STRESS IS: "<<yieldStress<<"\n";
		
		assembleTangentModulus();
		//cout<<Cep;
		

	}//if yielding
	if(yielding<0)
	{
		plastic=false;
		if(verbose)
		{
			std::cout<<"\nELASTIC BEHAVIOR\n";
			std::cout<<"YIELD FUNCTION: "<<yielding<<"\n";
		}
			
		return trialStress;
	}
	setConstitutiveMatrix();
	if(verbose)
	{
		cout<<"AFTER CALCULATION: "<<C;
	}

	return stressIncrements;
}
void VonMises::assembleTangentModulus()
{
	derivativeFSigma(stressIncrements);
	Matrix<double> Ctemp;
	Matrix<double> I=Matrix<double>(6,6);
	I(0,0)=1;
	I(1,1)=1;
	I(2,2)=1;
	I(3,3)=1;
	I(4,4)=1;
	I(5,5)=1;
	Matrix<double> Num;
	double den;

	den=dF_dSigma*(Cel*dF_dSigma)+plasticModulus*0.666666666;
	Ctemp.OuterProduct(dF_dSigma,dF_dSigma);
	
	Num=Cel*Ctemp*Cel;
	Num*=(1/den);
	Cep=Cel-Num;
 	/* for(int i=0;i<Cep.getColumns();++i)
	{
		for(int j=0;j<Cep.getColumns();++j)
		{
			if(abs(Cep(i,j))<1e-9)
				Cep(i,j)=0;
		}
	}  */
}
double VonMises::yieldFunction(Tensor& stresses)
{
	double eqStress=0;
	double sigmaHat=0;
	eqStress=equivalentStress(stresses);
	sigmaHat=yieldStress+plasticWork();
	if(false)
	{
		std::cout<<"THE EQUIVALENT STRESS IS: "<<eqStress<<"\n";
		std::cout<<"THE PLASTIC WORK IS: "<<plasticWork()<<"\n";
		std::cout<<"THE SIGMA HAT IS: "<<sigmaHat<<"\n";
	}
	
	return eqStress-sigmaHat;
}
double VonMises::equivalentStress(Tensor& stresses)
{

	Matrix<double> P(6,6);
	P(0,0)=2.0/3.0;
	P(1,1)=P(0,0);
	P(2,2)=P(1,1);
	P(3,3)=2;
	P(4,4)=2;
	P(5,5)=2;
	P(0,1)=-1.0/3.0;
	P(1,0)=P(0,1);
	P(2,0)=P(0,1);
	P(0,2)=P(2,0);
	P(2,1)=P(0,1);
	P(1,2)=P(1,0);
	
	return sqrt((stresses*(P*stresses))*(1.5)); 
}
double VonMises::plasticWork()
{
 	/* Matrix<double> Q(6,6);
	Q(0,0)=2.0/3.0;
	Q(1,1)=Q(0,0);
	Q(2,2)=Q(1,1);
	Q(3,3)=0.5;
	Q(4,4)=0.5;
	Q(5,5)=0.5;
	Q(0,1)=-1.0/3.0;
	Q(1,0)=Q(0,1);
	Q(2,0)=Q(0,1);
	Q(0,2)=Q(2,0);
	Q(2,1)=Q(0,1);
	Q(1,2)=Q(1,0);
	
	return plasticModulus*sqrt((plasticStrain*(Q*plasticStrain))*(2.0/3.0));  */ 
	return plasticModulus*dK;
}
void VonMises::derivativeFSigma(Tensor& stress)
{
	//double scalar=0.5/yieldStress;
	//Since we are updating the yield stress only after each step, we will use the eq stress
	//double scalar=0.5/yieldStress;
	double scalar=0.5/equivalentStress(stress);
	dF_dSigma[0]=2*stress[0]-stress[1]-stress[2];
	dF_dSigma[1]=2*stress[1]-stress[0]-stress[2];
	dF_dSigma[2]=2*stress[2]-stress[1]-stress[0];
	dF_dSigma[3]=6*stress[3];
	dF_dSigma[4]=6*stress[4];
	dF_dSigma[5]=6*stress[5];
	dF_dSigma*=scalar;
}

void VonMises::initializeModel()
{
	//initialize full elastic tensor
	//initialize plastic parameters
	df_dK=plasticModulus;
	dLambda=0;
	dK=0;
	/* double scalar=0.5/yieldStress;
	Jacobian(0,0)=2;
	Jacobian(1,1)=2;
	Jacobian(2,2)=2;
	Jacobian(0,1)=-1;
	Jacobian(0,2)=-1;
	Jacobian(1,2)=-1;
	Jacobian(2,1)=-1;
	Jacobian(1,0)=Jacobian(0,1);
	Jacobian(2,0)=Jacobian(0,2);
	Jacobian(3,3)=6;
	Jacobian(4,4)=6;
	Jacobian(5,5)=6;
	//cout<<Jacobian;
	Jacobian*=scalar; */
	calculateJacobian(stressIncrements);
	
}
void VonMises::calculateJacobian(Tensor& stress)
{
	double f=yieldStress;
	//double f=equivalentStress(stress);
	double den=0.5;
	//****************************
	double sxxd=2*stress[0]-stress[1]-stress[2];
	double syyd=2*stress[1]-stress[0]-stress[2];
	double szzd=2*stress[2]-stress[0]-stress[1];
	double sxyd=6*stress[3];
	double syzd=6*stress[4];
	double sxzd=6*stress[5];
	//****************************
	
	//****JACOBIAN DEFINTION, first the diagonal
	Jacobian(0,0)=2/f-0.5*pow(sxxd,2)/pow(f,3);
	Jacobian(1,1)=2/f-0.5*pow(syyd,2)/pow(f,3);
	Jacobian(2,2)=2/f-0.5*pow(szzd,2)/pow(f,3);
	Jacobian(3,3)=3/f-0.5*pow(sxyd,2)/pow(f,3);
	Jacobian(4,4)=3/f-0.5*pow(syzd,2)/pow(f,3);
	Jacobian(5,5)=3/f-0.5*pow(sxzd,2)/pow(f,3);
	
	//****Now fill the columns
	//1st COLUMN
	Jacobian(1,0)=-1/f-0.5*(sxxd)*(syyd)/pow(f,3);
	Jacobian(2,0)=-1/f-0.5*(sxxd)*(szzd)/pow(f,3);
	Jacobian(3,0)=-0.5*(sxxd)*(sxyd)/pow(f,3);
	Jacobian(4,0)=-0.5*(sxxd)*(sxyd)/pow(f,3);
	Jacobian(5,0)=-0.5*(sxxd)*(sxzd)/pow(f,3);
	//2nd COLUMN
	Jacobian(0,1)=-1/f-0.5*(sxxd)*(syyd)/pow(f,3);
	Jacobian(2,1)=-1/f-0.5*(szzd)*(syyd)/pow(f,3);
	Jacobian(3,1)=-0.5*(sxyd)*(syyd)/pow(f,3);
	Jacobian(4,1)=-0.5*(syzd)*(syyd)/pow(f,3);
	Jacobian(5,1)=-0.5*(sxzd)*(syyd)/pow(f,3);
	
	//3rd COLUMN
	Jacobian(0,2)=-1/f-0.5*(sxxd)*(szzd)/pow(f,3);
	Jacobian(1,2)=-1/f-0.5*(szzd)*(syyd)/pow(f,3);
	Jacobian(3,2)=-0.5*(sxyd)*(szzd)/pow(f,3);
	Jacobian(4,2)=-0.5*(syzd)*(szzd)/pow(f,3);
	Jacobian(5,2)=-0.5*(sxzd)*(szzd)/pow(f,3);
	
	//4th COLUMN
	Jacobian(0,3)=-0.5*(sxyd)*(sxxd)/pow(f,3);
	Jacobian(1,3)=-0.5*(sxyd)*(syyd)/pow(f,3);
	Jacobian(2,3)=-0.5*(sxyd)*(szzd)/pow(f,3);
	Jacobian(4,3)=-0.5*(sxyd)*(syzd)/pow(f,3);
	Jacobian(5,3)=-0.5*(sxyd)*(sxzd)/pow(f,3);
	
	//5th COLUMN
	Jacobian(0,4)=-0.5*(syzd)*(sxxd)/pow(f,3);
	Jacobian(1,4)=-0.5*(syzd)*(syyd)/pow(f,3);
	Jacobian(2,4)=-0.5*(syzd)*(szzd)/pow(f,3);
	Jacobian(3,4)=-0.5*(syzd)*(sxyd)/pow(f,3);
	Jacobian(5,4)=-0.5*(syzd)*(sxzd)/pow(f,3);
	
	//6th COLUMN
	Jacobian(0,5)=-0.5*(sxzd)*(sxxd)/pow(f,3);
	Jacobian(1,5)=-0.5*(sxzd)*(syyd)/pow(f,3);
	Jacobian(2,5)=-0.5*(sxzd)*(szzd)/pow(f,3);
	Jacobian(3,5)=-0.5*(sxzd)*(sxyd)/pow(f,3);
	Jacobian(4,5)=-0.5*(sxzd)*(syzd)/pow(f,3);
	Jacobian*=den;
	
	cout<<"THE JACOBIAN WITH STRESSES IS:\n";
	cout<<Jacobian; 
	/* double scalar=0.5/yieldStress;
	Jacobian(0,0)=2;
	Jacobian(1,1)=2;
	Jacobian(2,2)=2;
	Jacobian(0,1)=-1;
	Jacobian(0,2)=-1;
	Jacobian(1,2)=-1;
	Jacobian(2,1)=-1;
	Jacobian(1,0)=Jacobian(0,1);
	Jacobian(2,0)=Jacobian(0,2);
	Jacobian(3,3)=6;
	Jacobian(4,4)=6;
	Jacobian(5,5)=6;
	//cout<<Jacobian
	cout<<"THE JACOBIAN WITH CONSTANTS IS:\n";
	cout<<Jacobian;
	Jacobian*=scalar; */
}
void VonMises::assembleA(Tensor& previousStress)
{
	
	Matrix<double> I=Matrix<double>(6,6);
	I(0,0)=1;
	I(1,1)=1;
	I(2,2)=1;
	I(3,3)=1;
	I(4,4)=1;
	I(5,5)=1;
	Matrix<double> temp=Matrix<double>(6,6);
	temp=Cel*Jacobian;
	temp*=dLambda;
	temp+=I;//Equivalent to I+(dLamba*Cel*Jacobian)
	//COPY INTO 7x7 A MATRIX
	for(int i=0;i<6;++i)
	{
		for(int j=0;j<6;++j)
		{
			A(i,j)=temp(i,j);
		}
	}
/* 	cout<<"\nMATRIX H IS: \n";
	cout<<temp.inverse()*Cel; */
	
	//WE NEED THE 6x1 VECTOR FROM Cel*dF_dSigma
	Vector<double> temp2=Vector<double>(6);
	temp2=Cel*dF_dSigma;
	for(int i=0;i<6;++i)
	{
		A(i,6)=temp2[i];
	}
	//COPY NORMAL FLOW DIRECTION dF_dSigma
	for(int i=0;i<6;++i)
	{
		A(6,i)=dF_dSigma[i];
	}
	//AND PLASTIC MODULUS
	A(6,6)=-plasticModulus;
	
	
}
void VonMises::calculateResidual(Tensor& previousStress)
{
	Tensor temp;
	//dF_dSigma MUST BE UPDATED EXTERNALLY
	temp=previousStress-trialStress+(Cel*dF_dSigma*dLambda);//this is a 6x1
	
	//COPY TEMP INTO 6 FIRST ELEMENTS OF RESIDUAL
	for(int i=0;i<6;i++)
	{
		residual[i]=temp[i];
	}
	residual[6]=yieldFunction(previousStress);
}


void VonMises::linearProblemUpdate(Tensor& stressIncrements)
{
	//updateYieldStress();
	
	calculateJacobian(stressIncrements);
	derivativeFSigma(stressIncrements);
	calculatePlasticStrains();
	assembleA(stressIncrements);
	calculateResidual(stressIncrements);
}

void VonMises::updateSolution()
{
	for(int i=0;i<solution.size()-1;++i)
	{
		solution[i]=stressIncrements[i];
	}
	solution[6]=dLambda;
	solution[6]=dLambda;
}
void VonMises::dissasembleSolution()
{
	for(int i=0;i<solution.size()-1;++i)
	{
		stressIncrements[i]=solution[i];
	}
	dLambda=solution[6];
	dK=dLambda;
}

void VonMises::calculatePlasticStrains()
{
	plasticStrain=dF_dSigma*dLambda;
}
void VonMises::updateYieldStress()
{
	yieldStress+=(plasticModulus*dK);
}