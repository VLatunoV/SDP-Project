#pragma once
#include <fstream>
#include <string.h>
#include "sha256.h"
#include "sorted_array.h"

using namespace std;

struct meta
{
	size_t offset;
	size_t size;
};

class Storage
{
	static const size_t prime = 1000003;
	
	char* storage_name;
	char* data_path;
	char* metadata_path;
	inline void change_repo_name(const char*);
	
	size_t capacity;
	size_t num_data;
	double alpha;
	meta *data;
	void expand();
	size_t reduce(const unsigned int*) const;
	
	SortedArray<meta> holes;
	void register_hole(const meta&);
	
	size_t write_block(const unsigned char*, size_t, const unsigned int*);
	void write_single_entry(size_t) const;
	void write_metadata(unsigned int);
	bool write_metadata_with_backup(unsigned int);
	void read_metadata(unsigned int);

	static const size_t offset_to_num_data = 0;
	static const size_t offset_to_capacity = sizeof(size_t);
	static const size_t offset_to_num_holes = sizeof(size_t) * 2;
	static const size_t offset_to_metadata = sizeof(size_t) * 3;

	static const unsigned int num_data_bit = 0x1;
	static const unsigned int capacity_bit = 0x2;
	static const unsigned int num_holes_bit = 0x4;
	static const unsigned int metadata_bit = 0x8;
	static const unsigned int holes_bit = 0x10;

	static const unsigned int hash_size = 8 * sizeof(unsigned int);

	bool hash_exists(const unsigned int*) const;
	bool file_exists(const char*) const;
	bool file_create(const char*) const;
	
	Storage(const Storage&);
	Storage& operator= (const Storage&);
public:
	Storage();
	Storage(const char*);
	~Storage();
	bool Init(const char*);
	bool Create(const char*);
	bool Open(const char*);
	unsigned int* Store(const unsigned char*, size_t);
	bool Load(const unsigned int*, unsigned char*&, size_t&);
	bool Erase(const unsigned int*);
};

