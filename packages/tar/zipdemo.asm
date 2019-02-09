
format binary as 'zip'

include 'zip.inc'


zipfile 'hello.txt'

	db 'Hello world!',10

zipfile 'bytes.bin'

	repeat 256, c:0
		db c
	end repeat