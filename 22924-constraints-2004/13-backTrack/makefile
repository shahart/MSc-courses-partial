all:	clean mmn13

mmn13:	mmn13.c macros.inc saveload.inc bcssp.inc bt_labels.inc fc_labels.inc checks.inc ordering.inc problems.inc
	gcc -c mmn13.c -DVARS_NUM=20 -DDOMAIN_SIZE=7
	gcc -o mmn13. mmn13.o

queens:	driver.c
	gcc -c driver.c -DQUEENS -DVARS_NUM=30 -DDOMAIN_SIZE=30
	gcc -o queens. *.o

clean:
	cat /dev/null > demo.
	cat /dev/null > queens.
	rm demo.
	rm queens.
	rm *.o

