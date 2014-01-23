#pragma once
#include "Repository_Includes.h"


namespace Distributions
{
	namespace Types
	{
	}

	namespace Concepts
	{

	}
	
	namespace Prototypes
	{
		prototype struct Distribution
		{
			tag_as_prototype;

			// Probability density function
			define_feature_exists_check(Probability_Density_Value, has_pdf);
			template<typename TargetType> TargetType Probability_Density_Value(TargetType x ,requires(TargetType,check(ComponentType,has_pdf) && check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				return this_component()->Probability_Density_Value<TargetType>(x);
			}
			template<typename TargetType> TargetType Probability_Density_Value(TargetType x, requires(TargetType,!check(ComponentType,has_pdf) || !check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				assert_check(ComponentType,has_pdf, "Error, the Distribution ComponentType does not have a PDF defined."); 
				assert_check(typename remove_pointer<TargetType>::type, is_arithmetic, "Error, TargetType for this function must be arithmetic.");
			}
			template<typename TargetType> TargetType Probability_Density_Value(TargetType x, TargetType location, requires(TargetType,check(ComponentType,has_pdf) && check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				return this_component()->Probability_Density_Value<TargetType>(x, location);
			}
			template<typename TargetType> TargetType Probability_Density_Value(TargetType x, TargetType location, requires(TargetType,!check(ComponentType,has_pdf) || !check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				assert_check(ComponentType,has_pdf, "Error, the Distribution ComponentType does not have a PDF defined."); 
				assert_check(typename remove_pointer<TargetType>::type, is_arithmetic, "Error, TargetType for this function must be arithmetic.");
			}
			template<typename TargetType> TargetType Probability_Density_Value(TargetType x, TargetType location, TargetType scale, requires(TargetType,check(ComponentType,has_pdf) && check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				return this_component()->Probability_Density_Value<TargetType>(x, location, scale);
			}
			template<typename TargetType> TargetType Probability_Density_Value(TargetType x, TargetType location, TargetType scale, requires(TargetType,!check(ComponentType,has_pdf) || !check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				assert_check(ComponentType,has_pdf, "Error, the Distribution ComponentType does not have a PDF defined."); 
				assert_check(typename remove_pointer<TargetType>::type, is_arithmetic, "Error, TargetType for this function must be arithmetic.");
			}	
			template<typename TargetType> TargetType Probability_Density_Value(TargetType x, TargetType location, TargetType scale, TargetType shape,requires(TargetType,check(ComponentType,has_pdf) && check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				return this_component()->Probability_Density_Value<TargetType>(x, location, scale, shape);
			}
			template<typename TargetType> TargetType Probability_Density_Value(TargetType x, TargetType location, TargetType scale, TargetType shape,requires(TargetType,!check(ComponentType,has_pdf) || !check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				assert_check(ComponentType,has_pdf, "Error, the Distribution ComponentType does not have a PDF defined."); 
				assert_check(typename remove_pointer<TargetType>::type, is_arithmetic, "Error, TargetType for this function must be arithmetic.");
			}
			
			// Cumulative distribution function
			define_feature_exists_check(Cumulative_Distribution_Value, has_cdf);
			template<typename TargetType> TargetType Cumulative_Distribution_Value(TargetType x ,requires(TargetType,check(ComponentType,has_cdf) && check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				return this_component()->Cumulative_Distribution_Value<TargetType>(x);
			}
			template<typename TargetType> TargetType Cumulative_Distribution_Value(TargetType x, requires(TargetType,!check(ComponentType,has_cdf) || !check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				assert_check(ComponentType,has_cdf, "Error, the Distribution ComponentType does not have a CDF defined."); 
				assert_check(typename remove_pointer<TargetType>::type, is_arithmetic, "Error, TargetType for this function must be arithmetic.");
			}
			template<typename TargetType> TargetType Cumulative_Distribution_Value(TargetType x, TargetType location, requires(TargetType,check(ComponentType,has_cdf) && check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				return this_component()->Cumulative_Distribution_Value<TargetType>(x, location);
			}
			template<typename TargetType> TargetType Cumulative_Distribution_Value(TargetType x, TargetType location, requires(TargetType,!check(ComponentType,has_cdf) || !check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				assert_check(ComponentType,has_cdf, "Error, the Distribution ComponentType does not have a CDF defined."); 
				assert_check(typename remove_pointer<TargetType>::type, is_arithmetic, "Error, TargetType for this function must be arithmetic.");
			}
			template<typename TargetType> TargetType Cumulative_Distribution_Value(TargetType x, TargetType location, TargetType scale, requires(TargetType,check(ComponentType,has_cdf) && check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				return this_component()->Cumulative_Distribution_Value<TargetType>(x, location, scale);
			}
			template<typename TargetType> TargetType Cumulative_Distribution_Value(TargetType x, TargetType location, TargetType scale, requires(TargetType,!check(ComponentType,has_cdf) || !check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				assert_check(ComponentType,has_cdf, "Error, the Distribution ComponentType does not have a CDF defined."); 
				assert_check(typename remove_pointer<TargetType>::type, is_arithmetic, "Error, TargetType for this function must be arithmetic.");
			}			
			template<typename TargetType> TargetType Cumulative_Distribution_Value(TargetType x, TargetType location, TargetType scale, TargetType shape, requires(TargetType,check(ComponentType,has_cdf) && check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				return this_component()->Cumulative_Distribution_Value<TargetType>(x, location, scale, shape);
			}
			template<typename TargetType> TargetType Cumulative_Distribution_Value(TargetType x, TargetType location, TargetType scale, TargetType shape, requires(TargetType,!check(ComponentType,has_cdf) || !check(typename remove_pointer<TargetType>::type, is_arithmetic)))
			{
				assert_check(ComponentType,has_cdf, "Error, the Distribution ComponentType does not have a CDF defined."); 
				assert_check(typename remove_pointer<TargetType>::type, is_arithmetic, "Error, TargetType for this function must be arithmetic.");
			}
		};
	}

}

using namespace Distributions::Prototypes;