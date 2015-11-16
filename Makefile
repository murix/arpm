
all:
	gcc arpm.c -o arpm -lcurl -Wall -lsqlite
	install arpm /bin/arpm
	rm -f arpm


