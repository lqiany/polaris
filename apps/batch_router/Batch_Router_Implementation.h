#pragma once

#include "Batch_Router_Prototype.h"
#include "Movement_Plan_Prototype.h"
#include "Routing_Prototype.h"


namespace Batch_Router_Components
{
	namespace Types
	{

	}

	namespace Concepts
	{
	}
	
	namespace Implementations
	{
		implementation struct Routed_Trip_Implementation : public Polaris_Component<MasterType,INHERIT(Routed_Trip_Implementation),Execution_Object>
		{
			// Tag as implementation
			typedef typename Polaris_Component<MasterType,INHERIT(Routed_Trip_Implementation),Execution_Object>::Component_Type ComponentType;

			m_prototype(Network_Components::Prototypes::Network,typename MasterType::network_type, network_reference, NONE, NONE);

			m_prototype(Routing_Components::Prototypes::Routing,typename MasterType::routing_type, router, NONE, NONE);
			typedef strip_modifiers(router_type) router_interface;

			m_prototype(Activity_Location_Components::Prototypes::Activity_Location,typename MasterType::activity_location_type, origin, NONE, NONE);
			m_prototype(Activity_Location_Components::Prototypes::Activity_Location,typename MasterType::activity_location_type, destination, NONE, NONE);
			typedef strip_modifiers(origin_type) location_interface;

			m_data(Vehicle_Components::Types::Vehicle_Type_Keys, mode, NONE,NONE);

			member_component_and_feature_accessor(departure_time, Value, Basic_Units::Prototypes::Time, Basic_Units::Implementations::Time_Implementation<NT>);

			m_data(bool, link_failure,NONE,NONE);
			m_data(bool, turn_failure,NONE,NONE);

			m_container(concat(std::vector<pair<bool,location_interface*>>),origin_routing_results, NONE, NONE);
			m_container(concat(std::vector<pair<bool,location_interface*>>),destination_routing_results, NONE, NONE);

			typedef strip_modifiers(network_reference_type) network_itf;
			typedef strip_modifiers(router_type) router_itf;
			typedef Pair_Associative_Container<typename network_itf::get_type_of(zones_container)> zones_itf;
			typedef Zone_Components::Prototypes::Zone<get_component_type(zones_itf)> zone_itf;
			typedef Activity_Location_Components::Prototypes::Activity_Location<typename remove_pointer<typename zone_itf::get_type_of(origin_activity_locations)::value_type>::type> location_itf;
			typedef Random_Access_Sequence<typename zone_itf::get_type_of(origin_activity_locations),location_itf*> locations_itf;
			typedef Random_Access_Sequence<typename location_itf::get_type_of(origin_links)> links_itf;
			typedef Link_Components::Prototypes::Link<get_component_type(links_itf)> link_itf;
			typedef Random_Access_Sequence<typename link_itf::get_type_of(outbound_turn_movements)> turns_itf;
			typedef Turn_Movement_Components::Prototypes::Movement<get_component_type(turns_itf)> turn_itf;
			typedef Movement_Plan_Components::Prototypes::Movement_Plan< typename router_itf::get_type_of(movement_plan)> _Movement_Plan_Interface;
			typedef Random_Access_Sequence<typename _Movement_Plan_Interface::get_type_of(trajectory_container)> _Trajectory_Container_Interface;
			typedef Movement_Plan_Components::Prototypes::Trajectory_Unit<get_component_type(_Trajectory_Container_Interface)> _Trajectory_Unit_Interface;

			template<typename TargetType> void Get_Results()
			{
				_Movement_Plan_Interface* movement = this->_router->movement_plan<_Movement_Plan_Interface*>();

				if (movement->valid_trajectory<bool>())
				{
					cout <<"Travel time for trip was "<< movement->routed_travel_time<float>()<<endl;
					cout <<"Link.dir: travel time"<<endl;
					
					_Trajectory_Container_Interface* trajectory = movement->trajectory_container<_Trajectory_Container_Interface*>();

					for (_Trajectory_Container_Interface::iterator itr = trajectory->begin(); itr != trajectory->end(); ++itr)
					{
						_Trajectory_Unit_Interface* tu = (_Trajectory_Unit_Interface*)*itr;
						link_itf* link = tu->link<link_itf*>();
						cout << link->dbid<int>()<<"."<<link->direction<int>()<<": "<<tu->estimated_link_accepting_time<int>()<<endl;
					}					
				}
				else
				{
					cout<<"Error: movement was not routable."<<endl;
				}
				
			}
			
		};



	}

}
