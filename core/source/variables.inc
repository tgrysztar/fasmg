
characters db 256 dup ?

maximum_number_of_errors dd ?
maximum_depth_of_stack dd ?

variables:

    temporary_value dd 4 dup ?
    accumulator dd 3 dup ?

    source_file dd ?

    include_paths dd ?

    storage_blocks dd ?
    storage_free_space dd ?
    storage_free_space_length dd ?

    tree_blocks dd ?
    tree_reserve dd ?
    tree_reserve_length dd ?

    value_definition_chain dd ?
    retired_definition dd ?

    file_source_cache dd ?
    memory_source_cache dd ?
    file_data_cache dd ?

    operator_table dd ?
    root_namespace dd ?
    root_parameter_namespace dd ?
    local_parameter_namespace dd ?
    interceptor_symbol dd ?
    label_interceptor_symbol dd ?
    proxy_number dd ?

    source_context dd ?
    source_context_maximum_length dd ?
    current_pass dd ?
    current_context RecognitionContext
    local_namespace dd ?
    parameter_namespace dd ?

    directives_stack dd ?
    directives_stack_base dd ?
    directives_stack_end dd ?

    current_counter dd ?
    counters_stack_base dd ?
    counters_stack_end dd ?

    first_error dd ?

    current_area dd ?
    output_areas_list dd ?
    output_areas_list_end dd ?
    initial_output_area_entry dd ?
    current_output_area_entry dd ?
    prior_uninitialized_length dq ?
    virtual_area dd ?
    auxiliary_output_areas dd ?

    base_path dd ?
    output_extension dd ?
    output_extension_length dd ?
    output_failures dd ?

    preprocessing_workspace Workspace
    assembly_workspace Workspace
    identifier_workspace Workspace
    value_workspace Workspace
    expression_workspace Workspace
    calculation_workspace Workspace
    auxiliary_workspace Workspace

    line_start dd ?
    line_end dd ?
    line_embeddings dd ?
    line_embeddings_maximum_length dd ?
    number_of_line_embeddings dd ?
    preprocessed_context dd ?
    line_context dd ?
    embedded_context dd ?
    number_of_enclosings dd ?

    recognition_context RecognitionContext
    name_token dd ?
    case_sensitive_hash dd ?
    case_insensitive_hash dd ?
    kept_symbol dd ?
    current_symbol dd ?
    tree_stack_base dd ?
    tree_stack_end dd ?
    symbol_root dd ?
    symbol_branch dd ?
    minor_identifier dd ?

    source_end dd ?
    symbol_start dd ?
    symbol_data dd ?
    symbol_value_start dd ?
    symbol_value_end dd ?

    interceptor dd ?
    label_branch dd ?
    label_leaf dd ?
    label_interceptor dd ?
    label_instruction_start dd ?
    label_instruction_context dd ?

    display_buffer dd ?
    display_buffer_length dd ?
    display_data_length dd ?

    macro_buffer dd ?
    macro_buffer_length dd ?
    macro_leaf dd ?
    macro_end_position dd ?

    constituent_symbol dd ?
    constituent_value dd ?
    constituent_whitespace dd ?

    alm_namespace dd ?
    calm_code_buffer Workspace
    calm_code_cursor dd ?
    calm_literals_buffer Workspace
    calm_literals_cursor dd ?
    calm_auxiliary_buffer Workspace
    calm_auxiliary_cursor dd ?
    calm_line_number dd ?
    calm_instruction_number dd ?
    calm_value dd ?
    calm_literals dd ?
    calm_source_pointer dd ?
    calm_rollback_offset dd ?

    value_position dd ?
    kept_value dd ?
    converter_position dd ?
    expression_position dd ?
    expression_end dd ?
    operator_stack dd ?
    operator_stack_position dd ?
    operator_stack_base dd ?
    operator_stack_end dd ?
    calculation_position dd ?
    source_term dd ?
    destination_term dd ?
    temporary_terms dd ?
    free_temporary_terms dd ?
    temporary_floats dd ?
    iterations dd ?
    exponent dd ?
    multiplier dd ?
    divisor dd ?
    long_dividend_term dd ?
    long_divisor_term dd ?
    long_divisor_length dd ?
    division_temporary_terms dd ?
    mantissa_tail dd ?
    predicted_shift dd ?

    condition_stack dd ?
    condition_stack_base dd ?
    condition_stack_end dd ?
    initial_parentheses dd ?
    comparator dd ?
    outer_expression dd ?

    assembly_stack_base dd ?
    assembly_stack_end dd ?
    previous_symbol_end dd ?
    further_whitespace dd ?
    zero_digits dd ?
    decimal_places dd ?
    literal_exponent dd ?
    update_function dd ?
    argument_start dd ?
    macro_parameters_context dd ?
    new_local_namespace dd ?
    file_name dd ?
    string_end dd ?
    message_end dd ?
    data_area_symbol dd ?
    data_area dd ?
    data_offset dd ?
    file_offset dq ?
    data_length dd ?
    local_path dd ?

    error_line_start dd ?
    error_line_end dd ?
    error_symbol dd ?
    last_source_entry dd ?
    last_file_source_entry dd ?
    preprocessed_text_end dd ?

    source_text dd ?
    source_text_length dd ?
    tokenization_buffer dd ?
    tokenization_buffer_length dd ?
    buffer_end_offset dd ?
    file_data dd ?
    file_cache_pointer dd ?

    address_length dd ?
    data_unit_length dd ?
    uninitialized_length dq ?

    directive_block dd ?
    number_of_iterations dd ?
    number_of_parameters dd ?
    number_of_values dd ?
    parameter_index dd ?
    value_index dd ?
    value dd ?
    value_length dd ?
    metadata_length dd ?
    sequence_header_cursor dd ?
    sequence_header_length dd ?
    expression_sequence_cursor dd ?
    expression_sequence_end dd ?
    context_boundary dd ?
    whitespace_boundary dd ?
    transforming_namespace dd ?

    pattern_start dd ?
    pattern_end dd ?
    matched_context dd ?
    substitutions_end dd ?
    stored_position dd ?
    stored_substitution dd ?
    stored_context dd ?
    stored_pattern dd ?
    brackets dd ?

    instruction_value dd ?
    instruction_branch dd ?
    instruction_body dd ?

    number_of_lines dd ?

    preprocessing_mode db ?
    assembly_mode db ?
    next_pass_needed db ?
    shift_tracking db ?

    name_kind db ?
    name_volatile db ?
    symbol_class db ?
    symbol_required db ?
    symbol_expected db ?
    kept_symbol_flags db ?
    chosen_class db ?
    expected_class db ?
    symbol_definition db ?
    symbol_solid db ?
    recognizer_setting db ?
    label_solid db ?
    value_type db ?
    waiting_for_digit db ?
    literal_exponent_sign db ?
    literal_fractional_part db ?
    float_literal_status db ?
    use_raw_values db ?

    last_token db ?
    breakpoint_token db ?
    current_constituent db ?
    kept_value_in_workspace db ?
    leave_opening_parentheses db ?
    operator_argument_expected db ?
    term_type db ?
    bit_shift db ?
    shift_overflow db ?
    multiplier_sign db ?
    high_bits_count db ?
    first_sign db ?
    second_sign db ?
    comparator_priority db ?
    result_type db ?
    defined_element db ?

    source_context_affected db ?
    raw_value db ?
    substitution_active db ?
    stored_substitution_activity db ?
    unprocessed_matching db ?
    whitespace_matching db ?
    macro_definition_active db ?
    macro_flags db ?
    macro_greedy db ?
    calm_definition_active db ?

    size_specified db ?

    message_volatile db ?
    displayed_byte db ?

    alm_statement db ?
    calm_result db ?
    hidden_context db ?

variables_end:

    trace_mode db ?
