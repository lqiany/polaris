//*********************************************************
//	Draw_Layer.h - Function for drawing a general layer
//*********************************************************

#pragma once
#include "Canvas_Implementation.h"

//---------------------------------------------------------
//	Draw_Layer
//--------------------------------------------------------

template<typename MasterType,typename ParentType,typename InheritanceList>
void Canvas_Implementation<MasterType,ParentType,InheritanceList>::Draw_Layer(int start_iteration, int end_iteration, Antares_Layer_Interface* layer)
{
	const bool draw=layer->draw<bool>();

	// skip hidden layers
	if(!draw) return;

	const PrimitiveType primitive_type=layer->primitive_type<PrimitiveType>();	
	
	const True_Color_RGBA<MasterType> head_color=layer->head_color<True_Color_RGBA<MasterType>>();
	const Point_3D<MasterType> head_normal=layer->head_normal<Point_3D<MasterType>>();
	const int head_size_value=layer->head_size_value<int>();
	
	const bool grouped=layer->grouped<bool>();
	const bool group_color=layer->group_color<bool>();
	const bool group_normal=layer->group_normal<bool>();
	
	const bool primitive_color=layer->primitive_color<bool>();
	const bool primitive_normal=layer->primitive_normal<bool>();
	const int primitive_stride = layer->primitive_stride<int>();
	
	const int vert_stride = layer->vert_stride<int>();

	const int data_stride=layer->data_stride<int>();
	
	const Dynamic_Multi_Buffer< vector<unsigned char>[_num_antares_threads] >& storage=layer->storage<Dynamic_Multi_Buffer< vector<unsigned char>[_num_antares_threads] >&>();

	//---- draw main layer ----

	switch(primitive_type)
	{
	case _POINT:
		glPointSize(head_size_value);
		glBegin(GL_POINTS);
		break;
	case _LINE:
		glLineWidth(head_size_value);
		glBegin(GL_LINES);
		break;
	case _TRIANGLE:
		glBegin(GL_TRIANGLES);
		break;
	case _QUAD:
		glBegin(GL_QUADS);
		break;
	default:
		assert(false);
		break;
	};

	glColor4ubv((GLubyte*)&head_color);
	glNormal3fv((GLfloat*)&head_normal);

	int current_iteration=start_iteration;
	
	while(current_iteration <= end_iteration)
	{
		const vector<unsigned char> (&geometry_by_thread)[_num_antares_threads] = storage[current_iteration];

		for(int i=0;i<_num_antares_threads;i++)
		{
			const unsigned char* geometry_itr = &geometry_by_thread[i].front();
			const unsigned char* const geometry_end = geometry_itr+geometry_by_thread[i].size();

			while(geometry_itr != geometry_end)
			{
				geometry_itr += data_stride;

				if(grouped)
				{
					if(group_color)
					{
						glColor4ubv((GLubyte*)geometry_itr);
						geometry_itr += sizeof(True_Color_RGBA<MasterType>);
					}

					if(group_normal)
					{
						glNormal3fv((GLfloat*)geometry_itr);
						geometry_itr += sizeof(Point_3D<MasterType>);
					}

					const int num_group_primitives=*((int*)geometry_itr);
					geometry_itr += sizeof(int);

					const unsigned char* const group_end = geometry_itr + primitive_stride * num_group_primitives;

					while( geometry_itr != group_end )
					{
						if(primitive_color)
						{
							glColor4ubv((GLubyte*)geometry_itr);
							geometry_itr += sizeof(True_Color_RGBA<MasterType>);
						}

						if(primitive_normal)
						{
							glNormal3fv((GLfloat*)geometry_itr);
							geometry_itr += sizeof(Point_3D<MasterType>);
						}

						const unsigned char* const geometry_vert_end = geometry_itr + vert_stride;

						while( geometry_itr != geometry_vert_end )
						{
							glVertex3fv((GLfloat*)geometry_itr);
							geometry_itr += sizeof(Point_3D<MasterType>);
						}
					}
				}
				else
				{
					if(primitive_color)
					{
						glColor4ubv((GLubyte*)geometry_itr);
						geometry_itr += sizeof(True_Color_RGBA<MasterType>);
					}

					if(primitive_normal)
					{
						glNormal3fv((GLfloat*)geometry_itr);
						geometry_itr += sizeof(Point_3D<MasterType>);
					}

					const unsigned char* const geometry_vert_end = geometry_itr + vert_stride;

					while( geometry_itr != geometry_vert_end )
					{
						glVertex3fv((GLfloat*)geometry_itr);
						geometry_itr += sizeof(Point_3D<MasterType>);
					}
				}

			}
		}

		current_iteration++;
	}

	glEnd();



	//---- draw layer accents in the second pass ----

	const Dynamic_Multi_Buffer< vector<unsigned char>[_num_antares_threads] >& accent_storage=layer->accent_storage<Dynamic_Multi_Buffer< vector<unsigned char>[_num_antares_threads] >&>();
	
	switch(primitive_type)
	{
	case _POINT:
		glPointSize( ceil(5.0f*(float)head_size_value) );
		glBegin(GL_POINTS);
		break;
	case _LINE:
		glLineWidth( ceil(5.0f*(float)head_size_value) );
		glBegin(GL_LINES);
		break;
	case _TRIANGLE:
		glBegin(GL_TRIANGLES);
		break;
	case _QUAD:
		glBegin(GL_QUADS);
		break;
	default:
		assert(false);
		break;
	};

	glColor4ubv((GLubyte*)&head_color);
	glNormal3fv((GLfloat*)&head_normal);
	
	current_iteration = start_iteration;
	
	while(current_iteration <= end_iteration)
	{
		const vector<unsigned char> (&geometry_by_thread)[_num_antares_threads] = accent_storage[current_iteration];

		for(int i=0;i<_num_antares_threads;i++)
		{
			const unsigned char* geometry_itr = &geometry_by_thread[i].front();
			const unsigned char* const geometry_end = geometry_itr+geometry_by_thread[i].size();

			while(geometry_itr != geometry_end)
			{
				geometry_itr += data_stride;

				if(grouped)
				{
					if(group_color)
					{
						glColor4ubv((GLubyte*)geometry_itr);
						geometry_itr += sizeof(True_Color_RGBA<MasterType>);
					}

					if(group_normal)
					{
						glNormal3fv((GLfloat*)geometry_itr);
						geometry_itr += sizeof(Point_3D<MasterType>);
					}

					const int num_group_primitives=*((int*)geometry_itr);
					geometry_itr += sizeof(int);

					const unsigned char* const group_end = geometry_itr + primitive_stride * num_group_primitives;

					while( geometry_itr != group_end )
					{
						if(primitive_color)
						{
							glColor4ubv((GLubyte*)geometry_itr);
							geometry_itr += sizeof(True_Color_RGBA<MasterType>);
						}

						if(primitive_normal)
						{
							glNormal3fv((GLfloat*)geometry_itr);
							geometry_itr += sizeof(Point_3D<MasterType>);
						}

						const unsigned char* const geometry_vert_end = geometry_itr + vert_stride;

						while( geometry_itr != geometry_vert_end )
						{
							glVertex3fv((GLfloat*)geometry_itr);
							geometry_itr += sizeof(Point_3D<MasterType>);
						}
					}
				}
				else
				{
					if(primitive_color)
					{
						glColor4ubv((GLubyte*)geometry_itr);
						geometry_itr += sizeof(True_Color_RGBA<MasterType>);
					}

					if(primitive_normal)
					{
						glNormal3fv((GLfloat*)geometry_itr);
						geometry_itr += sizeof(Point_3D<MasterType>);
					}

					const unsigned char* const geometry_vert_end = geometry_itr + vert_stride;

					while( geometry_itr != geometry_vert_end )
					{
						glVertex3fv((GLfloat*)geometry_itr);
						geometry_itr += sizeof(Point_3D<MasterType>);
					}
				}
			}
		}

		current_iteration++;
	}

	glEnd();
}