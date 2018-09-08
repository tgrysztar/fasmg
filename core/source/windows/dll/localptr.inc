
macro pushd arg
{
	if arg eqtype +ebp & arg relativeto ebp
		if arg - ebp
			push eax
			lea eax,[arg]
			xchg eax,[esp]
		else
			push ebp
		end if
	else if ~ arg eq
		pushd arg
	end if
}

macro mov dest,src
{
	if src eqtype +ebp & src relativeto ebp
		if src - ebp
			lea dest,[src]
		else
			mov dest,ebp
		end if
	else
		mov dest,src
	end if
}
