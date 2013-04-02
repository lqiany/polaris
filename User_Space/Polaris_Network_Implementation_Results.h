#pragma once
#include "Network_Prototype.h"

namespace Network_Components
{
	namespace Types
	{
	}

	namespace Concepts
	{
	}
	
	namespace Implementations
	{

		feature_implementation_definition void Polaris_Network_Implementation<MasterType,ParentType,InheritanceList>::write_vehicle_trajectory()
		{
			define_container_and_value_interface_unqualified_container(_Intersections_Container_Interface, _Intersection_Interface, type_of(intersections_container), Random_Access_Sequence_Prototype, Intersection_Components::Prototypes::Intersection_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Links_Container_Interface, _Link_Interface, type_of(links_container), Random_Access_Sequence_Prototype, Link_Components::Prototypes::Link_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Turn_Movements_Container_Interface, _Turn_Movement_Interface, type_of(turn_movements_container), Random_Access_Sequence_Prototype, Turn_Movement_Components::Prototypes::Movement_Prototype, ComponentType);
			define_container_and_value_interface(_Outbound_Inbound_Movements_Container_Interface, _Outbound_Inbound_Movements_Interface, typename _Intersection_Interface::get_type_of(outbound_inbound_movements), Random_Access_Sequence_Prototype, Intersection_Components::Prototypes::Outbound_Inbound_Movements_Prototype, ComponentType);			
			define_container_and_value_interface(_Inbound_Outbound_Movements_Container_Interface, _Inbound_Outbound_Movements_Interface, typename _Intersection_Interface::get_type_of(inbound_outbound_movements), Random_Access_Sequence_Prototype, Intersection_Components::Prototypes::Inbound_Outbound_Movements_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Activity_Locations_Container_Interface, _Activity_Location_Interface, type_of(activity_locations_container), Random_Access_Sequence_Prototype, Activity_Location_Components::Prototypes::Activity_Location_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Zones_Container_Interface, _Zone_Interface, type_of(zones_container), Random_Access_Sequence_Prototype, Zone_Components::Prototypes::Zone_Prototype, ComponentType);
			define_container_and_value_interface(_Vehicles_Container_Interface, _Vehicle_Interface, typename _Link_Interface::get_type_of(link_destination_vehicle_queue), Back_Insertion_Sequence_Prototype, Vehicle_Components::Prototypes::Vehicle_Prototype, ComponentType);
			define_component_interface(_Movement_Plan_Interface, typename _Vehicle_Interface::get_type_of(movement_plan), Movement_Plan_Components::Prototypes::Movement_Plan_Prototype, ComponentType);				
			define_container_and_value_interface(_Trajecotry_Container_Interface, _Trajectory_Unit_Interface, typename _Movement_Plan_Interface::get_type_of(trajectory_container), Random_Access_Sequence_Prototype, Movement_Plan_Components::Prototypes::Trajectory_Unit_Prototype, ComponentType);
			typedef Scenario_Prototype<typename MasterType::scenario_type> _Scenario_Interface;
			fstream& vehicle_trajectory_file = scenario_reference<ComponentType,CallerType,_Scenario_Interface*>()->template vehicle_trajectory_file<fstream&>();
				
			typename _Links_Container_Interface::iterator link_itr;
			for(link_itr = _links_container.begin(); link_itr != _links_container.end(); link_itr++)
			{
				_Link_Interface* destination_link = (_Link_Interface*)(*link_itr);
				int num_arrived_vehicls_of_a_link = (int)destination_link->template link_destination_vehicle_queue<_Vehicles_Container_Interface&>().size();
				if (num_arrived_vehicls_of_a_link > 0)
				{//output vehicle trajectory
					while(num_arrived_vehicls_of_a_link)
					{
						_Vehicle_Interface* vehicle = destination_link->template link_destination_vehicle_queue<_Vehicles_Container_Interface&>().front();
						_Movement_Plan_Interface* movement_plan = vehicle->template movement_plan<_Movement_Plan_Interface*>();
						destination_link->template link_destination_vehicle_queue<_Vehicles_Container_Interface&>().pop_front();

						int vehicle_id = vehicle->template uuid<int>();
						int origin_zone_index = 0;
						int destination_zone_index = 0;
						int origin_activity_location_index = 0;
						int destination_activity_location_index = 0;
						int origin_link_index = movement_plan->template origin<_Link_Interface*>()->template uuid<int>();
						int destination_link_index = movement_plan->template destination<_Link_Interface*>()->template uuid<int>();
						int num_links = (int)movement_plan->template trajectory_container<_Trajecotry_Container_Interface&>().size();

						int departure_time = movement_plan->template departed_time<int>();
						int arrival_time = movement_plan->template arrived_time<int>();
						float travel_time = float ((arrival_time - departure_time)/60.0f);

							
						vehicle_trajectory_file
							<< vehicle_id << ","
							<< 0 << ","
							<< 0 << ","
							<< 0 << ","
							<< 0 << ","
							<< origin_link_index << ","
							<< destination_link_index << ","
							<< num_links << ","
							<< convert_seconds_to_hhmmss(departure_time)<< ","
							<< convert_seconds_to_hhmmss(arrival_time)<< ","
							<< travel_time
							<<endl;

						float path_delayed_time = 0;
						for (int route_link_counter=0;route_link_counter<num_links;route_link_counter++)
						{

							_Trajectory_Unit_Interface* trajectory_unit = movement_plan->template trajectory_container<_Trajecotry_Container_Interface&>()[route_link_counter];
							_Link_Interface* route_link = trajectory_unit->template link<_Link_Interface*>();
							int route_link_id = route_link->template uuid<int>();
							int route_link_enter_time = trajectory_unit->template enter_time<int>();
							float route_link_delayed_time = float(trajectory_unit->template delayed_time<float>()/60.0f);
								
							int route_link_exit_time = movement_plan->template get_route_link_exit_time<NULLTYPE>(route_link_counter);
							float route_link_travel_time = float((route_link_exit_time - route_link_enter_time)/60.0f);

							path_delayed_time+=route_link_delayed_time;
					
							vehicle_trajectory_file
								<<route_link_counter + 1 << ","
								<<route_link_id << ","
								<<convert_seconds_to_hhmmss(route_link_enter_time) << ","
								<<route_link_travel_time << ","
								<<route_link_delayed_time
								<<endl;
						}

						//deallocate vehicle

						num_arrived_vehicls_of_a_link--;
					}
				}
			}

		};

		feature_implementation_definition void Polaris_Network_Implementation<MasterType,ParentType,InheritanceList>::write_network_link_flow()
		{
			define_container_and_value_interface_unqualified_container(_Links_Container_Interface, _Link_Interface, type_of(links_container), Random_Access_Sequence_Prototype, Link_Components::Prototypes::Link_Prototype, ComponentType);
			define_container_and_value_interface(_Vehicles_Container_Interface, _Vehicle_Interface, typename _Link_Interface::get_type_of(link_destination_vehicle_queue), Back_Insertion_Sequence_Prototype, Vehicle_Components::Prototypes::Vehicle_Prototype, ComponentType);
			typedef Scenario_Prototype<typename MasterType::scenario_type> _Scenario_Interface;
			typedef Network_Prototype<typename MasterType::network_type> _Network_Interface;
			int simulation_interval_length = ((_Scenario_Interface*)_global_scenario)->template simulation_interval_length<int>();
			int simulation_interval_index = ((_Network_Interface*)this)->template current_simulation_interval_index<int>();
				
			fstream& network_link_flow_file = scenario_reference<ComponentType,CallerType,_Scenario_Interface*>()->template network_link_flow_file<fstream&>();
				
			typename _Links_Container_Interface::iterator link_itr;
			for(link_itr = _links_container.begin(); link_itr != _links_container.end(); link_itr++)
			{
				//flow
				_Link_Interface* link = (_Link_Interface*)(*link_itr);
				float bwtt = (float) (link->template length<float>()/(link->template backward_wave_speed<float>()*5280.0/3600.0)); // in seconds
				float fftt = (float) (link->template length<float>()/(link->template free_flow_speed<float>()*5280.0/3600.0)); //in seconds

				int link_fftt_cached_simulation_interval_size = int(ceil(float(fftt/((float)simulation_interval_length))));
				int link_bwtt_cached_simulation_interval_size = int(ceil(float(bwtt/((float)simulation_interval_length))));
					
				network_link_flow_file
					<< convert_seconds_to_hhmmss(simulation_interval_index) <<  ","
					<< link->template internal_id<int>() <<  ","
					<< link_fftt_cached_simulation_interval_size <<  ","
					<< link_bwtt_cached_simulation_interval_size <<  ","
					<< link->template link_origin_cumulative_arrived_vehicles<int>() << ","
					<< link->template link_origin_cumulative_departed_vehicles<int>() << ","
					<< link->template link_destination_cumulative_arrived_vehicles<int>() << ","
					<< link->template link_upstream_cumulative_arrived_vehicles<int>() << ","
					<< link->template link_upstream_cumulative_vehicles<int>() << ","
					<< link->template link_shifted_cumulative_arrived_vehicles<int>() << ","
					<< link->template link_downstream_cumulative_vehicles<int>() << ","
					<< link->template link_num_vehicles_in_queue<int>()
					<<endl;		
			}
		};

		feature_implementation_definition void Polaris_Network_Implementation<MasterType,ParentType,InheritanceList>::write_network_link_turn_time()
		{
			define_container_and_value_interface_unqualified_container(_Links_Container_Interface, _Link_Interface, type_of(links_container), Random_Access_Sequence_Prototype, Link_Components::Prototypes::Link_Prototype, ComponentType);
			define_container_and_value_interface(_Vehicles_Container_Interface, _Vehicle_Interface, typename _Link_Interface::get_type_of(link_destination_vehicle_queue), Back_Insertion_Sequence_Prototype, Vehicle_Components::Prototypes::Vehicle_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Turn_Movements_Container_Interface, _Turn_Movement_Interface, type_of(turn_movements_container), Random_Access_Sequence_Prototype, Turn_Movement_Components::Prototypes::Movement_Prototype, ComponentType);
			typedef Scenario_Prototype<typename MasterType::scenario_type> _Scenario_Interface;
			define_container_and_value_interface_unqualified_container(_Intersections_Container_Interface, _Intersection_Interface, type_of(intersections_container), Random_Access_Sequence_Prototype, Intersection_Components::Prototypes::Intersection_Prototype, ComponentType);
			typedef Network_Prototype<typename MasterType::network_type> _Network_Interface;
			_Scenario_Interface* scenario = (_Scenario_Interface*)_scenario_reference;
			int simulation_interval_length = scenario->template simulation_interval_length<int>();
			int simulation_interval_index = ((_Network_Interface*)this)->template current_simulation_interval_index<int>();
			int simulation_start_time = scenario->template simulation_start_time<int>();
			int current_starting_time = simulation_start_time + simulation_interval_length*simulation_interval_index;

			fstream& network_link_turn_time_file = scenario->template network_link_turn_time_file<fstream&>();
			typename _Turn_Movements_Container_Interface::iterator movement_itr;
			for (movement_itr = _turn_movements_container.begin(); movement_itr != _turn_movements_container.end(); movement_itr++)
			{
				_Turn_Movement_Interface* movement = (_Turn_Movement_Interface*)(*movement_itr);
				_Link_Interface* link = movement->template inbound_link<_Link_Interface*>();
					
				float fftt = (float) (movement->template inbound_link<_Link_Interface*>()->template length<float>()/(movement->template inbound_link<_Link_Interface*>()->template free_flow_speed<float>()*5280.0/3600.0)); //in seconds
				int link_fftt_cached_simulation_interval_size = int(ceil(float(fftt/(simulation_interval_length*1.0))));
					
					
				network_link_turn_time_file
					<< convert_seconds_to_hhmmss(current_starting_time) <<  ","
					<< current_starting_time << ","
					<< movement->template inbound_link<_Link_Interface*>()->template downstream_intersection<_Intersection_Interface*>()->template uuid<int>() << ","
					<< movement->template uuid<int>() <<  ","
					<< movement->template inbound_link<_Link_Interface*>()->template uuid<int>() <<  ","
					<< movement->template outbound_link<_Link_Interface*>()->template uuid<int>() <<  ","
					<< fftt << ","
					<< link_fftt_cached_simulation_interval_size*simulation_interval_length << ","

					<< movement->template inbound_link<_Link_Interface*>()->template link_supply<float>() << ","
					<< movement->template outbound_link<_Link_Interface*>()->template link_supply<float>() << ","
					<< movement->template inbound_link<_Link_Interface*>()->template link_origin_arrived_vehicles<int>() << ","
					<<movement->template outbound_link<_Link_Interface*>()->template link_origin_arrived_vehicles<int>() << ","
					<< movement->template inbound_link<_Link_Interface*>()->template link_origin_departed_vehicles<int>() << ","
					<<movement->template outbound_link<_Link_Interface*>()->template link_origin_departed_vehicles<int>() << ","
					<< movement->template outbound_link_arrived_time_based_experienced_link_turn_travel_delay<float>() << ","
					<< movement->template green_time<float>() << ","
					<< movement->template movement_capacity<float>() << ","
					<< movement->template movement_demand<float>() << ","
					<< movement->template movement_supply<float>() << ","
					<< movement->template movement_flow<float>() << ","
					<< movement->template movement_transferred<float>() << ","
					//<< movement->template vehicles_container<_Vehicles_Container_Interface&>().size() << ","
					<< movement->template turn_travel_penalty<float>() << ","
					<< movement->template forward_link_turn_travel_time<float>()
					<<endl;

			}

		};

		feature_implementation_definition void Polaris_Network_Implementation<MasterType,ParentType,InheritanceList>::write_node_control_state()
		{

			define_container_and_value_interface_unqualified_container(_Intersections_Container_Interface, _Intersection_Interface, type_of(intersections_container), Random_Access_Sequence_Prototype, Intersection_Components::Prototypes::Intersection_Prototype, ComponentType);
			define_component_interface(_Intersection_Control_Interface, typename _Intersection_Interface::get_type_of(intersection_control), Intersection_Control_Components::Prototypes::Intersection_Control_Prototype, ComponentType);
			define_container_and_value_interface(_Control_Plans_Container_Interface, _Control_Plan_Interface, typename _Intersection_Control_Interface::get_type_of(control_plan_data_array), Random_Access_Sequence_Prototype, Intersection_Control_Components::Prototypes::Control_Plan_Prototype, ComponentType);
			define_container_and_value_interface(_Phases_Container_Interface, _Phase_Interface, typename _Control_Plan_Interface::get_type_of(phase_data_array), Random_Access_Sequence_Prototype, Intersection_Control_Components::Prototypes::Phase_Prototype, ComponentType);
			define_container_and_value_interface(_Phase_Movements_Container_Interface, _Phase_Movement_Interface, typename _Phase_Interface::get_type_of(turn_movements_in_the_phase_array), Random_Access_Sequence_Prototype, Intersection_Control_Components::Prototypes::Phase_Movement_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Movements_Container_Interface, _Movement_Interface, type_of(turn_movements_container), Random_Access_Sequence_Prototype, Turn_Movement_Components::Prototypes::Movement_Prototype, ComponentType);
			define_container_and_value_interface(_Approaches_Container_Interface, _Approach_Interface, typename _Control_Plan_Interface::get_type_of(approach_data_array), Random_Access_Sequence_Prototype, Intersection_Control_Components::Prototypes::Approach_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Links_Container_Interface, _Link_Interface, type_of(links_container), Random_Access_Sequence_Prototype, Link_Components::Prototypes::Link_Prototype, ComponentType);
			typedef Network_Prototype<typename MasterType::network_type> _Network_Interface;
			define_component_interface(_Scenario_Interface, type_of(scenario_reference), Scenario_Components::Prototypes::Scenario_Prototype, ComponentType);
			_Network_Interface* _this_ptr = (_Network_Interface*)this;				
			_Scenario_Interface* scenario = (_Scenario_Interface*)_global_scenario;

			fstream& network_node_control_state_file = scenario_reference<ComponentType,CallerType,_Scenario_Interface*>()->template network_node_control_state_file<fstream&>();
			for (int i = 0; i < (int)intersections_container<_Intersections_Container_Interface&>().size(); i++)
			{
				_Intersection_Interface* intersection = intersections_container<_Intersections_Container_Interface&>()[i];
				_Intersection_Control_Interface* intersection_control = intersection->template intersection_control<_Intersection_Control_Interface*>();
				_Control_Plan_Interface* current_control_plan = intersection_control->template current_control_plan<_Control_Plan_Interface*>();
				int starting_time = current_control_plan->template starting_time<int>();
				int ending_time = current_control_plan->template ending_time<int>();
				Intersection_Components::Types::Intersection_Type_Keys control_type = current_control_plan->template control_type<Intersection_Components::Types::Intersection_Type_Keys>();

				network_node_control_state_file
					<< convert_seconds_to_hhmmss(_this_ptr->template start_of_current_simulation_interval<int>()) <<  ","
					<< _this_ptr->template current_simulation_interval_index<int>() <<  ","
					<< _this_ptr->template current_simulation_time<int>() <<  ","
					<< intersection->template uuid<int>() <<  ","
					<< current_control_plan->template control_plan_index<int>() <<  ","
					<< current_control_plan->template control_type<int>() <<  ","
					<< convert_seconds_to_hhmmss(starting_time) <<  ","
					<< convert_seconds_to_hhmmss(ending_time) <<  ","
					<< current_control_plan->template approach_data_array<_Approaches_Container_Interface&>().size() <<  ",";
		
				if (control_type == Intersection_Control_Components::Types::PRE_TIMED_SIGNAL_CONTROL || control_type == Intersection_Control_Components::Types::ACTUATED_SIGNAL_CONTROL)
				{
					int num_phases = (int)current_control_plan->template phase_data_array<_Phases_Container_Interface&>().size();
					int last_phase = num_phases - 1;
					int cycle_index = current_control_plan->template cycle_index<int>();
					int cycle_starting_time = current_control_plan->template cycle_starting_time<int>();
					int cycle_ending_time = current_control_plan->template cycle_ending_time<int>();
					int cycle_length = cycle_ending_time - cycle_starting_time;

					network_node_control_state_file
						<< cycle_index <<  ","
						<< cycle_length <<  ","
						<< convert_seconds_to_hhmmss(cycle_starting_time) <<  ","
						<< convert_seconds_to_hhmmss(cycle_ending_time) <<  ","
						<< num_phases << ",";

					for (int iphase=0;iphase<num_phases;iphase++)
					{
						_Phase_Interface* phase = current_control_plan->template phase_data_array<_Phases_Container_Interface&>()[iphase];
						int green_starting_time = phase->template green_starting_time<int>();
						int yellow_starting_time = cycle_starting_time +  phase->template yellow_starting_time<int>();
						int red_starting_time = cycle_starting_time + phase->template red_start_time<int>();
						int phase_end_time = red_starting_time + phase->template all_red_time<int>();

						network_node_control_state_file
							<< iphase <<  ","
							<< convert_seconds_to_hhmmss(green_starting_time) <<  ","
							<< convert_seconds_to_hhmmss(yellow_starting_time) <<  ","
							<< convert_seconds_to_hhmmss(red_starting_time) <<  ","
							<< convert_seconds_to_hhmmss(phase_end_time) <<  ",";
					}
				}
				network_node_control_state_file <<endl;
			}
		}

		feature_implementation_definition void Polaris_Network_Implementation<MasterType,ParentType,InheritanceList>::write_output_summary()
		{

			define_container_and_value_interface_unqualified_container(_Intersections_Container_Interface, _Intersection_Interface, type_of(intersections_container), Random_Access_Sequence_Prototype, Intersection_Components::Prototypes::Intersection_Prototype, ComponentType);
			define_component_interface(_Intersection_Control_Interface, typename _Intersection_Interface::get_type_of(intersection_control), Intersection_Control_Components::Prototypes::Intersection_Control_Prototype, ComponentType);
			define_container_and_value_interface(_Control_Plans_Container_Interface, _Control_Plan_Interface, typename _Intersection_Control_Interface::get_type_of(control_plan_data_array), Random_Access_Sequence_Prototype, Intersection_Control_Components::Prototypes::Control_Plan_Prototype, ComponentType);
			define_container_and_value_interface(_Phases_Container_Interface, _Phase_Interface, typename _Control_Plan_Interface::get_type_of(phase_data_array), Random_Access_Sequence_Prototype, Intersection_Control_Components::Prototypes::Phase_Prototype, ComponentType);
			define_container_and_value_interface(_Phase_Movements_Container_Interface, _Phase_Movement_Interface, typename _Phase_Interface::get_type_of(turn_movements_in_the_phase_array), Random_Access_Sequence_Prototype, Intersection_Control_Components::Prototypes::Phase_Movement_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Movements_Container_Interface, _Movement_Interface, type_of(turn_movements_container), Random_Access_Sequence_Prototype, Turn_Movement_Components::Prototypes::Movement_Prototype, ComponentType);
			define_container_and_value_interface(_Approaches_Container_Interface, _Approach_Interface, typename _Control_Plan_Interface::get_type_of(approach_data_array), Random_Access_Sequence_Prototype, Intersection_Control_Components::Prototypes::Approach_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Links_Container_Interface, _Link_Interface, type_of(links_container), Random_Access_Sequence_Prototype, Link_Components::Prototypes::Link_Prototype, ComponentType);
			typedef Network_Prototype<typename MasterType::network_type> _Network_Interface;
			define_component_interface(_Scenario_Interface, type_of(scenario_reference), Scenario_Components::Prototypes::Scenario_Prototype, ComponentType);

			_Network_Interface* _this_ptr = (_Network_Interface*)this;
			_Scenario_Interface* scenario = scenario_reference<ComponentType,CallerType,_Scenario_Interface*>();

			fstream& output_summary_file = scenario->template output_summary_file<fstream&>();

            _current_cpu_time_in_seconds = (long)get_current_cpu_time_in_seconds();

            long elapsed_time = _current_cpu_time_in_seconds - _start_cpu_time_in_seconds;
            output_summary_file
                    << convert_seconds_to_hhmmss(_this_ptr->template start_of_current_simulation_interval_absolute<int>()).c_str() << ","
                    << scenario->template network_cumulative_loaded_vehicles<int>() <<  ","
                    << scenario->template network_cumulative_departed_vehicles<int>() << ","
                    << scenario->template network_cumulative_arrived_vehicles<int>() << ","
                    << scenario->template network_in_network_vehicles<int>() << ","
					<< _network_vmt << ","
					<< _network_vht << ","
                    << convert_seconds_to_hhmmss(elapsed_time).c_str() << ","
					<< _this_ptr->template start_of_current_simulation_interval_absolute<int>()
                    <<endl;
		}

		feature_implementation_definition void Polaris_Network_Implementation<MasterType,ParentType,InheritanceList>::output_moe_for_simulation_interval()
		{
			define_component_interface(_Scenario_Interface, type_of(scenario_reference), Scenario_Components::Prototypes::Scenario_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Links_Container_Interface, _Link_Interface, type_of(links_container), Random_Access_Sequence_Prototype, Link_Components::Prototypes::Link_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Turn_Movements_Container_Interface, _Turn_Movement_Interface, type_of(turn_movements_container), Random_Access_Sequence_Prototype, Turn_Movement_Components::Prototypes::Movement_Prototype, ComponentType);

			typedef Network_Prototype<typename MasterType::network_type> _Network_Interface;
			int time = ((_Network_Interface*)this)->template start_of_current_simulation_interval_absolute<int>()+((_Scenario_Interface*)_global_scenario)->template simulation_interval_length<int>();

			// output link moe
			typedef typename MasterType::link_type _link_component_type;
			typename _Links_Container_Interface::iterator link_itr;
			for(link_itr = _links_container.begin(); link_itr != _links_container.end(); link_itr++)
			{
				_link_component_type* link = (_link_component_type*)(*link_itr);
				((_Scenario_Interface*)_global_scenario)->template out_realtime_link_moe_file<fstream&>()
					<< convert_seconds_to_hhmmss(time).c_str() << ","
					<< time << ","
					<< link->_uuid << ","
					<< link->_upstream_intersection->_uuid << ","
					<< link->_downstream_intersection->_uuid << ","
					<< link->_link_type << ","	
					<< link->realtime_link_moe_data.link_travel_time << ","
					<< link->realtime_link_moe_data.link_travel_delay << ","
					<< link->realtime_link_moe_data.link_queue_length << ","
					<< link->realtime_link_moe_data.link_speed << ","
					<< link->realtime_link_moe_data.link_density << ","
					<< link->realtime_link_moe_data.link_in_volume << ","
					<< link->realtime_link_moe_data.link_out_volume << ","
					<< link->realtime_link_moe_data.link_travel_time_ratio << ","
					<< link->realtime_link_moe_data.link_speed_ratio << ","
					<< link->realtime_link_moe_data.link_density_ratio << ","
					<< endl;

			}
			// output turn movement moe
			typedef typename MasterType::turn_movement_type _movement_component_type;
			typename _Turn_Movements_Container_Interface::iterator movement_itr;
			for(movement_itr = _turn_movements_container.begin(); movement_itr != _turn_movements_container.end(); movement_itr++)
			{
				_movement_component_type* movement = (_movement_component_type*)(*movement_itr);
				((_Scenario_Interface*)_global_scenario)->template out_realtime_movement_moe_file<fstream&>() 
					<< convert_seconds_to_hhmmss(time).c_str() << ","
					<< time << ","
					<< movement->_uuid << ","
					<< movement->_inbound_link->_uuid << ","
					<< movement->_outbound_link->_uuid << ","
					<< movement->_inbound_link->_downstream_intersection->_uuid << ","
					<< movement->realtime_movement_moe_data.turn_penalty << ","
					<< movement->realtime_movement_moe_data.inbound_link_turn_time << ","
					<< movement->realtime_movement_moe_data.outbound_link_turn_time << ","
					<< movement->realtime_movement_moe_data.movement_flow_rate
					<<endl;

			}
			// output network moe

			((_Scenario_Interface*)_global_scenario)->template out_realtime_network_moe_file<fstream&>()
				<< convert_seconds_to_hhmmss(time).c_str() << ","
				<< time << ","
				<< realtime_network_moe_data.num_loaded_vehicles << ","
				<< realtime_network_moe_data.num_departed_vehicles << ","
				<< realtime_network_moe_data.num_arrived_vehicles << ","
				<< realtime_network_moe_data.network_avg_link_travel_time << ","
				<< realtime_network_moe_data.network_avg_link_speed << ","
				<< realtime_network_moe_data.network_avg_link_density << ","
				<< realtime_network_moe_data.network_avg_link_in_volume << ","
				<< realtime_network_moe_data.network_avg_link_out_volume << ","
				<< realtime_network_moe_data.network_avg_link_travel_time_ratio << ","
				<< realtime_network_moe_data.network_avg_link_speed_ratio << ","
				<< realtime_network_moe_data.network_avg_link_density_ratio << ","
				<< endl;

		}

		feature_implementation_definition void Polaris_Network_Implementation<MasterType,ParentType,InheritanceList>::output_moe_for_assignment_interval()
		{
			define_component_interface(_Scenario_Interface, type_of(scenario_reference), Scenario_Components::Prototypes::Scenario_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Links_Container_Interface, _Link_Interface, type_of(links_container), Random_Access_Sequence_Prototype, Link_Components::Prototypes::Link_Prototype, ComponentType);
			define_container_and_value_interface_unqualified_container(_Turn_Movements_Container_Interface, _Turn_Movement_Interface, type_of(turn_movements_container), Random_Access_Sequence_Prototype, Turn_Movement_Components::Prototypes::Movement_Prototype, ComponentType);

			typedef Network_Prototype<typename MasterType::network_type> _Network_Interface;
			int time = ((_Network_Interface*)this)->template start_of_current_simulation_interval_absolute<int>()+((_Scenario_Interface*)_global_scenario)->template simulation_interval_length<int>();

			//// output link moe
			//typedef typename MasterType::link_type _link_component_type;
			//typename _Links_Container_Interface::iterator link_itr;
			//for(link_itr = _links_container.begin(); link_itr != _links_container.end(); link_itr++)
			//{
			//	_link_component_type* link = (_link_component_type*)(*link_itr);
			//	((_Scenario_Interface*)_global_scenario)->template out_link_moe_file<fstream&>()
			//		<< convert_seconds_to_hhmmss(time).c_str() << ","
			//		<< time << ","
			//		<< link->_uuid << ","
			//		<< link->_upstream_intersection->_uuid << ","
			//		<< link->_downstream_intersection->_uuid << ","
			//		<< link->_link_type << ","	
			//		<< link->link_moe_data.link_travel_time << ","
			//		<< link->link_moe_data.link_travel_time_standard_deviation << ","
			//		<< link->link_moe_data.link_travel_delay << ","
			//		<< link->link_moe_data.link_travel_delay_standard_deviation << ","
			//		<< link->link_moe_data.link_queue_length << ","
			//		<< link->link_moe_data.link_speed << ","
			//		<< link->link_moe_data.link_density << ","
			//		<< link->link_moe_data.link_in_flow_rate << ","
			//		<< link->link_moe_data.link_out_flow_rate << ","
			//		<< link->link_moe_data.link_in_volume << ","
			//		<< link->link_moe_data.link_out_volume << ","
			//		<< link->link_moe_data.link_travel_time_ratio << ","
			//		<< link->link_moe_data.link_speed_ratio << ","
			//		<< link->link_moe_data.link_density_ratio << ","
			//		<< link->link_moe_data.link_in_flow_ratio << ","
			//		<< link->link_moe_data.link_out_flow_ratio << ","
			//		<< link->_link_vht << ","
			//		<< link->_link_vmt
			//		<< endl;

			//}
			//// output turn movement moe
			//typedef typename MasterType::turn_movement_type _movement_component_type;
			//typename _Turn_Movements_Container_Interface::iterator movement_itr;
			//for(movement_itr = _turn_movements_container.begin(); movement_itr != _turn_movements_container.end(); movement_itr++)
			//{
			//	_movement_component_type* movement = (_movement_component_type*)(*movement_itr);
			//	((_Scenario_Interface*)_global_scenario)->template out_movement_moe_file<fstream&>() 
			//		<< convert_seconds_to_hhmmss(time).c_str() << ","
			//		<< time << ","
			//		<< movement->_uuid << ","
			//		<< movement->_inbound_link->_uuid << ","
			//		<< movement->_outbound_link->_uuid << ","
			//		<< movement->_inbound_link->_downstream_intersection->_uuid << ","
			//		<< movement->movement_moe_data.turn_penalty << ","
			//		<< movement->movement_moe_data.turn_penalty_standard_deviation << ","
			//		<< movement->movement_moe_data.inbound_link_turn_time << ","
			//		<< movement->movement_moe_data.outbound_link_turn_time << ","
			//		<< movement->movement_moe_data.movement_flow_rate
			//		<<endl;

			//}

			// output network moe

			((_Scenario_Interface*)_global_scenario)->template out_network_moe_file<fstream&>()
				<< convert_seconds_to_hhmmss(time).c_str() << ","
				<< time << ","
				<< network_moe_data.num_loaded_vehicles << ","
				<< network_moe_data.num_departed_vehicles << ","
				<< network_moe_data.num_arrived_vehicles << ","
				<< network_moe_data.network_avg_link_travel_time << ","
				<< network_moe_data.network_avg_link_speed << ","
				<< network_moe_data.network_avg_link_density << ","
				<< network_moe_data.network_avg_link_in_flow_rate << ","
				<< network_moe_data.network_avg_link_out_flow_rate << ","
				<< network_moe_data.network_avg_link_in_volume << ","
				<< network_moe_data.network_avg_link_out_volume << ","
				<< network_moe_data.network_avg_link_travel_time_ratio << ","
				<< network_moe_data.network_avg_link_speed_ratio << ","
				<< network_moe_data.network_avg_link_density_ratio << ","
				<< network_moe_data.network_avg_link_in_flow_ratio << ","
				<< network_moe_data.network_avg_link_out_flow_ratio << ","
				<< _network_vht << ","
				<< _network_vmt 
				//<< network_moe_data.assignment_calculation_time << ","
				//<< network_moe_data.simulation_calculation_time << ","
				//<< network_moe_data.operation_calculation_time << ","
				//<< network_moe_data.output_calculation_time 
				<< endl;

		}

		//feature_implementation_definition void Polaris_Network_Implementation<MasterType,ParentType,InheritanceList>::output_moe()
		//{
		//	define_component_interface(_Scenario_Interface, type_of(scenario_reference), Scenario_Components::Prototypes::Scenario_Prototype, ComponentType);
		//	define_container_and_value_interface_unqualified_container(_Links_Container_Interface, _Link_Interface, type_of(links_container), Random_Access_Sequence_Prototype, Link_Components::Prototypes::Link_Prototype, ComponentType);
		//	define_container_and_value_interface_unqualified_container(_Turn_Movements_Container_Interface, _Turn_Movement_Interface, type_of(turn_movements_container), Random_Access_Sequence_Prototype, Turn_Movement_Components::Prototypes::Movement_Prototype, ComponentType);

		//	typedef Network_Prototype<typename MasterType::network_type> _Network_Interface;
		//	int time = ((_Network_Interface*)this)->template start_of_current_simulation_interval_absolute<int>()+((_Scenario_Interface*)_global_scenario)->template simulation_interval_length<int>();

		//	// output link moe
		//	typedef MasterType::link_type _link_component_type;
		//	typename _Links_Container_Interface::iterator link_itr;
		//	for(link_itr = _links_container.begin(); link_itr != _links_container.end(); link_itr++)
		//	{
		//		_link_component_type* link = (_link_component_type*)(*link_itr);
		//		fprintf_s(((_Scenario_Interface*)_global_scenario)->out_link_moe_file<FILE*>(),
		//			"%s,%d,%d,%d,%d,%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
		//			convert_seconds_to_hhmmss(time).c_str(),
		//			time,
		//			link->_uuid,
		//			link->_upstream_intersection->_uuid,
		//			link->_downstream_intersection->_uuid,
		//			link->_link_type,	
		//			link->link_moe_data.link_travel_time,
		//			link->link_moe_data.link_travel_time_standard_deviation,
		//			link->link_moe_data.link_travel_delay,
		//			link->link_moe_data.link_travel_delay_standard_deviation,
		//			link->link_moe_data.link_queue_length,
		//			link->link_moe_data.link_speed,
		//			link->link_moe_data.link_density,
		//			link->link_moe_data.link_in_flow_rate,
		//			link->link_moe_data.link_out_flow_rate,
		//			link->link_moe_data.link_travel_time_ratio,
		//			link->link_moe_data.link_speed_ratio,
		//			link->link_moe_data.link_density_ratio,
		//			link->link_moe_data.link_in_flow_ratio,
		//			link->link_moe_data.link_out_flow_ratio
		//			);
		//	}
		//	// output turn movement moe
		//	typedef MasterType::turn_movement_type _movement_component_type;
		//	typename _Turn_Movements_Container_Interface::iterator movement_itr;
		//	for(movement_itr = _turn_movements_container.begin(); movement_itr != _turn_movements_container.end(); movement_itr++)
		//	{
		//		_movement_component_type* movement = (_movement_component_type*)(*movement_itr);
		//		fprintf_s(((_Scenario_Interface*)_global_scenario)->out_link_moe_file<FILE*>(),
		//			"%s,%d,%d,%d,%d,%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
		//			convert_seconds_to_hhmmss(time).c_str(),
		//			time,
		//			movement->_uuid,
		//			movement->_inbound_link->_uuid,
		//			movement->_outbound_link->_uuid,
		//			movement->_inbound_link->_downstream_intersection->_uuid,
		//			movement->movement_moe_data.turn_penalty,
		//			movement->movement_moe_data.turn_penalty_standard_deviation,
		//			movement->movement_moe_data.inbound_link_turn_time,
		//			movement->movement_moe_data.outbound_link_turn_time,
		//			movement->movement_moe_data.movement_flow_rate
		//			);
		//	}
		//	// output network moe

		//	fprintf_s(((_Scenario_Interface*)_global_scenario)->out_network_moe_file<FILE*>(),
		//		"%s,%d,%d,%d,%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
		//		convert_seconds_to_hhmmss(time).c_str(),
		//		time,
		//		network_moe_data.num_loaded_vehicles,
		//		network_moe_data.num_departed_vehicles,
		//		network_moe_data.num_arrived_vehicles,
		//		network_moe_data.network_avg_link_travel_time,
		//		network_moe_data.network_avg_link_speed,
		//		network_moe_data.network_avg_link_density,
		//		network_moe_data.network_avg_link_in_flow_rate,
		//		network_moe_data.network_avg_link_out_flow_rate,
		//		network_moe_data.network_avg_link_travel_time_ratio,
		//		network_moe_data.network_avg_link_speed_ratio,
		//		network_moe_data.network_avg_link_density_ratio,
		//		network_moe_data.network_avg_link_in_flow_ratio,
		//		network_moe_data.network_avg_link_out_flow_ratio,
		//		network_moe_data.assignment_calculation_time,
		//		network_moe_data.simulation_calculation_time,
		//		network_moe_data.operation_calculation_time,
		//		network_moe_data.output_calculation_time
		//		);
		//}

	}
}
