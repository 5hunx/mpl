#pragma once

#include <stdint.h>

#include <string>
// leftover idee? anstatt immer zu rechenen 2 arrays haben

#define BUFFER 40


/*** Idee:
 * Erstmal: Der shit is small endian
 * Mult und Div opti -> bitlänge bis zur ersten 1 finden
 * erstmal nur unsigned
 */

class mp {
public:
	mp() {};
	mp(const int K) { // less "ambiguous" for compiler 
		initfromU64(K);
	};
	mp(const unsigned long long K) {
		initfromU64(K);
	};
	mp(const mp& K) {
		initfromMPI(K);
	};
	mp(const char* K) {
		initfromString(K);
	};


	void operator=(const unsigned long long K) {
		clear();
		initfromU64(K);
	};
	void operator=(const mp& K) {
		initfromMPI(K);
	};

	mp& operator+=(const unsigned long long K);
	mp operator+(const unsigned long long K) const;

	mp& operator+=(const mp& K);
	mp operator+(const mp& K) const;

	mp& operator-=(const unsigned long long K);
	mp operator-(const unsigned long long K);

	mp& operator-=(const mp& K);
	mp operator-(const mp& K);

	mp& operator<<=(unsigned K);
	mp operator<<(const unsigned K);

	mp& operator>>=(unsigned K);
	mp operator>>(const unsigned K);


	mp& operator*=(const unsigned long long K);
	mp operator*(const unsigned long long K) const;

	mp& operator*=(const mp& K);
	mp operator*(const mp& K) const;

	mp& operator/=(const unsigned long long K);
	mp operator/(const unsigned long long K) const;


	mp& operator/=(const mp& K);
	mp operator/(const mp& K) const;



	bool operator==(const unsigned long long K) const;
	bool operator==(const mp& K) const;

	bool operator!=(const unsigned long long K) const {
		return !operator==(K);
	};
	
	bool operator>(const unsigned long long K) const;
	bool operator<(const unsigned long long K) const;

	bool operator>(const mp& K) const;
	bool operator<(const mp& K) const;

	// void printBin();
	std::string toString() const;
	std::string getBinString() const;
	std::string getSimpleDecString() const;

	union ovfl {
		uint64_t B[BUFFER];
		uint32_t M[BUFFER * 2];
	} data = {0};


	unsigned long getBitSize();

	void initFormBinaryString(std::string K); 


private:
	inline uint32_t findFirst1();

	void mult32(uint32_t K);

	inline void stepL32(uint16_t n);
	inline void stepL64(uint16_t n);

	inline void stepR32(uint16_t n);
	inline void stepR64(uint16_t n);

	bool isBitInUnreadable() const;

	uint16_t findBitSize(uint16_t &index_start);
	void setBit(uint32_t pos);
	
	void clear();

	void initfromString(const std::string& K);
	void initfromU64(const unsigned long long K);
	void initfromMPI(const mp& K);


	// void debugOut(std::string K, int l = 0) const {
	// 	for(int i = 0; i < l; i++) std::cout << ' ';
	// 	std::cout << K << '\n';
	// 	for(int i = 0; i < l; i++) std::cout << ' ';
	// 	std::cout << getBinString() << '\n';
	// };
};


void mp::initfromString(const std::string& K) {
	constexpr uint64_t batch = 10000000000000000000ULL;
	uint32_t i = K.size() % 19;
	if(i != 0) {
		uint64_t insert = std::stoull(K.substr(0, i));
		data.B[0] += insert;
	}

	for(; (i + 18) < K.size(); i += 19) {
		operator*=(batch);
		uint64_t insert = std::stoull(K.substr(i, 19));
		operator+=(insert);
	}
}

void mp::initfromU64(const unsigned long long K) {
	data.B[0] = K;
}

void mp::initfromMPI(const mp& K) {
	for(uint_fast8_t i = 0; i < BUFFER; i++) {
		data.B[i] = K.data.B[i];
	}
}

void mp::initFormBinaryString(std::string K) {
	for(uint32_t i = 0; i < K.size(); i++) {
		if(K[i] == '1') {
			setBit(K.size() - i - 1);
		}
	}
}

// Kommt noch irgendwann
// void mp::initFormHexString(std::string K) {
// 	for(int32_t i = 0; i < K.size(); i++) {
// 		if(K[i] == '1') {
// 			setBit(K.size() - i - 1);
// 		}
// 	}
// }

#include "annoningerrortobechangedlater.cpp"