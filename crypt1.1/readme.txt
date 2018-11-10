To get this to run do:
	
	- sudo bash (just run this once, since need to be root user)		

	-make

	-make user

	-dmesg ( remember the major number that dmesg spits out should on the last line )
	
	-mknod /dev/crpytctl c {the major number} 0

	-./user_app



When updating any files, before you run the above command all over again do:

	- make clean
