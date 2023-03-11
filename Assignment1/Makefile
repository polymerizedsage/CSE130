#######################################################################
#
# Copyright (C) 2018-2021 David C. Harrison. All right reserved.
#
# You may not use, distribute, publish, or modify this code without 
# the express written permission of the copyright holder.
#
#######################################################################

CFLAGS = -Wall
CC = gcc -g

# Assignment No.
a = 1

all: psort usort

psort: psort.c merge.a
	$(CC) -o $@ $^ $(CFLAGS) -lrt merge.a 2>&1 | tee pmake.out

usort: usort.c merge.a
	$(CC) -o $@ $^ $(CFLAGS) merge.a 2>&1 | tee umake.out

grade: clean psort usort
	@./grade.sh 

clean:
	@rm -f psort usort *make.out

submit: clean
	@echo ""
	@echo "#### Only these file are submitted:"
	@echo ""
	@tar czvf ~/CSE130-Assignment$(a).tar.gz \
		psort.c usort.c
	@echo ""
	@echo "##### Confirming submission is good..."
	@echo ""
	@./confirm.sh $(a)
	@echo "#### If that failed, fix the problem and try again."
	@echo ""
	@echo "Finally, don't forget to upload ~/CSE130-Assignment$(a).tar.gz to Canvas!"
	@echo ""
