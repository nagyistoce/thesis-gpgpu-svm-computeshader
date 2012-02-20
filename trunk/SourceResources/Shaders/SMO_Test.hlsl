#include "../Includes/Kernel.hlsl"

// group size
#define thread_group_size_x 32
#define thread_group_size_y 1

[numthreads( thread_group_size_x, thread_group_size_y, 1 )]

void cs_entry(	uint3	threadIDInGroup : SV_GroupThreadID, 
				uint3	groupID : SV_GroupID, 
				uint	groupIndex : SV_GroupIndex, 
				uint3	dispatchThreadID : SV_DispatchThreadID ){
	
}