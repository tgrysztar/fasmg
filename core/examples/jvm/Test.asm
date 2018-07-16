
; This example uses a very simple set of macroinstructions to generate
; the basic structures of JVM class file.
; Please refer to "The Java Virtual Machine Specification" for the detailed
; information on this file format.

include 'jclass.inc'

format binary as 'class'

  u4 0xcafebabe 		; magic
  u2 0,49			; minor and major version

  constant_pool

	_Code			constant_utf8		'Code'
	_init			constant_utf8		'<init>'
	_main			constant_utf8		'main'
	_void_arrstr		constant_utf8		'([Ljava/lang/String;)V'
	Test_class		constant_class		_Test
	_Test			constant_utf8		'Test'

	Object_init		constant_methodref	Object_class,init_method
	Object_class		constant_class		_Object
	_Object 		constant_utf8		'java/lang/Object'
	init_method		constant_nameandtype	_init,_void
	_void			constant_utf8		'()V'

	System.out		constant_fieldref	System_class,out_field
	System_class		constant_class		_System
	_System 		constant_utf8		'java/lang/System'
	out_field		constant_nameandtype	_out,PrintStream_type
	_out			constant_utf8		'out'
	PrintStream_type	constant_utf8		'Ljava/io/PrintStream;'

	PrintStream_println	constant_methodref	PrintStream_class,println_method
	PrintStream_class	constant_class		_PrintStream
	_PrintStream		constant_utf8		'java/io/PrintStream'
	println_method		constant_nameandtype	_println,_void_str
	_println		constant_utf8		'println'
	_void_str		constant_utf8		'(Ljava/lang/String;)V'

	Integer_toString	constant_methodref	Integer_class,toString_method
	Integer_class		constant_class		_Integer
	_Integer		constant_utf8		'java/lang/Integer'
	toString_method 	constant_nameandtype	_toString,_str_int
	_toString		constant_utf8		'toString'
	_str_int		constant_utf8		'(I)Ljava/lang/String;'

	number			constant_integer	17

  end constant_pool

  u2 ACC_PUBLIC+ACC_SUPER	; access flags
  u2 Test_class 		; this class
  u2 Object_class		; super class

  interfaces

  end interfaces

  fields

  end fields

  methods

     method_info ACC_PUBLIC, _init, _void				; public void Test()

       attribute _Code

	 u2 1 ; max_stack
	 u2 1 ; max_locals

	 bytecode

		aload 0
		invokespecial Object_init
		return

	 end bytecode

	 exceptions
	 end exceptions

	 attributes
	 end attributes

       end attribute

     end method_info

     method_info ACC_PUBLIC+ACC_STATIC, _main, _void_arrstr		; public static void main(String[] args)

       attribute _Code

	 u2 3 ; max_stack
	 u2 2 ; max_locals

	 bytecode

		ldc number
		istore 1
	   example_loop:
		iload 1
		dup
		imul
		invokestatic Integer_toString
		getstatic System.out
		swap
		invokevirtual PrintStream_println
		iinc 1,-1
		iload 1
		ifne example_loop

		return

	 end bytecode

	 exceptions
	 end exceptions

	 attributes
	 end attributes

       end attribute

     end method_info

  end methods

  attributes

  end attributes
