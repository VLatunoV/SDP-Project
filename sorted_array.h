#pragma once
#include <stdint.h>

template<typename T> class SortedArray
{
	size_t capacity;
	size_t size;
	T* data;
	int (*cmp)(const T& a, const T& b);
	void expand()
	{
		capacity = capacity * 2 + 1;
		T* temp = new T[capacity];
		for(size_t i = 0; i < size; ++i)
		{
			temp[i] = data[i];
		}
		delete[] data;
		data = temp;
	}
	size_t binary_search(const T& in)
	{
		if(!size || cmp(in, data[size - 1]) >= 0)
		{
			return size;
		}
		size_t l = 0, r = size - 1, mid = (r + l) / 2;
		int v;
		while(l != r)
		{
			v = cmp(in, data[mid]);
			if(v == 0)
			{
				return mid;
			}
			else if(v < 0)
			{
				r = mid;
			}
			else
			{
				l = mid + 1;
			}
			mid = (l + r) / 2;
		}
		return mid;
	}
	
	SortedArray();
public:
	SortedArray(int (*c)(const T&, const T&)): capacity(0xFF), size(0), data(0), cmp(c)
	{
		data = new T[capacity];
	}
	~SortedArray()
	{
		delete[] data;
	}
	size_t Add(const T& in)
	{
		size_t index = 0;
		if(size == capacity)
		{
			expand();
		}
		if(size)
		{
			index = binary_search(in);
			for(size_t i = size; i > index; --i)
			{
				data[i] = data[i - 1];
			}
			data[index] = in;
			++size;
		}
		else
		{
			data[size++] = in;
		}
		return index;
	}
	void RemoveAt(size_t index)
	{
		--size;
		for(size_t i = index; i < size; ++i)
		{
			data[i] = data[i + 1];
		}
	}
	void Search(const T& elem, size_t& index)
	{
		index = binary_search(elem);
		//return (bool)cmp(elem, data[index]);
	}
	void InitFromSortedArray(const T* arr, size_t size)
	{
		this->size = size;
		if (capacity <= size)
		{
			while (capacity <= size)
			{
				capacity = capacity * 2 + 1;
			}
			delete[] data;
			data = new T[capacity];
		}
		for (size_t i = 0; i < size; ++i)
		{
			data[i] = arr[i];
		}
	}
	size_t Size() const
	{
		return size;
	}
	T& operator[](size_t index)
	{
		return data[index];
	}
};

