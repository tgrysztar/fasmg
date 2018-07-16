
macro export dllname,exports&
	iterate <label,string>, exports

		local module,addresses,names,ordinal,count
		count = %%
		dd 0,0,0,RVA module,1
		dd count,count,RVA addresses,RVA names,RVA ordinal
		addresses:
		repeat count
			indx %
			dd RVA label
		end repeat
		names:
		repeat count
			dd RVA names.name#%
		end repeat
		ordinal:
		repeat count
			dw %-1
		end repeat
		module db dllname,0
		repeat count
			indx %
			names.name#% db string,0
		end repeat

		local x,y,z,str1,str2,v1,v2
		x = count shr 1
		while x > 0
			y = x
			while y < count
				z = y
				while z-x >= 0
					load v1:dword from names+z*4
					str1 = ($-(RVA $))+v1
					load v2:dword from names+(z-x)*4
					str2 = ($-(RVA $))+v2
					while v1 > 0
						load v1:byte from str1+%-1
						load v2:byte from str2+%-1
						if v1 <> v2
							break
						end if
					end while
					if v1 < v2
						load v1:dword from names+z*4
						load v2:dword from names+(z-x)*4
						store v1:dword at names+(z-x)*4
						store v2:dword at names+z*4
						load v1:word from ordinal+z*2
						load v2:word from ordinal+(z-x)*2
						store v1:word at ordinal+(z-x)*2
						store v2:word at ordinal+z*2
					else
						break
					end if
					z = z-x
				end while
				y = y+1
			end while
			x = x shr 1
		end while

		break
	end iterate
end macro
