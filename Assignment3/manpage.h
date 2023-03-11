/*********************************************************************
 *
 * Copyright (C) 2020-2021 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 *
 ***********************************************************************/

/*
 * Returns paragraph identifier in the range 0..6 for the current thread
 */
int getParagraphId();

/*
 * Shows paragraph corresponding to the identifier returned by getParagraphId()
 */
void showParagraph();
