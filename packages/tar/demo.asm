
format binary as 'tar'

include 'tar.inc'


tar_record 'hello.txt'

	db 'Hello world!',10

end tar_record

tar_record 'bytes.bin'

	repeat 256, c:0
		db c
	end repeat

end tar_record

db 2*512 dup 0	; two null records to mark the end of tarball
