#pragma once
#include "User_Space_Includes.h"

namespace Transit_Route_Components
{
	namespace Types
	{
	}

	namespace Concepts
	{
	}

	namespace Prototypes
	{
		prototype struct Transit_Route ADD_DEBUG_INFO
		{
			tag_as_prototype;

			accessor(internal_id, NONE, NONE);
			accessor(uuid, NONE, NONE);
			accessor(name, NONE, NONE);
			accessor(agency, NONE, NONE);
			accessor(direction, NONE, NONE);
			accessor(route, NONE, NONE);
			accessor(pattern, NONE, NONE);
			accessor(sequence_number, NONE, NONE);
			accessor(arrival_seconds, NONE, NONE);
			accessor(departure_seconds, NONE, NONE);
		};


	}
}

using namespace Transit_Route_Components::Prototypes;
