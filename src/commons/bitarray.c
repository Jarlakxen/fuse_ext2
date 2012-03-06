/*
 * bitarray.c
 *
 *  Created on: 06/03/2012
 *      Author: fviale
 */

#include <stdlib.h>
#include "bitarray.h"

t_bitarray *bitarray_create(unsigned char *bitarray, unsigned int size) {
	t_bitarray *self = malloc(sizeof(t_bitarray));

	self->bitarray = bitarray;
	self->size = size;

	return self;
}

bool bitarray_test_bit(t_bitarray *self, unsigned int bit) {
	return((self->bitarray[BIT_CHAR(bit)] & BIT_IN_CHAR(bit)) != 0);
}

void bitarray_set_bit(t_bitarray *self, unsigned int bit) {

	if (bitarray_get_max_bit(self) <= bit) {
		return; /* bit out of range */
	}

	self->bitarray[BIT_CHAR(bit)] |= BIT_IN_CHAR(bit);
}

void bitarray_clean_bit(t_bitarray *self, unsigned int bit){
    unsigned char mask;

	if (bitarray_get_max_bit(self) <= bit) {
		return; /* bit out of range */
	}

    /* create a mask to zero out desired bit */
    mask =  BIT_IN_CHAR(bit);
    mask = ~mask;

    self->bitarray[BIT_CHAR(bit)] &= mask;
}

unsigned int bitarray_get_max_bit(t_bitarray *self) {
	return self->size * CHAR_BIT;
}

void bitarray_destroy(t_bitarray *self) {
	free(self);
}
