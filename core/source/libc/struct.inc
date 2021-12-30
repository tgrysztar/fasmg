
macro struct name*
{
	local body
	define body struc name \{
	irp d, db,rb,dw,rw,dd,rd,dq,rq \{ struc d value& \\{ define body \\\.\\#. d value \\} \}
	macro ends
	\{
		define body \\}
		irp d, db,rb,dw,rw,dd,rd,dq,rq \\{ restruc d \\}
		irpv line, body \\{ line \\}
		virtual at 0
			name name
			sizeof.#name = $
		end virtual
	\}
}
