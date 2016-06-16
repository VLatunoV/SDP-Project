#include "sha256.h"
#define RR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

static const unsigned int k[64] =
{
	0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
	0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
	0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
	0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
	0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
	0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
	0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
	0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

static inline void parse_block(unsigned int* in_block, unsigned int* ob)
{
	unsigned int sigma0, sigma1, temp1, temp2, maj, ch;
	for (int i = 0; i < 64; ++i)
	{
		sigma1 = RR(ob[4], 6) ^ RR(ob[4], 11) ^ RR(ob[4], 25);
		ch = (ob[4] & ob[5]) ^ (~ob[4] & ob[6]);
		temp1 = ob[7] + sigma1 + ch + k[i] + in_block[i];
		sigma0 = RR(ob[0], 2) ^ RR(ob[0], 13) ^ RR(ob[0], 22);
		maj = (ob[0] & ob[1]) ^ (ob[0] & ob[2]) ^ (ob[1] & ob[2]);
		temp2 = sigma0 + maj;

		ob[7] = ob[6];
		ob[6] = ob[5];
		ob[5] = ob[4];
		ob[4] = ob[3] + temp1;
		ob[3] = ob[2];
		ob[2] = ob[1];
		ob[1] = ob[0];
		ob[0] = temp1 + temp2;
	}
}
unsigned int* SHA256(const unsigned char* data, size_t size)
{
	if(!data)
	{
		return 0;
	}
	bool little_endian;
	unsigned int check = 1;
	little_endian = *(char*)&check == 1;

	unsigned int wv[8];
	unsigned int* result = new unsigned int [8];
	result[0] = 0x6a09e667;
	result[1] = 0xbb67ae85;
	result[2] = 0x3c6ef372;
	result[3] = 0xa54ff53a;
	result[4] = 0x510e527f;
	result[5] = 0x9b05688c;
	result[6] = 0x1f83d9ab;
	result[7] = 0x5be0cd19;
	
	unsigned int w[64];
	size_t i, j;
	
	size_t num_chunks = (size + 9) / 64 + 1; // 1 byte for 0x80 and 8 for the length
	unsigned int num_additional_chunks = size % 64 + 9 > 64 ? 2 : 1;

	// Make an array of pointers to the chunks so that we keep the data linear with minimal copying
	unsigned char **chunks = new unsigned char* [num_chunks];
	
	for(i = 0; i < num_chunks - num_additional_chunks; ++i)
	{
		chunks[i] = (unsigned char*)(data + i * 64);
	}
	chunks[i] = new unsigned char[64]();
	if(num_additional_chunks == 2)
	{
		chunks[i + 1] = new unsigned char[64]();
	}
	j = 0;
	size_t index = i * 64 + j;
	while (index < size)
	{
		chunks[i][j] = data[index]; // here j cannot increase over 63
		++j;
		index = i * 64 + j;
	}

	chunks[i + j / 64][j % 64] = 0x80; // Append bit '1' to the message. Here j could be 64
	if (j >= 64 - sizeof(size)) ++i; // If there are less bits than sizeof(size) then move to the next block
	j = 64 - sizeof(size);
	size <<= 3;
	if(little_endian)
	{
		while(j < 64)
		{
			chunks[i][j] = (unsigned char) *(((unsigned char*)&size) + (63 - j));
			++j;
		}
	}
	else
	{
		*((size_t*)(chunks[i] + j)) = size;
	}
	
	for(i = 0; i < num_chunks; ++i)
	{
		for(j = 0; j < 8; ++j)
		{
			wv[j] = result[j];
		}
		for(j = 0; j < 16; ++j)
		{
			if (little_endian)
			{
				w[j] = ((unsigned int)(chunks[i][j * 4]) << 24) +
					   ((unsigned int)(chunks[i][j * 4 + 1]) << 16) +
					   ((unsigned int)(chunks[i][j * 4 + 2]) << 8) +
					   ((unsigned int)(chunks[i][j * 4 + 3]));
			}
			else
			{
				w[j] = ((unsigned int*)chunks[i])[j];
			}
		}
		for(; j < 64; ++j)
		{
			w[j] = w[j - 16] + w[j - 7] +
				((w[j - 15] >> 3) ^ RR(w[j - 15], 7) ^ RR(w[j - 15], 18)) +
				((w[j - 2] >> 10) ^ RR(w[j - 2], 17) ^ RR(w[j - 2], 19));
		}
		parse_block(w, wv);
		for(j = 0; j < 8; ++j)
		{
			result[j] += wv[j];
		}
	}
	
	for(i = num_chunks - num_additional_chunks; i < num_chunks; ++i)
	{
		delete[] chunks[i];
	}
	delete[] chunks;
	
	return result;
}
