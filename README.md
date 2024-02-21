# textbin-db

This is a binary database which is loaded into RAM at program runtime. 
We will implement a project where we will try to compile text file databases to binary databases.
This binary database will be loaded to program's RAM at startup. There are some way to encrypt binary database for more security.

1. There are text database files which contain only two things below:
	+ Comment sections: must start with /* and end with */
	+ DB entries: must be formed like this: key	type	value
		- key must be like: /production_type/something/something. Key accepts only one simple wildcard string "x", "x" will be replaced by any numerical number.
		- type: must be like: U8, U32, char,...
		- value: is an array of type

	For example: if key is U8 and value is 10, 11, 12. This means there will be an U8* pointer to an array of U8 data with 3 consecutive elements.
		Delimitter in this case will be considered as ", " for non-char type and as " " for char type.

Type will be mapped to an enum, for example:
enum TypeOfEntry_e {
	TYPE_OF_ENTRY_UNDEFINED	= 0,
	TYPE_OF_ENTRY_U8	= 1,
	...
};

Example: databaseA.txt
/* This is comment sections */
/board_1.12.1/temperatureLevels		U8	-10, 30, 80, 150
/board_1.41.x/initPatterns		char	"L P F"		/* This means all boards for 1.41.x generation will be applied for this key */

2. Next step, will we write an script, python or maybe C program to concatenate all text database files into one file probably named swdb.bin.txt.
The program/script will remove all comments in text database files as well.

Example: swdb.bin.text
/board_1.12.1/temperatureLevels         U8      -10, 30, 80, 150
/board_1.41.x/initPatterns              char    "L P F"
/board_1.23.5/compensatePeriod		U32     5000
/board_1.11.x/isFeatureAbcEnabled       U8    	1

3, Next, we will write a helper program which will convert swapp.bin.text file to swapp.bin binary database file. The binary file will look like below.

Example: swdb.bin
5402 0000 0000 0000 0100 aaaa aaaa aaaa		T. .. .. .. .. 

+ H = header, must be the first byte of swdb.bin
+ The second byte is databaseRevision dbRev
+ The next 4 bytes are reserved additional parameters for dbRev
+ The next 4 bytes are the number bytes of actual database payload, length of the entire payload
+ The next are consecutive dbEntries with format: F<key>'\0'<type><length_of_value><value>
	- F is the start signature of each dbEntry.
	- <key> is the string "key" as above, note that it includes string termination character '\0' as well.
	- <type> is 1 byte to indicate TypeOfEntry_e.
	- <length_of_value> is 3 bytes showing the number bytes of the "value".
	- <value> is an array of "value". For example, if "value" = 15, 14, 13 -> <value> will be an byte array = 0f0e0d.
+ After all dbEntries, that means end of payload, there must be an byte 'E' indicate end of payload.
+ There are some padding bytes with zero value before the last 4 bytes for CRC checksum.
