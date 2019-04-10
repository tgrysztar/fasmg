
; Uncompressed ZIP archive generator, based on a prototype by Grom PE.

ZIP::

namespace ZIP
	VersionToExtract := 1
	VersionMadeBy := 1
	FILE_INDEX = 0
end namespace

virtual at 0
	crc32table::
	repeat 256
		r = %-1
		repeat 8
			r = r shr 1 xor (0xEDB88320 * r and 1)
		end repeat
		dd r
	end repeat
end virtual

macro zipfile? name*
	namespace ZIP
		endzipfile

		local e
		entry equ e

		e.FileName := string name
		e.FileNameLength := lengthof e.FileName
		e.LocalHeaderOffset := $%
		e.GeneralPurpose := 0
		e.CompressionMethod := 0
		e.FileTime := 0
		e.FileDate := 0
		e.FileAttributes := 0
		e.FileAttributesExt := 0
		e.ExtraField := ''
		e.ExtraFieldLength := lengthof e.ExtraField
		e.Comment := ''
		e.CommentLength := lengthof e.Comment

		macro endzipfile?
			purge endzipfile?
			FILE_INDEX = FILE_INDEX + 1
			e.UncompressedSize = $% - e.FileOffset
			e.CompressedSize = e.UncompressedSize
		end macro

		postpone ?
			c = 0xffffffff
			repeat e.CompressedSize, a:0
				load b:byte from :e.FileOffset+a
				load t:dword from crc32table:(b xor (c and 0xFF))*4
				c = c shr 8 xor t
			end repeat
			store c xor 0xffffffff :dword at :e.Crc32Offset
			store c xor 0xffffffff :dword at :e.Crc32Offset2
		end postpone

		org $%

				db 'PK',3,4
				dw VersionToExtract
				dw e.GeneralPurpose
				dw e.CompressionMethod
				dw e.FileTime
				dw e.FileDate
		e.Crc32Offset	dd ?
				dd e.CompressedSize
				dd e.UncompressedSize
				dw e.FileNameLength
				dw e.ExtraFieldLength
				db e.FileName
				db e.ExtraField
		e.FileOffset:

		org 0
	end namespace
end macro

macro endzipfile?
end macro

postpone
	namespace ZIP
		endzipfile

		NUMBER_OF_FILES := FILE_INDEX

		org $%

		central:
		irpv e,entry
				db 'PK',1,2
				dw VersionMadeBy
				dw VersionToExtract
				dw e.GeneralPurpose
				dw e.CompressionMethod
				dw e.FileTime
				dw e.FileDate
		e.Crc32Offset2	dd ?
				dd e.CompressedSize
				dd e.UncompressedSize
				dw e.FileNameLength
				dw e.ExtraFieldLength
				dw e.CommentLength
				dw 0 ; DiskNumber
				dw e.FileAttributes
				dd e.FileAttributesExt
				dd e.LocalHeaderOffset
				db e.FileName
				db e.ExtraField
				db e.Comment
		end irpv

		tail:
				db 'PK',5,6
				dw 0 ; Number of this disk
				dw 0 ; Number of the disk with the start of the central repository
				dw NUMBER_OF_FILES
				dw NUMBER_OF_FILES
				dd tail - central
				dd central
				dw 0 ; Comment length
				db '' ; Comment
	end namespace
end postpone
