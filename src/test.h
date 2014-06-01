/*
 * test.h
 *
 *  Created on: Jun 1, 2014
 *      Author: zolkko
 */

#ifndef TEST_H_
#define TEST_H_

template<typename T>
class demoz_t {
private:
	T data;
public:
	demoz_t() : data() {
	}
};


class test_t {
private:
	uint32_t data;
public:
	test_t() : data(0) {
	}

	void print(void);
};


#endif /* TEST_H_ */
