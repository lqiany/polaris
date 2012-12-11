#pragma once

#include "User_Space\Population_Synthesis\Population_Unit_Prototype.h"

namespace PopSyn
{
	namespace Implementations
	{
		implementation struct ADAPTS_Population_Unit_Implementation
		{
			member_data(double,ID,none,none);
			member_data(double,Weight,none,none);
			member_data(uint,index,none,none);				 //index into the joint-distribution matrix of the region (convert using region.get_index())
			member_container(vector<double>, Characteristics,none,none);
		};
	}
}