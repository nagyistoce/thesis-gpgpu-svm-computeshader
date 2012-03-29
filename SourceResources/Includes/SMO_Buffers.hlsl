typedef float svm_precision;

cbuffer SharedBuffer : register(b0)
{
	svm_precision	cb_kernelParam1;
	svm_precision	cb_kernelParam2;
	uint	cb_instanceLength;
	uint	cb_instanceCount;
	uint	cb_classIndex;

	// Run flags
	uint	cb_kernel;
	svm_precision	cb_param1;
	svm_precision	cb_param2;
	int		cb_ind1;
	int		cb_ind2;
}

StructuredBuffer<svm_precision>		b_inputBuffer : register(t0);
StructuredBuffer<uint>				b_kernelEvalBuffer : register(t1);
StructuredBuffer<uint>				b_trainingIndices : register(t2);
StructuredBuffer<uint>				b_testingIndices : register(t3);
StructuredBuffer<svm_precision>		b_class : register(t4);
StructuredBuffer<svm_precision>		b_alpha : register(t5);

RWStructuredBuffer<svm_precision>	b_selfProd : register(u0);
RWStructuredBuffer<svm_precision>	b_outputBuffer : register(u1);