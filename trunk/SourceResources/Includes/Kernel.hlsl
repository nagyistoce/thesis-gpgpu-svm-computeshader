#include "../Includes/SMO_Buffers.hlsl"

svm_precision dotProd(uint i1,uint i2){
	svm_precision result = 0;

	for(uint i=0; i<cb_instanceLength; i++){
		if(i != cb_classIndex)
			result += b_inputBuffer[(cb_instanceLength*i1)+i]*b_inputBuffer[(cb_instanceLength*i2)+i];
	}

	return result;
}

svm_precision pukKernel(svm_precision dp, int i1, int i2){
	svm_precision squaredDifference = -2.0 * dp + b_selfProd[i1] + b_selfProd[i2];
	svm_precision intermediate = cb_kernelParam1 * sqrt(asfloat(squaredDifference));
	return (1.0 / pow(asfloat(1.0 + intermediate * intermediate),asfloat(cb_kernelParam2)));
}

svm_precision rbfKernel(svm_precision dp, int i1, int i2){
	return (cb_kernelParam1 * (2. * dp - b_selfProd[i1] - b_selfProd[i2]));
}

svm_precision evaluateKernel(svm_precision dp, int i1, int i2){
	if(i1 == i2)
		return 1.0;

	if(cb_kernel == 0)
		return pukKernel(dp,i1,i2);
	else if(cb_kernel == 1)
		return rbfKernel(dp,i1,i2);
	else
		return 1.0;
}

svm_precision absPrec(svm_precision value){
	if(value < 0)
		return -value;
	else
		return value;
}