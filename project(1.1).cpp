/**********************************************
	Ryan Rady
	Homework 02

	The second part of this project requires that you implement a simple file system. In particular, you are 	going to write the software which which will handle dynamic file management. This part of the project will require you to implement the class Filesys along with member functions. In the description below, FAT refers to the File Allocation Table and ROOT refers to the Root Directory. 
**********************************************/

#include <string>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <iomanip>
#include <fstream>
#include <vector>
#include <stdio.h>

using namespace std;

/**********************************************
	Sdisk
**********************************************/
class Sdisk
{
public:
	Sdisk(string diskname, int numberofblocks, int blocksize);
	int getblock(int blocknumber, string& buffer);
	int putblock(int blocknumber, string buffer);
	int getblocksize() { return blocksize; }
	int getnumberofblocks() { return numberofblocks; }
protected:
	string diskname;        // file name of pseudo-disk
	int numberofblocks;     // number of blocks on disk
	int blocksize;          // block size in bytes
};

//constructor
Sdisk::Sdisk(string diskname, int numberofblocks, int blocksize)
{
	string user_input;

	this->numberofblocks = numberofblocks;
	this->blocksize = blocksize;

	//test if filename exists.
	bool exist;

	ifstream f(diskname.c_str());
	if (f.good()) {
		f.close();
		exist = true;
		cout << "the file " << diskname << " exists!\n";
	} else { 
		f.close();
		exist = false;
		cout << "the file " << diskname << " does not exist!\n";
	}
	
	if (exist) {
		cout << "Would you like to format the sdisk? : ";
		cin >> user_input;
		while (user_input != "y" && user_input != "Y" && user_input != "n" && user_input != "N") {
			cout << "Invalid entry, Please enter (Y or N): ";
			cin >> user_input;
		}
		if (user_input == "Y" || user_input =="y") {
			exist = false;
		} else {
			this->diskname = diskname;
		}
	}

	if (!exist) {
		//used to format the disk.
		cout << "Creating a new Sdisk..... \n";
		cout << " Number of blocks: " << numberofblocks << "\n";
		cout << " Block size: " << blocksize << "\n";
		fstream cfile;
		cfile.open(diskname.c_str(), fstream::out);
		for (int i = 0; i < numberofblocks; i++){
			for(int j = 0; j < blocksize-1; j++) {
				cfile << "#";
			}
		cfile << "\n";
		}
		cout << "New Sdisk successfully created!\n";
	cfile.close();
	}
}

//returns a block into the &input_buffer
int Sdisk::getblock(int input_blocknumber, string& input_buffer)
{

	fstream infile;
	string temp_string;
	
	infile.open(this->diskname.c_str());
	infile.seekg((input_blocknumber*blocksize)+input_blocknumber);
	infile >> input_buffer;
	infile.close();

	//for testing purposes.
	//cout << "\ngetblock()\n";
	//cout << " getblock() - diskname: " << diskname << "\n";
	//cout << " getblock() - input_blocknumber: " << input_blocknumber << "\n";
	//cout << " getblock() - Input_buffer: " << input_buffer << "\n";
	//system("PAUSE");

	return 0;
}

//places input_buffer to the file.
int Sdisk::putblock(int input_blocknumber, string input_buffer)
{
	fstream ofile;
	string temp_string;

	ofile.open(diskname.c_str(), ios::in|ios::out);
	ofile.seekp((input_blocknumber*blocksize)+input_blocknumber, ios::beg);
	ofile << input_buffer;
	ofile.close();

	//for testing purposes.
	//cout << "\nputblock()\n";
	//cout << " putblock() - diskname: " << diskname << "\n";
	//cout << " putblock() - input_blocknumber: " << input_blocknumber << "\n";
	//cout << " putblock() - Input_buffer: " << input_buffer << "\n";
	//system("PAUSE");

	return 0;
}

/**********************************************
FileSystem:
The module creates a file system on the sdisk by creating an intial FAT and ROOT. A file system on the disk will have the following segments: 
**********************************************/

class Filesys: public Sdisk
{
public:
	Filesys(string diskname, int numberofblocks, int blocksize);
	int fsclose();
	void fssynch();
	int newfile(string file);
	int rmfile(string file);
	int getfirstblock(string file);
	int addblock(string file, string buffer);
	int delblock(string file, int blocknumber);
	int readblock(string file, int blocknumber, string& buffer);
	int writeblock(string file, int blocknumber, string buffer);
	int nextblock(string file, int blocknumber);
private :
	int rootsize;          			// maximum number of entries in ROOT
	int fatsize;            		// number of blocks occupied by FAT
	vector<string> filename;   		// filenames in ROOT
	vector<int> firstblock; 		// firstblocks in ROOT
	vector<int> fat;             	// FAT
};

/************************************************************
	global block function
************************************************************/
vector<string> block(string inputstring, int stringsize) {
	vector<string> outputvector;
	string tempstring;
	int inc_stringsize;

	inc_stringsize = ((inputstring.length() + (stringsize -1)) / stringsize);

	outputvector.push_back(inputstring.substr(0,stringsize));

	for (int i = 1; i < inc_stringsize -1; i++)
		outputvector.push_back(inputstring.substr(stringsize * i,stringsize));
	
	if (inc_stringsize > 1)
		outputvector.push_back(inputstring.substr(stringsize * (inc_stringsize - 1)));
	
	return outputvector;
}

/**********************************************
This constructor reads from the sdisk created with "filename" and either opens the existing file system on the disk or creates one for an empty disk. Recall the sdisk is a file of characters which we will manipulate as a raw hard disk drive. This file is logically divided up into number_of_blocks many blocks where each block has block_size many characters. Information is first read from block 1 to determine if an existing file system is on the disk. If a filesystem exists, it is opened and made available. Otherwise, the file system is created. 
**********************************************/
Filesys::Filesys(string diskname, int numberofblocks, int blocksize):Sdisk(diskname, numberofblocks, blocksize)
{
	//there are 2 cases:
	//	-filesystem exists
	//	-filesystem does not exist.

	string buffer, user_input;
	string compstring = "###";

	this->diskname = diskname;
	this->numberofblocks = numberofblocks;
	this->blocksize = blocksize;

	//for testing purposes.
	//cout << "\nFilesys()\n";
	//cout << " Filesys() - diskname: " << diskname << "\n";
	//cout << " Filesys() - number of blocks: " << numberofblocks << "\n";
	//cout << " Filesys() - blocksize: " << blocksize << "\n";

	//first block on disk should contain the filesystem. 
	getblock(1, buffer);

	/*
	fstream infile;
	string temp_string;
	
	infile.open(diskname.c_str());
	infile.seekg((1*blocksize)+1);
	infile >> buffer;
	infile.close();

	//getblock wont work.
	*/
	if(buffer[1] != compstring[1]) { //filesystem exists.
		cout << "Error in filesystem constructor: Filesystem already exists!\n";
		cout << "Would you like to format the filesystem? (Y or N) *This will destroy all data: ";
		cin >> user_input;
		while (user_input != "y" && user_input != "n" && user_input != "Y" && user_input != "N") {
			cout <<"\n Please input (Y or N): ";
			cin >> user_input;
		}
	} 
	if (buffer[1] == compstring[1] || user_input =="Y" || user_input =="y") { 
		//no filesystem, build and write it to Sdisk.
		rootsize = ((getblocksize()-5) / 11);

		for (int i = 0; i < rootsize; i++) {
			filename.push_back("xxxxx");
			firstblock.push_back(0);
		}


		fatsize = getnumberofblocks(); //this needs to be corrected. 
		int firstdatablock = 1 + ((fatsize - 1) / blocksize );

		fat.push_back(firstdatablock);
		fat.push_back(-1); // first entry of the FAT reserved for root. 

		//create a free block list.
		for (int i = firstdatablock; i < getblocksize(); i++)
			fat.push_back(i+1);

		fat[getblocksize() - 1] = 0;
		fssynch();
	} 
	if (user_input == "n" || user_input == "N") {
		cout << "You have chosen not to reformat the filesystem.\n";
		cout << "The pre-existing filesystem may not work with your Opersating System!\n";
	}
}

//This module writes FAT and ROOT to the sdisk. It should be used every time FAT and ROOT are modified. 
void Filesys::fssynch()
{
	cout << "synched\n";
	string buffer;
	rootsize = ((getblocksize()-5)/11);
	ostringstream rootstream;
	//first bit of data in the root is reserved for the rootsize. 
	rootstream << rootsize << " ";
	
	//synch the root data structures.
	for (int i = 0; i < rootsize; i++)
		rootstream << filename[i] << " " << firstblock[i] << " ";

	buffer = rootstream.str();
	
	vector<string> blocks = block(buffer, getblocksize());
	putblock(1,blocks[0]);

	//synch the F.A.T.
	ostringstream fatstream;
	
	for (int i = 0; i < fat.size(); i++)
		fatstream << fat[i] << " ";
	
	buffer = fatstream.str();
	blocks = block(buffer, getblocksize());

	for (int i = 0; i < blocks.size(); i++)
		putblock(i+2, blocks[i]);

	return;
}

//This module writes FAT and ROOT to the sdisk (closing the sdisk).
int Filesys::fsclose()
{
	fssynch();
	//close sdisk.
	return 0;
}

//This function adds an entry for the string file in ROOT with an initial first block of 0 (empty). It returns error codes of 1 if successful and 0 otherwise (no room or file already exists). 
int Filesys::newfile(string file)
{
	//return 1 if successful otherwise returns 0
	for (int i = 0; i < filename.size(); i++) {
		string tempfile(filename[i]); //convert char* to string.
		if (tempfile == file){
			cout << "Error in File System (newfile) - The file " << file << " already exists!" << "\n";
			return 0;
		}
	}
	
	for (int i = 0; i < filename.size(); i++) {
		string tempfile(filename[i]); //convert char* to string.
		if (tempfile == "xxxxx") {
			filename[i] = file;
			firstblock[i] = 0;
			fssynch();
			return 1;
		}
	}

	cout << "Error in File System (newfile) - There is no room for file: " << file << "\n";
	return 0;
}

//This function removes the entry file from ROOT if the file is empty (first block is 0). It returns error codes of 1 if successful and 0 otherwise (not empty or file does not exist). 
int Filesys::rmfile(string file)
{
	for (int i=1; i < filename.size(); i++) {
		if (filename[i] == file){
			if (firstblock[i] == 0){
				filename[i] = "xxxxx";
				fssynch();
				return 1;
			} else {
				cout << "Error in File System (rmfile) - Conditions not met. The firstblock of ";
				cout << file << " is not zero" << "\n";
				return 0;
			}
		}
	}
	cout << "Error in File System (rmfile) - The file " << file << " does not exist!" << "\n";
	return 0;
}

//This function returns the block number of the first block in file. It returns the error code of -1 if the file does not exist. 
int Filesys::getfirstblock(string file)
{
	
	//cout << "getfirstblock - diskname: " << diskname << " file: " << file << "\n"; //what do i do here.....
	//system("PAUSE");
	for (int i = 0; i < filename.size(); i++) {
		if (filename[i] == file)
			return firstblock[i];
	}
	cout << "Error in File System (getfirstblock) - The file " << file << " does not exist!" << "\n";
	return -1;
}

//This function adds a block of data stored in the string buffer to the end of file F and returns the block number. It returns error code of 1 if successful, 0 if the file does not exist, and returns -1 if there are no available blocks (file system is full!). 
int Filesys::addblock(string file, string buffer)
{
	int i = 0;

	for (i = 0; i < filename.size(); i++) {
		if (filename[i] == file)
		break;
	}
	
	if (i == filename.size()) {
		cout << "Error in File System (addblock) - The file " << file << " does not exist!" << "\n";
		return -1;
	}

	int allocate = fat[0];
	if (allocate == 0) {	
		cout << "Error in File System (addblock) - no room in FAT" << "\n";
		return -1;
	}

	fat[0] = fat[fat[0]];
	fat[allocate] = 0;
	
	if (firstblock[i] == 0)
		firstblock[i] = allocate;
	else {
		int blockid = firstblock[i];
		while (fat[blockid] !=0) {
			blockid = fat[blockid];
		}
		fat[blockid] = allocate;
	}
	
	putblock(allocate, buffer);
	fssynch();
	return allocate;
}

//The function removes block numbered blocknumber from file and returns an error code of 1 if successful and 0 otherwise.
int Filesys::delblock(string file, int blocknumber)
{
	int blockid = getfirstblock(file);

	if (blockid == -1) {
		cout << "Error in File System (delblock) - The file " << file << " does not exist!" << "\n";
		return 0;
	}

	if (blockid == blocknumber) {
		for (int i = 0; i < filename.size()-1; i++) {
			if (filename[i] == file)
				firstblock[i] = fat[blockid];
		}
	} else {
		while (fat[blockid] != 0 && fat[blockid] != blocknumber) {
			blockid = fat[blockid];
		}

		if (fat[blockid] == blocknumber)
			fat[blockid] = fat[blocknumber];
	
		if (fat[blockid] == 0) {
			cout << "Error in File System (delblock) - The file " << file;
			cout << " does not contain block "<< blocknumber << "\n";
			return 0;
		}
	}

	fat[blocknumber] = fat[0];
	fat[0] = blocknumber;
	fssynch();
	return 1;
}

//gets block numbered blocknumber from file and stores the data in the string buffer. It returns an error code of 1 if successful and 0 otherwise.
int Filesys::readblock(string file, int blocknumber, string& buffer)
{
	int blockid = getfirstblock(file);
	if (blockid == -1) {
		cout << "Error in File System (readblock) - The file " << file << "does not exist!\n";
		return 0;
	}

	bool isfound = false;
	
	while (fat[blockid] != 0) {
		blockid = fat[blockid];
		if (fat[blockid] == blocknumber) {
			isfound = true;
			break;
		}
	}

	if (!isfound) {
		cout << "Error in File System (readblock) - The block " << blocknumber << "not in file " << file << "\n";
		return 0;
	}

	getblock(blocknumber, buffer);

	return 1;

}

//writes the buffer to the block numbered blocknumber in file. It returns an appropriate error code. 
int Filesys::writeblock(string file, int blocknumber, string buffer)
{
	int blockid = getfirstblock(file);
	if (blockid == -1) {
		cout << "Error in File System (writeblock) - The file " << file << "does not exist!\n";
		return 0;
	}

	bool isfound = false;
	
	while (fat[blockid] != 0) {
		blockid = fat[blockid];
		if (fat[blockid] == blocknumber) {
			isfound = true;
			break;
		}
	}

	if (!isfound) {
		cout <<"Error in File System (writeblock) - The block " << blocknumber << "not in file " << file << "\n";
		return 0;
	}

	putblock(blocknumber, buffer);

	return 1;
}

//returns the number of the block that follows blocknumber in file. It will return 0 if blocknumber is the last block and -1 if some other error has occurred (such as file is not in the root directory, or blocknumber is not a block in file.) 
int Filesys::nextblock(string file, int blocknumber)
{
	//need to check if blocknumber == 0;

	int blockid = getfirstblock(file);
	if (blockid == -1) {
		cout << "Error in File System (nextblock) - The file " << file << "does not exist!\n";
		return -1;
	}

	bool isfound = false;
	
	while (fat[blockid] != 0) {
		blockid = fat[blockid];
		if (fat[blockid] == blocknumber) {
			isfound = true;
			break;
		}
	}

	if (!isfound) {
		cout <<"Error in File System (nextblock) - The block " << blocknumber << "not in file " << file << "\n";
		return 0;
	}


	return fat[blockid];
}

/************************************************************
	Main
************************************************************/
// You can use this to test your Filesys class 

int main()
{
  //Sdisk disk1("disk1",256,128);
  Filesys fsys("disk1",256,128);
  //system("PAUSE");
  fsys.newfile("file1");
  //system("PAUSE");
  fsys.newfile("file2");
 // system("PAUSE");

  string bfile;

  for (int i=1; i<=1024; i++)
     {
       bfile+="1";
     }

  vector<string> blocks=block(bfile,128); 

  int blocknumber=0;

  for (int i=0; i < blocks.size(); i++)
     {
		 blocknumber=fsys.addblock("file1",blocks[i]);
     }

  fsys.delblock("file1",fsys.getfirstblock("file1"));

  for (int i=1; i<=2048; i++)
     {
       bfile+="2";
     }

  blocks=block(bfile,128);

  for (int i=0; i < blocks.size(); i++)
     {
       blocknumber=fsys.addblock("file2",blocks[i]);
     }

  //blocknumber is being update by addblock() which returns allocate.
  //since we are returning allocate after adding blocks, allocate will always have a fat = 0;
  //we cannot delete the entry in fat that desginates the end of that file (fat = 0)
  //therefore this next line will output an error.
  fsys.delblock("file2",blocknumber);

  //but atleast we know it works. 

  system("PAUSE");
}


