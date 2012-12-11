#pragma once

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <exception>

typedef unsigned int uint;

using namespace std;

//-------------------------------------------------------------
//	MULTIDIMENSIONAL ARRAY CLASS WITH INDETERMINATE DIMENSIONS
//-------------------------------------------------------------
template<class T>
class m_array_iterator
{

public:
       typedef std::random_access_iterator_tag	iterator_category;
       typedef T								value_type;
       typedef value_type &						reference;
       typedef value_type   *					pointer;
       typedef int								difference_type;
	   typedef uint								size_type;

      m_array_iterator()
       {      // default constructor
		   _fixed_dim = -1;
		   _fixed_dim_val = -1;
		   m_Ptr = NULL;
       }

      m_array_iterator(T* data, vector<size_type>* dim_sizes, vector<size_type>* cursor)
       {	// copy constructor
			_fixed_dim = -1;
			_fixed_dim_val = -1;
			_dimensions = (vector<size_type>*)dim_sizes;
			for (size_type i = 0; i< cursor->size(); i++) _cursor.push_back((*cursor)[i]);
			m_Ptr = &data[get_index(_cursor)];
			_data = data;
       }

	  m_array_iterator(T* data, vector<size_type>* dim_sizes, vector<size_type>* cursor, size_type fixed_dimension_index, size_type fixed_dimension_value)
       {	// copy constructor
			_fixed_dim = fixed_dimension_index;
			_fixed_dim_val = fixed_dimension_value;
			_dimensions = dim_sizes;
			for (size_type i = 0; i< cursor->size(); i++) _cursor.push_back((*cursor)[i]);
			_cursor[_fixed_dim] = _fixed_dim_val;
			m_Ptr = &data[get_index(_cursor)];
			_data = data;
       }

      reference operator*() const
       {  
		   // return designated object
           return (*m_Ptr);
       }

	   m_array_iterator& operator=(const m_array_iterator& obj)
       {      
              m_Ptr = obj.m_Ptr;
			  _dimensions = obj._dimensions;
			  _cursor = obj._cursor;
			  _data = obj._data;
			  _fixed_dim = obj._fixed_dim;
			  _fixed_dim_val = obj._fixed_dim_val;

             return (*this);
       }

       m_array_iterator& operator++()
       {    // preincrement
			//++m_Ptr;
			//return (*this);

			for (size_type d = (size_type)_dimensions->size()-1; d >= 0; d--)
			{
				if (d != _fixed_dim)
				{
					++(_cursor)[d];

					if ((_cursor)[d] >= (*_dimensions)[d])
					{
						(_cursor)[d] = 0;

						if (d == 0 || (d == 1 && _fixed_dim == 0))
						{
							m_Ptr = NULL;
							return (*this);
						}
					}
					else break;
				}
			}
			m_Ptr = &_data[get_index(_cursor)];

			return (*this);
       }

      m_array_iterator& operator--()
       {      // predecrement
              --m_Ptr;

             return (*this);
       }

      m_array_iterator operator++(int)
       {      // postincrement
              m_array_iterator _Tmp = *this;
              ++*this;
              return (_Tmp);
       }

      m_array_iterator operator--(int)

      {      // postdecrement
              m_array_iterator _Tmp = *this;
              --*this;
              return (_Tmp);
       }

      bool operator==(const m_array_iterator &_Right) const
       {      // test for iterator equality
              return (m_Ptr == _Right.m_Ptr);
       }

      bool operator!=(const m_array_iterator &_Right) const
       {      // test for iterator inequality
              return (!(m_Ptr == _Right.m_Ptr));
       }

	const vector<size_type>& get_index()
	{
		return _cursor;
	}

private:
	T *m_Ptr;                                            // pointer to container value type
	vector<size_type>* _dimensions;								// dimensional sizes 
	vector<size_type> _cursor;
	T * _data;
	int _fixed_dim;
	int _fixed_dim_val;
	uint get_index(vector<size_type>& index)
	{
		uint ind=0;

		if (index.size() != _dimensions->size()) throw new std::exception("Error, incorrect number of dimensions in index.");

		for (int i = 0; i< index.size(); i++)
		{
			if (index[i] >= (*_dimensions)[i]) throw new std::exception( string("Error, index outside of array bounds for dimension: %i", i).c_str());

			int multiplier = 1;
			for (int j=i+1; j< index.size(); j++)
			{
				multiplier = multiplier * (*_dimensions)[j];
			}

			ind += index[i] * multiplier;
		}
		return ind;
	}
 };

 template <class T>
class m_array
{
public:

	typedef T							value_type;
	typedef value_type&					reference;
	typedef value_type*					pointer;
	typedef const value_type&			const_reference;
	typedef m_array_iterator<T>			iterator;
	typedef const m_array_iterator<T>	const_iterator;
	typedef int							difference_type;
	typedef uint						size_type;
	typedef const uint					const_size_type;
	typedef vector<size_type>			index_type;
	typedef const vector<size_type>&	const_index_type;
	typedef const vector<size_type>&	const_dimensional_type;

	// Members added for STL compliance
	reference		at(const_index_type i){return _data[get_index(i)];}
	const_reference	at(const_index_type i) const {return _data[get_index(i)];}
	reference		front(){return _data[0];}
	const_reference	front() const {return _data[0];}
	reference		front(size_type fixed_dimension_index, size_type fixed_dimension_value)
	{
		_cursor_start();
		_cursor[fixed_dimension_index] = fixed_dimension_value;
		return this->operator[](_cursor);
	}
	const_reference	front(size_type fixed_dimension_index, size_type fixed_dimension_value) const 
	{
		_cursor_start();
		_cursor[fixed_dimension_index] = fixed_dimension_value;
		return this->operator[](_cursor);
	}
	reference		back(){return _data[_size-1];}
	const_reference	back() const {return _data[_size-1];}
	reference		back(size_type fixed_dimension_index, size_type fixed_dimension_value)
	{
		_cursor_end();
		_cursor[fixed_dimension_index] = fixed_dimension_value;
		return this->operator[](_cursor);
	}
	const_reference	back(size_type fixed_dimension_index, size_type fixed_dimension_value) const 
	{
		_cursor_end();
		_cursor[fixed_dimension_index] = fixed_dimension_value;
		return this->operator[](_cursor);
	}
	iterator		begin()
    {
		for (size_type i = 0; i< _dim_sizes.size(); i++) _cursor[i] = 0;
		return iterator(this->_data, &_dim_sizes, &_cursor);
    }
	iterator		begin(size_type fixed_dimension_index, size_type fixed_dimension_value)
    {
		for (size_type i = 0; i< _dim_sizes.size(); i++) _cursor[i] = 0;
		return iterator(this->_data, &_dim_sizes, &_cursor, fixed_dimension_index, fixed_dimension_value);
    }
	iterator		end() {return iterator();}
	bool			empty(){ return (_size==0);}
	void			clear(){_cleanup();}
	void			resize(vector<size_type> new_dimensions)
	{
		m_array<T> tmp = m_array<T>(*this);
		this->_cleanup();
		this->_init(new_dimensions);
		*this = m_array<T>(new_dimensions, value);

		iterator itr = this->begin();

		for (itr; itr != this->end(); ++itr)
		{
			const vector<size_type>& index = itr.get_index();
			if (tmp.valid_index(index))
			{
				size_type i = tmp.get_index(index);
				(*itr) = tmp._data[i];
			}
			else (*itr) = value;
			
		}

		for (int i = 0; i< this->size(); i++)
		{
			cout <<"**"<< this->_data[i] << " @ " << &this->_data[i]<<endl;
		}
	}
	void			resize(vector<size_type> new_dimensions, value_type value)
	{
		m_array<T> tmp = m_array<T>(*this);
		this->_cleanup();
		this->_init(new_dimensions);
		*this = m_array<T>(new_dimensions, value);

		iterator itr = this->begin();

		for (itr; itr != this->end(); ++itr)
		{
			const index_type index = (index_type)itr.get_index();
			if (tmp.valid_index(index))
			{
				size_type i = tmp.get_index(index);
				(*itr) = tmp._data[i];
			}
			else (*itr) = value;
			
		}
	}

	// MArray constructors/destructor
	m_array (void){_size = 0;}
	m_array (const_index_type dim_sizes);
	m_array (const_index_type dim_sizes, T init_val);
	m_array (const m_array& obj);
	void Init(const_index_type dim_sizes);
	void Copy(const m_array& obj);
	m_array& operator=(const m_array& obj);
	~m_array (void);


	// MArray operators overloads for data acces
	reference operator[](const_index_type index) // get data at given index
	{
		size_type i = get_index(index);
		return _data[i];
	} 
	const_reference operator[](const_index_type index) const // get data at given index
	{
		size_type i = get_index(index);
		return _data[i];
	} 
	reference operator[](size_type index) // get data at given index
	{
		return _data[index];
	} 
	const_reference operator[](const_size_type index) const // get data at given index
	{
		return _data[index];
	} 
	
	// Property access members
	const size_type& size() {return _size;}
	const size_type& size(size_type dimension) {return _dim_sizes[dimension];}
	const_index_type dimensions(){return _dim_sizes;}
	const size_type& num_dimensions() {return _ndim;}

	// display member
	void print(ostream& stream);

protected:
	index_type _dim_sizes;
	index_type _cursor;
	size_type _ndim;
	size_type _size;
	pointer _data;

	void _init(const_index_type dim_sizes);
	void _copy(const m_array& obj);
	void _cleanup()
	{
		if (_size > 0) delete _data;
		_size=0;
		_ndim=0;
		_dim_sizes.clear();
		_cursor.clear();
	}

	size_type get_index(const_index_type index)
	{
		size_type ind=0;

		if (index.size() != _ndim)
		{
			const char* mess = "Error, incorrect number of dimensions in index.";
			throw new std::exception(mess);
		}
		for (size_type i = 0; i< index.size(); i++)
		{
			if (index[i] >= _dim_sizes[i]) 
			{
				const char* mess = string("Error, index outside of array bounds for dimension: %i", i).c_str();
				throw new std::exception(mess);
			}
			size_type multiplier = 1;
			for (size_type j=i+1; j< index.size(); j++)
			{
				multiplier = multiplier * _dim_sizes[j];
			}

			ind += index[i] * multiplier;
		}
		return ind;
	}
	bool valid_index(const_index_type index)
	{
		if (index.size() != _ndim)
		{
			return false;
		}
		for (size_type i = 0; i< index.size(); i++)
		{
			if (index[i] >= _dim_sizes[i]) return false;
		}
		return true;
	}
	void _cursor_start()
	{
		for (int i=0; i<_cursor.size(); i++) _cursor[i] = 0;
	}
	void _cursor_end()
	{
		for (int i=0; i<_cursor.size(); i++) _cursor[i] = _dim_sizes[i]-1;
	}

	void print(ostream& stream, int n);

};

// Multi-dim Array Constructors, copiers, assignment, etc.
template <class T>
m_array<T>::m_array(const_index_type dim_sizes)
{
	_init(dim_sizes);
}
template <class T>
m_array<T>::m_array(const_index_type dim_sizes, T init_val)
{
	_init(dim_sizes);
	for (size_type i=0; i<_size; i++) _data[i]=init_val;
}
template <class T>
m_array<T>::m_array(const m_array<T>& obj)
{
	_copy(obj);
}
template <class T>
void m_array<T>::Copy(const m_array<T>& obj)
{
	if (this==&obj) return;
	_cleanup();
	_copy(obj);
}
template <class T>
void m_array<T>::_copy(const m_array<T>& obj)
{
	_dim_sizes.clear();
	_cursor.clear();
	_ndim = obj._ndim;
	_size = obj._size;

	for (size_type i=0; i<obj._dim_sizes.size(); i++)
	{
		_dim_sizes.push_back(obj._dim_sizes[i]);
		_cursor.push_back(0);
	}

	_data = new T[_size];

	for (size_type i=0; i<_size; i++) _data[i] = obj._data[i];
}
template <class T>
void m_array<T>::Init(const_index_type dim_sizes)
{
	_cleanup();
	_init(dim_sizes);
}
template <class T>
void m_array<T>::_init(const_index_type dim_sizes)
{
	_ndim = (size_type)dim_sizes.size();
	_size = 1;

	for (size_type i=0; i<_ndim; i++)
	{
		_dim_sizes.push_back(dim_sizes[i]);
		_cursor.push_back(0);
		_size = _size*dim_sizes[i];
	}

	_data = new value_type[_size];
}
template <class T>
m_array<T>& m_array<T>::operator=(const m_array<T>& obj)
{
	if (this != &obj)
	{
		_cleanup();
		_copy(obj);
	}
	return *this;
}


// M_array destructor
template <class T>
m_array<T>::~m_array(void)
{
	_cleanup();
}

// display member functions
template <class T>
void m_array<T>::print(ostream& stream)
{
	this->begin();
	print(stream, 0);
}
template <class T>
void m_array<T>::print(ostream& stream, int n)
{
	if (n == _ndim-2)
	{	
		// print header for higher dimensions
		if(_ndim>2)
		{
			stream<<"Higher Dimensions: ";
			for (uint k=0; k<_ndim-2; k++) stream<<"D"<<k<<"="<<_cursor[k]<<",";
			stream<<endl;
		}

		// print header for columns (last dimension in matrix)
		stream<<"\t";
		for (uint k=0; k<_dim_sizes[n+1]; k++) 
		{
			stream<</*setw(6)<<*/"D"<<n+1;
			//cout<<n+1;
			stream<<"=";
			stream<<k;
			stream<<"\t";
		}
		stream<<endl;

		// print 2d matrix of last 2 dimensions
		for (uint i=0; i<_dim_sizes[n]; i++)
		{
			_cursor[n] = i;
			stream<<"D"<<n<<"="<<i<<"\t";
			for (uint j=0; j<_dim_sizes[n+1]; j++)
			{
				_cursor[n+1]=j;

				stream<</*setw(10)<<*/this->operator[](this->_cursor)<<"\t";
			}
			stream<<endl;
		}
		stream<<endl<<endl;
		return;
	}

	for (uint i=0; i<_dim_sizes[n]; i++)
	{
		_cursor[n] = i;
		print(stream, n+1);
	}
}
