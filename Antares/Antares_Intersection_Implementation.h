//*********************************************************
//	Graphical_Intersection_Implementation.h - Graphical Intersection Variant
//*********************************************************

#pragma once
#include "Antares_Includes.h"
#include "User_Space\Polaris_Intersection_Implementation.h"

namespace Intersection_Components
{
	namespace Types
	{
	}

	namespace Concepts
	{
	}
	
	namespace Implementations
	{
		implementation struct Antares_Intersection_Implementation:public Polaris_Intersection_Implementation<MasterType,ParentType,APPEND_CHILD(Antares_Intersection_Implementation)>
		{
			static void on_select(const list<void*>& removed,const list<void*>& added,const list<void*>& selected,vector<pair<string,string>>& bucket)
			{
				if(removed.size())
				{
					((type_of(MasterType::network)*) _global_network)->_link_lines->Clear_Accented<NT>();

					if(selected.size())
					{
						for(list<void*>::const_iterator itr=selected.begin();itr!=selected.end();itr++)
						{
							((ComponentType*)*itr)->Accent_Self<ComponentType,ComponentType,NT>();
						}
					}
				}
				else if(added.size())
				{
					for(list<void*>::const_iterator itr=added.begin();itr!=added.end();itr++)
					{
						((ComponentType*)*itr)->Accent_Self<ComponentType,ComponentType,NT>();
					}
				}

				if(selected.size())
				{
					((ComponentType*) (selected.back()))->Display_Attributes<ComponentType,ComponentType,NT>(bucket);
				}
			}
			
			feature_implementation void Accent_Self()
			{
				Intersection_Polygon<MasterType> polygon;

				float _intersection_radius = 50;

				Point_3D<MasterType> v_node;
				v_node._x = _x_position;
				v_node._y = _y_position;
				Scale_Coordinates<typename MasterType::type_of(canvas),NT,Target_Type<NULLTYPE,void,Point_3D<MasterType>&>>(v_node);

				float x = v_node._x;
				float y = v_node._y;
				float sin_alpha = sin(45.0/2.0*3.14159265/180.0);
				float cos_alpha = cos(45.0/2.0*3.14159265/180.0);
				float r = _intersection_radius;

				polygon.vertices[0]._x = x - r * sin_alpha;
				polygon.vertices[0]._y = y - r * cos_alpha;
				polygon.vertices[0]._z = 0.5;
					
				polygon.vertices[1]._x = x + r * sin_alpha;
				polygon.vertices[1]._y = y - r * cos_alpha;
				polygon.vertices[1]._z = 0.5;

				polygon.vertices[2]._x = x + r * cos_alpha;
				polygon.vertices[2]._y = y - r * sin_alpha;
				polygon.vertices[2]._z = 0.5;

				polygon.vertices[3]._x = x + r * cos_alpha;
				polygon.vertices[3]._y = y + r * sin_alpha;
				polygon.vertices[3]._z = 0.5;

				polygon.vertices[4]._x = x + r * sin_alpha;
				polygon.vertices[4]._y = y + r * cos_alpha;
				polygon.vertices[4]._z = 0.5;

				polygon.vertices[5]._x = x - r * sin_alpha;
				polygon.vertices[5]._y = y + r * cos_alpha;
				polygon.vertices[5]._z = 0.5;

				polygon.vertices[6]._x = x - r * cos_alpha;
				polygon.vertices[6]._y = y + r * sin_alpha;
				polygon.vertices[6]._z = 0.5;

				polygon.vertices[7]._x = x - r * cos_alpha;
				polygon.vertices[7]._y = y - r * sin_alpha;
				polygon.vertices[7]._z = 0.5;

				//True_Color_RGBA<NT> color;
				//color = get_intersection_color_by_type(intersection->template intersection_type<int>());
				//polygon.color = color;
				polygon.color._r = 255;
				polygon.color._g = 0;
				polygon.color._b = 0;
				polygon.data = (void*)(this);
				((type_of(MasterType::network)*) _global_network)->_intersection_polygons->Push_Element<Accented_Element>(&polygon);
			}

			feature_implementation void Display_Attributes(vector<pair<string,string>>& bucket)
			{
				stringstream s;
				//char str_buf[128];

				pair<string,string> key_value_pair;
				
				key_value_pair.first="Id";
				s << _internal_id;
				key_value_pair.second=s.str();
				s.str("");				
				bucket.push_back(key_value_pair);
				

				key_value_pair.first="Type";
				
				switch(_intersection_type)
				{
				case Types::Intersection_Type_Keys::NO_CONTROL:
					key_value_pair.second="NO_CONTROL";
					break;
				case Types::Intersection_Type_Keys::YIELD_SIGN:
					key_value_pair.second="YIELD_SIGN";
					break;
				case Types::Intersection_Type_Keys::ALL_WAY_STOP_SIGN:
					key_value_pair.second="ALL_WAY_STOP_SIGN";
					break;
				case Types::Intersection_Type_Keys::TWO_WAY_STOP_SIGN:
					key_value_pair.second="TWO_WAY_STOP_SIGN";
					break;
				case Types::Intersection_Type_Keys::PRE_TIMED_SIGNAL_CONTROL:
					key_value_pair.second="PRE_TIMED_SIGNAL_CONTROL";
					break;
				case Types::Intersection_Type_Keys::ACTUATED_SIGNAL_CONTROL:
					key_value_pair.second="ACTUATED_SIGNAL_CONTROL";
					break;
				case Types::Intersection_Type_Keys::ADAPTIVE_SIGNAL_CONTROL:
					key_value_pair.second="ADAPTIVE_SIGNAL_CONTROL";
					break;
				default:
					key_value_pair.second="NON_RECOGNIZED";
					break;
				}
				bucket.push_back(key_value_pair);

			}

			static bool fetch_attributes(Antares_Intersection_Implementation* _this,vector<string>& bucket)
			{
				stringstream s;
				s << _this->_internal_id;
				bucket.push_back(s.str());
				s.str("");
				
				switch(_this->_intersection_type)
				{
				case Types::Intersection_Type_Keys::NO_CONTROL:
					bucket.push_back("NO_CONTROL");
					break;
				case Types::Intersection_Type_Keys::YIELD_SIGN:
					bucket.push_back("YIELD_SIGN");
					break;
				case Types::Intersection_Type_Keys::ALL_WAY_STOP_SIGN:
					bucket.push_back("ALL_WAY_STOP_SIGN");
					break;
				case Types::Intersection_Type_Keys::TWO_WAY_STOP_SIGN:
					bucket.push_back("TWO_WAY_STOP_SIGN");
					break;
				case Types::Intersection_Type_Keys::PRE_TIMED_SIGNAL_CONTROL:
					bucket.push_back("PRE_TIMED_SIGNAL_CONTROL");
					break;
				case Types::Intersection_Type_Keys::ACTUATED_SIGNAL_CONTROL:
					bucket.push_back("ACTUATED_SIGNAL_CONTROL");
					break;
				case Types::Intersection_Type_Keys::ADAPTIVE_SIGNAL_CONTROL:
					bucket.push_back("ADAPTIVE_SIGNAL_CONTROL");
					break;
				default:
					bucket.push_back("NON_RECOGNIZED");
					break;
				}
				return true;
			}
		};
	}
}

using namespace Intersection_Components::Implementations;
