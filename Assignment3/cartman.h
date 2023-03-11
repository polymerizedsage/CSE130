/*********************************************************************
 *
 * Copyright (C) 2020-2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

enum track { 
  Red, Orange, Yellow, Green, Blue, Indigo, Violet, 
  Azure, Black, Brown, Cyan, Gray, Magenta, Mauve, 
  Pink, Purple, Rose };

enum junction { A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q };

/*
 * Ask CART to start crossing the critical section of TRACK starting
 * at JUNCTION. CARTs take aproximately one second to get safely across.
 *
 * System will exit if:
 * 
 *   1. CART is not known to the system
 *   2. JUNCTION is not reserved by CART
 *   3. Junction opposite JUNCTION is not reserved by CART
 *   4. CART is not currently at JUNCTION 
 *   5. CART is no longer active having already crossed 
 */
void cross(unsigned int cart, enum track track, enum junction junction); 

/*
 * Reserve JUNCTION for use by CART.
 *
 * System will exit if:
 * 
 *   1. CART is not known to the system
 *   2. JUNCTION is already reserved
 *   3. CART is not currently at JUNCTION 
 *   4. CART has already crossed JUNCTION
 */
void reserve(unsigned int cart, enum junction junction);

/*
 * Release JUNCTION reserved by CART.
 *
 * System will exit if:
 * 
 *   1. CART is not known to the system
 *   2. JUNCTION is not currently reserved by CART
 */
void release(unsigned int cart, enum junction junction);
