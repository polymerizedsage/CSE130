/************************************************************************
 *
 * CSE130 Assignment 4
 *
 * Copyright (C) 2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 * 
 ************************************************************************/

/*********************************************************
 **               DO NOT MODIFY THIS FILE               ** 
 *********************************************************/

#pragma once

/**
 * Supported page replacement algorithms.
 */
enum algorithm {
  FIRST_IN_FIRST_OUT,
  LEAST_RECENTLY_USED,
  LEAST_FREQUENTLY_USED, // Infinite history
  SECOND_CHANCE,
  CIRCULAR_QUEUE,
};

/**
 * Request the contents of virtual page PAGE. A page fault will be registered
 * by the simulator.
 */
void sim_get_page(unsigned int page);

/**
 * Returns the current simulator time in abstract units equal to one programable 
 * clock cycle.
 */
int sim_time();
