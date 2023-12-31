#include "smpl2l.hpp"

#include <string>

mp& mp::operator+=(const mp& K) {
	if(K == 0) return *this;
	for(uint_fast8_t i = 0; i < BUFFER; i++) {	// upper bound maybe nur bis 3??
		if(K.data.B[i] == 0) continue;
		data.B[i] += K.data.B[i];
		if(data.B[i] < K.data.B[i] && i + 1 < BUFFER) {
			data.B[i + 1]++;
			for(uint_fast8_t index = i + 2; index < BUFFER && data.B[index - 1] == 0; index++) {
				data.B[index]++;
			}
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

mp& mp::operator-=(const unsigned long long K) {
	if(data.B[0] < K) {
		int big_index = 1;
		for(; big_index < BUFFER && data.B[big_index] == 0; big_index++) {
			data.B[big_index] = UINT64_MAX;
		}
		data.B[big_index]--;
	}
	data.B[0] -= K;

	return *this;
}
mp mp::operator-(const unsigned long long K) {
	mp out(*this);
	out -= K;
	return out;
}

mp& mp::operator-=(const mp& K) {
	for(int i = 0; i < BUFFER; i++) {
		if(data.B[i] < K.data.B[i]) {
			int big_index = i + 1;
			for(; big_index < BUFFER - 1 && data.B[big_index] == 0; big_index++) {
				data.B[big_index] = UINT64_MAX;
			}
			data.B[big_index]--;
		}
		data.B[i] -= K.data.B[i];
	}

	return *this;
}

mp mp::operator-(const mp& K) {
	mp out(*this);
	out -= K;
	return out;
}


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
	if(data.B[0] >= K) return false;
	for(int i = 1; i < BUFFER; i++) {
		if(data.B[i] != 0) return false;
	}
	return true;
}

bool mp::operator>(const mp& K) const {
	for(int i = BUFFER - 1; i >= 0; i--) {
		if(data.B[i] > K.data.B[i]) return true;
	}
	return false;
}
bool mp::operator<(const mp& K) const {
	for(int i = BUFFER - 1; i >= 0; i--) {
		if(data.B[i] < K.data.B[i]) return true;
		if(data.B[i] > K.data.B[i]) return false;
	}
	return false;
}

unsigned long mp::getBitSize() {
	uint8_t index = BUFFER - 1, pos = 0;
	while(index < BUFFER && data.B[index] == 0) index--;
	for(unsigned long long m = 1ULL << (64 - 1); m > 0 && !(m & data.B[index]); m /= 2) pos++;

	return (index + 1) * 64 - pos;
}


// ah yes i love strings
std::string mp::getBinString() const {
	uint8_t index = BUFFER - 1, pos = 0;
	while(index < BUFFER && data.B[index] == 0 && index != 0) index--;
	for(unsigned long long m = 1ULL << (64 - 1); m > 0 && !(m & data.B[index]); m /= 2) pos++;
	uint32_t len = (index + 1) * 64 - pos;
	if(len == 0) return "0";
	std::string out;
	out.reserve(len);  // später schauen ob ich 500 iq habe (ich habe 500 iq)

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


// MULTIPLICATION


mp& mp::operator<<=(const unsigned K) {
	if(K == 0) return *this;
	if(K >= BUFFER * 64) {
		clear();
		return *this;
	}
	uint_fast32_t G = K;
	if(K % 32 == 0 && K % 64 != 0) {
		stepL32(K / 32);
		return *this;
	} else if(G >= 64) {
		uint_fast16_t counter = 0;
		for(; G >= 64; G -= 64) counter++;
		stepL64(counter);
	}

	// ka ob ich das brauch
	if(G != 0) {
		const unsigned offset = 64 - G;
		uint_fast64_t buff, buff_old = 0;
		for(int i = 0; i < BUFFER; i++) {		   // evtl opti wenn BUFFER-1 und lastcase hinter for
			if(data.B[i] == 0 && buff_old == 0) {  // die mit den nullen muss ich ja nicht ändern
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

mp& mp::operator>>=(const unsigned K) {
	if(K == 0) return *this;
	if(K >= BUFFER * 64) {
		clear();
		return *this;
	}
	uint_fast32_t G = K;
	if(K % 32 == 0 && K % 64 != 0) {
		stepR32(K / 32);
		return *this;
	} else if(G >= 64) {
		uint_fast16_t counter = 0;
		for(; G >= 64; G -= 64) counter++;
		stepR64(counter);
	}

	// du brauchst das, ist viel schneller
	if(G != 0) {
		const unsigned offset = 64 - G;
		uint_fast64_t buff, buff_old = 0;
		for(int i = BUFFER - 1; i >= 0; i--) {	   // evtl opti wenn BUFFER-1 und lastcase hinter for
			if(data.B[i] == 0 && buff_old == 0) {  // die mit den nullen muss ich ja nicht ändern
				buff_old = 0;
				continue;
			}
			buff = data.B[i] << offset;
			data.B[i] >>= G;
			data.B[i] |= buff_old;
			buff_old = buff;
		}
	}

	return *this;
}
mp mp::operator>>(const unsigned K) {
	mp out(*this);
	out >>= K;
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
	temp.stepL32(1);

	// EVTL NICHT alle sondern nur die bis jetzt beschriebenen ändern
	// temp<<=32;

	operator+=(temp);


	return *this;
}


void mp::mult32(uint32_t K) {
	if(K == 0) {
		clear();
		return;
	}
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

void mp::stepL32(uint16_t n) {
	if(n >= BUFFER * 2) {
		clear();
		return;
	}

	uint_fast16_t upper = BUFFER * 2 - 1 - n;

	// clang-format off
	for(; upper > 0 && data.M[upper] == 0; upper--);
	// clang-format on
	for(int i = upper + n; i >= n; i--) {
		data.M[i] = data.M[i - n];
	}
	for(int i = 0; i < n; i++) data.M[i] = 0;
}

void mp::stepL64(uint16_t n) {
	if(n >= BUFFER) {
		clear();
		return;
	}

	uint_fast16_t upper = BUFFER - 1 - n;

	// clang-format off
	for(; upper > 0 && data.B[upper] == 0; upper--);
	// clang-format on
	for(int i = upper + n; i >= n; i--) {
		data.B[i] = data.B[i - n];
	}
	for(int i = 0; i < n; i++) data.B[i] = 0;
}

void mp::stepR32(uint16_t n) {
	uint_fast16_t upper = BUFFER * 2 - 1;
	// clang-format off
	for(; upper > 0 && data.M[upper] == 0; upper--);
	// clang-format on
	if(upper < n) {
		// wir deleten einfach; Garkein problem bruder;
		for(int i = 0; i <= upper; i++) data.M[i] = 0;
		return;
	}
	for(int i = 0; i <= upper - n; i++) {
		data.M[i] = data.M[i + n];
	}
	for(int i = upper - n + 1; i <= upper; i++) data.M[i] = 0;
}

void mp::stepR64(uint16_t n) {
	uint_fast16_t upper = BUFFER - 1;
	// clang-format off
	for(; upper > 0 && data.B[upper] == 0; upper--);
	// clang-format on
	if(upper < n) {
		// wir deleten einfach; Garkein problem bruder;
		for(int i = 0; i <= upper; i++) data.B[i] = 0;
		return;
	}
	for(int i = 0; i <= upper - n; i++) {
		data.B[i] = data.B[i + n];
	}
	for(int i = upper - n + 1; i <= upper; i++) data.B[i] = 0;
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
		temp.stepL64(i);
		temp *= K.data.B[i];
		adder += temp;
	}

	operator=(adder);

	return *this;
}

mp mp::operator*(const mp& K) const {
	mp out(*this);
	out *= K;
	return out;
}


mp& mp::operator/=(const unsigned long long K) {
	return operator/=(mp(K));
}

mp mp::operator/(const unsigned long long K) const {
	mp out(*this);
	out /= mp(K);
	return out;
}


uint16_t mp::findBitSize(uint16_t& index_start) {
	uint8_t pos = 0;
	while(index_start < BUFFER && data.B[index_start] == 0 && index_start != 0) index_start--;
	for(unsigned long long m = 1ULL << (64 - 1); m > 0 && !(m & data.B[index_start]); m /= 2) {
		pos++;
	}
	return (index_start + 1) * 64 - pos;
}


void mp::setBit(uint32_t pos) {
	uint16_t arr = 0;
	for(; pos >= 64; pos -= 64) arr++;
	data.B[arr] |= ((uint64_t)1 << pos);
}


mp& mp::operator/=(const mp& K) {
	if(operator<(K)) {
		clear();
		return *this;
	}
	if(operator==(K)) {
		clear();
		data.B[0] = 1;
		return *this;
	}

	mp nenner(K), adder;
	// bitshift as far as possible and then check for NENNER == 1
	uint16_t BitstartZ = findFirst1(), BitstartN = nenner.findFirst1(),
			 smallest = (BitstartZ < BitstartN) ? BitstartZ : BitstartN;

	if(smallest != 0) {
		nenner >>= smallest;
		operator>>=(smallest);
	}

	if(nenner == 1) {
		return *this;
	}

	uint32_t BIT_Z;
	uint16_t index_Z = BUFFER - 1;
	uint32_t BIT_N;
	BIT_N = nenner.findBitSize(index_Z);
	index_Z = BUFFER - 1;
	BIT_Z = findBitSize(index_Z);

	int32_t current_bit = BIT_Z - BIT_N;
	nenner <<= current_bit;
	BIT_N = BIT_Z;

	if(operator<(nenner)) {
		nenner >>= 1;
		current_bit--;
		BIT_N--;
	}

	while(current_bit >= 0) {
		operator-=(nenner);
		adder.setBit(current_bit);

		BIT_Z = findBitSize(index_Z);
		if(BIT_Z == 0) break;
		nenner >>= BIT_N - BIT_Z;
		current_bit -= BIT_N - BIT_Z;
		BIT_N = BIT_Z;

		if(operator<(nenner)) {
			nenner >>= 1;
			current_bit--;
			BIT_N--;
		}
	}

	operator=(adder);

	return *this;
}

mp mp::operator/(const mp& K) const {
	mp out(*this);
	out /= K;
	return out;
}


bool mp::isBitInUnreadable() const {
	for(int i = 1; i < BUFFER - 1; i++) {
		if(data.B[i] != 0) return true;
	}
	return false;
}

std::string mp::toString() const {
	if(!isBitInUnreadable()) {
		return std::to_string(data.B[0]);
	}

	// neuer ansatzt => scheiß auf effizienz
	uint32_t dec_stellen = 0;
	mp temp(*this), subtractor(*this);
	for(; temp != 0; temp /= 10) dec_stellen++;
	std::string out, holder;
	out.reserve(dec_stellen);
	holder.reserve(19);

	// first case
	uint16_t over19 = dec_stellen % 19;
	if(over19 != 0) {
		temp = *this;
		for(uint16_t i = 0; i < dec_stellen - over19; i++) temp /= 10;
		out += std::to_string(temp.data.B[0]);
		for(uint16_t i = 0; i < dec_stellen - over19; i++) temp *= 10;
		subtractor -= temp;
		dec_stellen -= over19;
	}
	if(dec_stellen == 0) return out;

	dec_stellen /= 19;

	// subsequent
	mp divider;
	for(uint32_t i = 1; i < dec_stellen; i++) {
		temp = subtractor;
		divider.data.B[0] = 10000000000000000000ULL;
		for(uint32_t n = 1; n < dec_stellen - i; n++) divider *= 10000000000000000000ULL;
		temp /= divider;
		holder = std::to_string(temp.data.B[0]);
		if(19 - holder.size() != 0)
			out += std::string(19 - holder.size(), '0');  // adds zeros only if needed (doesnt happen often)
		out += holder;
		// calling mult only once is a lot faster...
		temp *= divider;
		subtractor -= temp;
		divider.clear();
	}

	// last
	holder = std::to_string(subtractor.data.B[0]);
	if(19 - holder.size() != 0)
		out += std::string(19 - holder.size(), '0');  // adds zeros only if needed (doesnt happen often)
	out += holder;

	return out;
}