#ifndef MD5_H
#define MD5_H

#include "../define.h"
#include <string>
#include <fstream>

using namespace std;
using namespace CommBaseOut;

/* MD5 declaration. */
class MD5 {
public:
	MD5();
	MD5(const void *input, size_t length);
	MD5(const string &str);
	MD5(ifstream &in);
	void Update(const void *input, size_t length);
	void Update(const string &str);
	void Update(ifstream &in);
	const BYTE* Digest();
	string ToString();
	void Reset();
private:
	void Update(const BYTE *input, size_t length);
	void Final();
	void Transform(const BYTE block[64]);
	void Encode(const DWORD *input, BYTE *output, size_t length);
	void Decode(const BYTE *input, DWORD *output, size_t length);
	string BytesToHexString(const BYTE *input, size_t length);

	/* class uncopyable */
	MD5(const MD5&);
	MD5& operator=(const MD5&);
private:
	DWORD _state[4];	/* state (ABCD) */
	DWORD _count[2];	/* number of bits, modulo 2^64 (low-order word first) */
	BYTE _buffer[64];	/* input buffer */
	BYTE _digest[16];	/* message digest */
	bool _finished;		/* calculate finished ? */

	static const BYTE PADDING[64];	/* padding for calculate */
	static const char HEX[16];
	static const size_t BUFFER_SIZE = 1024;
};

#endif/*MD5_H*/
