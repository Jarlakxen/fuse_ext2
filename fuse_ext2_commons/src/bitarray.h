/*
 * bitarray.h
 *
 *  Created on: 06/03/2012
 *      Author: fviale
 */

#ifndef BITARRAY_H_
#define BITARRAY_H_

	#include <stdbool.h>
	#include <limits.h>

	/* position of bit within character */
	#define BIT_CHAR(bit)         ((bit) / CHAR_BIT)

	/* array index for character containing bit */
	#define BIT_IN_CHAR(bit)      (1 << (CHAR_BIT - 1 - ((bit)  % CHAR_BIT)))


	typedef struct {
		unsigned char *bitarray;
		unsigned int size;
	} t_bitarray;

	t_bitarray 	*bitarray_create(unsigned char *bitarray, unsigned int size);
	bool 		 bitarray_test_bit(t_bitarray*, unsigned int bit);
	void		 bitarray_set_bit(t_bitarray*, unsigned int bit);
	void		 bitarray_clean_bit(t_bitarray*, unsigned int bit);
	unsigned int bitarray_get_max_bit(t_bitarray*);
	void 		 bitarray_destroy(t_bitarray*);

#endif /* BITARRAY_H_ */
