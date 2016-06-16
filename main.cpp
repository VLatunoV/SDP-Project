#include <iostream>
#include "storage.h"
#include "sorted_array.h"

using namespace std;

void print_inst()
{
	cout << "1. Change storage\n";
	cout << "2. Store\n";
	cout << "3. Load\n";
	cout << "4. Erase\n";
	cout << "-1. Return\n";
	cout << ">> ";
}

int main()
{
	Storage repo;
	int choose;
	char cmd[1024];
	unsigned int* hash;
	unsigned int key[8];

	cout << "Open storage: ";
	cin.getline(cmd, 1024);

	if (!repo.Init(cmd))
	{
		cout << "Failed to open\n";
		return 0;
	}

	while (true)
	{
		print_inst();
		cin >> choose;
		switch (choose)
		{
		case 1:
			cout << "Open storage: ";
			do
			{
				cin.getline(cmd, 1024);
			} while (cmd[0] == 0);
			if (!repo.Init(cmd))
			{
				cout << "Could not open that storage.\n";
				exit(0);
			}
			break;
		case 2:
			do
			{
				cout << "Message: ";
				do
				{
					cin.getline(cmd, 1024);
				} while (cmd[0] == 0);
				if (!strcmp(cmd, "-1"))
				{
					break;
				}
				fstream file;
				file.open("hashes.txt", ios::out | ios::app);
				hash = repo.Store((const unsigned char*)cmd, strlen(cmd));
				if (hash)
				{
					file << hash[0] << " "
						<< hash[1] << " "
						<< hash[2] << " "
						<< hash[3] << " "
						<< hash[4] << " "
						<< hash[5] << " "
						<< hash[6] << " "
						<< hash[7] << endl;
				}
				file.close();
			} while (true);
			break;
		case 3:
			unsigned char *block;
			cout << "Key: ";
			cin >> key[0] >> key[1] >> key[2] >> key[3] >> key[4] >> key[5] >> key[6] >> key[7];
			size_t l;
			if (repo.Load(key, block, l))
			{
				for (size_t i = 0; i < l; ++i)
				{
					cout << (char)block[i];
				}
				cout << "\n";
				delete[] block;
			}
			else
			{
				cout << "That key does not exist\n";
			}
			break;
		case 4:
			cout << "Key: ";
			cin >> key[0] >> key[1] >> key[2] >> key[3] >> key[4] >> key[5] >> key[6] >> key[7];
			if (!repo.Erase(key))
			{
				cout << "That key does not exist\n";
			}
			else
			{
				cout << "Successfully erased\n";
			}
			break;
		default:
			break;
		}
	}
	return 0;
}
