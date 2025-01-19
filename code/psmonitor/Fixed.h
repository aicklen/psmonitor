#pragma once
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

using fixed = int32_t;

namespace Fixed {

    fixed int2fixed(const int16_t value);
    int16_t fixed2int(const fixed value);

    fixed multiply(const fixed value1, const fixed value2);
    fixed invert(const fixed value);

}

#endif
