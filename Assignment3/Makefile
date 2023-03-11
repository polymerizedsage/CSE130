#######################################################################
#
# Copyright (C) 2020-2021 David C. Harrison. All right reserved.
#
# You may not use, distribute, publish, or modify this code without 
# the express written permission of the copyright holder.
#
#######################################################################

CFLAGS = -Wall -lpthread
CC = gcc -g

# Assignment No.
a = 3

default: manpage cartman

manpage: manpage.c manpage.a
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) manpage.a 2>&1 | tee make-manpage.out

cartman: cartman.c cartman.a
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) cartman.a 2>&1 | tee make-cartman.out

grade: clean manpage cartman 
	@./grade.sh 

clean:
	@rm -f manpage cartman make*.out

submit: clean
	@echo ""
	@echo "#### Only these file are submitted:"
	@echo ""
	@tar czvf ~/CSE130-Assignment$(a).tar.gz \
		--exclude=*.h --exclude=Makefile manpage.c cartman.c 
	@echo ""
	@echo "##### Confirming submission is good..."
	@echo ""
	@./confirm.sh $(a)
	@echo "#### If that failed, fix the problem and try again."
	@echo ""
	@echo "Finally, don't forget to upload ~/CSE130-Assignment$(a).tar.gz to Canvas!"
	@echo ""

