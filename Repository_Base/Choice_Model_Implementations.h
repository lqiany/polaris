#pragma once

#include "Choice_Model_Prototypes.h"
#include <random>


namespace Choice_Model_Components
{
	//==================================================================================================================
	/// Namespace of Base classes related to the Interface(s) from the component namespace.
	/// The bases are combined with the interfaces to create components by adding data.
	//------------------------------------------------------------------------------------------------------------------
	namespace Implementations
	{
		//------------------------------------------------------------------------------------------------------------------
		/// Example of a Base class.  There can be many versions of a base class for a given Interface
		/// The template parameter MasterType contains information about all types defined int the current application, and
		/// be used to reference other types in the program
		//------------------------------------------------------------------------------------------------------------------
		implementation struct Choice_Option_Implementation
		{
			member_data(float,probability,requires(TargetType,is_arithmetic),requires(TargetType,is_arithmetic));

			// Component pointer for the mode choice data
			member_data(void*, choice_data_interface, requires(TargetType,is_pointer), requires(TargetType,is_pointer));
		};

		implementation struct Utility_Choice_Option_Implementation : Choice_Option_Implementation<MasterType>
		{
			typedef true_type Utility_Based_Choice;
			member_data(float,utility,requires(TargetType,is_arithmetic),requires(TargetType,is_arithmetic));
		};

		//------------------------------------------------------------------------------------------------------------------
		/// Example of a Base class.  There can be many versions of a base class for a given Interface
		/// The template parameter MasterType contains information about all types defined int the current application, and
		/// be used to reference other types in the program
		//------------------------------------------------------------------------------------------------------------------
		implementation struct Choice_Model_Implementation
		{
			typedef Prototypes::Choice_Option_Prototype<typename MasterType::CHOICE_OPTION_TYPE,NULLTYPE>* choice_itf;

			//==========================================================================================
			/// Handler for a general Initializer dispatched from an Interface
			feature_implementation void Initialize(call_requires_2(ComponentType,CallerType, Is_Same_Entity))
			{
				cout <<endl<< "Initializing: Choice Model Base."<<endl;
				_rand_dist = uniform_real<float>(0.0f,1.0f);
			}
			/// Error Handler for general Initializers.  In this case, stops the initializer from being called directly
			feature_implementation void Initialize(call_requires_2(ComponentType,CallerType, !Is_Same_Entity))
			{
				assert_requirements_2(ComponentType,CallerType,Is_Same_Entity,"ComponentType and CallerType are not the same entity");
			}

			//==========================================================================================
			/// Choice Model random probability generator (uses random number generator defined below)
			uniform_real<float> _rand_dist; // create the data
			tag_getter(random_probability_value);
			feature_implementation TargetType random_probability_value(call_requires(TargetType, is_arithmetic))
			{
				return (TargetType)_rand_dist(*_generator);
			}

			//==========================================================================================
			/// Choice Model random number generator
			typename MasterType::RANDOM_GENERATOR_TYPE* _generator;
			tag_setter(generator);
			feature_implementation void generator(TargetType set_value, call_requirements(requires(TargetType, is_pointer) && requires_2(TargetType, typename MasterType::RANDOM_GENERATOR_TYPE*, is_convertible)))
			{
				_generator = (MasterType::RANDOM_GENERATOR_TYPE*)set_value;
			}

			//==========================================================================================
			/// Choice Options		
			member_container_basic(vector<typename MasterType::CHOICE_OPTION_TYPE*>,choice_options);
		};

		implementation struct Utility_Choice_Model_Implementation : public Choice_Model_Implementation<MasterType>
		{
			// Define the type of choice to make
			typedef true_type Probabilistic_Choice;
			typedef true_type Utility_Based_Choice;

			//==========================================================================================
			/// Handler for a general Initializer dispatched from an Interface
			feature_implementation void Initialize(call_requires_2(ComponentType,CallerType, Is_Same_Entity))
			{
				cout <<endl<< "Initializing: Utility Choice Model Base."<<endl;
				//Choice_Model_Base<MasterType>::Initialize<ComponentType,CallerType,TargetType>();
			}
			/// Error Handler for general Initializers.  In this case, stops the initializer from being called directly
			feature_implementation void Initialize(call_requires_2(ComponentType,CallerType, !Is_Same_Entity))
			{
				assert_requirements_2(ComponentType,CallerType,Is_Same_Entity,"ComponentType and CallerType are not the same entity");
			}

			//==========================================================================================
			/// Member Variables
			member_container_basic(vector<typename MasterType::UTILITY_CHOICE_OPTION_TYPE*>,choice_options);
		};

		implementation struct MNL_Choice_Model_Implementation : public Utility_Choice_Model_Implementation<MasterType>
		{
			// Define the type of choice to make
			typedef true_type MNL_Model;
		};
	}


	//==================================================================================================================
	/// Components namespace.  This is where the objects created using the interface(s) / base(s) reside
	//------------------------------------------------------------------------------------------------------------------
	namespace Components
	{
		//==========================================================================================
		/// The struct below creates the polaris component comprised of the interface/base and any parent types.
		/// The polaris component can be accessed through the "type" member of the struct;
		template<typename MasterType>
		struct Choice_Option
		{
			typedef Polaris_Component<Implementations::Choice_Option_Implementation,MasterType> type;
		};
		template<typename MasterType>
		struct Utility_Choice_Option
		{
			typedef Polaris_Component<Implementations::Utility_Choice_Option_Implementation,MasterType> type;
		};
		template<typename MasterType>
		struct Choice_Model
		{
			typedef Polaris_Component<Implementations::Choice_Model_Implementation,MasterType> type;
		};
		template<typename MasterType>
		struct Utility_Choice_Model
		{
			typedef Polaris_Component<Implementations::Utility_Choice_Model_Implementation,MasterType> type;
		};
		template<typename MasterType>
		struct MNL_Choice_Model
		{
			typedef Polaris_Component<Implementations::MNL_Choice_Model_Implementation,MasterType> type;
		};
	}
}