#include "../Includes/Kernel.hlsl"

// group size
#define thread_group_size_x 32
#define thread_group_size_y 1

groupshared algorithmParams sdata[thread_group_size_x];
[numthreads( thread_group_size_x, thread_group_size_y, 1 )]
void cs_entry(	uint3	groupID : SV_GroupID, 
				uint	groupIndex : SV_GroupIndex,
				uint3	dispatchThreadID : SV_DispatchThreadID){
	/*if(((groupID.x*thread_group_size_x)+groupIndex) <= cb_instanceCount){
		sdata[groupIndex].s_iLow = b_kernelEvalBuffer[((groupID.x*thread_group_size_x)+groupIndex)];
		sdata[groupIndex].s_iUp = sdata[groupIndex].s_iLow;
		sdata[groupIndex].s_bLow = b_errorBuffer[sdata[groupIndex].s_iLow];
		sdata[groupIndex].s_bUp = sdata[groupIndex].s_bLow;
	}
	else{
		sdata[groupIndex].s_iLow = -1;
		sdata[groupIndex].s_iUp = -1;
		sdata[groupIndex].s_bLow = -100000000;
		sdata[groupIndex].s_bUp = 1000000000;
	}
	GroupMemoryBarrierWithGroupSync();

	if(groupIndex < 16){
		if(sdata[groupIndex].s_bUp > sdata[groupIndex+16].s_bUp){
			sdata[groupIndex].s_bUp = sdata[groupIndex+16].s_bUp;
			sdata[groupIndex].s_iUp = sdata[groupIndex+16].s_iUp;
		}
		if(sdata[groupIndex].s_bLow < sdata[groupIndex+16].s_bLow){
			sdata[groupIndex].s_bLow = sdata[groupIndex+16].s_bLow;
			sdata[groupIndex].s_iLow = sdata[groupIndex+16].s_iLow;
		}
	}
	GroupMemoryBarrierWithGroupSync();
	if(groupIndex < 8){
		if(sdata[groupIndex].s_bUp > sdata[groupIndex+8].s_bUp){
			sdata[groupIndex].s_bUp = sdata[groupIndex+8].s_bUp;
			sdata[groupIndex].s_iUp = sdata[groupIndex+8].s_iUp;
		}
		if(sdata[groupIndex].s_bLow < sdata[groupIndex+8].s_bLow){
			sdata[groupIndex].s_bLow = sdata[groupIndex+8].s_bLow;
			sdata[groupIndex].s_iLow = sdata[groupIndex+8].s_iLow;
		}
	}
	GroupMemoryBarrierWithGroupSync();
	if(groupIndex < 4){
		if(sdata[groupIndex].s_bUp > sdata[groupIndex+4].s_bUp){
			sdata[groupIndex].s_bUp = sdata[groupIndex+4].s_bUp;
			sdata[groupIndex].s_iUp = sdata[groupIndex+4].s_iUp;
		}
		if(sdata[groupIndex].s_bLow < sdata[groupIndex+4].s_bLow){
			sdata[groupIndex].s_bLow = sdata[groupIndex+4].s_bLow;
			sdata[groupIndex].s_iLow = sdata[groupIndex+4].s_iLow;
		}
	}
	GroupMemoryBarrierWithGroupSync();
	if(groupIndex < 2){
		if(sdata[groupIndex].s_bUp > sdata[groupIndex+2].s_bUp){
			sdata[groupIndex].s_bUp = sdata[groupIndex+2].s_bUp;
			sdata[groupIndex].s_iUp = sdata[groupIndex+2].s_iUp;
		}
		if(sdata[groupIndex].s_bLow < sdata[groupIndex+2].s_bLow){
			sdata[groupIndex].s_bLow = sdata[groupIndex+2].s_bLow;
			sdata[groupIndex].s_iLow = sdata[groupIndex+2].s_iLow;
		}
	}

	if(groupIndex == 0){
		b_algoParamBuffer[groupID.x] = sdata[groupIndex];
	}*/
}