#pragma once

#include "Household_Prototype.h"
#include "Movement_Plan_Prototype.h"
#include "Network_Skimming_Prototype.h"
#include "Activity_Prototype.h"


namespace Household_Components
{
	namespace Implementations
	{
		//==================================================================================
		/// Person Agent classes
		//----------------------------------------------------------------------------------
		implementation struct Household_Implementation : public Polaris_Component<MasterType,INHERIT(Household_Implementation),Data_Object>
		{
			// Tag as Implementation
			typedef typename Polaris_Component<MasterType,INHERIT(Household_Implementation),Data_Object>::Component_Type ComponentType;


			//=======================================================================================================================================================================
			// DATA MEMBERS
			//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
			m_prototype(PopSyn::Prototypes::Synthesis_Zone< typename MasterType::zone>, home_synthesis_zone, NONE, NONE);	
			m_prototype(Prototypes::Household_Properties< typename MasterType::household_properties_type>, Properties, NONE,check_2(ComponentType, Is_Same_Entity));
			m_prototype(Prototypes::Household_Properties<typename MasterType::household_static_properties_type>, Static_Properties, NONE, NONE);
			
			// Physical world elements
			m_prototype(Network_Components::Prototypes::Network< typename MasterType::network_type>, network_reference, NONE, NONE);
			m_prototype(Scenario_Components::Prototypes::Scenario< typename MasterType::scenario_type>, scenario_reference, NONE, NONE);

			// Agent ID
			m_data(long,uuid, NONE, NONE);
			m_data(long,internal_id, NONE, NONE);
			
	
			// Household members
			m_container(boost::container::vector<typename MasterType::person_type*>, Persons_Container, NONE, NONE);

			//=======================================================================================================================================================================
			// INTERFACE DEFINITIONS
			//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
			typedef  Zone_Components::Prototypes::Zone<typename remove_pointer< typename network_reference_interface::get_type_of(zones_container)::value_type>::type>  zone_interface;
			typedef Pair_Associative_Container< typename network_reference_interface::get_type_of(zones_container), zone_interface*> zones_container_interface;

			typedef  Activity_Location_Components::Prototypes::Activity_Location<typename remove_pointer< typename network_reference_interface::get_type_of(activity_locations_container)::value_type>::type>  location_interface;
			typedef Random_Access_Sequence< typename network_reference_interface::get_type_of(activity_locations_container), location_interface*> locations_container_interface;


			//=======================================================================================================================================================================
			// FEATURES
			//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------
			template<typename TargetType> void Initialize(TargetType id)
			{	
				// Set the initial iteration to process
				//this->First_Iteration<Time_Minutes>(1.0);

				// Create and Initialize the Properties faculty
				_Properties = (Properties_interface*)Allocate<type_of(Properties)>();
				_Properties->template Initialize<void>();
				_Properties->template Parent_Household<ComponentType*>(this);

					
				// Add basic traveler properties							
				this->template uuid<int>(id);
				this->template internal_id<int>(id);

				
			}
			template<typename TargetType> void Initialize(typename TargetType::ParamType id, typename TargetType::Param2Type home_zone, typename TargetType::Param3Type network_ref, typename TargetType::Param4Type scenario_ref)
			{
				this->Initialize< typename TargetType::ParamType>(id);
				this->home_synthesis_zone< typename TargetType::Param2Type>(home_zone);
				this->_network_reference = (network_reference_interface*)network_ref;
				this->_scenario_reference = (scenario_reference_interface*)scenario_ref;

			}
			tag_feature_as_available(Initialize);

			template<typename TargetType> void Set_Home_Location()
			{
//TODO
//				_Properties->template Initialize<Target_Type<NT,void,home_synthesis_zone_interface*> >(this->_home_synthesis_zone);
			}

		};

	}
}