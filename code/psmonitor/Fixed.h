/**
 * @file Fixed.h
 * @copyright
 * Copyright 2024 Gregory Aicklen.
 * Licensed under the MIT License
 *
 * @brief Header file defining data structures and functions supporting
 *        fixed point arithmetic.
 *
 */
 
#ifndef _FIXED_H
#define _FIXED_H

typedef int32_t fixed;

namespace Fixed {

    const int shift PROGMEM = 5;

    fixed int2fixed(int16_t value);
    int16_t fixed2int(fixed value);

    fixed multiply(fixed value1, fixed value2);
    fixed invert(fixed value);

}

#endif
