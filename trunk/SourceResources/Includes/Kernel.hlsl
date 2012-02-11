#include "../Includes/SMO_Buffers.hlsl"

float dotProd(uint i1,uint i2){
	float result = 0;

	for(uint i=0; i<instanceLength; i++)
		result += i1*i2;

	return result;
}

float pukKernel(uint i1, uint i2){
	if (i1 == i2) {
		return 1.0;
	}
	else{
		float squaredDifference = -2.0 * dotProd(i1, i2) + dotProd(i1, i1) + dotProd(i2,i2);
		float intermediate = kernelParam1 * sqrt(squaredDifference);
		return (1.0 / pow(1.0 + intermediate * intermediate,kernelParam2));
	}
}

float rbfKernel(uint i1, uint i2){
	if (i1 == i2){
		return 1.0;
	} 
	else{
		return (kernelParam1 * (2. * dotProd(i1, i2) - dotProd(i1, i1) - dotProd(i2,i2)));
	}
}