#include "../Includes/SMO_Buffers.hlsl"

float dotProd(uint i1,uint i2){
	float result = 0;

	for(uint i=0; i<cb_instanceLength; i++){
		if(i != cb_classIndex)
			result += b_inputBuffer[(cb_instanceLength*i1)+i]*b_inputBuffer[(cb_instanceLength*i2)+i];
	}

	return result;
}

float pukKernel(float dp, int i1, int i2){
	float squaredDifference = -2.0 * dp + b_selfProd[i1] + b_selfProd[i2];
	float intermediate = cb_kernelParam1 * sqrt(squaredDifference);
	return (1.0 / pow(1.0 + intermediate * intermediate,cb_kernelParam2));
}

float rbfKernel(float dp, int i1, int i2){
	return (cb_kernelParam1 * (2. * dp - b_selfProd[i1] - b_selfProd[i2]));
}

float evaluateKernel(float dp, int i1, int i2){
	if(cb_kernel == 0)
		return pukKernel(dp,i1,i2);
	else if(cb_kernel == 1)
		return rbfKernel(dp,i1,i2);
	else
		return 1.0;
}