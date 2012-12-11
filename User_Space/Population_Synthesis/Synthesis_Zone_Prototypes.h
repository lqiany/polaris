#pragma once

#include "User_Space\User_Space_includes.h"
#include "User_Space\Population_Synthesis\Population_Unit_Implementations.h"

using namespace std;

//---------------------------------------------------------
//	SUMMARY FILE CLASS
//---------------------------------------------------------
namespace PopSyn
{
	namespace Concepts
	{
		concept struct Is_IPF_Capable
		{
			check_typename_defined(Has_Value_Type, Value_Type);
			check_typename_state(Has_Marginals, Has_Marginals_In_Distribution, true_type);
			define_default_check(Has_Marginals && Has_Value_Type);
		};

		concept struct Is_Probabilistic_Selection
		{
			check_typename_defined(Has_Value_Type, Value_Type);
			check_typename_state(Has_Probabilistic_Selection_Defined, Probabilstic_Selection_Type, true_type);
			define_default_check(Has_Probabilistic_Selection_Defined && Has_Value_Type);
		};

		concept struct Is_Loss_Function_Selection
		{
			check_typename_defined(Has_Value_Type, Value_Type);
			check_typename_state(Has_Loss_Function_Selection_Defined, Loss_Function_Selection_Type, true_type);
			define_default_check(Has_Probabilistic_Selection_Defined && Has_Value_Type);
		};


		concept struct Is_IPF_Solver_Setting
		{
			check_typed_member(Has_IPF_Tolerance_d, IPF_Tolerance, double);
			check_typed_member(Has_IPF_Tolerance_f, IPF_Tolerance, float);
			check_typed_member(Has_Max_Iterations_int, Max_Iterations, int);
			check_typed_member(Has_Max_Iterations_uint, Max_Iterations, unsigned int);
			check_typed_member(Has_Max_Iterations_long, Max_Iterations, long);
			check_typed_member(Has_Max_Iterations_ulong, Max_Iterations, long);

			define_default_check((Has_IPF_Tolerance_d || Has_IPF_Tolerance_f) && (Has_Max_Iterations_int || Has_Max_Iterations_uint || Has_Max_Iterations_long || Has_Max_Iterations_ulong));
		};
	}

	namespace Prototypes
	{
		prototype struct Solver_Settings_Prototype
		{
			tag_as_prototype;

			feature_accessor(Tolerance,check(ReturnValueType, is_arithmetic),check(SetValueType, is_arithmetic));
			feature_accessor(Iterations,check(ReturnValueType, is_arithmetic),check(SetValueType, is_arithmetic));
		};

		prototype struct Synthesis_Zone_Prototype
		{
			tag_as_prototype;

			feature_prototype void Initialize()
			{
				this_component()->Initialize<ComponentType,CallerType,TargetType>();
			}

			feature_prototype void Fit_Joint_Distribution_To_Marginal_Data(requires(check(ComponentType,Concepts::Is_IPF_Capable)))
			{
				// Get the solution settings
				define_component_interface(solution_settings_itf,get_type_of(Solver_Settings),Prototypes::Solver_Settings_Prototype,NULLTYPE);
				solution_settings_itf& settings = this->Solver_Settings<solution_settings_itf&>();

				// IPF version of fitting the joint distribution to marginal distribution
				typedef get_type_of(Target_Joint_Distribution)::unqualified_value_type value_type;
				define_simple_container_interface(mway_itf, get_type_of(Target_Joint_Distribution),Multidimensional_Random_Access_Array_Prototype,value_type,NULLTYPE);
				define_simple_container_interface(marg_itf, get_type_of(Target_Marginal_Distribution),Multidimensional_Random_Access_Array_Prototype,value_type,NULLTYPE);

				mway_itf::iterator itr;
				marg_itf::iterator marg_itr;
				typedef marg_itf::index_type index;


				// get the distribution
				mway_itf& mway = this->Target_Joint_Distribution<mway_itf&>();
				mway_itf::const_index_type dimensions = mway.dimensions();
				mway_itf::size_type num_dim = (mway_itf::size_type)(dimensions.size());

				// get the marginals
				marg_itf& marg = this->Target_Marginal_Distribution<marg_itf&>();
				
				// Main Execution loop - loop over each dimension, and each index within each dimensions and fit to the marginal
				value_type max_error = (value_type)INT_MAX;
				int iterations = 0;
				while (iterations < settings.Iterations<uint>() && max_error > settings.Tolerance<value_type>())
				{
					// 0. reset max error to 0, then store the highest value for current iteration
					max_error = 0;

					// 1.  Loop over each dimension
					for (int d = 0; d < num_dim; ++d)
					{
						// 2. loop over each index in the marginal for each dimension
						for (int i = 0; i < dimensions[d]; ++i)
						{
							// 3. get the current sum in the distribution for the current marginal
							value_type sum = 0;
							for (itr = mway.begin(d,i); itr != mway.end(); ++itr) sum += *itr;

							// 4. calculate the error against the known marginal value
							value_type marg_val = marg[index(d,i)];
							value_type temp_err = (marg_val != 0 ) ? (sum / marg_val) : 1;
							if (abs((value_type)(temp_err - 1.0)) > max_error) max_error = abs((value_type)(temp_err - 1.0));

							// 5. update the values in the distribution by the error factor
							for (itr = mway.begin(d,i); itr != mway.end(); ++itr) *itr = *itr / temp_err;
						}
					}

					iterations++;
				}
						
			}

			feature_prototype void Fit_Joint_Distribution_To_Marginal_Data(requires(check(ComponentType,!Concepts::Is_IPF_Capable)))
			{
				assert_check(ComponentType,Concepts::Is_IPF_Capable,"Not IPF Capable");
				assert_sub_check(ComponentType,Concepts::Is_IPF_Capable,Has_Joint_Distribution_Double,"doesn't have a double joint distribution");
				assert_sub_check(ComponentType,Concepts::Is_IPF_Capable,Has_Joint_Distribution_Float,"doesn't have a float joint distribution");
				assert_sub_check(ComponentType,Concepts::Is_IPF_Capable,Has_Value_Type,"doesn't have a value_type");
				assert_sub_check(ComponentType,Concepts::Is_IPF_Capable,Has_Marginals,"doesn't have marginals");
				
			}

			feature_prototype void Select_Synthetic_Population_Units(requires(check(ComponentType, Concepts::Is_Probabilistic_Selection)))
			{

			}

			feature_accessor(Target_Joint_Distribution,none,none);

			feature_accessor(Target_Marginal_Distribution,none,none);

			feature_accessor(Sample_Data,none,none);

			feature_accessor(ID,none,none);

			feature_accessor(Solver_Settings,none,none);

			feature_accessor(Selection_Settings,none,none);
		};

	}
}

//class Zone : public m_array<double>
//{
//public:
//	// constructors
//	Zone (){}
//	Zone (double ID, vector<int> &dim_sizes);
//	Zone (const Zone& obj);
//
//	// Processing methods
//	void initialize(m_array Data);
//	bool IPF(double TOL, int MAX_ITER);
//	bool Select_HH(int MAX_ITER, const hash_map<uint,vector<Pop_Unit>> &sample, Prob_Generator& Rand);
//	void Add_Sample(const Pop_Unit& P);
//
//	// Property access methods
//	double& marginal(int dim, int index) {return _marginals[dim][index];} //marginals get/set
//	const double& Id(void){return _id;} // ID get
//
//	// I/O methods
//	bool write(void);
//	bool write(File_Writer &fw);
//	bool write_sample(File_Writer &fw);
//
//protected:
//	// Marginal totals across categories
//	vector<vector<double>> _marginals;
//	// List of households in zone
//	hash_map<uint, vector<Pop_Unit>> _sample;
//	// Unique zone ID
//	double _id;
//	
//
//private:
//
//};
//