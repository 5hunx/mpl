#include <string>
#include <bitset>


#include "smpl2l.hpp"

mp& mp::operator+=(const mp& K) {
	for(uint_fast8_t i = 0; i < BUFFER; i++) {	// upper bound maybe nur bis 3??
		data.B[i] += K.data.B[i];
		if(data.B[i] < K.data.B[i]) {
			uint_fast8_t index = i;
			do {
				index++;
				data.B[index]++;
			} while(index < BUFFER && data.B[index] == 0);
		}
	}

	return *this;
}

mp mp::operator+(const mp& K) const {
	mp out(*this);
	out += K;
	return out;
}

mp& mp::operator+=(const unsigned long long K) {
	data.B[0] += K;
	if(data.B[0] < K) {
		uint_fast8_t index = 0;
		do {
			index++;
			data.B[index]++;
		} while(index < BUFFER && data.B[index] == 0);
	}

	return *this;
}

mp mp::operator+(const unsigned long long K) const {
	mp out(K);
	out += *this;
	return out;
}

// void mp::printBin() {
// 	for(int i = 0; i < BUFFER; i++) std::cout << data[i] << " ";
// 	std::cout << ":\n";
// 	for(int i = 3; i >= 0; i--) {
// 		if(data[i] != 0) {
// 			std::cout << std::bitset<64>(data[i]);
// 		}
// 	}
// 	std::cout << "\n";
// }



bool mp::operator==(const unsigned long long K) const {
	if(data.B[0] != K) return false;
	for(int i = 0; i < BUFFER; i++) {
		if(data.B[i] != 0) return false;
	}
	return true;
}
bool mp::operator==(const mp& K) const {
	for(int i = 0; i < BUFFER; i++) {
		if(data.B[i] != K.data.B[i]) return false;
	}
	return true;
}



bool mp::operator>(const unsigned long long K) const {
	if(data.B[0] > K) return true;
	for(int i = 1; i < BUFFER; i++) {
		if(data.B[i] != 0) return true;
	}
	return false;
}
bool mp::operator<(const unsigned long long K) const {
	if(data.B[0] > K) return false;
	for(int i = 1; i < BUFFER; i++) {
		if(data.B[i] != 0) return false;
	}
	return true;
}

bool mp::operator>(const mp& K) const {
	for(int i = BUFFER; i >= 0; i--) {
		if(data.B[i] > K.data.B[i]) return true;
	}
	return false;
}
bool mp::operator<(const mp& K) const {
	for(int i = BUFFER; i >= 0; i--) {
		if(data.B[i] < K.data.B[i]) return true;
	}
	return false;
}


// ah yes i love strings
std::string mp::getBinString() const {
	uint8_t index = BUFFER - 1, pos = 0;
	while(index < BUFFER && data.B[index] == 0) index--;
	for(unsigned long long m = 1ULL << (64 - 1); m > 0 && !(m & data.B[index]); m /= 2) pos++;

	std::string out;
	out.reserve((index + 1) * 64 - pos);  // später schauen ob ich 500 iq habe (ich habe 500 iq)

	for(; index < BUFFER; index--) {
		for(unsigned long long m = 1ULL << (64 - 1 - pos); m > 0; m /= 2) {
			out.push_back((m & data.B[index]) ? '1' : '0');
		}
		pos = 0;
	}
	return out;
}

std::string mp::getSimpleDecString() const {
	return std::to_string(data.B[0]);
}


void mp::clear() {
	for(int i = 0; i < BUFFER; i++) {
		data.B[i] = 0;
	}
}


//MULTIPLICATION


mp& mp::operator<<=(const unsigned K) {
	uint_fast32_t G = K;
	if(G >= 64) {
		uint_fast16_t counter = 0;
		for(; G >= 64; G -= 64) counter++;
		step64(counter);
	}  // Hiernach ist K%64 also maximal 63

	// ka ob ich das brauch
	if(G != 0) {
		const unsigned offset = 64 - G;
		uint_fast64_t buff, buff_old = 0;
		for(int i = 0; i < BUFFER; i++) {  // evtl opti wenn BUFFER-1 und lastcase hinter for
			if(data.B[i] == 0 && buff_old == 0) {		   // die mit den nullen muss ich ja nicht ändern
				buff_old = 0;
				continue;
			}
			buff = data.B[i] >> offset;
			data.B[i] <<= G;
			data.B[i] |= buff_old;
			buff_old = buff;
		}
	}

	return *this;
}

mp mp::operator<<(const unsigned K) {
	mp out(*this);
	out <<= K;
	return out;
}


mp& mp::operator*=(const unsigned long long K) {
	// check self == 0

	if(K == 0 || operator==(0)) {
		clear();
		return *this;
	}

	union {
		uint64_t B;
		uint32_t S[2];
	} vals = {K};


	if(vals.S[1] == 0) {  // for u32 faster
		mult32(vals.S[0]);
		return *this;
	}

    

	mp temp(*this);
    mult32(vals.S[0]);
	temp.mult32(vals.S[1]);
	temp.step32();
	
    // EVTL NICHT alle sondern nur die bis jetzt beschriebenen ändern
	// temp<<=32;

	operator+=(temp);

	return *this;
}

void mp::mult32(uint32_t K) {
	uint16_t shift = 0;
	while(K % 2 == 0) {
		shift++;
		K /= 2;
	}
	if(shift != 0) operator<<=(shift);



	if(K != 1) {
		mp adder;
		union {
			uint64_t B;
			uint32_t S[2];
		} overflower;
		for(int i = 0; i < BUFFER * 2 - 1; i++) {
			if(data.M[i] == 0) continue;  // evtl viele rechnungen sparen...
			overflower.B = (uint64_t)data.M[i] * K;
			data.M[i] = overflower.S[0];
			adder.data.M[i + 1] = overflower.S[1];
		}

		data.M[BUFFER * 2 - 1] *= K;
        operator+=(adder);
	}
}

void mp::step32() {
	uint_fast16_t upper = BUFFER * 2;
	// clang-format off
	for(; upper > 0 && data.M[upper] != 0; upper--);
	// clang-format on
	for(int i = upper + 1; i > 0; i--) {
		data.M[i] = data.M[i - 1];
	}
	data.M[0] = 0;
}

void mp::step64(uint16_t n) {
	uint_fast16_t upper = BUFFER;
    // clang-format off
	for(; upper > 0 && data.B[upper] != 0; upper--);
    // clang-format on
	for(int i = upper + 1; i >= n; i--) {
		data.B[i] = data.B[i - n];
	}
	for(int i = 0; i < n; i++) data.B[i] = 0;
}


mp mp::operator*(const unsigned long long K) const {
	mp out(*this);
	out *= K;
	return out;
}

mp& mp::operator*=(const mp& K) {
	// damn shits expensive DAMN
	if(operator==(0) || K == 0) {
		clear();
		return *this;
	}

	if(operator==(1)) {
		operator=(K);
		return *this;
	}

	// bitshift magic here  no divide tho :(

	mp adder, temp;

	if(K.data.B[0] != 0) {
		temp = *this;
		temp.operator*=(K.data.B[0]);
		adder = temp;
	}

	for(int i = 1; i < BUFFER; i++) {
		if(K.data.B[i] == 0) continue;
		temp = *this;
		temp.step64(i);
		temp *= K.data.B[i];
		adder += temp;
	}

	operator=(adder);

    // debugOut("nach mult ka");

	return *this;
}

mp mp::operator*(const mp& K) const {
	mp out(*this);
	out *= K;
	return out;
}



void incExpDouble(double &K, uint16_t val) {
    // constexpr uint16_t maskExp = 0b0111111111110000,
    //                 maskExpInv = 0b1000000000001111;   
    
    // uint16_t* EXP_P = (uint16_t*)(&K)+3;
    // uint16_t D = *EXP_P & maskExp; 
    // val<<=4;
    // D += val;
    // // D &= maskExp;
    // *EXP_P &= maskExpInv;
    // *EXP_P |= D;
    

    // same stuff aber bisl langsamer (aber zumidest lesbar SKULL EMOJI)
    // DoubleFucker* b = reinterpret_cast<DoubleFucker*>(&K);

    // b->exponent += val;
}

// DIVISION

double mp::div32(const uint32_t K, const int32_t index) {
    double out, S;
    DoubleFucker* b = reinterpret_cast<DoubleFucker*>(&S);
    const double D = (double)K;
    for(int i = 0; i < BUFFER*2; i++) {
        if(data.M[i] == 0) continue;
        S = data.M[i] / D;
        b->exponent += (i - index) * 32;
        out += S;
    }
    return out;
}



mp& mp::operator/=(const unsigned long long K) {
    if(operator<(K)) {
        clear();
        return *this;
    }
    if(K == 0) {
        // ja bro was soll das
    }

    for(int i = 0; i < BUFFER*2; i++) {
        
    }


    return *this;
}

// mp mp::operator/(const unsigned long long K) const {
//     mp out(*this);
//     out /= K;
//     return out;
// }


// mp& mp::operator/=(const mp& K) {
//     if(operator<(K)) {
//         clear();
//         return *this;
//     }



//     return *this;
// }
// mp mp::operator/(const mp& K) const {}
