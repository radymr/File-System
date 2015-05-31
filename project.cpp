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
class Sdisk {
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
Sdisk::Sdisk(string diskname, int numberofblocks, int blocksize) {
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
		//removed the option to not format disk. 
		cout << diskname << " will be formatted and the previous sdisk is being clobbered!...\n";
		exist = false;
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
int Sdisk::getblock(int input_blocknumber, string& input_buffer) {
	fstream infile;
	string temp_string;
	
	infile.open(this->diskname.c_str());
	infile.seekg((input_blocknumber*blocksize)+input_blocknumber);
	getline(infile, input_buffer);
	infile.close();

	return 0;
}

//places input_buffer to the file.
int Sdisk::putblock(int input_blocknumber, string input_buffer) {
	fstream ofile;
	string temp_string;

	ofile.open(diskname.c_str(), ios::in|ios::out);
	ofile.seekp((input_blocknumber*blocksize)+input_blocknumber, ios::beg);
	ofile << input_buffer;
	ofile.close();

	return 0;
}

/**********************************************
FileSystem:
The module creates a file system on the sdisk by creating an intial FAT and ROOT. A file system on the disk will have the following segments: 
**********************************************/

class Filesys: public Sdisk {
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
	vector<string> ls();
protected :
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
Filesys::Filesys(string diskname, int numberofblocks, int blocksize):Sdisk(diskname, numberofblocks, blocksize) {
	//there are 2 cases:
	//	-filesystem exists
	//	-filesystem does not exist.

	string buffer;
	string compstring = "###";

	this->diskname = diskname;
	this->numberofblocks = numberofblocks;
	this->blocksize = blocksize;

	//first block on disk should contain the filesystem. 
	getblock(1, buffer);

	if(buffer[1] != compstring[1]) { //filesystem exists.
		cout << "Error in filesystem constructor: Filesystem already exists!\n";
		cout << "filesystem will now be formatted... \n";
		buffer[1] = compstring[1];
	} 
	if (buffer[1] == compstring[1]) { 
		//no filesystem, build and write it to Sdisk.
		rootsize = ((getblocksize()-5) / 11);

		for (int i = 0; i < rootsize; i++) {
			filename.push_back("xxxxx");
			firstblock.push_back(0);
		}


		fatsize = getnumberofblocks(); //this needs to be corrected. 
		int firstdatablock = (1 + ((fatsize - 1) / blocksize ));

		fat.push_back(firstdatablock);
		fat.push_back(-1); // first entry of the FAT reserved for root. 

		//create a free block list.
		for (int i = firstdatablock; i < getnumberofblocks(); i++)
			fat.push_back(i+1);

		fat[getnumberofblocks() - 1] = 0;

		stringstream ss;
		for (int i = 0; i < fat.size(); i++)
			ss << i;
		string str = ss.str();
		vector<string> tempbuff = block(str, getblocksize());

		for (int i = 0; i < tempbuff.size()+1; i++) {
			fat[0] = fat[0] + 1;
			fat[2+i] = -1;
		}
		fssynch();
	} 

}

//This module writes FAT and ROOT to the sdisk. It should be used every time FAT and ROOT are modified. 
void Filesys::fssynch() {
	//cout << "synched\n";
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
int Filesys::fsclose() {
	fssynch();
	//close sdisk.
	return 0;
}

//This function adds an entry for the string file in ROOT with an initial first block of 0 (empty). It returns error codes of 1 if successful and 0 otherwise (no room or file already exists). 
int Filesys::newfile(string file) {
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
int Filesys::rmfile(string file) {
	for (int i=0; i < filename.size(); i++) {
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
int Filesys::getfirstblock(string file) {
	for (int i = 0; i < filename.size(); i++) {
		if (filename[i] == file)
			return firstblock[i];
	}
	cout << "Error in File System (getfirstblock) - The file " << file << " does not exist!" << "\n";
	return -1;
}

//This function adds a block of data stored in the string buffer to the end of file F and returns the block number. It returns error code of 1 if successful, 0 if the file does not exist, and returns -1 if there are no available blocks (file system is full!). 
int Filesys::addblock(string file, string buffer) {
	int i = 0;

	for (i = 0; i < filename.size(); i++) {
		if (filename[i] == file)
		break;
	}
	
	if (i == filename.size() && i > 1) {
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
int Filesys::delblock(string file, int blocknumber) {
	string buffer;
	for (int i = 0; i < getblocksize(); i++)
		buffer += "#";

	int blockid = getfirstblock(file);

	if (blockid == -1) {
		cout << "Error in File System (delblock) - The file " << file << " does not exist!" << "\n";
		return 0;
	}

	if (blockid == blocknumber) {
		for (int i = 0; i < filename.size(); i++) {
			if (filename[i] == file)
				firstblock[i] = fat[blockid];
				putblock(blockid, buffer);
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
int Filesys::readblock(string file, int blocknumber, string& buffer) {
	bool isfound = false;
	int blockid = getfirstblock(file);
	if (blockid == -1) {
		cout << "Error in File System (readblock) - The file " << file << "does not exist!\n";
		return 0;
	}

	
	while(!isfound && blockid !=0) {
		if(blockid == blocknumber)
			isfound = true;
		else
			blockid = nextblock(file, blockid);
	}

	if (!isfound) {
		cout << "Error in File System (readblock) - The block " << blocknumber << " not in file " << file << "\n";
		return 0;
	}

	getblock(blocknumber, buffer);

	return 1;

}

//writes the buffer to the block numbered blocknumber in file. It returns an appropriate error code. 
int Filesys::writeblock(string file, int blocknumber, string buffer) {
	bool isfound = false;
	int blockid = getfirstblock(file);

	if (blockid == -1) {
		cout << "Error in File System (writeblock) - The file " << file << "does not exist!\n";
		return 0;
	}

	if (blockid == blocknumber)
		isfound = true;

	if (!isfound) {
		cout <<"Error in File System (writeblock) - The block " << blocknumber << "not in file " << file << "\n";
		return 0;
	}

	putblock(blocknumber, buffer);

	return 1;
}

//returns the number of the block that follows blocknumber in file. It will return 0 if blocknumber is the last block and -1 if some other error has occurred (such as file is not in the root directory, or blocknumber is not a block in file.) 
int Filesys::nextblock(string file, int blocknumber) {
	//need to check if blocknumber == 0;

	bool isfound = false;
	int blockid = getfirstblock(file);

	if (blockid == -1) {
		cout << "Error in File System (nextblock) - The file " << file << "does not exist!\n";
		return -1;
	} 

	if (blockid == blocknumber)
		isfound = true;

	while(blockid != 0 && isfound == false) {
		if (blockid == blocknumber) {
			isfound = true;
			break;
		} else {
			blockid = fat[blockid];
		}
	}

	if (!isfound) {
		cout <<"Error in File System (nextblock) - The block " << blocknumber << " not in file " << file << "\n";
		return 0;
	}

	return fat[blockid];
}

// This function is part of the Filesys class
// Prototype: vector<string> ls();

vector<string> Filesys::ls() { 
vector<string> flist;
	for (int i=0; i<filename.size(); i++) {
		if (filename[i] != "xxxxx") {
			flist.push_back(filename[i]);
		}
	}
return flist;
}

/************************************************************
	 This laboratory is designed to get you started with the the third part of the project which involves writing a shell. 
************************************************************/
class Shell: public Filesys {
public:
	Shell(string diskname, int blocksize, int numberofblocks);
	int dir();// lists all files
	int add(string file);// add a new file using input from the keyboard
	int del(string file);// deletes the file
	int type(string file);//lists the contents of file
	int copy(string file1, string file2);//copies file1 to file2
};

Shell::Shell(string diskname, int numberofblocks, int blocksize):Filesys(diskname, numberofblocks, blocksize) {
	this->diskname = diskname;
	this->blocksize = blocksize;
	this->numberofblocks = numberofblocks;
}

// dir lists files in the class Shell
// Prototype: int Shell::dir() 
int Shell::dir() {
	cout << "Displaying current directory....\n";
	vector<string> flist=ls();
	if (flist.size() == 0)
		return 0;
	for (int i=0; i<flist.size(); i++) {
		cout << flist[i] << endl;
	}
	return 0;
}

int Shell::add(string file) {
	int ecode = newfile(file);
	if (ecode < 1) {
		return 0;
	}
	cout << "Enter data followed by a '^': ";
	string buff;
	char x;
	cin.get(x);
	while (x != '^') {
		buff += x;
		cin.get(x);
	}
	buff += "\n";

	vector<string> blocks = block(buff, getblocksize());
	for(int i = 0; i < blocks.size(); i++ ) {
		int blockid = addblock(file, blocks[i]);
	}
	fssynch();
	cout << "\n";
	return 1;
}

int Shell::del(string file) {
	//returns -1 if error occurs.
	cout << "Deleting file: " << file << "\n";
	int iter = getfirstblock(file);
	if (iter == 0) {
		cout << "Error in Shell del: File is empty! \n";
		return 0;
	}
	if (iter == -1) {
		cout << "Error in Shell del: file does not exist! \n";
		return -1;
	}
	int trash = 0;

	while (iter != 0) {
		trash = iter;
		iter = nextblock(file, iter);
		delblock(file, trash);
	}

	rmfile(file);
	cout << file << " has been removed..\n";
	return 0;
}

int Shell::type(string file) {
	//returns -1 if error occurs.
	cout << "contents of file " << file << ": \n";
	int iter = getfirstblock(file);
	string buffer;

	if (iter == 0) {
		cout << "Error in Shell del: File is empty! \n";
		return 0;
	}

	if (iter == -1) {
		cout << "Error in Shell del: file does not exist! \n";
		return -1;
	}

	while (iter != 0) {
		readblock(file, iter, buffer);
		cout << " " << buffer << "\n";
		iter = nextblock(file, iter);
	}
	cout << "\n";
	return 0;
}

int Shell::copy(string file1, string file2) {
	//returns -1 if error occurs.
	cout << "Copying " << file1 << "\n";
	int iter1 = getfirstblock(file1);
	newfile(file2);
	int iter2 = getfirstblock(file2);
	string buffer;

	if (iter1 == 0) {
		cout << "Error in Shell del: File is empty! \n";
		return 0;
	}

	if (iter1 == -1) {
		cout << "Error in Shell del: file does not exist! \n";
		return -1;
	}

	while (iter1 != 0) {
		readblock(file1, iter1, buffer);
		buffer += "/n";
		addblock(file2, buffer);
		iter1 = nextblock(file1, iter1);
	}
	return 0;
}
/************************************************************
	table
************************************************************/
class Table:public Filesys {
public:
	Table(string diskname, int numberofblocks, int blocksize);	
	int Build_Table(string input_file);
	int Search(string value);
	void tsynch();
private:
	string flatfile;
	string indexfile;
	int IndexSearch(string value);
	int numberofrecords;
};//, string flatfile, string indexfile

Table::Table(string diskname, int numberofblocks, int blocksize):Filesys(diskname, numberofblocks, blocksize) {
	this->diskname = diskname;
	this->numberofblocks = 256;
	this->blocksize = 128;
	ofstream openflat;
	openflat.open ("flatfile");
	openflat.close();
	ofstream openindexfile;
	openindexfile.open("indexfile");
	openindexfile.close();
}

int Table::Build_Table(string input_file) {
	//build the files flatfile and indexfile
	//build sdisk with 128 byte blocks, 256 blocks.
	//using 128 bytes on 256 block filesystem.
	string tempf = "flatfile";
	string tempi = "indexfile";
	ifstream infile;
	infile.open(input_file.c_str());
	if (!infile.is_open())
		cout << "an error occured opening file " << input_file.c_str() << "\n";

	string r;
	ostringstream outstream;

	getline(infile, r);

	newfile(tempf);
	newfile(tempi);

	while (infile.good()) {
		vector<string> blocks = block(r, getblocksize());
		int b = addblock(tempf,blocks[0]);
		string k = r.substr(0,5);
		outstream << b << " " << k << " ";
		getline(infile,r);
	}

	string buffer = outstream.str();

	vector<string> blocks = block(buffer, getblocksize());

	for (int i = 0; i < blocks.size(); i++) {
		addblock(tempi, blocks[i]);
	}

	//how many records in the indexfile
	//to fix this, add an attribute to table class
	//called number of records.
	return 0;
}

int Table::IndexSearch(string value) {
	//read in indexfile

	int tblock = getfirstblock("indexfile");
	string buffer, smallbuffer;

	int errorcode = readblock("indexfile", tblock, smallbuffer);

	while (tblock != 0) {
		buffer += smallbuffer;
		tblock = nextblock("indexfile", tblock);
		if (tblock == 0)
			break;
		if(tblock != 0) {
			errorcode = readblock("indexfile", tblock, smallbuffer);
		}
	}
	istringstream instream; 
	instream.str(buffer);
	string key;
	int blockid;

	for (int i = 0; i < 256; i++) {
		instream >> blockid >> key;
		if (key == value) {
			return blockid;
		}
	}
	return -1;
}

int Table::Search(string value) {
	
	int blocknumber = IndexSearch(value);

	if(blocknumber == -1) {
		cout << "no records found.";
	} else {
		string buffer;
		int errcode = readblock("flatfile", blocknumber, buffer);
		cout << buffer << "\n";
	}
	return 0;
}
/************************************************************
	Main
************************************************************/
int main() {
	string restart = "y";
	while(restart == "y" || restart == "Y") {
		string maincommand = "null";

		cout << "1. Create / Modify a Shell Program.\n";
		cout << "2. Create / Modify a Table Program.\n";
		cout << "Please select as option: ";
		cin >> maincommand;

		while (maincommand != "1" && maincommand != "2") {
			cout << "\n\n1. Create / Modify a Shell Program.\n";
			cout << "2. Create / Modify a Table Program.\n";
			cout << "Please select as option: ";
			cin >> maincommand;
		}

		if (maincommand == "1") {
			//This main program inputs commands to the shell.
			//It inputs commands as : command op1 op2
			//You should modify it to work for your implementation.
			string s;
			string dname = "null";
			string command="go";
			string op1,op2;
			int dnumberofblocks, dblocksize;
			cout << "\nPlease choose a name for your shell / filesystem: ";
			cin >> dname;
		
			fstream infile(dname.c_str());
			if (infile.good()) {
				cout << "File already exists!\n";
				cout << "Clobbering previous data in " << dname << "....\n";
			}

			while (dname == "null" && dname.length() > 5) {
				cout << "\nInvalid name for filesystem, please select a name within 5 characters: ";
				cin >> dname; 
			}

			cout << "\nHow many blocks are going to created with your filesystem: ";
			cin >> dnumberofblocks;

			while (dnumberofblocks < 32 && dnumberofblocks > 1024) {
				cout << "\nInvalid number, please choose a number from 32 - 1024: ";
				cin >> dnumberofblocks;
			}

			cout << "\nHow big will the blocks be in your filesystem: ";
			cin >> dblocksize;

			while (dblocksize < 128 && dblocksize > 1024) {
				cout << "\nInvalid Blocksize! Please choose a number from 128 - 1024: ";
				cin >> dblocksize;
			}
			Shell myshell(dname, dnumberofblocks, dblocksize);

			cout << "\nType help for assistance\n";

			cout << "$";
			getline(cin,s);

			while (command != "quit") {
				command.clear();
				op1.clear();
				op2.clear();
				cout << "$";
				getline(cin,s);
				int firstblank=s.find(' ');
				if (firstblank < s.length()) s[firstblank]='#';
				int secondblank=s.find(' ');
				command=s.substr(0,firstblank);
				if (firstblank < s.length())
					op1=s.substr(firstblank+1,secondblank-firstblank-1);
				if (secondblank < s.length())
					op2=s.substr(secondblank+1);

				while (command != "dir" && command != "add" && command != "del" && command != "type" && command != "copy" && command != "quit" && command != "help") {
					cout << "Invalid command. \n$";
					getline(cin,s);
					int firstblank=s.find(' ');
					if (firstblank < s.length()) s[firstblank]='#';
					int secondblank=s.find(' ');
					command=s.substr(0,firstblank);
					if (firstblank < s.length())
						op1=s.substr(firstblank+1,secondblank-firstblank-1);
					if (secondblank < s.length())
						op2=s.substr(secondblank+1);
				}

				if (command=="dir") {
						myshell.dir();
				}
				if (command=="add") {
						myshell.add(op1);
				}
				if (command=="del") {
						myshell.del(op1);
				}
				if (command=="type") {
						myshell.type(op1);
				}
				if (command=="copy") {
						myshell.copy(op1, op2);
				}
				if (command=="help") {
					cout << "\nYou have a filesystem: " << dname << " with " << dnumberofblocks << " blocks of size " << dblocksize << "!\n";
					cout << "\nThe following are valid commands: command op1 op2 \n";
					cout << "         dir: lists the directory. \n";
					cout << "     add op1: op1 is a new file. Adds file to filesystem. \n";
					cout << "     del op1: op1 is a file. Deletes file from filesystem. \n";
					cout << "    type op1: op1 is a file. \n";
					cout << "copy op1 op2: op1 is source file, op2 is destination file. Copies op1 to op2. \n";
					cout << "        quit: quits the shell program \n";
				}
			}
		}//end if (maincommand == "1")


		if (maincommand == "2") {
			string tname = "null";
			string command ="go";
			string op1,s;

			cout << "\nPlease choose a name for your table: ";
			cin >> tname;
	
			fstream infile(tname.c_str());
			if (infile.good()) {
				cout << "Table already exists!\n";
				cout << "Clobbering previous data in " << tname << "....\n";
			}

			while (tname == "null" && tname.length() > 5) {
				cout << "\nInvalid name for table, please select a name within 5 characters: ";
				cin >> tname; 
			}

			Table mytable(tname, 256, 128);

			cout << "\nType help for assistance\n";
			cout << "$";
			getline(cin,s);

			while (command != "quit") {
				command.clear();
				op1.clear();
				cout << "$";
				getline(cin,s);
				int firstblank=s.find(' ');
				if (firstblank < s.length()) s[firstblank]='#';
				command=s.substr(0,firstblank);
				if (firstblank < s.length())
					op1=s.substr(firstblank+1,s.length());

				while (command != "build" && command != "search" && command != "help" && command != "quit") {
					cout << "Invalid command. \n$";
					getline(cin,s);
					int firstblank=s.find(' ');
					if (firstblank < s.length()) s[firstblank]='#';
					command=s.substr(0,firstblank);
					if (firstblank < s.length())
						op1=s.substr(firstblank+1,s.length());
				}

				if (command=="build") {
						cout << "building table ...\n";
						mytable.Build_Table(op1);
						cout << "table sucessfully built \n";
				}
				if (command=="search") {
						cout << "searching for value: " << op1 << "\n";
						mytable.Search(op1);
				}

				if (command=="help") {
					cout << "\nYou have a table: " << tname << " with 256 blocks of size 128!\n";
					cout << "\nThe following are valid commands: command op1 \n";
					cout << "   build op1: op1 is a string conatining new file name. builds a new table. \n";
					cout << "  search op1: op1 is a string conatining a value. searches the table. \n";
					cout << "        quit: quits the table program \n";
				}

			}//quit

		}//end if (maincommand == "2")

	cout << "Would you like to restart the program? ";
	cin >> restart;
	while(restart != "n" && restart != "N" && restart != "Y" && restart != "y") {
		cout << "\ninvalid input, please select Y or N";
		cin >> restart;
	}

	}//restart
	return 1;
}//end main