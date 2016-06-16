#include <stdio.h>
#include "storage.h"

void Storage::change_repo_name(const char* name)
{
	delete[] storage_name;
	delete[] data_path;
	delete[] metadata_path;
	size_t l = strlen(name);

	storage_name = new char[l + 1];
	data_path = new char [l + 6];
	metadata_path = new char [l + 6];

	strcpy(storage_name, name);
	strcpy(data_path, name);
	strcpy(metadata_path, name);

	strcat(data_path, ".data");
	strcat(metadata_path, ".meta");
}
void Storage::expand()
{
	size_t old_cap = capacity;
	meta* new_data;
	unsigned int h[8];
	size_t index;
	fstream data_file;
	data_file.exceptions(ios::badbit | ios::failbit);
	capacity = capacity * 2 + 1;
	new_data = new meta[capacity]();
	data_file.open(data_path, ios::in | ios::binary);
	for (size_t i = 0; i < old_cap; ++i)
	{
		if (data[i].size)
		{
			data_file.seekg(data[i].offset);
			data_file.read((char*)h, hash_size);
			index = reduce(h);
			while (new_data[index].size)
			{
				index = (index + prime) % capacity;
			}
			new_data[index] = data[i];
		}
	}
	data_file.close();
	delete[] data;
	data = new_data;
}
size_t Storage::reduce(const unsigned int* hash) const
{
	size_t result = 0;
	for(int i = 0; i < 8; ++i)
	{
		// Distribute the 8 hash value's bit among size_t's
		result ^= (size_t)(hash[i]) << ((7 - i) * sizeof(size_t) / sizeof(unsigned int) / 8) * 32;
	}
	return result % capacity;
}
void Storage::register_hole(const meta& in)
{
	size_t index;
	holes.Search(in, index);
	if (index != holes.Size() && in.offset + in.size == holes[index].offset)
	{
		holes[index].offset = in.offset;
		holes[index].size += in.size;
		if (index != 0 && holes[index - 1].offset + holes[index - 1].size == holes[index].offset)
		{
			holes[index - 1].size += holes[index].size;
			holes.RemoveAt(index);
		}
	}
	else if (index != 0 && holes[index - 1].offset + holes[index - 1].size == in.offset)
	{
		holes[index - 1].size += in.size;
	}
	else
	{
		holes.Add(in);
	}
}
size_t Storage::write_block(const unsigned char* block, size_t size, const unsigned int* hash)
{
	bool hole_found = false;
	size_t offset;
	size_t i;
	fstream data_file;
	data_file.exceptions(ios::badbit | ios::failbit);
	data_file.open(data_path, ios::in | ios::out | ios::binary);
	for (i = 0; i < holes.Size(); ++i)
	{
		if (holes[i].size >= size + hash_size)
		{
			offset = holes[i].offset;
			hole_found = true;
			break;
		}
	}
	if (hole_found)
	{
		data_file.seekp(offset, ios::beg);
	}
	else
	{
		data_file.seekp(0, ios::end);
		offset = data_file.tellp();
	}
	data_file.write((const char*)hash, hash_size);
	data_file.write((const char*)block, size);
	data_file.close();
	if (hole_found)
	{
		holes[i].offset += size + hash_size;
		holes[i].size -= size + hash_size;
		if (holes[i].size == 0)
		{
			holes.RemoveAt(i);
		}
	}
	return offset;
}
void Storage::write_single_entry(size_t index) const
{
	fstream file;
	file.exceptions(ios::badbit | ios::failbit);
	file.open(metadata_path, ios::in | ios::out | ios::binary);
	file.seekp(offset_to_metadata + index * sizeof(*data));
	file.write((const char*)&(data[index]), sizeof(*data));
	file.close();
}
void Storage::write_metadata(unsigned int bits = 0xFFFFFFFF)
{
	size_t s = holes.Size();

	fstream file;
	file.exceptions(ios::badbit | ios::failbit);
	file.open(metadata_path, ios::in | ios::out | ios::binary);
	if (bits & num_data_bit)
	{
		file.seekp(offset_to_num_data);
		file.write((const char*)&num_data, sizeof(num_data));
	}
	if (bits & capacity_bit)
	{
		file.seekp(offset_to_capacity);
		file.write((const char*)&capacity, sizeof(capacity));
	}
	if (bits & num_holes_bit)
	{
		file.seekp(offset_to_num_holes);
		file.write((const char*)&s, sizeof(s));
	}
	if (bits & metadata_bit)
	{
		file.seekp(offset_to_metadata);
		file.write((const char*)data, sizeof(*data) * capacity);
	}
	if (bits & holes_bit)
	{
		if (s)
		{
			file.seekp(offset_to_metadata + sizeof(*data) * capacity);
			file.write((const char*)&(holes[0]), sizeof(holes[0]) * s);
		}
	}
	file.close();
}
bool Storage::write_metadata_with_backup(unsigned int bits = 0xFFFFFFFF)
{
	char* backup = new char[strlen(metadata_path) + 8];
	strcpy(backup, storage_name);
	strcat(backup, "_backup.meta");
	remove(backup);
	if (rename(metadata_path, backup))
	{
		perror("Could not create backup");
		delete[] backup;
		return false;
	}
	try
	{
		file_create(metadata_path);
		write_metadata(bits);
	}
	catch (...)
	{
		remove(metadata_path);
		rename(backup, metadata_path);
		delete[] backup;
		throw;
	}
	remove(backup);
	delete[] backup;
	return true;
}
void Storage::read_metadata(unsigned int bits = 0xFFFFFFFF)
{
	size_t s = holes.Size();
	fstream file;
	file.exceptions(ios::badbit | ios::failbit);
	file.open(metadata_path, ios::in | ios::binary);
	if (bits & num_data_bit)
	{
		file.seekg(offset_to_num_data);
		file.read((char*)&num_data, sizeof(num_data));
	}
	if (bits & capacity_bit)
	{
		file.seekg(offset_to_capacity);
		file.read((char*)&capacity, sizeof(capacity));
		delete[] data;
		data = new meta[capacity]();
	}
	if (bits & num_holes_bit)
	{
		file.seekg(offset_to_num_holes);
		file.read((char*)&s, sizeof(s));
	}
	if (bits & metadata_bit)
	{
		file.seekg(offset_to_metadata);
		file.read((char*)data, sizeof(*data) * capacity);
	}
	if (bits & holes_bit)
	{
		if (s)
		{
			meta* temp = new meta[s];
			try
			{
				file.seekg(offset_to_metadata + sizeof(*data) * capacity);
				file.read((char*)temp, sizeof(*temp) * s);
				holes.InitFromSortedArray(temp, s);
			}
			catch (...)
			{
				delete[] temp;
				throw;
			}
			delete[] temp;
		}
	}
}
bool Storage::hash_exists(const unsigned int* in) const
{
	fstream file;
	size_t index = reduce(in);
	unsigned int h[8];
	file.exceptions(ios::badbit | ios::failbit);
	file.open(data_path, ios::in | ios::binary);
	while (data[index].size)
	{
		file.seekg(data[index].offset);
		file.read((char*)h, hash_size);
		if (h[0] == in[0] &&
			h[1] == in[1] &&
			h[2] == in[2] &&
			h[3] == in[3] &&
			h[4] == in[4] &&
			h[5] == in[5] &&
			h[6] == in[6] &&
			h[7] == in[7])
		{
			return true;
		}
		else
		{
			index = (index + prime) % capacity;
		}
	}
	file.close();
	return false;
}
bool Storage::file_exists(const char* name) const
{
	ifstream file;
	file.open(name);
	return bool(file);
}
bool Storage::file_create(const char* name) const
{
	ofstream file;
	file.open(name);
	return bool(file);
}
Storage::Storage():
	storage_name(0), data_path(0), metadata_path(0), capacity(0x4), num_data(0), alpha(0.0), data(0),
	holes(
		[](const meta& x, const meta& y) -> int
		{
			if(x.offset == y.offset) return 0;
			return x.offset > y.offset ? 1 : -1;
		}
	)
{
	data = new meta[capacity]();
}
Storage::Storage(const char* name):
	storage_name(0), data_path(0), metadata_path(0), capacity(0x4), num_data(0), alpha(0.0), data(0),
	holes(
		[](const meta& x, const meta& y) -> int
		{
			if(x.offset == y.offset) return 0;
			return x.offset > y.offset ? 1 : -1;
		}
	)
{
	data = new meta[capacity]();
	Init(name);
}
Storage::~Storage()
{
	delete[] storage_name;
	delete[] data_path;
	delete[] metadata_path;
	delete[] data;
}
bool Storage::Init(const char* name)
{
	if (!Open(name))
	{
		return Create(name);
	}
	return true;
}
bool Storage::Create(const char* name)
{
	change_repo_name(name);
	if (file_exists(data_path) || file_exists(metadata_path))
	{
		return false;
	}
	file_create(data_path);
	file_create(metadata_path);
	try
	{
		write_metadata();
	}
	catch (ios::failure e)
	{
		perror("Error while writting meta file");
		return false;
	}
	return true;
}
bool Storage::Open(const char* name)
{
	change_repo_name(name);
	if (!file_exists(data_path))
	{
		return false;
	}
	if (!file_exists(metadata_path))
	{
		char* backup = new char[strlen(metadata_path) + 8];
		strcpy(backup, storage_name);
		strcat(backup, "_backup.meta");
		if (rename(backup, metadata_path))
		{
			delete[] backup;
			return false;
		}
		delete[] backup;
	}
	try
	{
		read_metadata();
	}
	catch (ios::failure e)
	{
		perror("Error while reading meta file");
		return false;
	}
	return true;
}
unsigned int* Storage::Store(const unsigned char* pBlock, size_t Size)
{
	if(!pBlock || !Size)
	{
		return 0;
	}
	unsigned int* result = SHA256(pBlock, Size);

	try
	{
		if (hash_exists(result))
		{
			printf("That key already exists!\n");
		}
	}
	catch (ios::failure e)
	{
		perror("Error while reading data");
		delete[] result;
		return 0;
	}
	size_t offset;
	try
	{
		offset = write_block(pBlock, Size, result);
	}
	catch (ios::failure e)
	{
		perror("Error while writting data block");
		delete[] result;
		return 0;
	}
	size_t index = reduce(result);
	while(data[index].size)
	{
		index = (index + prime) % capacity;
	}
	data[index].offset = offset;
	data[index].size = Size + hash_size;
	++num_data;
	alpha = (double)num_data / (double)capacity;
	try
	{
		if (alpha > 0.9)
		{
			expand();
			if (!write_metadata_with_backup())
			{
				// Will leave the block hidden in the file
				delete[] result;
				return 0;
			}
		}
		else
		{
			write_metadata(num_data_bit | num_holes_bit | holes_bit);
			write_single_entry(index);
		}
	}
	catch (ios::failure e)
	{
		perror("Error while writting metadata");
		delete[] result;
		return 0;
	}
	return result;
}
bool Storage::Load(const unsigned int* key, unsigned char*& block, size_t& size)
{
	fstream data_file;
	size_t index = reduce(key);
	unsigned int h[8];
	data_file.exceptions(ios::badbit | ios::failbit);
	try
	{
		data_file.open(data_path, ios::in | ios::binary);
		while (data[index].size)
		{
			data_file.seekg(data[index].offset);
			data_file.read((char*)h, hash_size);
			if (h[0] == key[0] &&
				h[1] == key[1] &&
				h[2] == key[2] &&
				h[3] == key[3] &&
				h[4] == key[4] &&
				h[5] == key[5] &&
				h[6] == key[6] &&
				h[7] == key[7])
			{
				size = data[index].size - hash_size;
				block = new unsigned char[size];
				data_file.read((char*)block, size);
				data_file.close();
				return true;
			}
			else
			{
				index = (index + prime) % capacity;
			}
		}
		data_file.close();
	}
	catch (ios::failure e)
	{
		perror("Error while loading data block");
	}
	return false;
}
bool Storage::Erase(const unsigned int* key)
{
	fstream data_file;
	size_t index = reduce(key);
	unsigned int h[8];
	data_file.exceptions(ios::badbit | ios::failbit);
	try
	{
		data_file.open(data_path, ios::in | ios::binary);
		while (data[index].size)
		{
			data_file.seekg(data[index].offset);
			data_file.read((char*)h, hash_size);
			if (h[0] == key[0] &&
				h[1] == key[1] &&
				h[2] == key[2] &&
				h[3] == key[3] &&
				h[4] == key[4] &&
				h[5] == key[5] &&
				h[6] == key[6] &&
				h[7] == key[7])
			{
				register_hole(data[index]);
				data[index] = { 0, 0 };
				--num_data;
				meta temp;
				size_t new_index;
				for (index = (index + prime) % capacity; data[index].size; index = (index + prime) % capacity)
				{
					temp = data[index];
					data[index] = { 0, 0 };
					data_file.seekg(temp.offset);
					data_file.read((char*)h, hash_size);
					new_index = reduce(h);
					while (data[new_index].size)
					{
						new_index = (new_index + prime) % capacity;
					}
					data[new_index] = temp;
				}
				data_file.close();
				if (!write_metadata_with_backup())
				{
					perror("Failed to erase block");
					return false;
				}
				return true;
			}
			else
			{
				index = (index + prime) % capacity;
			}
		}
		data_file.close();
	}
	catch (ios::failure e)
	{
		perror("Error while erasing data block");
	}
	return false;
}
