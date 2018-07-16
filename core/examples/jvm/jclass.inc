
ACC_PUBLIC	 = 0x0001
ACC_PRIVATE	 = 0x0002
ACC_PROTECTED	 = 0x0004
ACC_STATIC	 = 0x0008
ACC_FINAL	 = 0x0010
ACC_SUPER	 = 0x0020
ACC_SYNCHRONIZED = 0x0020
ACC_NATIVE	 = 0x0200
ACC_INTERFACE	 = 0x0200
ACC_ABSTRACT	 = 0x0400
ACC_STRICT	 = 0x0800

macro u1 values&
  irp v,values
    db v
  end irp
end macro

macro u2 values&
  irp v,values
    db (v) bswap 2
  end irp
end macro

macro u4 values&
  irp v,values
    db (v) bswap 4
  end irp
end macro

macro constant_pool

  u2 constant_pool_count
  constant_pool_counter = 1

  struc constant_utf8 string&
      . = constant_pool_counter
      constant_pool_counter = constant_pool_counter + 1
      local data,length
      u1 1
      u2 length
      data: db string
      length = $ - data
  end struc

  struc constant_integer value
    . = constant_pool_counter
    constant_pool_counter = constant_pool_counter + 1
    u1 3
    u4 value
  end struc

  struc constant_float value
    . = constant_pool_counter
    constant_pool_counter = constant_pool_counter + 1
    u1 4
    u4 value
  end struc

  struc constant_long value
    . = constant_pool_counter
    constant_pool_counter = constant_pool_counter + 1
    u1 5
    u4 value shr 32,value and 0FFFFFFFFh
  end struc

  struc constant_double value
    . = constant_pool_counter
    constant_pool_counter = constant_pool_counter + 1
    u1 6
    u4 value shr 32,value and 0FFFFFFFFh
  end struc

  struc constant_class name_index
    . = constant_pool_counter
    constant_pool_counter = constant_pool_counter + 1
    u1 7
    u2 name_index
  end struc

  struc constant_string string_index
    . = constant_pool_counter
    constant_pool_counter = constant_pool_counter + 1
    u1 8
    u2 string_index
  end struc

  struc constant_fieldref class_index,name_and_type_index
    . = constant_pool_counter
    constant_pool_counter = constant_pool_counter + 1
    u1 9
    u2 class_index
    u2 name_and_type_index
  end struc

  struc constant_methodref class_index,name_and_type_index
    . = constant_pool_counter
    constant_pool_counter = constant_pool_counter + 1
    u1 10
    u2 class_index
    u2 name_and_type_index
  end struc

  struc constant_interfacemethodref class_index,name_and_type_index
    . = constant_pool_counter
    constant_pool_counter = constant_pool_counter + 1
    u1 11
    u2 class_index
    u2 name_and_type_index
  end struc

  struc constant_nameandtype name_index,descriptor_index
    . = constant_pool_counter
    constant_pool_counter = constant_pool_counter + 1
    u1 12
    u2 name_index
    u2 descriptor_index
  end struc

end macro

macro end?.constant_pool
  constant_pool_count = constant_pool_counter
  restruc constant_utf8,constant_integer,constant_float,constant_long,constant_double
  restruc constant_class,constant_string
  restruc constant_fieldref,constant_methodref,constant_interfacemethodref,constant_nameandtype
end macro

macro interfaces
  u2 interfaces_count
  interfaces_counter = 0
  macro interface interface
    interfaces_counter = interfaces_counter + 1
    u2 interface
  end macro
end macro

macro end?.interfaces
  interfaces_count = interfaces_counter
  purge interface
end macro

macro attributes
  local count,counter
  u2 count
  counter = 0
  attributes_count equ count
  attributes_counter equ counter
  macro attribute attribute_name_index
    match sym,attributes_counter
      sym = sym + 1
    end match
    u2 attribute_name_index
    local start,length
    u4 length
    start = $
    attribute_start equ start
    attribute_length equ length
  end macro
  macro end?.attribute
    match sym,attribute_length
      sym = $ - attribute_start
    end match
    restore atribute_start,attribute_length
  end macro
end macro

macro end?.attributes
  match sym,attributes_count
    sym = attributes_counter
  end match
  restore attributes_count,attributes_counter
  purge attribute
end macro

macro fields
  u2 fields_count
  fields_counter = 0
  macro field_info access_flags,name_index,descriptor_index
    fields_counter = fields_counter + 1
    u2 access_flags
    u2 name_index
    u2 descriptor_index
    attributes
  end macro
  macro end?.field_info
    end?.attributes
  end macro
end macro

macro end?.fields
  fields_count = fields_counter
  purge field_info,end?.field_info
end macro

macro methods
  u2 methods_count
  methods_counter = 0
  macro method_info access_flags,name_index,descriptor_index
    methods_counter = methods_counter + 1
    u2 access_flags
    u2 name_index
    u2 descriptor_index
    attributes
  end macro
  macro end?.method_info
    end?.attributes
  end macro
end macro

macro end?.methods
  methods_count = methods_counter
  purge method_info,end?.method_info
end macro

macro bytecode
  local length
  bytecode_length equ length
  u4 length
  bytecode_offset = $
  org 0
end macro

macro end?.bytecode
  match sym,bytecode_length
    sym = $
  end match
  org bytecode_offset+bytecode_length
  restore bytecode_length
end macro

macro exceptions
  local length
  exception_table_length equ length
  u2 length
  exception_counter = 0
  macro exception start_pc,end_pc,handler_pc,catch_type
    exception_counter = exception_counter + 1
    u2 start_pc
    u2 end_pc
    u2 handler_pc
    u2 catch_type
  end macro
end macro

macro end?.exceptions
  match sym,exception_table_length
    sym = exception_counter
  end match
  restore exception_table_length
end macro

include 'bytecode.inc'
