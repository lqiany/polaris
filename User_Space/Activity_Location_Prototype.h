#pragma once
#include "User_Space.h"

namespace Activity_Location_Components
{
	namespace Types
	{
	}

	namespace Concepts
	{
	}
	
	namespace Prototypes
	{
		prototype struct Activity_Location_Prototype
		{
			tag_as_prototype;

			feature_accessor(origin_links, none, none);
			feature_accessor(destination_links, none, none);
			feature_accessor(origin_link_choice_cdfs, none, none);
			feature_accessor(destination_link_choice_cdfs, none, none);
			feature_accessor(zone, none, none);
			feature_accessor(uuid, none, none);
			feature_accessor(internal_id, none, none);
		};
	}
}

using namespace Activity_Location_Components::Prototypes;