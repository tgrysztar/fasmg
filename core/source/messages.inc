
_macro_source db 'macro ',0
_preprocessed_source db 'assemble ',0
_unnamed_source db '?',0
_memory_source db 'eval',0
_space db ' ',0
_line_number_prefix db ' [',0
_line_number_suffix db ']',0
_line_content_prefix db ':'
_line_segment_prefix db LINE_FEED,9,0
_calm_source db ' (CALM)',0
_single_quote db 27h,0
_preprocessed_text_prefix db 'Processed: ',0
_error_prefix db 'Error: ',0
_custom_error_prefix db 'Custom error: ',0
_message_suffix db '.'
_new_line db LINE_FEED,0

_source_file_not_found db "source file '%s' not found",0
_error_reading_file db "error reading file '%s'",0
_missing_end_quote db "missing end quote",0
_illegal_instruction db "illegal instruction",0
_invalid_argument db "invalid argument",0
_missing_argument db "missing argument",0
_extra_characters_on_line db "extra characters on line",0
_invalid_identifier db "invalid identifier",0
_invalid_number db "invalid number",0
_undefined_symbol db "symbol '%i' is undefined or out of scope",0
_invalid_symbol_value db "the value of symbol '%i' is not valid for this use",0
_conflicting_definition db "definition of '%i' in conflict with already defined symbol",0
_cannot_apply_to_constant db "cannot apply this operation to constant",0
_symbolic_self_reference db "detected a circular reference of symbolic values",0
_unexpected_instruction db "unexpected instruction",0
_missing_end_directive db "missing end directive",0
_missing_closing_chevron db "missing closing chevron",0
_invalid_value db "value of type not allowed in this context",0
_invalid_expression db "invalid expression",0
_missing_closing_parenthesis db "missing closing parenthesis",0
_excess_closing_parenthesis db "excess closing parenthesis",0
_value_out_of_range db "value out of allowed range",0
_indeterminate_result db "expression has indeterminate, infinite, or infinitesimal result",0
_misused_variable_term db "variable term used where not expected",0
_nonlinear_polynomial db "cannot multiply variable terms",0
_subdivided_variable_term db "cannot subdivide variable term",0
_values_not_comparable db "values not comparable",0
_assertion_failed db "assertion failed",0
_area_overflow db "exceeded the maximum allowed length of output area",0
_address_out_of_range db "address out of range",0
_invalid_area db "invalid or inaccessible addressing area",0
_stack_limit_exceeded db "exceeded the maximum allowed depth of stack",0
_repeated_declaration db "choice has already been declared",0
_undefined_jump_target db "the target of the jump is not defined in this instruction",0
