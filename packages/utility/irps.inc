
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