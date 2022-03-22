#ifndef MD5WRAPPER_H
#define MD5WRAPPER_H

#include "../Header/Acedia.h"
class MD5;

class md5wrapper{
	private:
		MD5 *md5;
	
		std::string hashit(std::string text);

		std::string convToString(unsigned char *bytes);
	public:
		md5wrapper();
		~md5wrapper();
		
		std::string getHashFromString(std::string text);

		std::string getHashFromFile(std::string filename);
};

#endif