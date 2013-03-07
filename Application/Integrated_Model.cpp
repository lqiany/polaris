#include "Model_Selection.h"

//#define FOR_LINUX_PORTING
//#define EXCLUDE_DEMAND

#ifdef IntegratedModelApplication
#define DBIO
#ifdef DBIO
#define WINDOWS
#include "Application_Includes.h"
#include "../File_IO/network_models.h"
struct MasterType
{
	typedef MasterType M;

#ifdef ANTARES
	typedef Conductor_Implementation<M> conductor_type;
	typedef Control_Panel_Implementation<M> control_panel_type;
	typedef Time_Panel_Implementation<M> time_panel_type;
	typedef Information_Panel_Implementation<M> information_panel_type;
	typedef Canvas_Implementation<M> canvas_type;
	typedef Antares_Layer_Implementation<M> antares_layer_type;
	typedef Layer_Options_Implementation<M> layer_options_type;
	typedef Attributes_Panel_Implementation<M> attributes_panel_type;
	typedef Control_Dialog_Implementation<M> control_dialog_type;

	typedef Graphical_Network_Implementation<M> graphical_network_type;
	typedef Graphical_Link_Implementation<M> graphical_link_type;
	typedef Graphical_Intersection_Implementation<M> graphical_intersection_type;
	typedef Vehicle_Components::Implementations::Graphical_Vehicle_Implementation<M> vehicle_type;	
	typedef Zone_Components::Implementations::Graphical_Zone_Implementation<M> zone_type;
	//typedef Zone_Components::Implementations::Polaris_Zone_Implementation<M> zone_type;
	typedef Zone_Components::Implementations::Graphical_Zone_Group_Implementation<M> graphical_zone_group_type;
#else
	typedef Vehicle_Components::Implementations::Polaris_Vehicle_Implementation<M> vehicle_type;
	typedef Zone_Components::Implementations::Polaris_Zone_Implementation<M> zone_type;
#endif

	//==============================================================================================
	// Network Types
	typedef Scenario_Components::Implementations::Polaris_Scenario_Implementation<M> scenario_type;
	typedef Network_Components::Implementations::Integrated_Polaris_Network_Implementation<M> network_type;
	typedef Network_Components::Implementations::Network_DB_Reader_Implementation<M> network_db_reader_type;
	typedef Intersection_Components::Implementations::Polaris_Intersection_Implementation<M> intersection_type;
	typedef Turn_Movement_Components::Implementations::Polaris_Movement_Implementation<M> movement_type;
	typedef Link_Components::Implementations::Polaris_Link_Implementation<M> link_type;
	typedef Turn_Movement_Components::Implementations::Polaris_Movement_Implementation<M> turn_movement_type;
	typedef Routing_Components::Implementations::Routable_Network_Implementation<M> routable_network_type;
	typedef Routing_Components::Implementations::Polaris_Routing_Implementation<M> routing_type;
	typedef Routing_Components::Implementations::Polaris_Skim_Routing_Implementation<M> skim_routing_type;
	typedef Intersection_Components::Implementations::Routable_Intersection_Implementation<M> routable_intersection_type;
	typedef Link_Components::Implementations::Routable_Link_Implementation<M> routable_link_type;
	typedef Activity_Location_Components::Implementations::Polaris_Activity_Location_Implementation<M> activity_location_type;
	typedef Traveler_Components::Implementations::Polaris_Traveler_Implementation<M> traveler_type;
	typedef Intersection_Components::Implementations::Polaris_Inbound_Outbound_Movements_Implementation<M> inbound_outbound_movements_type;
	typedef Intersection_Components::Implementations::Polaris_Outbound_Inbound_Movements_Implementation<M> outbound_inbound_movements_type;
	typedef Intersection_Components::Implementations::Routable_Inbound_Outbound_Movements_Implementation<M> routable_inbound_outbound_movements_type;
	typedef Intersection_Components::Implementations::Routable_Outbound_Inbound_Movements_Implementation<M> routable_outbound_inbound_movements_type;
	typedef Intersection_Components::Implementations::Routable_Movement_Implementation<M> routable_movement_type;
	typedef Operation_Components::Implementations::Polaris_Operation_Implementation<M> operation_type;
	typedef Intersection_Control_Components::Implementations::Polaris_Intersection_Control_Implementation<M> intersection_control_type;
	typedef Intersection_Control_Components::Implementations::Polaris_Control_Plan_Implementation<M> control_plan_type;
	typedef Intersection_Control_Components::Implementations::Polaris_Phase_Implementation<M> phase_type;
	typedef Intersection_Control_Components::Implementations::Polaris_Phase_Movement_Implementation<M> phase_movement_type;
	typedef Intersection_Control_Components::Implementations::Polaris_Approach_Implementation<M> approach_type;
	typedef Plan_Components::Implementations::Polaris_Plan_Implementation<M> plan_type;
	typedef Movement_Plan_Components::Implementations::Polaris_Movement_Plan_Implementation<M> movement_plan_type;
	typedef Movement_Plan_Components::Implementations::Polaris_Trajectory_Unit_Implementation<M> trajectory_unit_type;
	typedef Network_Skimming_Components::Implementations::Basic_Network_Skimming_Implementation<M> network_skim_type;
	
	// DEMAND AGENT Types
	typedef Person_Components::Implementations::Person_Implementation<M> person_type;
	typedef Person_Components::Implementations::CTRAMP_Person_Planner_Implementation<M, person_type> person_planner_type;
	typedef Person_Components::Implementations::CTRAMP_Activity_Generator_Implementation<M, person_type> activity_generator_type;
	typedef Person_Components::Implementations::ADAPTS_Person_Properties_Implementation<M,person_type> person_properties_type;
	typedef RNG_Components::Implementations::RngStream_Implementation<M> RNG;
	typedef Activity_Components::Implementations::Activity_Plan_Implementation<M,person_type> activity_plan_type;

	
	// POPULATION SYNTHESIS CLASSES
	typedef PopSyn::Implementations::Synthesis_Zone_Implementation<M> zone;
	typedef PopSyn::Implementations::Synthesis_Region_Implementation<M> region;
	typedef PopSyn::Implementations::IPF_Solver_Settings_Implementation<M> IPF_Solver_Settings;
	typedef PopSyn::Implementations::ADAPTS_Population_Unit_Implementation<M> pop_unit;
	typedef PopSyn::Implementations::ADAPTS_Population_Synthesis_Implementation<M> popsyn_solver;
};


ostream* stream_ptr;

int main(int argc,char** argv)
{
	//==================================================================================================================================
	// Start Antares UI
	//----------------------------------------------------------------------------------------------------------------------------------
	#ifdef ANTARES
	START_UI(argc,argv, MasterType);
	#endif


	//==================================================================================================================================
	// NETWORK MODEL STUFF
	//----------------------------------------------------------------------------------------------------------------------------------
	#pragma region Copy from NetworkModel.cpp
	Network_Components::Types::Network_IO_Maps network_io_maps;
	typedef Network_Components::Types::Network_Initialization_Type<Network_Components::Types::ODB_Network,Network_Components::Types::Network_IO_Maps&> Net_IO_Type;

	// OUTPUT OPTIONS
	ofstream log_file("signal_log3.txt");
	ostream output_stream(log_file.rdbuf());
	stream_ptr = &output_stream;	
	//----------------
	//stream_ptr = &cout;

	string output_dir_name = "";

	network_models::network_information::scenario_data_information::ScenarioData scenario_data_for_output;
	network_models::network_information::network_data_information::NetworkData network_data_for_output;
	network_models::network_information::demand_data_information::DemandData demand_data_for_output;
	network_models::network_information::operation_data_information::OperationData operation_data_for_output;
	network_models::initialization(scenario_data_for_output,network_data_for_output,demand_data_for_output,operation_data_for_output);


	//data
	cout << "allocating data structures..." <<endl;	
	define_component_interface(_Scenario_Interface, MasterType::scenario_type, Scenario_Prototype, NULLTYPE);
	
	_Scenario_Interface* scenario=(_Scenario_Interface*)Allocate<typename MasterType::scenario_type>();
	_global_scenario = scenario;

	define_component_interface(_Network_Interface, MasterType::network_type, Network_Prototype, NULLTYPE);
	
	_Network_Interface* network=(_Network_Interface*)Allocate<typename MasterType::network_type>();
	_global_network = network;
	network->template scenario_reference<_Scenario_Interface*>(scenario);
	
	////data input
	cout << "reading scenario data..." <<endl;
	scenario->read_scenario_data<Scenario_Components::Types::ODB_Scenario>();
	cout << "converting scenario data..." << endl;
	scenario->write_scenario_data<NULLTYPE>(scenario_data_for_output);
	network_models::network_information::scenario_data_information::write_scenario_data(scenario_data_for_output);
	cout<<"writing scenario data..."<<endl;
	network_models::network_information::scenario_data_information::write_scenario_data(scenario_data_for_output);	

	cout << "reading network data..." <<endl;	
	network->read_network_data<Net_IO_Type>(network_io_maps);
	cout << "converting network data..." << endl;
	network->write_network_data<Target_Type<NULLTYPE,void,network_models::network_information::network_data_information::NetworkData&>>(network_data_for_output);
	network_models::network_information::network_data_information::write_network_data("", network_data_for_output);
	cout<<"writing network data..."<<endl;
	//network_models::network_information::network_data_information::write_network_data(output_dir_name,network_data_for_output);


	cout << "initializing simulation..." <<endl;	
	network->simulation_initialize<NULLTYPE>();

	//define_component_interface(_Demand_Interface, MasterType::demand_type, Demand_Prototype, NULLTYPE);
	//_Demand_Interface* demand = (_Demand_Interface*)Allocate<typename MasterType::demand_type>();
	//demand->scenario_reference<_Scenario_Interface*>(scenario);
	//demand->network_reference<_Network_Interface*>(network);
	//cout << "reading demand data..." <<endl;
	//demand->read_demand_data<Net_IO_Type>(network_io_maps);
	//cout << "converting demand data..." << endl;
	//demand->write_demand_data<NULLTYPE>(demand_data_for_output);
	//cout<<"writing demand data..."<<endl;
	//network_models::network_information::demand_data_information::write_demand_vehicle(output_dir_name,scenario_data_for_output,demand_data_for_output,network_data_for_output);


	define_component_interface(_Operation_Interface, MasterType::operation_type, Operation_Components::Prototypes::Operation_Prototype, NULLTYPE);
	_Operation_Interface* operation = (_Operation_Interface*)Allocate<typename MasterType::operation_type>();
	operation->network_reference<_Network_Interface*>(network);
	if (scenario->intersection_control_flag<int>() == 1) {
		cout <<"reading operation data..." << endl;
		operation->read_operation_data<Net_IO_Type>(network_io_maps);
	}
	cout <<"converting operation data..." << endl;
	operation->write_operation_data<NULLTYPE>(network_data_for_output, operation_data_for_output);
	cout<<"writing operation data..."<<endl;
	network_models::network_information::operation_data_information::write_operation_data(output_dir_name,scenario_data_for_output,operation_data_for_output,network_data_for_output);

	//network_models::write_data("",scenario_data_for_output,demand_data_for_output,network_data_for_output, operation_data_for_output);

	////initialize network agents	
	cout << "initializing link agents..." <<endl;
	define_container_and_value_interface(_Links_Container_Interface, _Link_Interface, _Network_Interface::get_type_of(links_container), Random_Access_Sequence_Prototype, Link_Prototype, NULLTYPE);
	_Links_Container_Interface::iterator links_itr;

	for(links_itr=network->links_container<_Links_Container_Interface&>().begin();
		links_itr!=network->links_container<_Links_Container_Interface&>().end();
		links_itr++)
	{
		((_Link_Interface*)(*links_itr))->Initialize<NULLTYPE>();
	}

	cout << "initializing intersection agents..." <<endl;
	define_container_and_value_interface(_Intersections_Container_Interface, _Intersection_Interface, _Network_Interface::get_type_of(intersections_container), Random_Access_Sequence_Prototype, Intersection_Prototype, NULLTYPE);
	_Intersections_Container_Interface::iterator intersections_itr;

	for(intersections_itr=network->intersections_container<typename MasterType::network_type::intersections_container_type&>().begin();
		intersections_itr!=network->intersections_container<typename MasterType::network_type::intersections_container_type&>().end();
		intersections_itr++)
	{
		((_Intersection_Interface*)(*intersections_itr))->Initialize<NULLTYPE>();
	}
	cout << "starting sim..." <<endl;
	#pragma endregion


	//==================================================================================================================================
	// Set up graphical display
	//----------------------------------------------------------------------------------------------------------------------------------
	#ifdef ANTARES
	define_container_and_value_interface(_Zones_Container_Interface, _Zone_Interface, typename _Network_Interface::get_type_of(zones_container), Containers::Associative_Container_Prototype, Zone_Components::Prototypes::Zone_Prototype, NULLTYPE);
	_Zones_Container_Interface::iterator zone_itr;
	_Zones_Container_Interface* zone_list = network->zones_container<_Zones_Container_Interface*>();
	typedef Canvas<MasterType::canvas_type,MasterType::graphical_zone_group_type> canvas_itf;
	canvas_itf* canvas_ptr = (canvas_itf*) canvas;
	define_component_interface(graphical_network_interface,typename canvas_itf::get_type_of(graphical_network),Network_Components::Prototypes::Network_Prototype,NULLTYPE);
	//--------------------------------------------------------------------------------------------
	// Graphical zone group display - integrate to graphical network when database is fixed
	typedef Zone_Components::Prototypes::Graphical_Zone_Group<MasterType::graphical_zone_group_type,NULLTYPE> zone_group_interface;
	zone_group_interface* _graphical_zone_group = (zone_group_interface*) Allocate<MasterType::graphical_zone_group_type>();	
	_graphical_zone_group->canvas<canvas_itf*>( (canvas_itf*) canvas );
	// initialize zone static reference to the graphical zone group
	MasterType::zone_type::_graphical_zone_group=(Zone_Components::Prototypes::Graphical_Zone_Group<MasterType::graphical_zone_group_type,MasterType::zone_type>*)_graphical_zone_group;
	_graphical_zone_group->configure_zones_layer<NULLTYPE>();
	// get offsets from graphical network
	_graphical_zone_group->input_offset<Point_2D<MasterType>*>(canvas_ptr->graphical_network<graphical_network_interface*>()->input_offset<Point_2D<MasterType>*>());
	#endif


	//==================================================================================================================================
	// Network Skimming stuff
	//----------------------------------------------------------------------------------------------------------------------------------
	define_component_interface(_network_skim_itf, _Network_Interface::get_type_of(skimming_faculty),Network_Skimming_Components::Prototypes::Network_Skimming_Prototype,NULLTYPE);
	_network_skim_itf* skimmer = (_network_skim_itf*)Allocate<_Network_Interface::get_type_of(skimming_faculty)>();
	skimmer->Initialize<_Network_Interface*>(network);
	network->skimming_faculty<_network_skim_itf*>(skimmer);
	
	//==================================================================================================================================
	// POPSYN stuff
	//----------------------------------------------------------------------------------------------------------------------------------
	ofstream out;
	out.open("full_population_chicag.csv",ios_base::out);
	ofstream marg_out;
	marg_out.open("marginals_and_distributions_chicago.csv",ios_base::out);

	// IPF Solver Settings
	define_component_interface(solver_itf,MasterType::IPF_Solver_Settings,PopSyn::Prototypes::Solver_Settings_Prototype,NULLTYPE);
	solver_itf* solver = (solver_itf*)Allocate<MasterType::IPF_Solver_Settings>();
	// Solver settings - IPF tolerance, Percentage of population to synthesis, maximum ipf and selection iterations
	solver->Initialize<Target_Type<NULLTYPE,void,double,int>>(0.05,1.0,100);

	define_component_interface(popsyn_itf,MasterType::popsyn_solver,PopSyn::Prototypes::Population_Synthesizer_Prototype,NULLTYPE);
	popsyn_itf* popsyn = (popsyn_itf*)Allocate<MasterType::popsyn_solver>();
	popsyn->write_output_flag<bool>(true);
	popsyn->linker_file_path<string>(string("linker_file.txt"));
	popsyn->Solution_Settings<solver_itf*>(solver);
	popsyn->Output_Stream<ostream&>(out);
	popsyn->Marginal_Output_Stream<ostream&>(marg_out);
	popsyn->network_reference<_Network_Interface*>(network);
	popsyn->scenario_reference<_Scenario_Interface*>(scenario);
	popsyn->Initialize<NULLTYPE>();
	//----------------------------------------------------------------------------------------------------------------------------------

	//==================================================================================================================================
	// Logging of activity generation / scheduling outputs
	//----------------------------------------------------------------------------------------------------------------------------------
	stringstream logfilename;
	MasterType::person_planner_type::_write_activity_files = false;
	for (int i = 0; i < _num_threads; ++i)
	{
		logfilename.str("");
		logfilename << "generated_acts_" << i << ".csv";
		MasterType::person_planner_type::logs[i].open(logfilename.str());
		MasterType::person_planner_type::logs[i] << "PERID,DEPART,ORIG,DEST,EST_TTIME"<<endl;
	}
	//----------------------------------------------------------------------------------------------------------------------------------
	
	try
	{
	START();
	}
	catch (std::exception ex)
	{
		cout << ex.what();
	}

	cout << "Finished!" << endl;
	system("PAUSE");
}
#endif

//============================================
// USE THIS FOR RUNNING ON LINUX
//--------------------------------------------
#ifndef DBIO
#define WINDOWS
#include "Application_Includes.h"
#include "../File_IO/network_models.h"
struct MasterType
{
	typedef MasterType M;

#ifdef ANTARES
	typedef Conductor_Implementation<M> conductor_type;
	typedef Control_Panel_Implementation<M> control_panel_type;
	typedef Time_Panel_Implementation<M> time_panel_type;
	typedef Information_Panel_Implementation<M> information_panel_type;
	typedef Canvas_Implementation<M> canvas_type;
	typedef Antares_Layer_Implementation<M> antares_layer_type;
	typedef Layer_Options_Implementation<M> layer_options_type;
	typedef Attributes_Panel_Implementation<M> attributes_panel_type;
	typedef Control_Dialog_Implementation<M> control_dialog_type;

	typedef Graphical_Network_Implementation<M> graphical_network_type;
	typedef Graphical_Link_Implementation<M> graphical_link_type;
	typedef Graphical_Intersection_Implementation<M> graphical_intersection_type;
	typedef Vehicle_Components::Implementations::Graphical_Vehicle_Implementation<M> vehicle_type;	
	typedef Zone_Components::Implementations::Graphical_Zone_Implementation<M> zone_type;
	//typedef Zone_Components::Implementations::Polaris_Zone_Implementation<M> zone_type;
	typedef Zone_Components::Implementations::Graphical_Zone_Group_Implementation<M> graphical_zone_group_type;
#else
	typedef Vehicle_Components::Implementations::Polaris_Vehicle_Implementation<M> vehicle_type;
	typedef Zone_Components::Implementations::Polaris_Zone_Implementation<M> zone_type;
#endif

	//==============================================================================================
	// Network Types
	typedef Scenario_Components::Implementations::Polaris_Scenario_Implementation<M> scenario_type;
	typedef Network_Components::Implementations::Integrated_Polaris_Network_Implementation<M> network_type;
	typedef Intersection_Components::Implementations::Polaris_Intersection_Implementation<M> intersection_type;
	typedef Turn_Movement_Components::Implementations::Polaris_Movement_Implementation<M> movement_type;
	typedef Link_Components::Implementations::Polaris_Link_Implementation<M> link_type;
	typedef Turn_Movement_Components::Implementations::Polaris_Movement_Implementation<M> turn_movement_type;
	typedef Routing_Components::Implementations::Routable_Network_Implementation<M> routable_network_type;
	typedef Routing_Components::Implementations::Polaris_Routing_Implementation<M> routing_type;
	typedef Routing_Components::Implementations::Polaris_Skim_Routing_Implementation<M> skim_routing_type;
	typedef Intersection_Components::Implementations::Routable_Intersection_Implementation<M> routable_intersection_type;
	typedef Link_Components::Implementations::Routable_Link_Implementation<M> routable_link_type;
	typedef Activity_Location_Components::Implementations::Polaris_Activity_Location_Implementation<M> activity_location_type;
	typedef Traveler_Components::Implementations::Polaris_Traveler_Implementation<M> traveler_type;
	typedef Intersection_Components::Implementations::Polaris_Inbound_Outbound_Movements_Implementation<M> inbound_outbound_movements_type;
	typedef Intersection_Components::Implementations::Polaris_Outbound_Inbound_Movements_Implementation<M> outbound_inbound_movements_type;
	typedef Intersection_Components::Implementations::Routable_Inbound_Outbound_Movements_Implementation<M> routable_inbound_outbound_movements_type;
	typedef Intersection_Components::Implementations::Routable_Outbound_Inbound_Movements_Implementation<M> routable_outbound_inbound_movements_type;
	typedef Intersection_Components::Implementations::Routable_Movement_Implementation<M> routable_movement_type;
	typedef Operation_Components::Implementations::Polaris_Operation_Implementation<M> operation_type;
	typedef Intersection_Control_Components::Implementations::Polaris_Intersection_Control_Implementation<M> intersection_control_type;
	typedef Intersection_Control_Components::Implementations::Polaris_Control_Plan_Implementation<M> control_plan_type;
	typedef Intersection_Control_Components::Implementations::Polaris_Phase_Implementation<M> phase_type;
	typedef Intersection_Control_Components::Implementations::Polaris_Phase_Movement_Implementation<M> phase_movement_type;
	typedef Intersection_Control_Components::Implementations::Polaris_Approach_Implementation<M> approach_type;
	typedef Plan_Components::Implementations::Polaris_Plan_Implementation<M> plan_type;
	typedef Movement_Plan_Components::Implementations::Polaris_Movement_Plan_Implementation<M> movement_plan_type;
	typedef Movement_Plan_Components::Implementations::Polaris_Trajectory_Unit_Implementation<M> trajectory_unit_type;
	typedef Network_Skimming_Components::Implementations::Basic_Network_Skimming_Implementation<M> network_skim_type;
	
	// DEMAND AGENT Types
	typedef Person_Components::Implementations::Person_Implementation<M> person_type;
	typedef Person_Components::Implementations::CTRAMP_Person_Planner_Implementation<M, person_type> person_planner_type;
	typedef Person_Components::Implementations::CTRAMP_Activity_Generator_Implementation<M, person_type> activity_generator_type;
	typedef Person_Components::Implementations::ADAPTS_Person_Properties_Implementation<M,person_type> person_properties_type;
	typedef RNG_Components::Implementations::RngStream_Implementation<M> RNG;
	typedef Activity_Components::Implementations::Activity_Plan_Implementation<M,person_type> activity_plan_type;

	
	// POPULATION SYNTHESIS CLASSES
	typedef PopSyn::Implementations::Synthesis_Zone_Implementation<M> zone;
	typedef PopSyn::Implementations::Synthesis_Region_Implementation<M> region;
	typedef PopSyn::Implementations::IPF_Solver_Settings_Implementation<M> IPF_Solver_Settings;
	typedef PopSyn::Implementations::ADAPTS_Population_Unit_Implementation<M> pop_unit;
	typedef PopSyn::Implementations::ADAPTS_Population_Synthesis_Implementation<M> popsyn_solver;
};

ostream* stream_ptr;




int main(int argc,char** argv)
{
	cout << endl << "Time unit: " << sizeof(Basic_Units::Implementations::Time_Implementation<MasterType>);
	cout << endl << "RNG: " << sizeof(MasterType::RNG);
	cout << endl << "Pop unit: " << sizeof(MasterType::pop_unit);
	cout << endl << "Pop region: " << sizeof(MasterType::region);
	cout << endl << "Pop zone: " << sizeof(MasterType::zone);
	cout << endl << "Person: " << sizeof(MasterType::person_type);
	cout << endl << "Person - planner: " << sizeof(MasterType::person_planner_type);
	cout << endl << "Person - properties: " << sizeof(MasterType::person_properties_type);
	cout << endl << "Person - router: " << sizeof(MasterType::routing_type);
	cout << endl << "Person - vehicle: " << sizeof(MasterType::vehicle_type);
	cout << endl << "Movement plans: " << sizeof(MasterType::movement_plan_type);
	cout << endl << "plan hashmaps: " << sizeof(hash_multimap<long,MasterType::activity_plan_type*>);
	cout << endl << "plan vectors: " << sizeof(vector<MasterType::activity_plan_type*>);

#ifdef ANTARES
	START_UI(argc,argv, MasterType);
#endif

#pragma region COPY FROM NETWORKMODEL.CPP
	Network_Components::Types::Network_IO_Maps network_io_maps;
	typedef Network_Components::Types::Network_Initialization_Type<Network_Components::Types::File_Network,network_models::network_information::network_data_information::NetworkData&> Net_IO_Type;

	//reading from files
	network_models::network_information::scenario_data_information::ScenarioData scenario_data;
	network_models::network_information::network_data_information::NetworkData network_data;
	network_models::network_information::demand_data_information::DemandData demand_data;
	network_models::network_information::operation_data_information::OperationData operation_data;
	network_models::initialization(scenario_data,network_data,demand_data,operation_data);
	scenario_data.input_dir_name = "";
	scenario_data.output_dir_name = "";
	network_models::read_data(scenario_data,network_data,demand_data,operation_data);

	//converting between k and p
	network_models::network_information::scenario_data_information::ScenarioData scenario_data_for_output;
	network_models::network_information::network_data_information::NetworkData network_data_for_output;
	network_models::network_information::demand_data_information::DemandData demand_data_for_output;
	network_models::network_information::operation_data_information::OperationData operation_data_for_output;
	network_models::initialization(scenario_data_for_output,network_data_for_output,demand_data_for_output,operation_data_for_output);

	cout << "reading input data..." <<endl;	
	define_component_interface(_Scenario_Interface, MasterType::scenario_type, Scenario_Prototype, NULLTYPE);
	_Scenario_Interface* scenario=(_Scenario_Interface*)Allocate<typename MasterType::scenario_type>();
	_global_scenario = scenario;
	scenario->read_scenario_data<Scenario_Components::Types::File_Scenario>(scenario_data);
	//scenario->write_scenario_data<NULLTYPE>(scenario_data_for_output);

	define_component_interface(_Network_Interface, MasterType::network_type, Network_Prototype, NULLTYPE);
	_Network_Interface* network = (_Network_Interface*)Allocate<typename MasterType::network_type>();
	_global_network = network;
	network->scenario_reference<_Scenario_Interface*>(scenario);
	network->read_network_data<Net_IO_Type>(network_data);
	//network->write_network_data<NULLTYPE>(network_data_for_output);


	define_component_interface(_Operation_Interface, MasterType::operation_type, Operation_Components::Prototypes::Operation_Prototype, NULLTYPE);
	_Operation_Interface* operation = (_Operation_Interface*)Allocate<typename MasterType::operation_type>();
	operation->network_reference<_Network_Interface*>(network);
	operation->read_operation_data<Operation_Components::Types::File_Operation>(scenario_data, network_data, operation_data);
	//operation->write_operation_data<NULLTYPE>(network_data_for_output, operation_data_for_output);
	
	//output data
	//network_models::write_data("",scenario_data_for_output,demand_data_for_output,network_data_for_output, operation_data);


	network->simulation_initialize<NULLTYPE>();
	cout << "world started..." << endl;
	////initialize network agents
	
	define_container_and_value_interface(_Links_Container_Interface, _Link_Interface, _Network_Interface::get_type_of(links_container), Random_Access_Sequence_Prototype, Link_Prototype, NULLTYPE);
	_Links_Container_Interface::iterator links_itr;

	for(links_itr=network->links_container<_Links_Container_Interface&>().begin();
		links_itr!=network->links_container<_Links_Container_Interface&>().end();
		links_itr++)
	{
		((_Link_Interface*)(*links_itr))->Initialize<NULLTYPE>();
	}

	define_container_and_value_interface(_Intersections_Container_Interface, _Intersection_Interface, _Network_Interface::get_type_of(intersections_container), Random_Access_Sequence_Prototype, Intersection_Prototype, NULLTYPE);
	_Intersections_Container_Interface::iterator intersections_itr;

	for(intersections_itr=network->intersections_container<typename MasterType::network_type::intersections_container_type&>().begin();
		intersections_itr!=network->intersections_container<typename MasterType::network_type::intersections_container_type&>().end();
		intersections_itr++)
	{

		((_Intersection_Interface*)(*intersections_itr))->Initialize<NULLTYPE>();		
	}

#pragma endregion

	//==================================================================================================================================
	// Set up graphical display
	//----------------------------------------------------------------------------------------------------------------------------------
	#ifdef ANTARES
	define_container_and_value_interface(_Zones_Container_Interface, _Zone_Interface, typename _Network_Interface::get_type_of(zones_container), Containers::Associative_Container_Prototype, Zone_Components::Prototypes::Zone_Prototype, NULLTYPE);
	_Zones_Container_Interface::iterator zone_itr;
	_Zones_Container_Interface* zone_list = network->zones_container<_Zones_Container_Interface*>();
	typedef Canvas<MasterType::canvas_type,MasterType::graphical_zone_group_type> canvas_itf;
	canvas_itf* canvas_ptr = (canvas_itf*) canvas;
	define_component_interface(graphical_network_interface,typename canvas_itf::get_type_of(graphical_network),Network_Components::Prototypes::Network_Prototype,NULLTYPE);
	//--------------------------------------------------------------------------------------------
	// Graphical zone group display - integrate to graphical network when database is fixed
	typedef Zone_Components::Prototypes::Graphical_Zone_Group<MasterType::graphical_zone_group_type,NULLTYPE> zone_group_interface;
	zone_group_interface* _graphical_zone_group = (zone_group_interface*) Allocate<MasterType::graphical_zone_group_type>();	
	_graphical_zone_group->canvas<canvas_itf*>( (canvas_itf*) canvas );
	// initialize zone static reference to the graphical zone group
	MasterType::zone_type::_graphical_zone_group=(Zone_Components::Prototypes::Graphical_Zone_Group<MasterType::graphical_zone_group_type,MasterType::zone_type>*)_graphical_zone_group;
	_graphical_zone_group->configure_zones_layer<NULLTYPE>();
	// get offsets from graphical network
	_graphical_zone_group->input_offset<Point_2D<MasterType>*>(canvas_ptr->graphical_network<graphical_network_interface*>()->input_offset<Point_2D<MasterType>*>());
	#endif


	//==================================================================================================================================
	// Network Skimming stuff
	//----------------------------------------------------------------------------------------------------------------------------------
	define_component_interface(_network_skim_itf, _Network_Interface::get_type_of(skimming_faculty),Network_Skimming_Components::Prototypes::Network_Skimming_Prototype,NULLTYPE);
	_network_skim_itf* skimmer = (_network_skim_itf*)Allocate<_Network_Interface::get_type_of(skimming_faculty)>();

	skimmer->Initialize<_Network_Interface*>(network);
	network->skimming_faculty<_network_skim_itf*>(skimmer);
	
	//==================================================================================================================================
	// POPSYN stuff
	//----------------------------------------------------------------------------------------------------------------------------------
	ofstream out;
	out.open("full_population_chicag.csv",ios_base::out);
	ofstream marg_out;
	marg_out.open("marginals_and_distributions_chicago.csv",ios_base::out);

	// IPF Solver Settings
	define_component_interface(solver_itf,MasterType::IPF_Solver_Settings,PopSyn::Prototypes::Solver_Settings_Prototype,NULLTYPE);
	solver_itf* solver = (solver_itf*)Allocate<MasterType::IPF_Solver_Settings>();
	// Solver settings - IPF tolerance, Percentage of population to synthesis, maximum ipf and selection iterations
	solver->Initialize<Target_Type<NULLTYPE,void,double,int>>(0.05,1.0,100);

	define_component_interface(popsyn_itf,MasterType::popsyn_solver,PopSyn::Prototypes::Population_Synthesizer_Prototype,NULLTYPE);
	popsyn_itf* popsyn = (popsyn_itf*)Allocate<MasterType::popsyn_solver>();
	popsyn->write_output_flag<bool>(true);
	popsyn->linker_file_path<string>(string("linker_file.txt"));
	popsyn->Solution_Settings<solver_itf*>(solver);
	popsyn->Output_Stream<ostream&>(out);
	popsyn->Marginal_Output_Stream<ostream&>(marg_out);
	popsyn->network_reference<_Network_Interface*>(network);
	popsyn->scenario_reference<_Scenario_Interface*>(scenario);
	popsyn->Initialize<NULLTYPE>();
	//----------------------------------------------------------------------------------------------------------------------------------

	//==================================================================================================================================
	// Logging of activity generation / scheduling outputs
	//----------------------------------------------------------------------------------------------------------------------------------
	stringstream logfilename;
	MasterType::person_planner_type::_write_activity_files = false;
	for (int i = 0; i < _num_threads; ++i)
	{
		logfilename.str("");
		logfilename << "generated_acts_" << i << ".csv";
		MasterType::person_planner_type::logs[i].open(logfilename.str());
		MasterType::person_planner_type::logs[i] << "PERID,DEPART,ORIG,DEST,EST_TTIME"<<endl;
	}
	//----------------------------------------------------------------------------------------------------------------------------------
	
	try
	{
	START();
	}
	catch (std::exception ex)
	{
		cout << ex.what();
	}

	cout << "Finished!" << endl;
	system("PAUSE");
}
#endif

#endif