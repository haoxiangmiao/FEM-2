#ifndef VONMISES_H
#define VONMISES_H

#include"ElastoPlasticMaterial.h"

class VonMises:public ElastoPlasticMaterial
{
	public: 
		VonMises(double E, double mu=0, double yS=0, double pM=0);
		//THIS IS WHY THIS CLASS MUST REMAIN BEING ABSTRACT; HAS TO BE CONVERTED INTO PLANE STRESS OR STRAIN
		//virtual void assembleTensors(Vector<double>& v, Tensor& strains, Tensor& stresses)=0;
	private:
		/*\brief Implements elastic predictor-plastic corrector algorithm
		 */
		virtual void radialReturn()override;
		/*!\brief Returns the result of evaluating the yield function on a stress state
		 */
		virtual double yieldFunction()override;
};
#endif