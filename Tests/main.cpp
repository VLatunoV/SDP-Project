#include "unit.h"
#include "..\sha256.h"
#include "..\storage.h"
#include "..\sorted_array.h"
#include "..\storage.h"
#include <cstdlib>
#include <time.h>

UTest(Sorting_Properly)
{
	SortedArray<int> Test_List(
		[](const int& x, const int& y) -> int
		{
			return x - y;
		}
	);
	srand(time(0));
	for (unsigned int i = 0; i <= 1000; ++i)
	{
		Test_List.Add(rand());
	}
	for (unsigned int i = 0; i < 1000; ++i)
	{
		AssertLessEq(Test_List[i], Test_List[i + 1]);
	}
	return true;
}
UTest(Hashing_Properly)
{
	unsigned int h_true[] = { 1156609165, 1538935238, 1387974122, 850032726, 2430490107, 2374609677, 1420584058, 1719285282 };
	unsigned int* h;
	const char* t = "osdfi sdojrghyejyttukFGYJGFJdYJjh4%6y6j7*#45Gh*k8OnbtdgYHb$56H*Bh8^n e75gW46v^b7g54bh@%654HbiB8hjNTWgWv54^w%$5qq5$G^q546Q%6Qvg$%BghtHYDhbQ45G^dIOf894 DS34ds fsd %$$fdst%srdg&&^ggr34ggh35F#$G$g34ghffgfdfHFgDsfDf hg ghfd bwtGrt erTrEh1342Guhb89kuh";
	h = SHA256((const unsigned char*)t, strlen(t));
	for (unsigned int i = 0; i < 8; ++i)
	{
		AssertEqual(h[i], h_true[i]);
	}
	delete[] h;
	return true;
}
UTest(Storage_Initialize)
{
	Storage S;
	fclose(fopen("test.data", "w"));
	AssertFalse(S.Open("test"));
	AssertFalse(S.Create("test"));
	remove("test.data");
	AssertTrue(S.Init("test"));
	remove("test.data");
	remove("test.meta");
	return true;
}
UTest(Storage_Initialize_from_backup)
{
	Storage S("test");
	rename("test.meta", "test_backup.meta");
	AssertTrue(S.Open("test"));
	remove("test_backup.data");
	remove("test.data");
	remove("test.meta");
	return true;
}
UTest(Storage_Function)
{
	Storage S("test");
	const unsigned int num_blocks = 5000;
	const unsigned int max_block_size = 100000;
	unsigned char* blocks[num_blocks] = {};
	unsigned int* hashes[num_blocks] = {};
	size_t sizes[num_blocks] = {};
	srand(time(0));
	for (unsigned int i = 0; i < num_blocks; ++i)
	{
		do {
			sizes[i] = rand() % max_block_size;
		} while (sizes[i] == 0);
		blocks[i] = new unsigned char[sizes[i]];
		for (size_t q = 0; q < sizes[i]; ++q)
		{
			blocks[i][q] = (unsigned char)rand();
		}
		hashes[i] = S.Store(blocks[i], sizes[i]);
		AssertNotEq(hashes[i], 0);
	}
	for (unsigned int i = 0; i < num_blocks; ++i)
	{
		if (i % 2)
		{
			AssertTrue(S.Erase(hashes[i]));
		}
	}
	unsigned char* read_block = 0;
	for (unsigned int i = 0; i < num_blocks; ++i)
	{
		if (i % 2)
		{
			AssertFalse(S.Load(hashes[i], read_block, sizes[i]));
		}
		else
		{
			AssertTrue(S.Load(hashes[i], read_block, sizes[i]));
			for (size_t q = 0; q < sizes[i]; ++q)
			{
				AssertEqual(blocks[i][q], read_block[q]);
			}
			delete[] read_block;
		}
	}
	const unsigned int add_more = 1000;
	unsigned int* more_hashes[add_more] = {};
	unsigned char* more_blocks[add_more] = {};
	size_t more_sizes[add_more] = {};
	for (unsigned int i = 0; i < add_more; ++i)
	{
		do {
			more_sizes[i] = rand() % max_block_size;
		} while (more_sizes[i] == 0);
		more_blocks[i] = new unsigned char[more_sizes[i]];
		for (size_t q = 0; q < more_sizes[i]; ++q)
		{
			more_blocks[i][q] = (unsigned char)rand();
		}
		more_hashes[i] = S.Store(more_blocks[i], more_sizes[i]);
		AssertNotEq(more_hashes[i], 0);
	}
	for (unsigned int i = 0; i < add_more; ++i)
	{
		AssertTrue(S.Load(more_hashes[i], read_block, more_sizes[i]));
		for (size_t q = 0; q < more_sizes[i]; ++q)
		{
			AssertEqual(more_blocks[i][q], read_block[q]);
		}
		delete[] read_block;
	}
	for (unsigned int i = 0; i < num_blocks; ++i)
	{
		delete[] hashes[i];
		delete[] blocks[i];
	}
	for (unsigned int i = 0; i < add_more; ++i)
	{
		delete[] more_hashes[i];
		delete[] more_blocks[i];
	}
	remove("test.data");
	remove("test.meta");
	return true;
}
int main()
{
	Test::RunTests();
	return 0;
}
