
; Control directive IRPS that iterates through all the tokens of given text,
; similarly to its namesake in fasm 1.

; Example of use:
;	irps s, var = var + 1
;		display `s,13,10	; there should be five iterations
;	end irps

macro irps?! parameter*, text&
	local buffer,symbol  

	define buffer text   
	while 1   
		match car cdr, buffer	
			define symbol car   
			define buffer cdr   
		else   
			match any, buffer   
				define symbol any   
			end match   
			break	
		end match   
	end while   

	outscope irpv parameter, symbol
end macro 

macro end?.irps?! 
	end irpv 
end macro