/*
 * test_constants.h
 *
 *  Created on: 2013-7-15
 *      Author: king
 */

#ifndef _TEST_XIAOMI_CONSTANTS_H_
#define _TEST_XIAOMI_CONSTANTS_H_

#include <gflags/gflags.h>

DECLARE_bool(print_verbose);

#define TEST_LOG_ERROR_PREFIX FLAGS_print_verbose && std::cerr << "**** ERROR **** "
#define TEST_LOG_VERBOSE_PREFIX FLAGS_print_verbose && std::cout << "**** VERBOSE **** "
#define TEST_LOG_VERBOSE_SHORT_PREFIX FLAGS_print_verbose && std::cout

#endif /* _TEST_XIAOMI_CONSTANTS_H_ */
