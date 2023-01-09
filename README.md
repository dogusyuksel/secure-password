This app is used for saving your passwords securely in your files ystem.
Sudo permission and aes encryption is used simultaneously.
End to forget passwords!!

help for 'spwd' ver: 00.01

	--version:	returns version
	--set:	requires arg, like password keyword. Also requires 'data' option
	--data:	requires arg like the password itself. Also requires 'set' option
	--del:	requires arg, like password keyword.
	--dump:	requires arg. Will dump all similar-content-keyword-passwords
	--dump-all:	no arg required. Will dump all keyword-passwords pair

eg:
	sudo ./spwd --set myPWD1 --data 12345
	sudo ./spwd --set myPWD2 --data 67890
	sudo ./spwd --del myPWD
	sudo ./spwd --dump pwd
	sudo ./spwd --dump-all