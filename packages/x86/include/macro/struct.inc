
include '..\..\..\utility\xcalm.inc'

define struct? struct?

namespace struct?

	calminstruction instantiate: instance*, sname*, values&

		local tmp
		arrange tmp, =label instance : =sizeof.sname
		assemble tmp
		arrange tmp, =namespace instance
		assemble tmp

		match , values
		jyes default_body

		arrange tmp, =struct?.=initialize sname, values
		assemble tmp

		local i
		compute i, 1

		local field, val, stack
	    process_statement:
		arrange field, sname.=__label#i
		transform field
		jyes labeled_statement
		arrange tmp, sname.=__statement#i
		transform tmp
		jyes statement_ready
		jump finish

	    labeled_statement:
		arrange tmp, sname.=__definition#i
		transform tmp
		arrange val, =__init__#field
		transform val
		jyes given_value
		arrange val, sname.=__default__#field
		transform val
		jyes got_value
		jump statement_ready
	    given_value:
		take stack, field
	    got_value:
		arrange tmp, tmp val

	    statement_ready:

		take stack, i
		take stack, sname
		assemble tmp
		take , sname
		take sname, stack
		take , i
		take i, stack

		take field, stack
		jno next_statement
		arrange tmp, sname.=__size__#field - (=$ - field)
		compute tmp, tmp
		check tmp = 0
		jyes next_statement
		check tmp > 0
		jyes fill_up
		stringify field
		asm err 'value too long to fit in ',field
		jump next_statement
	    fill_up:
		arrange tmp, =rb tmp
		assemble tmp

	    next_statement:
		compute i, i + 1
		jump	process_statement

	    default_body:
		arrange tmp, sname.=__body
		assemble tmp

	    finish:
		asm end namespace
	end calminstruction

	calminstruction initialize sname*, values&

		local j, a
		compute j, 0
		arrange a, sname.=__argument

		local tmp, field, val, sub
	    process_argument:
		match field=, values, values, <>
		jyes got_argument
		arrange field, values
		arrange values,
	    got_argument:
		match field:val?, field, <>
		jyes labeled_argument
		check j < 0
		jyes mixed_arguments
		compute j, j + 1
		arrange val, field
		match <val>, val
		arrange field, a#j
		transform field
		jno excess_arguments
		match <tmp>, field
		jno got_argument_field
		take a, tmp
		compute tmp, 0
		take j, tmp
		take values, val
		jump process_argument

	    excess_arguments:
		asm err 'excess arguments'
		jump arguments_processed
	    mixed_arguments:
		asm err 'unsupported mixing of labeled and ordered values'
		jump arguments_processed

	    labeled_argument:
		check j > 0
		jyes mixed_arguments
		compute j, -1
		match <val>, val
	    got_argument_field:
		arrange tmp, sname.=__default__#field
		transform tmp
		jyes argument_ok
		match field.sub, field
		jno unknown_field
		arrange tmp, sname.=__default__#field
		transform tmp
		jno unknown_field
		arrange val, sub:val
		arrange tmp, =__multi__#field
		transform tmp
		jno append_value
		arrange val, tmp, val
		arrange tmp, =__multi__#field
	    append_value:
		publish tmp, val
		jump argument_ok
	    unknown_field:
		stringify field
		arrange tmp, sname
		stringify tmp
		asm err tmp,' has no field named ',field
		jump next_argument

	    argument_ok:
		arrange tmp, =__init__#field
		publish tmp, val
	    next_argument:
		match , values
		jno process_argument
	    arguments_processed:
		take , values
		take , j
		take , a
		take values, values
		jyes next_argument

	end calminstruction

	define pname
	define i
	define j
	define u
	define a

	calminstruction (label) collect?! definition&
		local sym, default, tmp
		match , definition
		jyes plain
		match :tmp?, definition
		jyes ignored
		match ==tmp?, definition
		jyes ignored
		arrange default,
		match definition= default, definition
		arrange definition, label definition
		arrange sym, pname.=__definition#i
		publish sym:, definition
		match ?label, label
		arrange sym, pname.=__label#i
		publish sym:, label
		arrange sym, pname.=__default__#label
		publish sym:, default
		arrange definition, definition default
		assemble definition
		arrange definition, =__size__#label == =$ - label
		assemble definition
		match , a
		jyes done
		compute j, j + 1
		arrange sym, a#j
		publish sym:, label
	    done:
		exit
	    ignored:
		arrange label, label definition
	    plain:
		assemble label
	end calminstruction

	calminstruction collect?! statement&
		local proto, sym, tmp
		check i
		jyes in_body

		compute u, 0
		compute i, 1
		compute j, 1
		match pname= proto, pname
		arrange statement, =macro pname.=__body
		assemble statement
		jno prototype_copied
	    use_prototype:
		arrange a,
		arrange tmp, proto.=__statement#i
		transform tmp
		jno prototype_arguments
		arrange sym, pname.=__statement#i
		publish sym:, tmp
		assemble tmp
		compute i, i + 1
		jump use_prototype
	    prototype_arguments:
		arrange tmp, proto.=__argument#j
		transform tmp
		jno prototype_copied
		arrange sym, pname.=__argument#j
		publish sym:, tmp
		compute j, j + 1
		jump	prototype_arguments
	    prototype_copied:
		compute j, j - 1
		arrange a, pname.=__argument
		exit

	    in_body:
		match =ends?, statement
		jyes close
		check u
		jno process_definition
		check u > 1
		jno union_divider_ok
		arrange tmp, =__union_divider
		arrange sym, pname.=__statement#i
		publish sym:, tmp
		assemble tmp
		compute i, i + 1
		arrange a,
	    union_divider_ok:
		compute u, u + 1
	    process_definition:
		match =struct?, statement
		jyes open_struct
		match =union?, statement
		jyes open_union
		arrange sym, pname.=__statement#i
		publish sym:, statement
		assemble statement
		compute i, i + 1
		exit
	    open_union:
		arrange tmp, =__union_start
		arrange sym, pname.=__statement#i
		publish sym:, tmp
		assemble tmp
		compute i, i + 1
		arrange tmp, a
		take a, tmp
		compute tmp, 1
		take u, tmp
		exit
	    open_struct:
		match , a
		jyes unlisted_substruct
		compute j, j + 1
		arrange sym, a#j
		arrange tmp, a#j#=_
		take a, tmp
		arrange tmp, <a>
		publish sym:, tmp
		jump begin_substruct
	    unlisted_substruct:
		arrange tmp,
		take a, tmp
	    begin_substruct:
		compute tmp, 0
		take j, tmp
		compute tmp, 0
		take u, tmp
		exit
	    close:
		check u
		jno close_struct
		arrange tmp, =__union_end
		arrange sym, pname.=__statement#i
		publish sym:, tmp
		assemble tmp
		compute i, i + 1
		take , a
		take , u
		exit
	    close_struct:
		take , a
		take , j
		take , u
		take a, a
		jyes done
		asm end macro
		assemble statement
	    done:
	end calminstruction

end namespace

macro struct? declaration*
	define struct?.pname declaration
	struct?.i = 0
	mvstruc ?, struct?.collect
	mvmacro ?, struct?.collect
	esc ; dummy line to launch the body definition
end macro

macro ends?!
	mvmacro struct?.collect, ?
	mvstruc struct?.collect, ?

	match name, struct?.pname

		virtual at 0
			namespace name
				name.__body
			end namespace
			sizeof.name := $
			label name: sizeof.name at 0
		end virtual

		local sname
		define sname name

		calminstruction (instance) name values&
			arrange values, =struct?.=instantiate instance, sname, values
			assemble values
		end calminstruction

		calminstruction name values&
			local ic
			init ic
			compute ic, ic + 1
			arrange values, =struct?.=instantiate sname#ic, sname, values
			assemble values
		end calminstruction

	end match
end macro

macro __union_start
	local union
	union:
	union.i = 0
	union.size = 0
	union.initialized = 0
	macro __union_open
		union.i = union.i + 1
		if (defined union.init & union.i <> union.init) | (~ defined union.init & union.i > 1)
			virtual at union
		end if
	end macro
	macro __union_close
		if $@ > union
			if union.i > 1
				union.init := union.i
			end if
			if union.initialized
				err 'conflicting initialization of union'
			else
				union.initialized = union.i
			end if
		end if
		if $ - union > union.size
			union.size = $ - union
		end if
		if (defined union.init & union.i <> union.init) | (~ defined union.init & union.i > 1)
			end virtual
		end if
	end macro
	macro __union_divider
		__union_close
		__union_open
	end macro
	macro __union_end
		__union_close
		if $ - union < union.size
			rb union.size - ($ - union)
		end if
		purge __union_open,__union_close,__union_divider,__union_end
	end macro
	__union_open
end macro
