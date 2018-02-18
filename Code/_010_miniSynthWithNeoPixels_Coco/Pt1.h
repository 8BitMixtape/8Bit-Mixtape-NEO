/*
 * Pt1.h
 *
 * Created: 20.02.2017 05:46:34

 */ 


#ifndef PT1_H_
#define PT1_H_

#include <stdint.h>
/*
 * Pt1.cpp
 *
 * Created: 20.02.2017 05:46:16

 */ 

class Pt1
{
	private:

		uint16_t  K_param;
		int16_t  T_param;

		//uint32_t rate_us;
		
		int32_t  y_old;

	public:
	
		Pt1(uint16_t t);
		//virtual ~Pt1();
		
		void setT(uint16_t t);
		void setOutput(int16_t y);
		int16_t filter(int16_t input);

};




#endif /* PT1_H_ */
