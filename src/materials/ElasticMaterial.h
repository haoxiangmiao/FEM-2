
#ifndef ELASTICMATERIAL_H
#define ELASTICMATERIAL_H



#include"Material.h"

class ElasticMaterial:public Material
{
	public:
		ElasticMaterial(double E, double mu);
		//virtual Matrix<double>& getConstitutiveMatrix()override;
		//Setters and getters
		//virtual std::string getType()=0;
		//double getPoisson();
		//double getModulus();
		/*!\brief Receives strain as vector and modifies tensors to 6x1 results
		 */
		//virtual void assembleTensors(Vector<double>& v, Tensor& strains, Tensor& stresses)=0;
		//friend std::ostream& operator<<(std::ostream &out, ElasticMaterial& mat);
	protected:
		//Matrix<double> C;
		//double mu;
		//double E;
		//std::string type;
		
		
	private:
		//virtual void setConstitutiveMatrix()=0;
};
#endif