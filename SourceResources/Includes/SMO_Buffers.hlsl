cbuffer SharedBuffer : register(b0)
{
	float	cb_kernelParam1;
	float	cb_kernelParam2;
	uint	cb_instanceLength;
	uint	cb_instanceCount;
	uint	cb_classIndex;

	// Run flags
	uint	cb_kernel;
	float	cb_param1;
	float	cb_param2;
	int		cb_ind1;
	int		cb_ind2;
}

struct algorithmParams{
	float	s_bLow,
			s_bUp;
	int		s_iLow,
			s_iUp;
};

StructuredBuffer<float>				b_inputBuffer : register(t0);
StructuredBuffer<uint>				b_kernelEvalBuffer : register(t1);
StructuredBuffer<uint>				b_trainingIndices : register(t2);
StructuredBuffer<uint>				b_testingIndices : register(t3);
StructuredBuffer<float>				b_class : register(t4);
StructuredBuffer<float>				b_alpha : register(t5);

RWStructuredBuffer<float>			b_selfProd : register(u0);
RWStructuredBuffer<float>			b_errorBuffer : register(u1);
RWStructuredBuffer<float>			b_outputBuffer : register(u2);
RWStructuredBuffer<algorithmParams>	b_algoParamBuffer : register(u3);