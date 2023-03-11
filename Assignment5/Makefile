#######################################################################
#
# Copyright (C) 2021 David C. Harrison. All right reserved.
#
# You may not use, distribute, publish, or modify this code without 
# the express written permission of the copyright holder.
#
#######################################################################

CFLAGS = -g -pedantic -Wall 
CC = gcc

# Assignment No.
a = 5

default: pager

pager: *.c simulator.a queue.a
	$(CC) -o $@ $^ $(CFLAGS) simulator.a queue.a 2>&1 | tee make.out

check:
	@./check.sh 

grade: clean pager
	@./grade.sh $(a)

clean:
	@rm -f pager *.out

submit: clean
	@echo ""
	@echo "#### Only these file are submitted:"
	@echo ""
	@tar czvf ~/CSE130-Assignment$(a).tar.gz *.c 
	@echo ""
	@echo "##### Confirming submission is good..."
	@echo ""
	@./confirm.sh $(a)
	@echo "#### If that failed, fix the problem and try again."
	@echo ""
	@echo "Finally, don't forget to upload ~/CSE130-Assignment$(a).tar.gz to Canvas!"
	@echo ""
