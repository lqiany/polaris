
#pragma once

#include "Core.h"
#include "Household_Properties_Prototype.h"
#include "Person_Properties_Prototype.h"

namespace Activity_Simulator
{
	namespace Activity_Concepts
	{
		concept struct Is_Activity_Plan_Prototype
		{
			check_template_method_name(has_id,Component_Type::template Activity_Plan_ID);
			define_default_check(has_id);
		};

		concept struct Is_Activity_Plan
		{
			check_template_method_name(has_id,template Activity_Plan_ID);
			check_concept(is_prototype, Is_Activity_Plan_Prototype, T, V);
			define_default_check(has_id || is_prototype);
		};
	}

	namespace General_Concepts
	{
		concept struct Has_Initialize
		{
			check_template_method_name(Has_Initialize_Defined,template Initialize);
			define_default_check(Has_Initialize_Defined);
		};
	}

	namespace Household_Concepts
	{
		concept struct Is_Household
		{
			check_template_method_name(Has_Initialize_Defined,template Initialize);
			check_template_method_name(Has_Properties_Defined,template Properties);
			check_template_method_name(Has_Planner_Defined,template Planning_Faculty);
			define_default_check(Has_Initialize_Defined && Has_Properties_Defined && Has_Planner_Defined);
		};

		concept struct Is_Census_ACS_Compliant
		{
			// component versions
			check_data_member_type(has_gender,Gender, Household_Components::Types::HHTYPE);
			check_template_method_name(has_race,template Race);
			check_data_member_type(has_employment_status,Employment_Status, Person_Components::Types::EMPLOYMENT_STATUS);

			check_template_method_name(has_work_travel_time,template Journey_To_Work_Travel_Time);
			check_data_member_type(has_work_travel_mode,Journey_To_Work_Mode, Person_Components::Types::JOURNEY_TO_WORK_MODE);
			check_template_method_name(has_work_arrival_time,template Journey_To_Work_Arrival_Time);

			// prototype versions
			check_data_member_type(has_gender_prototype,Component_Type::Gender, Person_Components::Types::GENDER);
			check_template_method_name(has_race_prototype,Component_Type::template Race);
			check_data_member_type(has_employment_status_prototype,Component_Type::Employment_Status, Person_Components::Types::EMPLOYMENT_STATUS);

			check_template_method_name(has_work_travel_time_prototype,Component_Type::template Journey_To_Work_Travel_Time);
			check_data_member_type(has_work_travel_mode_prototype,Component_Type::Journey_To_Work_Mode, Person_Components::Types::JOURNEY_TO_WORK_MODE);
			check_template_method_name(has_work_arrival_time_prototype,Component_Type::template Journey_To_Work_Arrival_Time);

			// concept checks
			define_sub_check(has_demographics, (has_gender && has_race && has_employment_status) || (has_gender_prototype && has_race_prototype && has_employment_status_prototype));
			define_sub_check(has_journey_to_work_info, (has_work_travel_time && has_work_travel_mode && has_work_arrival_time) || (has_work_travel_time_prototype && has_work_travel_mode_prototype && has_work_arrival_time_prototype));
			define_default_check(has_demographics && has_journey_to_work_info);
		};

		concept struct Characteristics_exists
		{
			check_method_name(has_Characteristics, Characteristics);
			define_default_check(has_Characteristics);
		};

	}

	namespace Person_Concepts
	{
		concept struct Is_Person_Mover_Prototype
		{
			check_template_method_name(Has_Parent_Person,ComponentType::template Parent_Person);
			check_template_method_name(Has_Movement, ComponentType::template Movement);
			define_default_check(Has_Parent_Person && Has_Movement);
		};

		concept struct Is_Person_Mover
		{
			check_template_method_name(Has_Parent_Person,template Parent_Person);
			check_template_method_name(Has_Movement,template Movement);
			check_concept(is_prototype, Is_Person_Mover_Prototype, T, V);
			define_default_check(is_prototype || (Has_Parent_Person && Has_Movement) );
		};

		concept struct Is_Person_Planner_Prototype
		{
			check_template_method_name(Has_Parent_Person,ComponentType::template Parent_Person);
			check_template_method_name(Has_Movement_Plans_Container, ComponentType::template Movement_Plans_Container);
			check_template_method_name(Has_Activity_Plans_Container, ComponentType::template Activity_Container);
			define_default_check(Has_Parent_Person && Has_Movement_Plans_Container && Has_Activity_Plans_Container);
		};

		concept struct Is_Person_Planner_Component
		{
			check_template_method_name(Has_Parent_Person,template Parent_Person);
			check_template_method_name(Has_Movement_Plans_Container, template Movement_Plans_Container);
			check_template_method_name(Has_Activity_Plans_Container, template Activity_Container);
			define_default_check(Has_Parent_Person && Has_Movement_Plans_Container && Has_Activity_Plans_Container);
		};

		concept struct Is_Person_Planner
		{
			check_concept(is_prototype, Is_Person_Planner_Prototype, T, V);
			check_concept(is_component, Is_Person_Planner_Component, T, V);
			define_default_check(is_prototype || is_component);
		};

		concept struct Is_Census_ACS_Compliant
		{
			// component versions
			check_data_member_type(has_gender,Gender, Person_Components::Types::GENDER);
			check_template_method_name(has_race,template Race);
			check_data_member_type(has_employment_status,Employment_Status, Person_Components::Types::EMPLOYMENT_STATUS);

			check_template_method_name(has_work_travel_time,template Journey_To_Work_Travel_Time);
			check_data_member_type(has_work_travel_mode,Journey_To_Work_Mode, Person_Components::Types::JOURNEY_TO_WORK_MODE);
			check_template_method_name(has_work_arrival_time,template Journey_To_Work_Arrival_Time);

			// prototype versions
			check_data_member_type(has_gender_prototype,Component_Type::Gender, Person_Components::Types::GENDER);
			check_template_method_name(has_race_prototype,Component_Type::template Race);
			check_data_member_type(has_employment_status_prototype,Component_Type::Employment_Status, Person_Components::Types::EMPLOYMENT_STATUS);

			check_template_method_name(has_work_travel_time_prototype,Component_Type::template Journey_To_Work_Travel_Time);
			check_data_member_type(has_work_travel_mode_prototype,Component_Type::Journey_To_Work_Mode, Person_Components::Types::JOURNEY_TO_WORK_MODE);
			check_template_method_name(has_work_arrival_time_prototype,Component_Type::template Journey_To_Work_Arrival_Time);

			// concept checks
			define_sub_check(has_demographics, (has_gender && has_race && has_employment_status) || (has_gender_prototype && has_race_prototype && has_employment_status_prototype));
			define_sub_check(has_journey_to_work_info, (has_work_travel_time && has_work_travel_mode && has_work_arrival_time) || (has_work_travel_time_prototype && has_work_travel_mode_prototype && has_work_arrival_time_prototype));
			define_default_check(has_demographics && has_journey_to_work_info);
		};

		concept struct Is_Person
		{
			check_template_method_name(Has_Initialize_Defined,template Initialize);
			check_template_method_name(Has_Properties_Defined,template Properties);
			check_template_method_name(Has_Planner_Defined,template Planning_Faculty);
			define_default_check(Has_Initialize_Defined && Has_Properties_Defined && Has_Planner_Defined);
		};

		concept struct Is_Transims_Person
		{
			check_concept(Is_Person_Check, Is_Person, T, V);
			check_typedef_type(Is_Transims_Check,Is_Transims,true_type);
			define_default_check(Is_Person_Check && Is_Transims_Check);
		};

		concept struct Is_CTRAMP_Person
		{
			check_concept(Is_Person_Check, Is_Person, T, V);
			check_typedef_type(Is_CTRAMP_Check,Is_Transims,true_type);
			define_default_check(Is_Person_Check && Is_CTRAMP_Check);
		};

	}

}