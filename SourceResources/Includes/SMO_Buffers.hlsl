cbuffer SharedBuffer : register(b0)
{
	float	kernelParam1;
	float	kernelParam2;
	uint	instanceLength;
	uint	instanceCount;
	uint	classIndex;
}

StructuredBuffer<float> inputBuffer : register(t0);
StructuredBuffer<uint> instanceIndices : register(t1);
RWStructuredBuffer<float> outputBuffer : register(u0);