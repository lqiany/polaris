﻿#pragma once
#include "Network_DB_Reader_Prototype.h"
#include "Intersection_Prototype.h"
#include "Turn_Movement_Prototype.h"
#include "Transit_Route_Implementation.h"
#include "Transit_Pattern_Implementation.h"
#include "Transit_Vehicle_Trip_Prototype.h"
#include "Activity_Location_Prototype.h"
#include "Link_Implementation.h"

namespace Network_Components
{
    //using std::shared_ptr;
	namespace Types
	{
		union Link_ID_Dir
		{
			struct
			{
				int id;
				int dir;
			};
			long long id_dir;
		};
	}
	
	namespace Implementations
	{
		implementation struct Network_DB_Reader_Implementation : public Polaris_Component<MasterType,INHERIT(Network_DB_Reader_Implementation),Execution_Object>
		{
			// Pointer to the Parent class
            typedef typename Polaris_Component<MasterType,INHERIT(Network_DB_Reader_Implementation),Execution_Object>::Component_Type ComponentType;
			m_prototype(Network,typename MasterType::network_type, network_reference, NONE, NONE);
			m_prototype(Scenario,typename MasterType::scenario_type, scenario_reference, NONE, NONE);

			//==================================================================================================================
			/// read from database
			//------------------------------------------------------------------------------------------------------------------

			template<typename TargetType> void read_network_data(Network_Components::Types::Network_IO_Maps& net_io_maps)
			{
				using namespace odb;
				using namespace polaris::io;
				typedef Scenario_Components::Prototypes::Scenario<typename MasterType::scenario_type> _Scenario_Interface;
				_scenario_reference = _network_reference->scenario_reference<_Scenario_Interface*>();

                cout << "Open database: "<<_scenario_reference->template database_name<string&>()<<"...";

				string name(_scenario_reference->template database_name<string&>());
				unique_ptr<database> db (open_sqlite_database (name));
				transaction t(db->begin());

				cout << "done."<<endl;

				read_intersection_data<TargetType>(db, net_io_maps);
				read_transit_route_data<TargetType>(db, net_io_maps);
				read_transit_pattern_data<TargetType>(db, net_io_maps);
				read_transit_vehicle_trip_data<TargetType>(db, net_io_maps);
				read_link_data<TargetType>(db, net_io_maps);
				read_turn_movement_data<TargetType>(db, net_io_maps);
				read_zone_data<TargetType>(db, net_io_maps);
				read_activity_location_data<TargetType>(db, net_io_maps);
				read_pocket_data<TargetType>(db, net_io_maps);

				if (!((_Scenario_Interface*)_global_scenario)->template multimodal_network_input<bool>())
				{
					clean_isolated_intersections<TargetType>();
				}
			}

			template<typename TargetType> void read_intersection_data(unique_ptr<odb::database>& db, Network_Components::Types::Network_IO_Maps& net_io_maps)
			{
				using namespace odb;
				using namespace polaris::io;

				typedef  Intersection_Components::Prototypes::Intersection<typename remove_pointer< typename type_of(network_reference)::get_type_of(intersections_container)::value_type>::type>  _Intersection_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(intersections_container), _Intersection_Interface*> _Intersections_Container_Interface;

				_Intersections_Container_Interface* intersections_container_ptr=_network_reference->template intersections_container<_Intersections_Container_Interface*>();
				typename type_of(network_reference)::type_of(intersections_container)& intersections_container_monitor=(typename type_of(network_reference)::type_of(intersections_container)&)(*intersections_container_ptr);
				typedef  Intersection_Control_Components::Prototypes::Intersection_Control< typename _Intersection_Interface::get_type_of(intersection_control)> _Intersection_Control_Interface;
				typedef  Intersection_Control_Components::Prototypes::Control_Plan<typename remove_pointer< typename _Intersection_Control_Interface::get_type_of(control_plan_data_array)::value_type>::type>  _Control_Plan_Interface;
				typedef  Random_Access_Sequence< typename _Intersection_Control_Interface::get_type_of(control_plan_data_array), _Control_Plan_Interface*> _Control_Plans_Container_Interface;

				
				intersections_container_ptr->clear();

				result<Node> node_result=db->template query<Node>(query<Node>::true_expr);

				net_io_maps.intersection_id_to_ptr.set_empty_key(-1);
				net_io_maps.intersection_id_to_ptr.set_deleted_key(-2);

				_Intersection_Interface* intersection;

				int counter=-1;

				cout << "Reading Nodes" << endl;

				for(typename result<Node>::iterator db_itr = node_result.begin (); db_itr != node_result.end (); ++db_itr)
				{
					counter++;
					if(counter%10000==0) cout << "\t" << counter << endl;

					intersection=(_Intersection_Interface*)Allocate<typename _Intersection_Interface::Component_Type>();

					intersection->template uuid<int>( db_itr->getNode() );
					intersection->template internal_id<int>(counter);
					intersection->template x_position<float>( _scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getX()));
					intersection->template y_position<float>( _scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getY()));
					intersection->template intersection_control<_Intersection_Control_Interface*>((_Intersection_Control_Interface*)nullptr);
					
					net_io_maps.intersection_id_to_ptr[db_itr->getNode()]=intersection;

					if (_scenario_reference->template intersection_control_flag<int>() == 0) 
					{
						intersection->template intersection_type<int>(Intersection_Components::Types::NO_CONTROL);

						_Intersection_Control_Interface* intersection_control = (_Intersection_Control_Interface*)Allocate<typename _Intersection_Control_Interface::Component_Type>();
						intersection_control->template intersection<_Intersection_Interface*>(intersection);
					
						_Control_Plan_Interface* control_plan = (_Control_Plan_Interface*)Allocate<typename _Control_Plan_Interface::Component_Type>();
						control_plan->template control_plan_index<int>(0.0);
						control_plan->template control_type<int>(Intersection_Components::Types::NO_CONTROL);
						control_plan->template starting_time<int>(0.0);
						control_plan->template ending_time<int>(24*60*60);
						intersection_control->template control_plan_data_array<_Control_Plans_Container_Interface&>().push_back(control_plan);
						intersection->template intersection_control<_Intersection_Control_Interface*>(intersection_control);
					}
					intersections_container_ptr->push_back(intersection);
				}

				result<TransitNode> tr_node_result = db->template query<TransitNode>(query<TransitNode>::true_expr); 
				
				cout << "Reading Transit Nodes" << endl;
				
				for (typename result<TransitNode>::iterator db_itr = tr_node_result.begin(); db_itr != tr_node_result.end(); ++db_itr)
				{
					counter++;
					if (counter % 10000 == 0) cout << "\t" << counter << endl;

					intersection = (_Intersection_Interface*)Allocate<typename _Intersection_Interface::Component_Type>();

					intersection->template uuid<int>(db_itr->getNode());
					intersection->template internal_id<int>(counter);
					intersection->template x_position<float>(_scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getX()));
					intersection->template y_position<float>(_scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getY()));
					intersection->template agency<std::string>(db_itr->getAgency());
					intersection->template code<std::string>(db_itr->getCode());
					intersection->template name<std::string>(db_itr->getName());
					intersection->template description<std::string>(db_itr->getDescription());
					intersection->template intersection_control<_Intersection_Control_Interface*>((_Intersection_Control_Interface*)nullptr);

					net_io_maps.intersection_id_to_ptr[db_itr->getNode()] = intersection;

					if (_scenario_reference->template intersection_control_flag<int>() == 0)
					{
						intersection->template intersection_type<int>(Intersection_Components::Types::NO_CONTROL);

						_Intersection_Control_Interface* intersection_control = (_Intersection_Control_Interface*)Allocate<typename _Intersection_Control_Interface::Component_Type>();
						intersection_control->template intersection<_Intersection_Interface*>(intersection);

						_Control_Plan_Interface* control_plan = (_Control_Plan_Interface*)Allocate<typename _Control_Plan_Interface::Component_Type>();
						control_plan->template control_plan_index<int>(0.0);
						control_plan->template control_type<int>(Intersection_Components::Types::NO_CONTROL);
						control_plan->template starting_time<int>(0.0);
						control_plan->template ending_time<int>(24 * 60 * 60);
						intersection_control->template control_plan_data_array<_Control_Plans_Container_Interface&>().push_back(control_plan);
						intersection->template intersection_control<_Intersection_Control_Interface*>(intersection_control);
					}
					intersections_container_ptr->push_back(intersection);
				}
			}

			template<typename TargetType> void read_transit_route_data(unique_ptr<odb::database>& db, Network_Components::Types::Network_IO_Maps& net_io_maps)
			{
				using namespace odb;
				using namespace polaris::io;

				typedef  Transit_Route_Components::Prototypes::Transit_Route<typename remove_pointer< typename type_of(network_reference)::get_type_of(transit_routes_container)::value_type>::type>  _Transit_Route_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(transit_routes_container), _Transit_Route_Interface*> _Transit_Routes_Container_Interface;

				_Transit_Routes_Container_Interface* transit_routes_container_ptr = _network_reference->template transit_routes_container<_Transit_Routes_Container_Interface*>();
				typename type_of(network_reference)::type_of(transit_routes_container)& transit_routes_container_monitor = (typename type_of(network_reference)::type_of(transit_routes_container)&)(*transit_routes_container_ptr);

				transit_routes_container_ptr->clear();

				result<Route> route_result = db->template query<Route>(query<Route>::true_expr);

				net_io_maps.transit_route_id_to_ptr.set_empty_key(-1);
				net_io_maps.transit_route_id_to_ptr.set_deleted_key(-2);

				_Transit_Route_Interface* transit_route;

				int counter = -1;

				cout << "Reading Transit Routes" << endl;

				for (typename result<Route>::iterator db_itr = route_result.begin(); db_itr != route_result.end(); ++db_itr)
				{
					counter++;
					if (counter % 10000 == 0) cout << "\t" << counter << endl;

					transit_route = (_Transit_Route_Interface*)Allocate<typename _Transit_Route_Interface::Component_Type>();

					transit_route->template uuid<int>(db_itr->getRoute());
					transit_route->template internal_id<int>(counter);
					transit_route->template name<std::string>(db_itr->getName());
					transit_route->template agency<std::string>(db_itr->getAgency());
					transit_route->template shortname<std::string>(db_itr->getShortname());
					transit_route->template longname<std::string>(db_itr->getLongname());
					transit_route->template description<std::string>(db_itr->getDescription());
					transit_route->template type<int>(db_itr->getType());	

					net_io_maps.transit_route_id_to_ptr[db_itr->getRoute()] = transit_route;
					transit_routes_container_ptr->push_back(transit_route);
				}				
			}

			template<typename TargetType> void read_transit_pattern_data(unique_ptr<odb::database>& db, Network_Components::Types::Network_IO_Maps& net_io_maps)
			{
				using namespace odb;
				using namespace polaris::io;

				typedef  Intersection_Components::Prototypes::Intersection<typename remove_pointer< typename type_of(network_reference)::get_type_of(intersections_container)::value_type>::type>  _Intersection_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(intersections_container), _Intersection_Interface*> _Intersections_Container_Interface;
				
				typedef  Transit_Route_Components::Prototypes::Transit_Route<typename remove_pointer< typename type_of(network_reference)::get_type_of(transit_routes_container)::value_type>::type>  _Transit_Route_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(transit_routes_container), _Transit_Route_Interface*> _Transit_Routes_Container_Interface;

				typedef  Transit_Pattern_Components::Prototypes::Transit_Pattern<typename remove_pointer< typename type_of(network_reference)::get_type_of(transit_patterns_container)::value_type>::type>  _Transit_Pattern_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(transit_patterns_container), _Transit_Pattern_Interface*> _Transit_Patterns_Container_Interface;

				_Transit_Patterns_Container_Interface* transit_patterns_container_ptr = _network_reference->template transit_patterns_container<_Transit_Patterns_Container_Interface*>();
				typename type_of(network_reference)::type_of(transit_patterns_container)& transit_patterns_container_monitor = (typename type_of(network_reference)::type_of(transit_patterns_container)&)(*transit_patterns_container_ptr);


				transit_patterns_container_ptr->clear();

				result<Pattern> pattern_result = db->template query<Pattern>(query<Pattern>::true_expr);

				net_io_maps.transit_pattern_id_to_ptr.set_empty_key(-1);
				net_io_maps.transit_pattern_id_to_ptr.set_deleted_key(-2);

				_Transit_Pattern_Interface* transit_pattern;

				int counter = -1;

				cout << "Reading Transit Patterns" << endl;

				for (typename result<Pattern>::iterator db_itr = pattern_result.begin(); db_itr != pattern_result.end(); ++db_itr)
				{
					counter++;
					if (counter % 10000 == 0) cout << "\t" << counter << endl;

					transit_pattern = (_Transit_Pattern_Interface*)Allocate<typename _Transit_Pattern_Interface::Component_Type>();

					transit_pattern->template uuid<int>(db_itr->getPattern());
					transit_pattern->template internal_id<int>(counter);
					transit_pattern->template route<_Transit_Route_Interface*>((_Transit_Route_Interface*)net_io_maps.transit_route_id_to_ptr[db_itr->getRoute()->getRoute()]);

					const string& stop_seq_string = db_itr->getStop_seq_string();
					std::istringstream ss(stop_seq_string);
					std::string sub_string;

					while (std::getline(ss, sub_string, ':'))
					{					
						_Intersection_Interface* pattern_stop = (_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[stoi(sub_string)];
						if (pattern_stop)
						{
							transit_pattern->template pattern_stops<_Intersections_Container_Interface&>().push_back(pattern_stop);
						}
						else
						{
							cout << "The pattern " << transit_pattern->_uuid << " points to a non-existing stop: " << sub_string << endl;
							system("pause");
							exit(0);
						}
					}

					net_io_maps.transit_pattern_id_to_ptr[db_itr->getPattern()] = transit_pattern;
					transit_patterns_container_ptr->push_back(transit_pattern);
					//DO the stop sequence!
				}
			}

			template<typename TargetType> void read_transit_vehicle_trip_data(unique_ptr<odb::database>& db, Network_Components::Types::Network_IO_Maps& net_io_maps)
			{
				using namespace odb;
				using namespace polaris::io;

				typedef  Intersection_Components::Prototypes::Intersection<typename remove_pointer< typename type_of(network_reference)::get_type_of(intersections_container)::value_type>::type>  _Intersection_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(intersections_container), _Intersection_Interface*> _Intersections_Container_Interface;

				typedef  Transit_Route_Components::Prototypes::Transit_Route<typename remove_pointer< typename type_of(network_reference)::get_type_of(transit_routes_container)::value_type>::type>  _Transit_Route_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(transit_routes_container), _Transit_Route_Interface*> _Transit_Routes_Container_Interface;

				typedef  Transit_Pattern_Components::Prototypes::Transit_Pattern<typename remove_pointer< typename type_of(network_reference)::get_type_of(transit_patterns_container)::value_type>::type>  _Transit_Pattern_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(transit_patterns_container), _Transit_Pattern_Interface*> _Transit_Patterns_Container_Interface;

				typedef  Transit_Vehicle_Trip_Components::Prototypes::Transit_Vehicle_Trip<typename remove_pointer< typename type_of(network_reference)::get_type_of(transit_vehicle_trips_container)::value_type>::type>  _Transit_Vehicle_Trip_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(transit_vehicle_trips_container), _Transit_Vehicle_Trip_Interface*> _Transit_Vehicle_Trips_Container_Interface;

				_Transit_Vehicle_Trips_Container_Interface* transit_vehicle_trips_container_ptr = _network_reference->template transit_vehicle_trips_container<_Transit_Vehicle_Trips_Container_Interface*>();
				typename type_of(network_reference)::type_of(transit_vehicle_trips_container)& transit_vehicle_trips_container_monitor = (typename type_of(network_reference)::type_of(transit_vehicle_trips_container)&)(*transit_vehicle_trips_container_ptr);


				transit_vehicle_trips_container_ptr->clear();

				result<TransitVehTrip> transit_vehtrip_result = db->template query<TransitVehTrip>(query<TransitVehTrip>::true_expr);

				net_io_maps.transit_vehicle_trip_id_to_ptr.set_empty_key(-1);
				net_io_maps.transit_vehicle_trip_id_to_ptr.set_deleted_key(-2);

				_Transit_Vehicle_Trip_Interface* transit_vehicle_trip;

				int counter = -1;

				cout << "Reading Transit Trips" << endl;

				for (typename result<TransitVehTrip>::iterator db_itr = transit_vehtrip_result.begin(); db_itr != transit_vehtrip_result.end(); ++db_itr)
				{
					counter++;
					if (counter % 10000 == 0) cout << "\t" << counter << endl;

					transit_vehicle_trip = (_Transit_Vehicle_Trip_Interface*)Allocate<typename _Transit_Vehicle_Trip_Interface::Component_Type>();

					transit_vehicle_trip->template uuid<int>(db_itr->getTrip());
					transit_vehicle_trip->template internal_id<int>(counter);
					transit_vehicle_trip->template direction<int>(db_itr->getDirection());
					transit_vehicle_trip->template pattern<_Transit_Pattern_Interface*>((_Transit_Pattern_Interface*)net_io_maps.transit_pattern_id_to_ptr[db_itr->getPattern()->getPattern()]);

					const string& arrivalseconds_string = db_itr->getArrivalseconds_string();
					std::stringstream ss(arrivalseconds_string);
					std::string sub_string;
					int myTime;
					int mySeq;

					while (std::getline(ss, sub_string, ':'))
					{
						myTime = stoi(sub_string);
						transit_vehicle_trip->template arrival_seconds<std::vector<int>&>().push_back(myTime);
					}

					const string& departureseconds_string = db_itr->getDepartureseconds_string();
					ss.clear();
					ss.str(departureseconds_string);

					while (std::getline(ss, sub_string, ':'))
					{
						myTime = stoi(sub_string);
						transit_vehicle_trip->template departure_seconds<std::vector<int>&>().push_back(myTime);
					}

					const string& seq_string = db_itr->getSeq_string();
					ss.clear();
					ss.str(seq_string);

					while (std::getline(ss, sub_string, ':'))
					{
						mySeq = stoi(sub_string);
						transit_vehicle_trip->template sequence_number<std::vector<int>&>().push_back(mySeq);
					}

					if (transit_vehicle_trip->_arrival_seconds.size() != transit_vehicle_trip->_sequence_number.size() ||
						transit_vehicle_trip->_arrival_seconds.size() != transit_vehicle_trip->_departure_seconds.size() ||
						transit_vehicle_trip->_arrival_seconds.size() != transit_vehicle_trip->_pattern->_pattern_stops.size()
						)
					{
						cout << "Inconsistency between at least two of the following:" << endl;
						cout << "Number of stops of the pattern; number of arrival times, departure times, or GTFS sequence numbers of the trip!" << endl;
						cout << "Trip ID: " << transit_vehicle_trip->_uuid << endl;
						cout << "Pattern ID: " << transit_vehicle_trip->_pattern->_uuid << endl;
						cout << "Stop sequence size of the pattern: " << transit_vehicle_trip->_pattern->_pattern_stops.size() << endl;
						cout << "Arrival times sequence size of the trip: " << transit_vehicle_trip->_arrival_seconds.size() << endl;
						cout << "Departure times sequence size of the trip: " << transit_vehicle_trip->_departure_seconds.size() << endl;
						cout << "GTFS sequence size of the trip: " << transit_vehicle_trip->_sequence_number.size() << endl;
						system("pause");
						exit(0);
					}
					
					int my_itr = 0;
					for (auto itr = transit_vehicle_trip->_arrival_seconds.begin(); itr != transit_vehicle_trip->_arrival_seconds.end(); ++itr)
					{
						if (transit_vehicle_trip->_arrival_seconds.at(my_itr) > transit_vehicle_trip->_departure_seconds.at(my_itr))
						{
							cout << "Departure before arrival error!" << endl;
							cout << "Trip ID: " << transit_vehicle_trip->_uuid << endl;
							cout << "Pattern ID: " << transit_vehicle_trip->_pattern->_uuid << endl;
							cout << "GTFS sequence number: " << transit_vehicle_trip->_sequence_number.at(my_itr) << endl;
							cout << "Internal sequence number (0 start): " << my_itr << endl;
							cout << "Stop ID: " << transit_vehicle_trip->_pattern->_pattern_stops.at(my_itr)->_uuid << endl;
							cout << "Arrival: " << transit_vehicle_trip->_arrival_seconds.at(my_itr) << endl;
							cout << "Departure: " << transit_vehicle_trip->_departure_seconds.at(my_itr) << endl;
							system("pause");
							exit(0);
						}

						if (my_itr + 1 < transit_vehicle_trip->_arrival_seconds.size())
						{
							if (transit_vehicle_trip->_arrival_seconds.at(my_itr) >= transit_vehicle_trip->_arrival_seconds.at(my_itr+1))
							{
								cout << "Arrival sequence timing error!" << endl;
								cout << "Trip ID: " << transit_vehicle_trip->_uuid << endl;
								cout << "Pattern ID: " << transit_vehicle_trip->_pattern->_uuid << endl;
								cout << "GTFS sequence number: " << transit_vehicle_trip->_sequence_number.at(my_itr) << endl;
								cout << "Internal sequence number (0 start): " << my_itr << endl;
								cout << "Stop ID: " << transit_vehicle_trip->_pattern->_pattern_stops.at(my_itr)->_uuid << endl;
								cout << "Arrival: " << transit_vehicle_trip->_arrival_seconds.at(my_itr) << endl;
								cout << "GTFS sequence number: " << transit_vehicle_trip->_sequence_number.at(my_itr+1) << endl;
								cout << "Internal sequence number (0 start): " << my_itr+1 << endl;
								cout << "Stop ID: " << transit_vehicle_trip->_pattern->_pattern_stops.at(my_itr+1)->_uuid << endl;
								cout << "Arrival: " << transit_vehicle_trip->_arrival_seconds.at(my_itr+1) << endl;
								system("pause");
								exit(0);
							}

							if (transit_vehicle_trip->_departure_seconds.at(my_itr) >= transit_vehicle_trip->_departure_seconds.at(my_itr+1))
							{
								cout << "Departure sequence timing error!" << endl;
								cout << "Trip ID: " << transit_vehicle_trip->_uuid << endl;
								cout << "Pattern ID: " << transit_vehicle_trip->_pattern->_uuid << endl;
								cout << "GTFS sequence number: " << transit_vehicle_trip->_sequence_number.at(my_itr) << endl;
								cout << "Internal sequence number (0 start): " << my_itr << endl;
								cout << "Stop ID: " << transit_vehicle_trip->_pattern->_pattern_stops.at(my_itr)->_uuid << endl;
								cout << "Departure: " << transit_vehicle_trip->_departure_seconds.at(my_itr) << endl;
								cout << "GTFS sequence number: " << transit_vehicle_trip->_sequence_number.at(my_itr + 1) << endl;
								cout << "Internal sequence number (0 start): " << my_itr + 1 << endl;
								cout << "Stop ID: " << transit_vehicle_trip->_pattern->_pattern_stops.at(my_itr + 1)->_uuid << endl;
								cout << "Departure: " << transit_vehicle_trip->_departure_seconds.at(my_itr + 1) << endl;
								system("pause");
								exit(0);
							}
						}
						++my_itr;
					}


					net_io_maps.transit_vehicle_trip_id_to_ptr[db_itr->getTrip()] = transit_vehicle_trip;
					transit_vehicle_trips_container_ptr->push_back(transit_vehicle_trip);					
				}
			}

			template<typename TargetType> void read_link_data(unique_ptr<odb::database>& db, Network_Components::Types::Network_IO_Maps& net_io_maps)
			{
				using namespace odb;
				using namespace polaris::io;

				const float link_length = 5280.0; // in foot
				const float speed_limit = 60.0; // in miles per hour
				const float speed_limit_ramp = 30.0; 
				float maximum_flow_rate = 2200.0; // in vehicles per hour per lane
				const float maximum_flow_rate_ramp = 600.0; // 
				const float maximum_flow_rate_arterial = 900;
				//const float jam_density = 220.0; // in vehiles per mile per lane
				const float jam_density = 250.0; // in vehiles per mile per lane
				const float backward_wave_speed = 12.0;
				const float distance_factor = 1.5;				
				float num_lanes = 0.0f;
				double scaling_factor = 0.5;

				_network_reference->template max_free_flow_speed<float>(-1);
				typedef  Intersection_Components::Prototypes::Intersection<typename remove_pointer< typename type_of(network_reference)::get_type_of(intersections_container)::value_type>::type>  _Intersection_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(intersections_container), _Intersection_Interface*> _Intersections_Container_Interface;

				typedef  Link_Components::Prototypes::Link<typename remove_pointer< typename type_of(network_reference)::get_type_of(links_container)::value_type>::type>  _Link_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(links_container), _Link_Interface*> _Links_Container_Interface;

				typedef  Ramp_Metering_Components::Prototypes::Ramp_Metering< typename _Link_Interface::get_type_of(ramp_meter)> _Ramp_Metering_Interface;
				_Links_Container_Interface* links_container_ptr=_network_reference->template links_container<_Links_Container_Interface*>();
				typename type_of(network_reference)::type_of(links_container)& links_container_monitor=(typename type_of(network_reference)::type_of(links_container)&)(*links_container_ptr);				
				typedef Scenario_Components::Prototypes::Scenario<typename MasterType::scenario_type> _Scenario_Interface;
				typedef std::unordered_map<int,std::vector<typename MasterType::link_type*>> id_to_links_type;

				typedef  Transit_Vehicle_Trip_Components::Prototypes::Transit_Vehicle_Trip<typename remove_pointer< typename type_of(network_reference)::get_type_of(transit_vehicle_trips_container)::value_type>::type>  _Transit_Vehicle_Trip_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(transit_vehicle_trips_container), _Transit_Vehicle_Trip_Interface*> _Transit_Vehicle_Trips_Container_Interface;

				Types::Link_ID_Dir link_id_dir;
				
				result<polaris::io::Link> link_result=db->template query<polaris::io::Link>(query<polaris::io::Link>::true_expr);

				net_io_maps.link_id_dir_to_ptr.set_empty_key(-1);
				net_io_maps.link_id_dir_to_ptr.set_deleted_key(-2);

				_Link_Interface* link;
				
				int counter=0;

				cout << "Reading Links" << endl;
				
				int link_counter=-1;

				for(typename result<polaris::io::Link>::iterator db_itr = link_result.begin (); db_itr != link_result.end (); ++db_itr)
				{
					const string& facility_type=db_itr->getType()->getLink_Type();
					bool do_this_link = false;
					
					if (((_Scenario_Interface*)_global_scenario)->template multimodal_network_input<bool>())
					{
						do_this_link = true;
					}
					else
					{
						if (facility_type == "WALKWAY" || facility_type == "BIKEWAY" || facility_type == "FERRY" || facility_type == "LIGHTRAIL" || facility_type == "HEAVYRAIL")
						{
							do_this_link = false;
						}
						else
						{
							do_this_link = true;
						}
					}
					
					if (!do_this_link)
					{
						continue;
					}
					
					counter++;
					if(counter%10000==0) cout << "\t" << counter << endl;
					//cout << "\t" << counter << endl;
					if(db_itr->getLanes_Ab()>0)
					{
						link=(_Link_Interface*)Allocate<typename MasterType::link_type>();
						
						link_id_dir.id=db_itr->getLink();
						link_id_dir.dir=0;

						net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir]=link;

						typedef typename MasterType::network_type::link_dbid_dir_to_ptr_map_type link_dbid_dir_to_ptr_map_type;
						//link_dbid_dir_to_ptr_map_type& link_dbid_dir_to_ptr_map = _network_reference->template link_dbid_dir_to_ptr_map<link_dbid_dir_to_ptr_map_type&>();
						//link_dbid_dir_to_ptr_map[link_id_dir.id_dir] = link;

						link_dbid_dir_to_ptr_map_type* link_dbid_dir_to_ptr_map = _network_reference->template link_dbid_dir_to_ptr_map<link_dbid_dir_to_ptr_map_type*>();
						
						(*link_dbid_dir_to_ptr_map)[link_id_dir.id_dir] = link;

						link->template dbid<int>(db_itr->getLink());
						link->template direction<int>(0.0);

						link->template upstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_A()->getNode()]);
						link->template downstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_B()->getNode()]);

						_Intersection_Interface* itx = link->template downstream_intersection<_Intersection_Interface*>();

						((typename MasterType::intersection_type*)itx)->_area_type = db_itr->getArea_Type()->getArea_Type();

						link->template internal_id<int>(++link_counter);
						link->template uuid<int>(link_id_dir.id * 2 + link_id_dir.dir);
						//link->template uuid<int>(link_id_dir.id /*link_counter*/);

						num_lanes = db_itr->getLanes_Ab();
						
						//if(facility_type=="EXPRESSWAY" || facility_type=="FREEWAY")
						//{
						//	if(num_lanes <= 3) num_lanes = 4;
						//}

						link->template length<float>(_scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getLength()));

						/*
						if(facility_type=="MAJOR" || facility_type=="MINOR" || facility_type=="LOCAL")
						{
							// add lanes proportional to that added by pocket lanes for increased capacity and space
							num_lanes += (int)min(ceil(2.0f*(60.0f/link->template length<float>())),2.0f);
						}
						*/

						link->template num_lanes<int>(num_lanes);
						
						float ffspd = _scenario_reference->template mepsToMiph<NULLTYPE>(db_itr->getFspd_Ab()+0.5f);
						link->template free_flow_speed<float>(ffspd);

						float speed_limit_estimate = link->template speed_limit_estimate<NT>();
						
						//if(facility_type=="EXPRESSWAY" || facility_type=="FREEWAY")
						//{
						//	if(ffspd < 65.0f)
						//	{
						//		ffspd = 65.0f;
						//		speed_limit_estimate = 55.0f;
						//		link->template free_flow_speed<float>(ffspd);
						//	}
						//}

						
						link->template speed_limit<float>(speed_limit_estimate);


						//link->template speed_limit<float>(floor(_scenario_reference->template mepsToMiph<NULLTYPE>(db_itr->getSpeed_Ab()) + 0.5f));
						link->template original_speed_limit<float>(link->template speed_limit<float>());
						
						link->template num_left_turn_bays<int>(db_itr->getLeft_Ab());
						link->template num_right_turn_bays<int>(db_itr->getRight_Ab());
						
						link->template left_turn_bay_length<float>(_scenario_reference->template meterToFoot<NULLTYPE>(0.0));
						link->template right_turn_bay_length<float>(_scenario_reference->template meterToFoot<NULLTYPE>(0.0));

						link->template num_inbound_turn_lanes<int>(0.0);
						
						//facility_type=="EXPRESSWAY" facility_type=="FREEWAY"
						
						
						if(facility_type=="FREEWAY")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::FREEWAY);
						}
						else if(facility_type=="EXPRESSWAY")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::EXPRESSWAY);
						}
						else if(facility_type=="RAMP")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::ON_RAMP);
						}
						else if(facility_type=="LOCAL" || facility_type=="COLLECTOR"/* || facility_type=="MINOR"*/)
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::LOCAL);
						}		
						else if(facility_type=="WALKWAY")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::WALKWAY);
						}
						else if(facility_type=="BIKEWAY")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::BIKEWAY);
						}
						else if(facility_type=="FERRY")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::WATERWAY);
						}
						else if(facility_type=="LIGHTRAIL")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::LIGHT_RAILWAY);
						}
						else if(facility_type=="HEAVYRAIL")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::HEAVY_RAILWAY);
						}
						else if(facility_type=="EXTERNAL")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::EXTERNAL);
						}
						else
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::ARTERIAL);
						}

						//link->template free_flow_speed<float>(_scenario_reference->template mepsToMiph<NULLTYPE>(db_itr->getSpeed_Ab()));
						//link->template free_flow_speed<float>(link->template free_flow_speed_estimate<NT>());
						
						//maximum_flow_rate = min(2200.0f, float(db_itr->getCap_Ab()) / link->template num_lanes<float>());
						
						//if(facility_type=="FREEWAY" || facility_type=="EXPRESSWAY" || facility_type=="EXTERNAL")
						//{
						//	maximum_flow_rate = 2000.0f;
						//}
						//else if(facility_type=="MAJOR" || facility_type=="MINOR" || facility_type=="LOCAL" || facility_type=="RAMP")
						//{
						//	maximum_flow_rate = 1800.0f;
						//}
						//else
						//{
						//	maximum_flow_rate = ((float)db_itr->getCap_Ab()) / link->template num_lanes<float>();
						//}

						//if(facility_type=="MAJOR" || facility_type=="MINOR" || facility_type=="LOCAL" || facility_type=="RAMP" || facility_type=="RAMP")
						
						
						if(link->template link_type<Link_Components::Types::Link_Type_Keys>() == Link_Components::Types::ARTERIAL)
						{
							// 1800 From HCM Equation 17-1
							//maximum_flow_rate = scaling_factor*max(1800.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
							maximum_flow_rate = max(1800.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
						}
						else if(link->template link_type<Link_Components::Types::Link_Type_Keys>() == Link_Components::Types::LOCAL)
						{
							//maximum_flow_rate = scaling_factor*max(1600.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
							maximum_flow_rate = max(1600.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
						}
						else if(link->template link_type<Link_Components::Types::Link_Type_Keys>() == Link_Components::Types::ON_RAMP || 
							link->template link_type<Link_Components::Types::Link_Type_Keys>() == Link_Components::Types::OFF_RAMP)
						{
							// From HCM 2010 Exhibit 13-10

							if(link->template free_flow_speed<float>() > 50.0f)
							{
								//maximum_flow_rate =  scaling_factor*max(2200.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
								maximum_flow_rate = max(2200.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
							}
							else if(link->template free_flow_speed<float>() > 40.0f)
							{
								//maximum_flow_rate = scaling_factor*max(2100.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
								maximum_flow_rate = max(2100.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
							}
							else if(link->template free_flow_speed<float>() > 30.0f)
							{
								//maximum_flow_rate = scaling_factor*max2(000.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
								maximum_flow_rate = max(2000.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
							}
							else if(link->template free_flow_speed<float>() > 20.0f)
							{
								//maximum_flow_rate = scaling_factor*max(1900.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
								maximum_flow_rate = max(1900.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
							}
							else
							{
								//maximum_flow_rate = scaling_factor*max(1800.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
								maximum_flow_rate = max(1800.0f,((float)db_itr->getCap_Ab()) / link->template num_lanes<float>());
							}
						}
						else if(facility_type=="FREEWAY" || facility_type=="EXPRESSWAY" || facility_type=="EXTERNAL")
						{
							//VS modified on 7/15, addde multiplier to capacity to mimic impact of CACC
							maximum_flow_rate = scaling_factor*2000.0f;
							//maximum_flow_rate = 2000.0f;
							//std::cout << maximum_flow_rate << "\n";
						}
						else
						{
							//maximum_flow_rate = scaling_factor*((float)db_itr->getCap_Ab()) / link->template num_lanes<float>();
							maximum_flow_rate = ((float)db_itr->getCap_Ab()) / link->template num_lanes<float>();
						}

						//link->template maximum_flow_rate<float>(scaling_factor*maximum_flow_rate);
						link->template maximum_flow_rate<float>(maximum_flow_rate);
						//the backward wave speed is considered constant
						link->template backward_wave_speed<float>(backward_wave_speed);
						//so is the jam density
						link->template jam_density<float>(jam_density);
						link->template original_free_flow_speed<float>(link->template free_flow_speed<float>());
						link->template original_maximum_flow_rate<float>(maximum_flow_rate);
						link->template original_num_lanes<int>(link->template num_lanes<int>());
						link->template ramp_meter<_Ramp_Metering_Interface*>(nullptr);

						_network_reference->template max_free_flow_speed<float>(max(_network_reference->template max_free_flow_speed<float>(),link->template free_flow_speed<float>()));

						link->template upstream_intersection<_Intersection_Interface*>()->template outbound_links<_Links_Container_Interface&>().push_back(link);
						link->template downstream_intersection<_Intersection_Interface*>()->template inbound_links<_Links_Container_Interface&>().push_back(link);

						links_container_ptr->push_back(link);
						
						typename id_to_links_type::iterator links_itr;
						id_to_links_type& id_to_links_map = _network_reference->template db_id_to_links_map<id_to_links_type&>();
						links_itr = id_to_links_map.find(link_id_dir.id);
						if (links_itr != id_to_links_map.end())
						{
							links_itr->second.push_back((typename MasterType::link_type*)link);
						}
						else
						{
							std::vector<typename MasterType::link_type*> links_arr;
							links_arr.push_back((typename MasterType::link_type*)link);
							id_to_links_map[link_id_dir.id] = links_arr;
						}
					}

					if(db_itr->getLanes_Ba()>0)
					{
						link=(_Link_Interface*)Allocate<typename _Link_Interface::Component_Type>();

						link_id_dir.id=db_itr->getLink();
						link_id_dir.dir=1;

						net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir]=link;
						
						typedef typename MasterType::network_type::link_dbid_dir_to_ptr_map_type link_dbid_dir_to_ptr_map_type;
						link_dbid_dir_to_ptr_map_type& link_dbid_dir_to_ptr_map = _network_reference->template link_dbid_dir_to_ptr_map<link_dbid_dir_to_ptr_map_type&>();
						link_dbid_dir_to_ptr_map[link_id_dir.id_dir] = link;

						link->template dbid<int>(db_itr->getLink());
						link->template direction<int>(1);


						link->template upstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_B()->getNode()]);
						link->template downstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_A()->getNode()]);
						
						_Intersection_Interface* itx = link->template downstream_intersection<_Intersection_Interface*>();

						((typename MasterType::intersection_type*)itx)->_area_type = db_itr->getArea_Type()->getArea_Type();

						
						//link->template uuid<int>(link_id_dir.id);
						link->template internal_id<int>(++link_counter);
						link->template uuid<int>(link_id_dir.id * 2 + link_id_dir.dir);

						num_lanes = db_itr->getLanes_Ba();
						
						//if(facility_type=="EXPRESSWAY" || facility_type=="FREEWAY")
						//{
						//	if(num_lanes <= 3) num_lanes = 4;
						//}

						link->template length<float>(_scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getLength()));
						
						/*
						if(facility_type=="MAJOR" || facility_type=="MINOR" || facility_type=="LOCAL")
						{
							// add lanes proportional to that added by pocket lanes for increased capacity and space
							num_lanes += (int)min(ceil(2.0f*(60.0f/link->template length<float>())),2.0f);
						}
						*/

						link->template num_lanes<int>(num_lanes);


						float ffspd = _scenario_reference->template mepsToMiph<NULLTYPE>(db_itr->getFspd_Ba()+0.5f);
						link->template free_flow_speed<float>(ffspd);

						float speed_limit_estimate = link->template speed_limit_estimate<NT>();
						
						//if(facility_type=="EXPRESSWAY" || facility_type=="FREEWAY")
						//{
						//	//cout << ffspd << endl;

						//	if(ffspd < 65.0f)
						//	{
						//		//cout << "ffspd updated on: " << link->template dbid<int>() << "," << 65.0f - ffspd << endl;
						//		ffspd = 65.0f;
						//		speed_limit_estimate = 55.0f;
						//		link->template free_flow_speed<float>(ffspd);
						//	}
						//}
						


						link->template speed_limit<float>(speed_limit_estimate);

						//link->template speed_limit<float>(floor(_scenario_reference->template mepsToMiph<NULLTYPE>(db_itr->getSpeed_Ba()) + 0.5f));
						link->template original_speed_limit<float>(link->template speed_limit<float>());
						
						link->template num_left_turn_bays<int>(db_itr->getLeft_Ba());
						link->template num_right_turn_bays<int>(db_itr->getRight_Ba());
						
						link->template left_turn_bay_length<float>(_scenario_reference->template meterToFoot<NULLTYPE>(0.0));
						link->template right_turn_bay_length<float>(_scenario_reference->template meterToFoot<NULLTYPE>(0.0));
						
						link->template num_inbound_turn_lanes<int>(0.0);
						
						if(facility_type=="FREEWAY")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::FREEWAY);
						}
						else if(facility_type=="EXPRESSWAY")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::EXPRESSWAY);
						}
						else if(facility_type=="RAMP")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::ON_RAMP);
						}
						else if(facility_type=="LOCAL" || facility_type=="COLLECTOR" /* || facility_type=="MINOR"*/)
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::LOCAL);
						}			
						else if(facility_type=="WALKWAY")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::WALKWAY);
						}
						else if(facility_type=="BIKEWAY")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::BIKEWAY);
						}
						else if(facility_type=="FERRY")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::WATERWAY);
						}
						else if(facility_type=="LIGHTRAIL")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::LIGHT_RAILWAY);
						}
						else if(facility_type=="HEAVYRAIL")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::HEAVY_RAILWAY);
						}
						else if(facility_type=="EXTERNAL")
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::EXTERNAL);
						}
						else
						{
							link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::ARTERIAL);
						}
						
						
						//link->template free_flow_speed<float>(_scenario_reference->template mepsToMiph<NULLTYPE>(db_itr->getSpeed_Ba()));
						//link->template free_flow_speed<float>(link->template free_flow_speed_estimate<NT>());
						//maximum_flow_rate = min(2200.0f, float(db_itr->getCap_Ba()) / link->template num_lanes<float>());

						//if(facility_type=="FREEWAY" || facility_type=="EXPRESSWAY" || facility_type=="EXTERNAL")
						//{
						//	maximum_flow_rate = 2000.0f;
						//}
						//else if(facility_type=="MAJOR" || facility_type=="MINOR" || facility_type=="LOCAL" || facility_type=="RAMP")
						//{
						//	maximum_flow_rate = 1800.0f;
						//}
						//else
						//{
						//	maximum_flow_rate = ((float)db_itr->getCap_Ba()) / link->template num_lanes<float>();
						//}

						//if(facility_type=="MAJOR" || facility_type=="MINOR" || facility_type=="LOCAL" || facility_type=="RAMP")
						if(link->template link_type<Link_Components::Types::Link_Type_Keys>() == Link_Components::Types::ARTERIAL)
						{
							// 1800 From HCM Equation 17-1
							//maximum_flow_rate = scaling_factor*max(1800.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
							maximum_flow_rate = max(1800.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
						}
						else if(link->template link_type<Link_Components::Types::Link_Type_Keys>() == Link_Components::Types::LOCAL)
						{
							//maximum_flow_rate = scaling_factor*max(1600.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
							maximum_flow_rate = max(1600.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
						}
						else if(link->template link_type<Link_Components::Types::Link_Type_Keys>() == Link_Components::Types::ON_RAMP || 
							link->template link_type<Link_Components::Types::Link_Type_Keys>() == Link_Components::Types::OFF_RAMP)
						{
							// From HCM 2010 Exhibit 13-10

							if(link->template free_flow_speed<float>() > 50.0f)
							{
								//maximum_flow_rate = scaling_factor*max(2200.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
								maximum_flow_rate = max(2200.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
							}
							else if(link->template free_flow_speed<float>() > 40.0f)
							{
								//maximum_flow_rate = scaling_factor*max(2100.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
								maximum_flow_rate = max(2100.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
							}
							else if(link->template free_flow_speed<float>() > 30.0f)
							{
								//maximum_flow_rate = scaling_factor*max(2000.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
								maximum_flow_rate = max(2000.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
							}
							else if(link->template free_flow_speed<float>() > 20.0f)
							{
								//maximum_flow_rate = scaling_factor*max(1900.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
								maximum_flow_rate = max(1900.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
							}
							else
							{
								//maximum_flow_rate = scaling_factor*max(1800.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
								maximum_flow_rate = max(1800.0f,((float)db_itr->getCap_Ba()) / link->template num_lanes<float>());
							}
						}
						else if(facility_type=="FREEWAY" || facility_type=="EXPRESSWAY" || facility_type=="EXTERNAL")
						{
							//VS modified on 7/15, addde multiplier to capacity to mimic impact of CACC
							//maximum_flow_rate = 2000.0f;
							maximum_flow_rate = scaling_factor*2000.0f;
							//std::cout << maximum_flow_rate << "\n";
						}
						else
						{
							//maximum_flow_rate = scaling_factor*((float)db_itr->getCap_Ba()) / link->template num_lanes<float>();
							maximum_flow_rate = ((float)db_itr->getCap_Ba()) / link->template num_lanes<float>();
						}


						//link->template maximum_flow_rate<float>(scaling_factor*maximum_flow_rate);
						link->template maximum_flow_rate<float>(maximum_flow_rate);
						link->template backward_wave_speed<float>(backward_wave_speed);
						link->template jam_density<float>(jam_density);
						link->template original_free_flow_speed<float>(link->template free_flow_speed<float>());
						link->template original_maximum_flow_rate<float>(maximum_flow_rate);
						link->template original_num_lanes<int>(link->template num_lanes<int>());
						link->template ramp_meter<_Ramp_Metering_Interface*>(nullptr);

						_network_reference->template max_free_flow_speed<float>(max(_network_reference->template max_free_flow_speed<float>(),link->template free_flow_speed<float>()));

						link->template upstream_intersection<_Intersection_Interface*>()->template outbound_links<_Links_Container_Interface&>().push_back(link);
						link->template downstream_intersection<_Intersection_Interface*>()->template inbound_links<_Links_Container_Interface&>().push_back(link);

						links_container_ptr->push_back(link);

						typename id_to_links_type::iterator links_itr;
						id_to_links_type& id_to_links_map = _network_reference->template db_id_to_links_map<id_to_links_type&>();
						links_itr = id_to_links_map.find(link_id_dir.id);
						if (links_itr != id_to_links_map.end())
						{
							links_itr->second.push_back((typename MasterType::link_type*)link);
						}
						else
						{
							std::vector<typename MasterType::link_type*> links_arr;
							links_arr.push_back((typename MasterType::link_type*)link);
							id_to_links_map[link_id_dir.id] = links_arr;
						}
					}
				}

				result<polaris::io::WalkLink> walk_link_result = db->template query<polaris::io::WalkLink>(query<polaris::io::WalkLink>::true_expr);

				cout << "Reading Walk Links" << endl;

				for (typename result<polaris::io::WalkLink>::iterator db_itr = walk_link_result.begin(); db_itr != walk_link_result.end(); ++db_itr)
				{
					const string& facility_type = db_itr->getType()->getLink_Type();
					bool do_this_link = true;

					counter++;
					if (counter % 10000 == 0) cout << "\t" << counter << endl;
					if (1 > 0)
					{
						link = (_Link_Interface*)Allocate<typename MasterType::link_type>();

						link_id_dir.id = db_itr->getLink();
						link_id_dir.dir = 0;

						net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir] = link;

						typedef typename MasterType::network_type::link_dbid_dir_to_ptr_map_type link_dbid_dir_to_ptr_map_type;
						link_dbid_dir_to_ptr_map_type* link_dbid_dir_to_ptr_map = _network_reference->template link_dbid_dir_to_ptr_map<link_dbid_dir_to_ptr_map_type*>();
						(*link_dbid_dir_to_ptr_map)[link_id_dir.id_dir] = link;

						link->template dbid<int>(db_itr->getLink());
						link->template name<std::string>(db_itr->getName());
						link->template grade<float>(db_itr->getGrade());
						link->template direction<int>(0.0);

						/*link->template upstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_A()->getNode()]);
						link->template downstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_B()->getNode()]);*/

						link->template upstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_A()]);
						link->template downstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_B()]);

						_Intersection_Interface* itx = link->template downstream_intersection<_Intersection_Interface*>();

						((typename MasterType::intersection_type*)itx)->_area_type = db_itr->getArea_Type()->getArea_Type();

						link->template internal_id<int>(++link_counter);
						link->template uuid<int>(link_id_dir.id * 2 + link_id_dir.dir);

						link->template length<float>(_scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getLength()));

						link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::WALK);

						link->template upstream_intersection<_Intersection_Interface*>()->template outbound_links<_Links_Container_Interface&>().push_back(link);
						link->template downstream_intersection<_Intersection_Interface*>()->template inbound_links<_Links_Container_Interface&>().push_back(link);

						links_container_ptr->push_back(link);

						typename id_to_links_type::iterator links_itr;
						id_to_links_type& id_to_links_map = _network_reference->template db_id_to_links_map<id_to_links_type&>();
						links_itr = id_to_links_map.find(link_id_dir.id);
						if (links_itr != id_to_links_map.end())
						{
							links_itr->second.push_back((typename MasterType::link_type*)link);
						}
						else
						{
							std::vector<typename MasterType::link_type*> links_arr;
							links_arr.push_back((typename MasterType::link_type*)link);
							id_to_links_map[link_id_dir.id] = links_arr;
						}
					}

					if (0 < 1)
					{
						link = (_Link_Interface*)Allocate<typename MasterType::link_type>();

						link_id_dir.id = db_itr->getLink();
						link_id_dir.dir = 1;

						net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir] = link;

						typedef typename MasterType::network_type::link_dbid_dir_to_ptr_map_type link_dbid_dir_to_ptr_map_type;
						link_dbid_dir_to_ptr_map_type* link_dbid_dir_to_ptr_map = _network_reference->template link_dbid_dir_to_ptr_map<link_dbid_dir_to_ptr_map_type*>();
						(*link_dbid_dir_to_ptr_map)[link_id_dir.id_dir] = link;

						link->template dbid<int>(db_itr->getLink());
						link->template name<std::string>(db_itr->getName());
						link->template grade<float>(db_itr->getGrade());
						link->template direction<int>(1);

						/*link->template upstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_B()->getNode()]);
						link->template downstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_A()->getNode()]);*/

						link->template upstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_B()]);
						link->template downstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_A()]);

						_Intersection_Interface* itx = link->template downstream_intersection<_Intersection_Interface*>();

						((typename MasterType::intersection_type*)itx)->_area_type = db_itr->getArea_Type()->getArea_Type();

						link->template internal_id<int>(++link_counter);
						link->template uuid<int>(link_id_dir.id * 2 + link_id_dir.dir);

						link->template length<float>(_scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getLength()));

						link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::WALK);

						link->template upstream_intersection<_Intersection_Interface*>()->template outbound_links<_Links_Container_Interface&>().push_back(link);
						link->template downstream_intersection<_Intersection_Interface*>()->template inbound_links<_Links_Container_Interface&>().push_back(link);

						links_container_ptr->push_back(link);

						typename id_to_links_type::iterator links_itr;
						id_to_links_type& id_to_links_map = _network_reference->template db_id_to_links_map<id_to_links_type&>();
						links_itr = id_to_links_map.find(link_id_dir.id);
						if (links_itr != id_to_links_map.end())
						{
							links_itr->second.push_back((typename MasterType::link_type*)link);
						}
						else
						{
							std::vector<typename MasterType::link_type*> links_arr;
							links_arr.push_back((typename MasterType::link_type*)link);
							id_to_links_map[link_id_dir.id] = links_arr;
						}
					}
				}

				result<polaris::io::TransitLink> transit_link_result = db->template query<polaris::io::TransitLink>(query<polaris::io::TransitLink>::true_expr);

				cout << "Reading Transit Links" << endl;

				for (typename result<polaris::io::TransitLink>::iterator db_itr = transit_link_result.begin(); db_itr != transit_link_result.end(); ++db_itr)
				{
					const string& facility_type = db_itr->getType()->getLink_Type();
					bool do_this_link = true;

					counter++;
					if (counter % 10000 == 0) cout << "\t" << counter << endl;
					if (1 > 0)
					{
						link = (_Link_Interface*)Allocate<typename MasterType::link_type>();

						link_id_dir.id = db_itr->getLink();
						link_id_dir.dir = 0;

						net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir] = link;

						typedef typename MasterType::network_type::link_dbid_dir_to_ptr_map_type link_dbid_dir_to_ptr_map_type;
						link_dbid_dir_to_ptr_map_type* link_dbid_dir_to_ptr_map = _network_reference->template link_dbid_dir_to_ptr_map<link_dbid_dir_to_ptr_map_type*>();
						(*link_dbid_dir_to_ptr_map)[link_id_dir.id_dir] = link;

						link->template dbid<int>(db_itr->getLink());
						link->template name<std::string>(db_itr->getName());
						link->template grade<float>(db_itr->getGrade());
						link->template direction<int>(0.0);

						link->template upstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_A()->getNode()]);
						link->template downstream_intersection<_Intersection_Interface*>((_Intersection_Interface*)net_io_maps.intersection_id_to_ptr[db_itr->getNode_B()->getNode()]);

						_Intersection_Interface* itx = link->template downstream_intersection<_Intersection_Interface*>();

						((typename MasterType::intersection_type*)itx)->_area_type = db_itr->getArea_Type()->getArea_Type();

						link->template internal_id<int>(++link_counter);
						link->template uuid<int>(link_id_dir.id * 2 + link_id_dir.dir);

						link->template length<float>(_scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getLength()));

						link->template link_type<Link_Components::Types::Link_Type_Keys>(Link_Components::Types::TRANSIT);

						link->template upstream_intersection<_Intersection_Interface*>()->template outbound_links<_Links_Container_Interface&>().push_back(link);
						link->template downstream_intersection<_Intersection_Interface*>()->template inbound_links<_Links_Container_Interface&>().push_back(link);

						const string& TripsByDepTime = db_itr->getTripsByDepTime();
						std::istringstream ss(TripsByDepTime);
						std::string sub_string;
						int mySeq;

						while (std::getline(ss, sub_string, ':'))
						{
							_Transit_Vehicle_Trip_Interface* link_trip = (_Transit_Vehicle_Trip_Interface*)net_io_maps.transit_vehicle_trip_id_to_ptr[stoi(sub_string)];

							if (link_trip)
							{
								link->template trips_by_dep_time<_Transit_Vehicle_Trips_Container_Interface&>().push_back(link_trip);
							}
							else
							{
								cout << "The link " << link->_dbid << " points to a non-existing trip: " << sub_string << endl;
								system("pause");
								exit(0);
							}

						}

						const string& IndexAlongTripOfStopA = db_itr->getIndexAlongTripOfStopA();
						ss.clear();
						ss.str(IndexAlongTripOfStopA);

						while (std::getline(ss, sub_string, ':'))
						{
							mySeq = stoi(sub_string);
							link->template index_along_trip_at_upstream_node<std::vector<int>&>().push_back(mySeq);
						}

						if ( link->_trips_by_dep_time.size() != link->_index_along_trip_at_upstream_node.size() )
						{
							cout << "Inconsistency between number of trips and trip indices on a link!" << endl;
							cout << "Link ID: " << link->_dbid << endl;
							cout << "Trip sequence size of the link: " << link->_trips_by_dep_time.size() << endl;
							cout << "Trip sequence indices size of the trip: " << link->_index_along_trip_at_upstream_node.size() << endl;
							system("pause");
							exit(0);
						}

						int my_itr = 0;
						for (auto itr = link->_trips_by_dep_time.begin(); itr != link->_trips_by_dep_time.end(); ++itr)
						{
							int temp_seq = link->_index_along_trip_at_upstream_node.at(my_itr);

							int temp_stop1 = link->_upstream_intersection->_uuid;
							int temp_stop2 = link->_trips_by_dep_time.at(my_itr)->_pattern->_pattern_stops.at(temp_seq)->_uuid;
							int temp_trip = link->_trips_by_dep_time.at(my_itr)->_uuid;
							int temp_pattern = link->_trips_by_dep_time.at(my_itr)->_pattern->_uuid;

							if (temp_stop1 != temp_stop2)
							{
								cout << "Link's upstream node does not match the stop ID of the trip!" << endl;
								cout << "Link ID: " << link->_dbid << endl;
								cout << "Trip's sequence by departure time (0 start) on link: " << my_itr << endl;
								cout << "Trip ID: " << link->_trips_by_dep_time.at(my_itr)->_uuid << endl;
								cout << "Pattern ID: " << link->_trips_by_dep_time.at(my_itr)->_pattern->_uuid << endl;
								cout << "Claimed sequence number of the upstream node along the trip: " << temp_seq << endl;
								cout << "Link's upstream node ID: " << temp_stop1 << endl;
								cout << "Node ID along the trip at " << temp_seq << " is: " << temp_stop2 << endl;
								system("pause");
								exit(0);
							}	

							if (my_itr + 1 < link->_trips_by_dep_time.size())
							{
								
								int temp_seq1 = link->_index_along_trip_at_upstream_node.at(my_itr);
								int temp_seq2 = link->_index_along_trip_at_upstream_node.at(my_itr + 1);

								int temp_time1 = link->_trips_by_dep_time.at(my_itr)->_departure_seconds.at(temp_seq1);
								int temp_time2 = link->_trips_by_dep_time.at(my_itr + 1)->_departure_seconds.at(temp_seq2);
								
								if (link->_trips_by_dep_time.at(my_itr)->_departure_seconds.at(temp_seq1) > link->_trips_by_dep_time.at(my_itr + 1)->_departure_seconds.at(temp_seq2))
								{
									cout << "Trips on the link are not ordered by departure time!" << endl;
									cout << "Link ID: " << link->_dbid << endl;
									cout << "Trip ID_1: " << link->_trips_by_dep_time.at(my_itr)->_uuid << endl;
									cout << "Trip ID_2: " << link->_trips_by_dep_time.at(my_itr + 1)->_uuid << endl;
									cout << "Trip 1's sequence by departure time (0 start) on link: " << my_itr << endl;
									cout << "Trip 2's sequence by departure time (0 start) on link: " << my_itr + 1 << endl;
									cout << "Sequence number of the upstream node along the trip 1: " << temp_seq1 << endl;
									cout << "Sequence number of the upstream node along the trip 2: " << temp_seq2 << endl;
									cout << "Departure time from the upstream node of trip 1: " << temp_time1 << endl;
									cout << "Departure time from the upstream node of trip 2: " << temp_time2 << endl;
									system("pause");
									exit(0);
									//
								}	
							}
							++my_itr;
						}
						
						links_container_ptr->push_back(link);

						typename id_to_links_type::iterator links_itr;
						id_to_links_type& id_to_links_map = _network_reference->template db_id_to_links_map<id_to_links_type&>();
						links_itr = id_to_links_map.find(link_id_dir.id);
						if (links_itr != id_to_links_map.end())
						{
							links_itr->second.push_back((typename MasterType::link_type*)link);
						}
						else
						{
							std::vector<typename MasterType::link_type*> links_arr;
							links_arr.push_back((typename MasterType::link_type*)link);
							id_to_links_map[link_id_dir.id] = links_arr;
						}
					}
				}
			}

			template<typename TargetType> void read_turn_movement_data(unique_ptr<odb::database>& db, Network_Components::Types::Network_IO_Maps& net_io_maps)
			{
				using namespace odb;
				using namespace polaris::io;

				Types::Link_ID_Dir link_id_dir;
				typedef Scenario_Components::Prototypes::Scenario<typename MasterType::scenario_type> _Scenario_Interface;

				typedef  Intersection_Components::Prototypes::Intersection<typename remove_pointer< typename type_of(network_reference)::get_type_of(intersections_container)::value_type>::type>  _Intersection_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(intersections_container), _Intersection_Interface*> _Intersections_Container_Interface;

				typedef  Link_Components::Prototypes::Link<typename remove_pointer< typename type_of(network_reference)::get_type_of(links_container)::value_type>::type>  _Link_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(links_container), _Link_Interface*> _Links_Container_Interface;

				typedef  Turn_Movement_Components::Prototypes::Movement<typename remove_pointer< typename type_of(network_reference)::get_type_of(turn_movements_container)::value_type>::type>  _Turn_Movement_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(turn_movements_container), _Turn_Movement_Interface*> _Turn_Movements_Container_Interface;


				_Turn_Movement_Interface* turn_movement;
				typename type_of(network_reference)::type_of(turn_movements_container)& turn_movements_monitor=_network_reference->template turn_movements_container<typename type_of(network_reference)::type_of(turn_movements_container)&>();
				_Turn_Movements_Container_Interface& turn_movements_container = _network_reference->template turn_movements_container<_Turn_Movements_Container_Interface&>();
				_Intersections_Container_Interface& intersections_container = _network_reference->template intersections_container<_Intersections_Container_Interface&>();
				result<Connect> connect_result=db->template query<Connect>(query<Connect>::true_expr);
				
				int counter=-1;

				cout << "Reading Connections" << endl;

				for(typename result<Connect>::iterator db_itr = connect_result.begin (); db_itr != connect_result.end (); ++db_itr)
				{
					const string& inbound_link_type = db_itr->getLink()->getType()->getLink_Type();
					const string& outbound_link_type = db_itr->getTo_Link()->getType()->getLink_Type();

					bool do_this_connection = false;
					
					if (((_Scenario_Interface*)_global_scenario)->template multimodal_network_input<bool>())
					{
						do_this_connection = true;
					}
					else
					{
						if (inbound_link_type == "WALKWAY" || inbound_link_type == "BIKEWAY" || inbound_link_type == "FERRY" || inbound_link_type == "LIGHTRAIL" || inbound_link_type == "HEAVYRAIL" ||
							outbound_link_type == "WALKWAY" || outbound_link_type == "BIKEWAY" || outbound_link_type == "FERRY" || outbound_link_type == "LIGHTRAIL" || outbound_link_type == "HEAVYRAIL")
						{
							do_this_connection = false;
						}
						else
						{
							do_this_connection = true;
						}
					}
					
					if (!do_this_connection)
					{
						continue;
					}


					if(counter%10000==0) cout << "\t" << counter << endl;

					_Link_Interface* inbound_link;
					_Link_Interface* outbound_link;

					link_id_dir.id=db_itr->getLink()->getLink();
					link_id_dir.dir=db_itr->getDir();
					inbound_link = (_Link_Interface*)net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir];
					assert(net_io_maps.link_id_dir_to_ptr.count(link_id_dir.id_dir));

					int target_intersection_id=((_Intersection_Interface&)inbound_link->template downstream_intersection<_Intersection_Interface&>() ).template internal_id<int>();
					
					link_id_dir.id=db_itr->getTo_Link()->getLink();
					link_id_dir.dir=0;


					if(net_io_maps.link_id_dir_to_ptr.count(link_id_dir.id_dir))
					{
						outbound_link=(_Link_Interface*)net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir];

						if(((_Intersection_Interface&)outbound_link->template upstream_intersection<_Intersection_Interface&>() ).template internal_id<int>()!=target_intersection_id)
						{
							link_id_dir.dir=1;
							assert(net_io_maps.link_id_dir_to_ptr.count(link_id_dir.id_dir));
							outbound_link=(_Link_Interface*)net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir];
						}
					}
					else
					{
						link_id_dir.dir=1;
						assert(net_io_maps.link_id_dir_to_ptr.count(link_id_dir.id_dir));
						outbound_link=(_Link_Interface*)net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir];
					}

					//TODO: what is the purpose of this? Already checking above...
					if ((inbound_link->template link_type<int>() != FREEWAY && 
						inbound_link->template link_type<int>() != ON_RAMP &&
						inbound_link->template link_type<int>() != OFF_RAMP &&
						inbound_link->template link_type<int>() != EXPRESSWAY &&
						inbound_link->template link_type<int>() != ARTERIAL &&
						inbound_link->template link_type<int>() != EXTERNAL && 
						inbound_link->template link_type<int>() != LOCAL) || 
						(outbound_link->template link_type<int>() != FREEWAY && 
						outbound_link->template link_type<int>() != ON_RAMP &&
						outbound_link->template link_type<int>() != OFF_RAMP &&
						outbound_link->template link_type<int>() != EXPRESSWAY &&
						outbound_link->template link_type<int>() != ARTERIAL &&
						outbound_link->template link_type<int>() != EXTERNAL && 
						outbound_link->template link_type<int>() != LOCAL))
					{
						continue;
					}
					
					counter++;
					//link=(_Link_Interface*)net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir];



					turn_movement = (_Turn_Movement_Interface*)Allocate<typename _Turn_Movement_Interface::Component_Type>();
					turn_movement->template inbound_link<_Link_Interface*>(inbound_link);

					turn_movement->template outbound_link<_Link_Interface*>(outbound_link);

					turn_movement->template uuid<int>(db_itr->getConn());


					turn_movement->template internal_id<int>(counter);

					const string& connect_type=db_itr->getType();

					if(connect_type=="THRU")
					{
						turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::THROUGH_TURN);
						turn_movement->template num_turn_lanes<int>(inbound_link->template num_lanes<int>());
					}
					else if(connect_type=="LEFT" || connect_type=="L_SPLIT" || connect_type=="L_MERGE")
					{
						turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::LEFT_TURN);
						turn_movement->template num_turn_lanes<int>(max(1,inbound_link->template num_left_turn_bays<int>()));
					}
					else if(connect_type=="RIGHT" || connect_type=="R_SPLIT" || connect_type=="R_MERGE")
					{
						turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::RIGHT_TURN);
						turn_movement->template num_turn_lanes<int>(max(1,inbound_link->template num_right_turn_bays<int>()));
					}
					else if(connect_type=="UTURN")
					{
						turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::U_TURN);
						turn_movement->template num_turn_lanes<int>(max(1,inbound_link->template num_left_turn_bays<int>()));
					}
					else
					{
						assert(false);
						break;
					}

					outbound_link->template num_inbound_turn_lanes<int&>() += turn_movement->template num_turn_lanes<int>();
					
					turn_movement->template movement_rule<Turn_Movement_Components::Types::Turn_Movement_Rule_Keys>(Turn_Movement_Components::Types::ALLOWED);
					//int inbound_link_id = turn_movement->template inbound_link<_Link_Interface*>()->template internal_id<int>();
					//int outbound_link_id = turn_movement->template outbound_link<_Link_Interface*>()->template internal_id<int>();
					int inbound_link_id = turn_movement->template inbound_link<_Link_Interface*>()->template uuid<int>();
					int outbound_link_id = turn_movement->template outbound_link<_Link_Interface*>()->template uuid<int>();
					typename MasterType::network_type::long_hash_key_type long_hash_key;
					long_hash_key.inbound_link_id = inbound_link_id;
					long_hash_key.outbound_link_id = outbound_link_id;

					typename MasterType::network_type::type_of_link_turn_movement_map& link_turn_movement_map = _network_reference->template link_turn_movement_map<typename MasterType::network_type::link_turn_movement_map_type&>();
					//link_turn_movement_map.insert(make_pair<long long,typename MasterType::turn_movement_type*>(long_hash_key.movement_id, (typename MasterType::turn_movement_type*)turn_movement));
					link_turn_movement_map[long_hash_key.movement_id] = (typename MasterType::turn_movement_type*)turn_movement;


					turn_movements_container.push_back(turn_movement);
				}

				typename _Intersections_Container_Interface::iterator intersections_itr;

				cout << "Adding multi-modal connections" << endl;

				for (intersections_itr = intersections_container.begin(); intersections_itr != intersections_container.end(); intersections_itr++)
				{
					_Intersection_Interface* intersection = (_Intersection_Interface*)(*intersections_itr);
					//typename type_of(network_reference)::type_of(intersections_container)::type_of(value)& intersection_monitor=(typename type_of(network_reference)::type_of(intersections_container)::type_of(value)&)*intersection;

					cout << "Intersection:" << intersection->_uuid << endl;

					_Links_Container_Interface& inbound_links = intersection->template inbound_links<_Links_Container_Interface&>();
					_Links_Container_Interface& outbound_links = intersection->template outbound_links<_Links_Container_Interface&>();

					typename _Links_Container_Interface::iterator in_links_itr;
					typename _Links_Container_Interface::iterator out_links_itr;

					for (in_links_itr = inbound_links.begin(); in_links_itr != inbound_links.end(); in_links_itr++)
					{
						_Link_Interface* inbound_link = (_Link_Interface*)(*in_links_itr);

						Link_Components::Types::Link_Type_Keys in_facility_type = inbound_link->template link_type<Link_Components::Types::Link_Type_Keys>();

						cout << "\tInbound link's A Node:" << inbound_link->_upstream_intersection->_uuid << "\tType:" << in_facility_type << endl;

						for (out_links_itr = outbound_links.begin(); out_links_itr != outbound_links.end(); out_links_itr++)
						{
							_Link_Interface* outbound_link = (_Link_Interface*)(*out_links_itr);
							Link_Components::Types::Link_Type_Keys out_facility_type = outbound_link->template link_type<Link_Components::Types::Link_Type_Keys>();

							cout << "\t\tOutbound link's B Node:" << outbound_link->_downstream_intersection->_uuid << "\tType:" << out_facility_type;

							if (in_facility_type == Link_Components::Types::Link_Type_Keys::WALK)
							{
								if (out_facility_type == Link_Components::Types::Link_Type_Keys::WALK)
								{
									cout << "\tWalk to Walk\n";
									counter++;
									turn_movement = (_Turn_Movement_Interface*)Allocate<typename _Turn_Movement_Interface::Component_Type>();
									turn_movement->template inbound_link<_Link_Interface*>(inbound_link);
									turn_movement->template outbound_link<_Link_Interface*>(outbound_link);
									//turn_movement->template uuid<int>(db_itr->getConn());
									turn_movement->template internal_id<int>(counter);
									turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::WALK_TO_WALK);
									turn_movement->template movement_rule<Turn_Movement_Components::Types::Turn_Movement_Rule_Keys>(Turn_Movement_Components::Types::ALLOWED); 
									int inbound_link_id = turn_movement->template inbound_link<_Link_Interface*>()->template uuid<int>();
									int outbound_link_id = turn_movement->template outbound_link<_Link_Interface*>()->template uuid<int>();
									typename MasterType::network_type::long_hash_key_type long_hash_key;
									long_hash_key.inbound_link_id = inbound_link_id;
									long_hash_key.outbound_link_id = outbound_link_id;
									typename MasterType::network_type::type_of_link_turn_movement_map& link_turn_movement_map = _network_reference->template link_turn_movement_map<typename MasterType::network_type::link_turn_movement_map_type&>();
									//link_turn_movement_map.insert(make_pair<long long,typename MasterType::turn_movement_type*>(long_hash_key.movement_id, (typename MasterType::turn_movement_type*)turn_movement));
									link_turn_movement_map[long_hash_key.movement_id] = (typename MasterType::turn_movement_type*)turn_movement;
									turn_movements_container.push_back(turn_movement);
								}
								else if (out_facility_type == Link_Components::Types::Link_Type_Keys::TRANSIT)
								{
									cout << "\tWalk to Transit\n";
									counter++;
									turn_movement = (_Turn_Movement_Interface*)Allocate<typename _Turn_Movement_Interface::Component_Type>();
									turn_movement->template inbound_link<_Link_Interface*>(inbound_link);
									turn_movement->template outbound_link<_Link_Interface*>(outbound_link);
									//turn_movement->template uuid<int>(db_itr->getConn());
									turn_movement->template internal_id<int>(counter);
									turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::WALK_TO_TRANSIT);
									turn_movement->template movement_rule<Turn_Movement_Components::Types::Turn_Movement_Rule_Keys>(Turn_Movement_Components::Types::ALLOWED);
									int inbound_link_id = turn_movement->template inbound_link<_Link_Interface*>()->template uuid<int>();
									int outbound_link_id = turn_movement->template outbound_link<_Link_Interface*>()->template uuid<int>();
									typename MasterType::network_type::long_hash_key_type long_hash_key;
									long_hash_key.inbound_link_id = inbound_link_id;
									long_hash_key.outbound_link_id = outbound_link_id;
									typename MasterType::network_type::type_of_link_turn_movement_map& link_turn_movement_map = _network_reference->template link_turn_movement_map<typename MasterType::network_type::link_turn_movement_map_type&>();
									//link_turn_movement_map.insert(make_pair<long long,typename MasterType::turn_movement_type*>(long_hash_key.movement_id, (typename MasterType::turn_movement_type*)turn_movement));
									link_turn_movement_map[long_hash_key.movement_id] = (typename MasterType::turn_movement_type*)turn_movement;
									turn_movements_container.push_back(turn_movement);
								}
								else
								{
									cout << "\tWalk to Drive\n";
									counter++;
									turn_movement = (_Turn_Movement_Interface*)Allocate<typename _Turn_Movement_Interface::Component_Type>();
									turn_movement->template inbound_link<_Link_Interface*>(inbound_link);
									turn_movement->template outbound_link<_Link_Interface*>(outbound_link);
									//turn_movement->template uuid<int>(db_itr->getConn());
									turn_movement->template internal_id<int>(counter);
									turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::WALK_TO_DRIVE);
									turn_movement->template movement_rule<Turn_Movement_Components::Types::Turn_Movement_Rule_Keys>(Turn_Movement_Components::Types::ALLOWED);
									int inbound_link_id = turn_movement->template inbound_link<_Link_Interface*>()->template uuid<int>();
									int outbound_link_id = turn_movement->template outbound_link<_Link_Interface*>()->template uuid<int>();
									typename MasterType::network_type::long_hash_key_type long_hash_key;
									long_hash_key.inbound_link_id = inbound_link_id;
									long_hash_key.outbound_link_id = outbound_link_id;
									typename MasterType::network_type::type_of_link_turn_movement_map& link_turn_movement_map = _network_reference->template link_turn_movement_map<typename MasterType::network_type::link_turn_movement_map_type&>();
									//link_turn_movement_map.insert(make_pair<long long,typename MasterType::turn_movement_type*>(long_hash_key.movement_id, (typename MasterType::turn_movement_type*)turn_movement));
									link_turn_movement_map[long_hash_key.movement_id] = (typename MasterType::turn_movement_type*)turn_movement;
									turn_movements_container.push_back(turn_movement);
								}
							}
							else if (in_facility_type == Link_Components::Types::Link_Type_Keys::TRANSIT)
							{
								if (out_facility_type == Link_Components::Types::Link_Type_Keys::WALK)
								{
									cout << "\tTransit to Walk\n";
									counter++;
									turn_movement = (_Turn_Movement_Interface*)Allocate<typename _Turn_Movement_Interface::Component_Type>();
									turn_movement->template inbound_link<_Link_Interface*>(inbound_link);
									turn_movement->template outbound_link<_Link_Interface*>(outbound_link);
									//turn_movement->template uuid<int>(db_itr->getConn());
									turn_movement->template internal_id<int>(counter);
									turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::TRANSIT_TO_WALK);
									turn_movement->template movement_rule<Turn_Movement_Components::Types::Turn_Movement_Rule_Keys>(Turn_Movement_Components::Types::ALLOWED);
									int inbound_link_id = turn_movement->template inbound_link<_Link_Interface*>()->template uuid<int>();
									int outbound_link_id = turn_movement->template outbound_link<_Link_Interface*>()->template uuid<int>();
									typename MasterType::network_type::long_hash_key_type long_hash_key;
									long_hash_key.inbound_link_id = inbound_link_id;
									long_hash_key.outbound_link_id = outbound_link_id;
									typename MasterType::network_type::type_of_link_turn_movement_map& link_turn_movement_map = _network_reference->template link_turn_movement_map<typename MasterType::network_type::link_turn_movement_map_type&>();
									//link_turn_movement_map.insert(make_pair<long long,typename MasterType::turn_movement_type*>(long_hash_key.movement_id, (typename MasterType::turn_movement_type*)turn_movement));
									link_turn_movement_map[long_hash_key.movement_id] = (typename MasterType::turn_movement_type*)turn_movement;
									turn_movements_container.push_back(turn_movement);
								}
								else if (out_facility_type == Link_Components::Types::Link_Type_Keys::TRANSIT)
								{
									cout << "\tTransit to Transit\n";
									counter++;
									turn_movement = (_Turn_Movement_Interface*)Allocate<typename _Turn_Movement_Interface::Component_Type>();
									turn_movement->template inbound_link<_Link_Interface*>(inbound_link);
									turn_movement->template outbound_link<_Link_Interface*>(outbound_link);
									//turn_movement->template uuid<int>(db_itr->getConn());
									turn_movement->template internal_id<int>(counter);
									turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::TRANSIT_TO_TRANSIT);
									turn_movement->template movement_rule<Turn_Movement_Components::Types::Turn_Movement_Rule_Keys>(Turn_Movement_Components::Types::ALLOWED);
									int inbound_link_id = turn_movement->template inbound_link<_Link_Interface*>()->template uuid<int>();
									int outbound_link_id = turn_movement->template outbound_link<_Link_Interface*>()->template uuid<int>();
									typename MasterType::network_type::long_hash_key_type long_hash_key;
									long_hash_key.inbound_link_id = inbound_link_id;
									long_hash_key.outbound_link_id = outbound_link_id;
									typename MasterType::network_type::type_of_link_turn_movement_map& link_turn_movement_map = _network_reference->template link_turn_movement_map<typename MasterType::network_type::link_turn_movement_map_type&>();
									//link_turn_movement_map.insert(make_pair<long long,typename MasterType::turn_movement_type*>(long_hash_key.movement_id, (typename MasterType::turn_movement_type*)turn_movement));
									link_turn_movement_map[long_hash_key.movement_id] = (typename MasterType::turn_movement_type*)turn_movement;
									turn_movements_container.push_back(turn_movement);
								}
								else
								{
									cout << "\tTransit to Drive\n";
									counter++;
									turn_movement = (_Turn_Movement_Interface*)Allocate<typename _Turn_Movement_Interface::Component_Type>();
									turn_movement->template inbound_link<_Link_Interface*>(inbound_link);
									turn_movement->template outbound_link<_Link_Interface*>(outbound_link);
									//turn_movement->template uuid<int>(db_itr->getConn());
									turn_movement->template internal_id<int>(counter);
									turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::TRANSIT_TO_DRIVE);
									turn_movement->template movement_rule<Turn_Movement_Components::Types::Turn_Movement_Rule_Keys>(Turn_Movement_Components::Types::ALLOWED);
									int inbound_link_id = turn_movement->template inbound_link<_Link_Interface*>()->template uuid<int>();
									int outbound_link_id = turn_movement->template outbound_link<_Link_Interface*>()->template uuid<int>();
									typename MasterType::network_type::long_hash_key_type long_hash_key;
									long_hash_key.inbound_link_id = inbound_link_id;
									long_hash_key.outbound_link_id = outbound_link_id;
									typename MasterType::network_type::type_of_link_turn_movement_map& link_turn_movement_map = _network_reference->template link_turn_movement_map<typename MasterType::network_type::link_turn_movement_map_type&>();
									//link_turn_movement_map.insert(make_pair<long long,typename MasterType::turn_movement_type*>(long_hash_key.movement_id, (typename MasterType::turn_movement_type*)turn_movement));
									link_turn_movement_map[long_hash_key.movement_id] = (typename MasterType::turn_movement_type*)turn_movement;
									turn_movements_container.push_back(turn_movement);
								}
							}
							else
							{
								if (out_facility_type == Link_Components::Types::Link_Type_Keys::WALK)
								{
									cout << "\tDrive to Walk\n";
									counter++;
									turn_movement = (_Turn_Movement_Interface*)Allocate<typename _Turn_Movement_Interface::Component_Type>();
									turn_movement->template inbound_link<_Link_Interface*>(inbound_link);
									turn_movement->template outbound_link<_Link_Interface*>(outbound_link);
									//turn_movement->template uuid<int>(db_itr->getConn());
									turn_movement->template internal_id<int>(counter);
									turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::DRIVE_TO_WALK);
									turn_movement->template movement_rule<Turn_Movement_Components::Types::Turn_Movement_Rule_Keys>(Turn_Movement_Components::Types::ALLOWED);
									int inbound_link_id = turn_movement->template inbound_link<_Link_Interface*>()->template uuid<int>();
									int outbound_link_id = turn_movement->template outbound_link<_Link_Interface*>()->template uuid<int>();
									typename MasterType::network_type::long_hash_key_type long_hash_key;
									long_hash_key.inbound_link_id = inbound_link_id;
									long_hash_key.outbound_link_id = outbound_link_id;
									typename MasterType::network_type::type_of_link_turn_movement_map& link_turn_movement_map = _network_reference->template link_turn_movement_map<typename MasterType::network_type::link_turn_movement_map_type&>();
									//link_turn_movement_map.insert(make_pair<long long,typename MasterType::turn_movement_type*>(long_hash_key.movement_id, (typename MasterType::turn_movement_type*)turn_movement));
									link_turn_movement_map[long_hash_key.movement_id] = (typename MasterType::turn_movement_type*)turn_movement;
									turn_movements_container.push_back(turn_movement);
								}
								else if (out_facility_type == Link_Components::Types::Link_Type_Keys::TRANSIT)
								{
									cout << "\tDrive to Transit\n";
									counter++;
									turn_movement = (_Turn_Movement_Interface*)Allocate<typename _Turn_Movement_Interface::Component_Type>();
									turn_movement->template inbound_link<_Link_Interface*>(inbound_link);
									turn_movement->template outbound_link<_Link_Interface*>(outbound_link);
									//turn_movement->template uuid<int>(db_itr->getConn());
									turn_movement->template internal_id<int>(counter);
									turn_movement->template movement_type<Turn_Movement_Components::Types::Turn_Movement_Type_Keys>(Turn_Movement_Components::Types::DRIVE_TO_TRANSIT);
									turn_movement->template movement_rule<Turn_Movement_Components::Types::Turn_Movement_Rule_Keys>(Turn_Movement_Components::Types::ALLOWED);
									int inbound_link_id = turn_movement->template inbound_link<_Link_Interface*>()->template uuid<int>();
									int outbound_link_id = turn_movement->template outbound_link<_Link_Interface*>()->template uuid<int>();
									typename MasterType::network_type::long_hash_key_type long_hash_key;
									long_hash_key.inbound_link_id = inbound_link_id;
									long_hash_key.outbound_link_id = outbound_link_id;
									typename MasterType::network_type::type_of_link_turn_movement_map& link_turn_movement_map = _network_reference->template link_turn_movement_map<typename MasterType::network_type::link_turn_movement_map_type&>();
									//link_turn_movement_map.insert(make_pair<long long,typename MasterType::turn_movement_type*>(long_hash_key.movement_id, (typename MasterType::turn_movement_type*)turn_movement));
									link_turn_movement_map[long_hash_key.movement_id] = (typename MasterType::turn_movement_type*)turn_movement;
									turn_movements_container.push_back(turn_movement);
								}
								else
								{
									cout << "\tDrive to Drive\n";
								}
							}



						}
					}
				}



				cout << "Configuring Connections" << endl;

				typename _Turn_Movements_Container_Interface::iterator turn_movements_itr;
				for(turn_movements_itr = turn_movements_container.begin(); turn_movements_itr != turn_movements_container.end(); turn_movements_itr++)
				{
					_Turn_Movement_Interface* turn_mvmt=(_Turn_Movement_Interface*)(*turn_movements_itr);

					//outbound turn movement of an inbound link
					_Link_Interface* inbound_link = turn_mvmt->template inbound_link<_Link_Interface*>();
					inbound_link->template outbound_turn_movements<_Turn_Movements_Container_Interface&>().push_back(turn_mvmt);
					//inbound turn movement of an outbound link
					_Link_Interface* outbound_link = turn_mvmt->template outbound_link<_Link_Interface*>();
					outbound_link->template inbound_turn_movements<_Turn_Movements_Container_Interface&>().push_back(turn_mvmt);
				}


				typedef  Intersection_Components::Prototypes::Outbound_Inbound_Movements<typename remove_pointer< typename _Intersection_Interface::get_type_of(outbound_inbound_movements)::value_type>::type>  _Outbound_Inbound_Movements_Interface;
				typedef  Random_Access_Sequence< typename _Intersection_Interface::get_type_of(outbound_inbound_movements), _Outbound_Inbound_Movements_Interface*> _Outbound_Inbound_Movements_Container_Interface;


				// configure outbound_inbound_movements
//				typename _Intersections_Container_Interface::iterator intersections_itr;

				cout << "Configuring Outbound Inbound Movements" << endl;

				for(intersections_itr = intersections_container.begin(); intersections_itr != intersections_container.end(); intersections_itr++)
				{
					_Intersection_Interface* intersection = (_Intersection_Interface*)(*intersections_itr);
					//typename type_of(network_reference)::type_of(intersections_container)::type_of(value)& intersection_monitor=(typename type_of(network_reference)::type_of(intersections_container)::type_of(value)&)*intersection;

					_Links_Container_Interface& outbound_links = intersection->template outbound_links<_Links_Container_Interface&>();
					
					// skip intersections_container that do not have any outbound links
					if (outbound_links.size() == 0) continue;
					
					typename _Links_Container_Interface::iterator links_itr;
					
					for (links_itr = outbound_links.begin(); links_itr != outbound_links.end(); links_itr++)
					{
						_Link_Interface* outboundLink = (_Link_Interface*)(*links_itr);
						
						_Outbound_Inbound_Movements_Interface* outboundInboundMovements = (_Outbound_Inbound_Movements_Interface*)Allocate<typename _Outbound_Inbound_Movements_Interface::Component_Type>();
						
						outboundInboundMovements->template outbound_link_reference<_Link_Interface*>(outboundLink);
						_Turn_Movements_Container_Interface& inboundTurnMovements = outboundLink->template inbound_turn_movements<_Turn_Movements_Container_Interface&>();
						
						for (int k = 0; k < (int)inboundTurnMovements.size(); k++)
						{
							outboundInboundMovements->template inbound_movements<_Turn_Movements_Container_Interface&>().push_back(inboundTurnMovements[k]);
						}

						intersection->template outbound_inbound_movements<_Outbound_Inbound_Movements_Container_Interface&>().push_back(outboundInboundMovements);
					}
				}



				typedef  Intersection_Components::Prototypes::Inbound_Outbound_Movements<typename remove_pointer< typename _Intersection_Interface::get_type_of(inbound_outbound_movements)::value_type>::type>  _Inbound_Outbound_Movements_Interface;
				typedef  Random_Access_Sequence< typename _Intersection_Interface::get_type_of(inbound_outbound_movements), _Inbound_Outbound_Movements_Interface*> _Inbound_Outbound_Movements_Container_Interface;


				cout << "Configuring Inbound Outbound Movements" << endl;

				// configure inbound_outbound_movements
				for (int i = 0; i < (int)intersections_container.size(); i++)
				{
					_Intersection_Interface* intersection = (_Intersection_Interface*)intersections_container[i];
					
					// skip intersections_container that do not have any inbound links
					if (intersection->template inbound_links<_Links_Container_Interface&>().size() == 0) continue;
					
					for (int j = 0; j < (int)intersection->template inbound_links<_Links_Container_Interface&>().size(); j++) 
					{
						_Inbound_Outbound_Movements_Interface* inboundOutboundMovements = (_Inbound_Outbound_Movements_Interface*)Allocate<typename _Inbound_Outbound_Movements_Interface::Component_Type>();
						_Link_Interface* inboundLink = (_Link_Interface*)intersection->template inbound_links<_Links_Container_Interface&>()[j];
						inboundOutboundMovements->template inbound_link_reference<_Link_Interface*>(inboundLink);
						_Turn_Movements_Container_Interface& outboundTurnMovements = inboundLink->template outbound_turn_movements<_Turn_Movements_Container_Interface&>();

						for (int k = 0; k < (int)outboundTurnMovements.size(); k++)
						{
							inboundOutboundMovements->template outbound_movements<_Turn_Movements_Container_Interface&>().push_back(outboundTurnMovements[k]);
						}
						intersection->template inbound_outbound_movements<_Inbound_Outbound_Movements_Container_Interface&>().push_back(inboundOutboundMovements);
					}
				}
			}

			template<typename TargetType> void read_zone_data(unique_ptr<odb::database>& db, Network_Components::Types::Network_IO_Maps& net_io_maps)
			{
				using namespace odb;
				using namespace polaris::io;

				//Types::Link_ID_Dir link_id_dir;

				cout << "Reading Zones" << endl;

				int counter=-1;
				typedef Random_Access_Sequence<typename type_of(network_reference)::get_type_of(zone_ids_container),int> _zone_ids_interface;
				typedef  Link_Components::Prototypes::Link<typename remove_pointer< typename type_of(network_reference)::get_type_of(links_container)::value_type>::type>  _Link_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(links_container), _Link_Interface*> _Links_Container_Interface;

				//%%%RLW - check into this again
				//typedef  Zone_Components::Prototypes::Zone<typename remove_pointer< typename type_of(network_reference)::get_type_of(zones_container)::value_type>::type>  _Zone_Interface;
				//typedef  Pair_Associative_Container< typename type_of(network_reference)::get_type_of(zones_container), _Zone_Interface*> _Zones_Container_Interface;

				typedef Zone_Components::Prototypes::Zone<typename remove_pointer<typename type_of(network_reference)::get_type_of(zones_container)::data_type>::type> _Zone_Interface;
				typedef dense_hash_map<int,_Zone_Interface*> _Zones_Container_Interface;
				typedef  Activity_Location_Components::Prototypes::Activity_Location<typename remove_pointer< typename type_of(network_reference)::get_type_of(activity_locations_container)::value_type>::type>  _Activity_Location_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(activity_locations_container), _Activity_Location_Interface*> _Activity_Locations_Container_Interface;

				_Zones_Container_Interface& zones_container = _network_reference->template zones_container<_Zones_Container_Interface&>();
				_zone_ids_interface& zone_ids_container = _network_reference->template zone_ids_container<_zone_ids_interface&>();

				// initialzie zone hash_map
				zones_container.set_empty_key(-1);
				zones_container.set_deleted_key(-2);
				typename _Zones_Container_Interface::iterator zone_itr;
		
				// get zones from database
				_Zone_Interface* zone;
				result<polaris::io::Zone> zone_result;
				try
				{
					zone_result=db->template query<polaris::io::Zone>(query<polaris::io::Zone>::true_expr);	
				}
				catch (const odb::exception& e)
				{
					THROW_EXCEPTION(endl<<endl<<"ERROR: the database using outdated specification for 'Zone' table.  Please run the database through the Network Editor to bring it up to date."<<endl<<"DB error: "<< e.what()<<endl);
				}

				int zone_count = 0;
				for(typename result<polaris::io::Zone>::iterator db_itr = zone_result.begin (); db_itr != zone_result.end (); ++db_itr, ++zone_count)
				{
					try
					{
						zone = (_Zone_Interface*)Allocate<typename _Zone_Interface::Component_Type>();
						zone->template Initialize<NULLTYPE>();
						zone->template uuid<int>(db_itr->getZone());
						zone->template internal_id<int>(zone_count);
						zone->template X<double>( _scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getX()));
						zone->template Y<double>( _scenario_reference->template meterToFoot<NULLTYPE>(db_itr->getY()));
						zone->template areatype<int>(db_itr->getArea_type()->getArea_Type());

						// test - moved here from the zonelanduse read area - combining tables
						zone->template average_household_income<Dollars>(db_itr->getHH_inc_avg());
						zone->template race_percent_white<float>(db_itr->getPercent_white());
						zone->template race_percent_black<float>(db_itr->getPercent_black());
						zone->template average_household_income<Dollars>(db_itr->getHH_inc_avg());
						zone->template area<Square_Meters>(db_itr->getArea());
						zone->template entertainment_area<Square_Meters>(db_itr->getEntertainment_area());
						zone->template industrial_area<Square_Meters>(db_itr->getIndustrial_area());
						zone->template institutional_area<Square_Meters>(db_itr->getInstitutional_area());
						zone->template mixed_use_area<Square_Meters>(db_itr->getMixed_use_area());
						zone->template office_area<Square_Meters>(db_itr->getOffice_area());
						zone->template other_area<Square_Meters>(db_itr->getOther_area());
						zone->template residential_area<Square_Meters>(db_itr->getResidential_area());
						zone->template retail_area<Square_Meters>(db_itr->getRetail_area());
						zone->template school_area<Square_Meters>(db_itr->getSchool_area());
						zone->template pop_households<int>(db_itr->getPop_households());
						zone->template pop_persons<int>(db_itr->getPop_persons());
						zone->template pop_group_quarters<int>(db_itr->getPop_group_quarters());
						zone->template employment_total<int>(db_itr->getEmployment_total());
						zone->template employment_retail<int>(db_itr->getEmployment_retail());
						zone->template employment_government<int>(db_itr->getEmployment_government());
						zone->template employment_manufacturing<int>(db_itr->getEmployment_manufacturing());
						zone->template employment_services<int>(db_itr->getEmployment_services());
						zone->template employment_industrial<int>(db_itr->getEmployment_industrial());
						zone->template employment_other<int>(db_itr->getEmployment_other());

						zones_container.insert(pair<int,_Zone_Interface*>(zone->template uuid<int>(), zone));
					}
					catch (const odb::exception& e)
					{
						cout << e.what()<<endl;
					}
				}

				zone_ids_container.resize(zone_count);
				for(zone_itr = zones_container.begin(); zone_itr != zones_container.end (); ++zone_itr)
				{
					zone = (_Zone_Interface*)zone_itr->second;
					int index = zone->template internal_id<int>();
					int uuid = zone->template uuid<int>();
					zone_ids_container[index]=uuid;
				}
			}
		
			template<typename TargetType> void read_activity_location_data(unique_ptr<odb::database>& db, Network_Components::Types::Network_IO_Maps& net_io_maps)
			{
				using namespace odb;
				using namespace polaris::io;

				Types::Link_ID_Dir link_id_dir;
				Types::Link_ID_Dir opp_link_id_dir;

				std::unordered_map<int,int> uuid_to_index;

				cout << "Reading Activity Locations..." << endl;

				int counter=0;

				typedef  Link_Components::Prototypes::Link<typename remove_pointer< typename type_of(network_reference)::get_type_of(links_container)::value_type>::type>  _Link_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(links_container), _Link_Interface*> _Links_Container_Interface;

				typedef Zone_Components::Prototypes::Zone<typename remove_pointer<typename type_of(network_reference)::get_type_of(zones_container)::data_type>::type> _Zone_Interface;
				typedef dense_hash_map<int,_Zone_Interface*> _Zones_Container_Interface;
				
				typedef  Activity_Location_Components::Prototypes::Activity_Location<typename remove_pointer< typename type_of(network_reference)::get_type_of(activity_locations_container)::value_type>::type>  _Activity_Location_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(activity_locations_container), _Activity_Location_Interface*> _Activity_Locations_Container_Interface;

				typedef dense_hash_map<int,int> _loc_id_to_idx_container_interface;

				_Activity_Locations_Container_Interface& activity_locations_container = _network_reference->template activity_locations_container<_Activity_Locations_Container_Interface&>();
				_loc_id_to_idx_container_interface& loc_id_to_idx_container = _network_reference->template activity_location_id_to_idx_container<_loc_id_to_idx_container_interface&>();

				activity_locations_container.clear();

				loc_id_to_idx_container.set_empty_key(-1);
				loc_id_to_idx_container.set_deleted_key(-2);

				result<Location> location_result;
				try
				{
					location_result=db->template query<Location>(query<Location>::true_expr);
				}
				catch (const odb::exception& e)
				{
					THROW_EXCEPTION(endl<<endl<<"ERROR: the database using outdated specification for 'Location' table.  Please run the database through the Network Editor to bring it up to date."<<endl<<"DB error: "<< e.what()<<endl)
				}

				_Zones_Container_Interface* zones = _network_reference->template zones_container<_Zones_Container_Interface*>();
				typename _Zones_Container_Interface::iterator zone_itr;

				_Activity_Location_Interface* activity_location;
				int skipped_counter=0;
				_Link_Interface* link;

				for(typename result<Location>::iterator db_itr = location_result.begin (); db_itr != location_result.end (); ++db_itr)
				{
					try
					{
						link_id_dir.id=db_itr->getLink()->getLink();
						link_id_dir.dir=db_itr->getDir();

				
						if(!net_io_maps.link_id_dir_to_ptr.count(link_id_dir.id_dir))
						{
							// TODO: may want to expand this so all activity locations can make left turn onto origin link
							// try the opposite direction to account for left turns onto the origin link
							link_id_dir.dir = abs(db_itr->getDir() - 1);
							if(!net_io_maps.link_id_dir_to_ptr.count(link_id_dir.id_dir))
							{
								if(++skipped_counter%1000==0)
								{
									cout << skipped_counter << " locations skipped" << endl;
								}
								continue;		
							}
						}

						
						if(counter%10000==0) cout << "\t" << counter << endl;


						// get the zone id and pull interface to zone from zone container
						int zone_id = db_itr->getZone()->getZone();
						if ((zone_itr=zones->find(zone_id)) == zones->end()) THROW_EXCEPTION("ERROR, zone id: "<<zone_id<<" was not found.");
						_Zone_Interface* zone = zone_itr->second;

						activity_location = (_Activity_Location_Interface*)Allocate<typename _Activity_Location_Interface::Component_Type>();
						assert(net_io_maps.link_id_dir_to_ptr.count(link_id_dir.id_dir));
						link=(_Link_Interface*)net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir];

						activity_location->template origin_links<_Links_Container_Interface&>().push_back(link);
						link->template activity_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);

						activity_location->template destination_links<_Links_Container_Interface&>().push_back(link);

						// add the opposite direction link if exists
						if (!this->_scenario_reference->template use_link_based_routing<bool>())
						{
							opp_link_id_dir.id = link_id_dir.id;
							opp_link_id_dir.dir = abs(link_id_dir.dir - 1);
							if(net_io_maps.link_id_dir_to_ptr.count(opp_link_id_dir.id_dir))
							{
								link=(_Link_Interface*)net_io_maps.link_id_dir_to_ptr[opp_link_id_dir.id_dir];
								activity_location->template origin_links<_Links_Container_Interface&>().push_back(link);
								link->template activity_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
								activity_location->template destination_links<_Links_Container_Interface&>().push_back(link);
							}
						}
				
						activity_location->template zone<_Zone_Interface*>(zone);
						activity_location->template uuid<int>(db_itr->getPrimaryKey());
						activity_location->template internal_id<int>(counter);

						//shared_ptr<LocationData> data_ptr = db_itr->getLocation_Data();
						//if (data_ptr == nullptr) continue;

						//activity_location->template x_position<Meters>(/*_scenario_reference->template meterToFoot<NULLTYPE>(*/data_ptr->getX()/*)*/);
						//activity_location->template y_position<Meters>(/*_scenario_reference->template meterToFoot<NULLTYPE>(*/data_ptr->getY()/*)*/);
						//activity_location->template census_zone_id<long long>(data_ptr->getCensus_Zone());
						activity_location->template x_position<Meters>(db_itr->getX());
						activity_location->template y_position<Meters>(db_itr->getY());
						activity_location->template census_zone_id<long long>(db_itr->getCensus_Zone());
	
						// set the location land use code
						//const char* land_use = data_ptr->getLand_Use().c_str();
						const char* land_use = db_itr->getLand_Use().c_str();

						Activity_Location_Components::Types::LAND_USE code;
						if (strcmp(land_use,"ALL")==0)
						{
							code=Activity_Location_Components::Types::LU_ALL;
						}
						else if (strcmp(land_use,"AGRICULTURE")==0)
						{
							code=Activity_Location_Components::Types::LU_AGRICULTURE;
						}
						else if (strcmp(land_use,"BUSINESS")==0)
						{
							code=Activity_Location_Components::Types::LU_BUSINESS;
						}
						else if (strcmp(land_use,"CIVIC")==0)
						{
								code=Activity_Location_Components::Types::LU_CIVIC_RELIGIOUS;
						}
						else if (strcmp(land_use,"CULTURE")==0)
						{
							code=Activity_Location_Components::Types::LU_CULTURAL;
						}
						else if (strcmp(land_use,"EDUCATION")==0)
						{
							code=Activity_Location_Components::Types::LU_EDUCATION;
						}
						else if (strcmp(land_use,"INDUSTRY")==0)
						{
							code=Activity_Location_Components::Types::LU_INDUSTRIAL;
						}
						else if (strcmp(land_use,"MAJ_SHOP")==0)
						{
							code=Activity_Location_Components::Types::LU_SHOPPING;
						}
						else if (strcmp(land_use,"MEDICAL")==0)
						{
							code=Activity_Location_Components::Types::LU_MEDICAL;
						}
						else if (strcmp(land_use,"MIX")==0)
						{
							code=Activity_Location_Components::Types::LU_MIXED_USE;
						}
						else if (strcmp(land_use,"NONE")==0)
						{
							code=Activity_Location_Components::Types::LU_NONE;
						}
						else if (strcmp(land_use,"RECREATION")==0)
						{
							code=Activity_Location_Components::Types::LU_RECREATION;
						}
						else if (strcmp(land_use,"RES")==0)
						{
							code=Activity_Location_Components::Types::LU_RESIDENTIAL;
						}
						else if (strcmp(land_use, "NON_RESIDENTIAL") == 0)
						{
							code = Activity_Location_Components::Types::LU_NON_RESIDENTIAL;
						}
						else if (strcmp(land_use, "RESIDENTIAL-SINGLE") == 0)
						{
							code = Activity_Location_Components::Types::LU_RESIDENTIAL;
						}
						else if (strcmp(land_use, "RESIDENTIAL-MULTI") == 0)
						{
							code = Activity_Location_Components::Types::LU_RESIDENTIAL_MULTI;
						}
						else if (strcmp(land_use,"SPECIAL_GEN")==0)
						{
							code=Activity_Location_Components::Types::LU_SPECIAL_GENERATOR;
						}
						else if (strcmp(land_use,"TRANSIT_STOP")==0)
						{
							code=Activity_Location_Components::Types::TRANSIT_STOP;
						}
						else
						{
							code=Activity_Location_Components::Types::LU_NONE;
						}
						activity_location->land_use_type(code);

						// add to the zone std::list
						zone->template origin_activity_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
						zone->template destination_activity_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);

						// add to zone land use type substd::lists
						if (code == Activity_Location_Components::Types::LU_ALL)
						{
							zone->template home_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template work_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template school_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template discretionary_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
						}
						if (code == Activity_Location_Components::Types::LU_MIXED_USE)
						{
							zone->template home_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template work_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template discretionary_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
						}
						if (code == Activity_Location_Components::Types::LU_CIVIC_RELIGIOUS)
						{
							zone->template work_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template school_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template discretionary_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
						}
						if (code == Activity_Location_Components::Types::LU_BUSINESS || code == Activity_Location_Components::Types::LU_INDUSTRIAL || code == Activity_Location_Components::Types::LU_SHOPPING || code == Activity_Location_Components::Types::LU_MEDICAL)
						{
							zone->template work_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template discretionary_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
						}
						if (code == Activity_Location_Components::Types::LU_CULTURAL || code == Activity_Location_Components::Types::LU_CIVIC_RELIGIOUS)
						{
							zone->template work_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template discretionary_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
						}
						if (code == Activity_Location_Components::Types::LU_RESIDENTIAL)
						{
							zone->template home_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template discretionary_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
						}
						if (code == Activity_Location_Components::Types::LU_RESIDENTIAL_MULTI)
						{
							zone->template home_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template discretionary_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
						}
						if (code == Activity_Location_Components::Types::LU_EDUCATION)
						{
							zone->template work_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
							zone->template school_locations<_Activity_Locations_Container_Interface&>().push_back(activity_location);
						}
			
						// Push to main network container
						activity_locations_container.push_back(activity_location);

						// store the id to index lookup info
						loc_id_to_idx_container.insert(pair<int,int>(activity_location->template uuid<int>(),activity_location->template internal_id<int>()));
						++counter;
					}
					catch (const odb::exception& e) {THROW_WARNING(e.what()); e.what(); continue;}
					//catch (exception e){THROW_WARNING(e.what()); continue;}
				}

				cout <<"done."<<endl;
			}
			
			template<typename TargetType> void read_pocket_data(unique_ptr<odb::database>& db, Network_Components::Types::Network_IO_Maps& net_io_maps)
			{
				using namespace odb;
				using namespace polaris::io;

				result<polaris::io::Pocket> pocket_result=db->template query<polaris::io::Pocket>(query<polaris::io::Pocket>::true_expr);
				
				int counter=0;

				cout << "Reading Pockets" << endl;
				
				Types::Link_ID_Dir link_id_dir;

				typedef Link_Components::Prototypes::Link<typename MasterType::link_type> Link_Interface;
				Link_Interface* link;
				Link_Components::Implementations::Pocket_Data* pocket_data;

				for(typename result<polaris::io::Pocket>::iterator db_itr = pocket_result.begin (); db_itr != pocket_result.end (); ++db_itr)
				{
					if(counter%10000==0) cout << "\t" << counter << endl;
					++counter;

					link_id_dir.id = db_itr->getLink()->getLink();
					link_id_dir.dir = db_itr->getDir();

					int lanes = 0;
					float length = 0.0f;

					if(net_io_maps.link_id_dir_to_ptr.count(link_id_dir.id_dir))
					{
						link = (Link_Interface*)net_io_maps.link_id_dir_to_ptr[link_id_dir.id_dir];

						pocket_data = link->template pocket_data<Link_Components::Implementations::Pocket_Data*>();

						lanes = db_itr->getLanes();

						if(lanes == 0) lanes=1;

						pocket_data->num_pockets += lanes;
						const string& type=db_itr->getType();

						if(type == "RIGHT_TURN" || type == "RIGHT_MERGE")
						{
							pocket_data->num_pockets_right += lanes;
						}
						else if(type == "LEFT_TURN" || type == "LEFT_MERGE")
						{
							pocket_data->num_pockets_left += lanes;
						}
						else
						{
							cout << "Unknown pocket type: " << type << endl;
						}

						length = db_itr->getLength()*3.28084;

						// average pocket length
						if(length == 0.0f) length = 203.412f;

						// convert to feet
						pocket_data->pocket_length = length;
					}
				}
					
			}

			template<typename TargetType> void clean_isolated_intersections()
			{
				typedef  Link_Components::Prototypes::Link<typename remove_pointer< typename type_of(network_reference)::get_type_of(links_container)::value_type>::type>  _Link_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(links_container), _Link_Interface*> _Links_Container_Interface;

				typedef  Intersection_Components::Prototypes::Intersection<typename remove_pointer< typename type_of(network_reference)::get_type_of(intersections_container)::value_type>::type>  _Intersection_Interface;
				typedef  Random_Access_Sequence< typename type_of(network_reference)::get_type_of(intersections_container), _Intersection_Interface*> _Intersections_Container_Interface;

				typename _Intersections_Container_Interface::iterator intersections_itr;
				_Intersections_Container_Interface& intersections_container = _network_reference->template intersections_container<_Intersections_Container_Interface&>();

				float counter = 0.0;
				for(intersections_itr = intersections_container.begin(); intersections_itr != intersections_container.end(); intersections_itr++)
				{
					_Intersection_Interface* intersection = (_Intersection_Interface*)(*intersections_itr);
					//typename type_of(network_reference)::type_of(intersections_container)::type_of(value)& intersection_monitor=(typename type_of(network_reference)::type_of(intersections_container)::type_of(value)&)*intersection;

					_Links_Container_Interface& outbound_links = intersection->template outbound_links<_Links_Container_Interface&>();
					_Links_Container_Interface& inbound_links = intersection->template inbound_links<_Links_Container_Interface&>();
					// skip intersections_container that do not have any outbound links
					if (outbound_links.size() == 0 && inbound_links.size() == 0)
					{
						intersections_container.erase(intersections_itr--);
					}
					else
					{
						intersection->template internal_id<int>(counter++);
					}
				}
			}
		};
	}
}

using namespace Network_Components::Implementations;
