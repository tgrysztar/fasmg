
macro directory definitions&
	iterate <type,label>, definitions
		root@resource dd 0,%t,0,%% shl 16
		local total,current,next
		total = %%
		current = -1
		next = 0
		while current < next
			current = next
			repeat total
				indx %
				if type = current
					dd type,80000000h+label-root@resource
				else if type > current & ( next = current | next > type )
					next = type
				end if
			end repeat
		end while
		break
	end iterate
end macro

macro resource dir,definitions&
	iterate <id,lang,label>, definitions
		dir dd 0,%t,0,%% shl 16
		local total,current,next,counter
		total = %%
		current = -1
		next = 0
		while current < next
			current = next
			counter = 0
			repeat total
				indx %
				if id = current
					if counter = 0
						dd id,80000000h+label.directory-root@resource
					end if
					counter = counter + 1
					label.count = counter
				else if id > current & ( next = current | next > id )
					next = id
				end if
			end repeat
		end while
		current = -1
		next = 0
		while current < next
			current = next
			counter = 0
			repeat total
				indx %
				if id = current
					if counter = 0
						label.directory dd 0,%t,0,label.count shl 16
						dd lang,label-root@resource
						label.resid = id
					else
						dd lang,label-root@resource
					end if
					counter = counter + 1
				else if id > current & ( next = current | next > id )
					next = id
				end if
			end repeat
		end while
		break
	end iterate
end macro

macro bitmap label,bitmap_file
	local data,size
	label dd RVA data,size,0,0
	data file bitmap_file:0Eh
	size = $ - data
	align 4
end macro

macro icon group,definitions&
	local data,size,position,header
	iterate <label,icon_file>, definitions
		virtual at 0
			file icon_file:6,16
			load size:dword from 8
			load position:dword from 12
		end virtual
		label dd RVA data#%,size,0,0
		data#% file icon_file:position,size
		if % = %%
			align 4
			group dd RVA header,6+%%*14,0,0
			header dw 0,1,%%
			repeat %%
				indx %
				file icon_file:6,12
				dw label.resid
			end repeat
			align 4
		end if
	end iterate
end macro

macro cursor group,definitions&
	local data,header
	iterate <label,cursor_file>, definitions
		virtual at 0
			file cursor_file:6,16
			load label.width:byte from 0
			load label.height:byte from 1
			load label.size:dword from 8
			load label.position:dword from 12
		end virtual
		label dd RVA data#%,label.size+4,0,0
		data#% file cursor_file:10,4
		file cursor_file:label.position,label.size
		if % = %%
			align 4
			group dd RVA header,6+%%*14,0,0
			header dw 0,2,%%
			repeat %%
				indx %
				dw label.width,label.height,1,0
				dd label.size+4
				dw label.resid
			end repeat
			align 4
		end if
	end iterate
end macro

macro menu label
	local data,size
	label dd RVA data,size,0,0
	data dw 1,4,0,0
	local menu_level
	menu_level = 1
	macro menuitem string,id,resinfo:0,status:0,type:0
		dd MFT_STRING or type,status,id
		dw resinfo
		du string,0
		align 4
		if resinfo and MFR_END
			menu_level = menu_level - 1
		end if
		if resinfo and MFR_POPUP
			menu_level = menu_level + 1
			dd 0
		end if
		if menu_level = 0
			size = $ - data
			purge menuitem,menuseparator
		end if
	end macro
	macro menuseparator resinfo:0
		dd MFT_SEPARATOR,0,0
		dw resinfo,0
		if resinfo and MFR_END
			menu_level = menu_level - 1
		end if
		if menu_level = 0
			size = $ - data
			purge menuitem,menuseparator
		end if
	end macro
end macro

macro dialog label,title,x,y,cx,cy,style,exstyle:0,menu:0,fontname:'MS Sans Serif',fontsize:8
	local data,size,items
	label dd RVA data,size,0,0
	data dd style or DS_SETFONT,exstyle
	dw items,x,y,cx,cy
	if menu <> 0
		dw 0FFFFh
	end if
	du menu,0,title,0
	du fontsize,fontname,0
	align 4
	local dialog_items_counter
	dialog_items_counter = 0
	macro dialogitem class,it_title,it_id,it_x,it_y,it_cx,it_cy,it_style,it_exstyle:0
		dd it_style or WS_CHILD,it_exstyle
		dw it_x,it_y,it_cx,it_cy,it_id
		if class eq 'BUTTON'
			dw 0FFFFh,80h
		else if class eq 'EDIT'
			dw 0FFFFh,81h
		else if class eq 'STATIC'
			dw 0FFFFh,82h
		else if class eq 'LISTBOX'
			dw 0FFFFh,83h
		else if class eq 'SCROLLBAR'
			dw 0FFFFh,84h
		else if class eq 'COMBOBOX'
			dw 0FFFFh,85h
		else
			du class,0
		end if
		match any=,any, it_title
			du it_title,0
		else if it_title eqtype ''
			du it_title,0
		else
			dw 0FFFFh,it_title
		end if
		dw 0
		align 4
		dialog_items_counter = dialog_items_counter + 1
	end macro
	macro enddialog
		size = $ - data
		items = dialog_items_counter
		purge dialogitem,enddialog
	end macro
end macro

macro accelerator label,definitions&
	local data,size
	label dd RVA data,size,0,0
	data:
	iterate <fvirt,key,cmd>, definitions
		if % = %%
			dw fvirt or 80h,key
			size = %% * 8
		else
			dw fvirt,key
		end if
		dd cmd
	end iterate
end macro

macro versioninfo label,fileos,filetype:0,filesubtype:0,lang,cp:0,values&
	local data,size,vivalue,visize
	label dd RVA data,size,0,0
	data dw size,visize,0
	du 'VS_VERSION_INFO',0,0
	vivalue dd 0FEEF04BDh,00010000h
	local version,count,shift,char,filever,productver
	filever = 0
	productver = 0
	iterate <name,value>, values
		if name eq 'FileVersion' | name eq 'ProductVersion'
			virtual at 0
				db value
				count = $
				version = 0
				shift = 16
				repeat count
					load char:byte from %-1
					if char='.'
						if shift mod 32
							shift = shift-16
						else
							shift = shift+32+16
						end if
					else
						version = (version and not (0FFFFh shl shift)) or ((version shr shift and 0FFFFh)*10+char-'0') shl shift
					end if
				end repeat
			end virtual
			if name eq 'FileVersion'
				filever = version
			else if name eq 'ProductVersion'
				productver = version
			end if
		end if
	end iterate
	dq filever,productver
	dd 0,0,fileos,filetype,filesubtype,0,0
	visize = $ - vivalue
	local sfi_data,sfi_size
	sfi_data dd sfi_size
	du 1,'StringFileInfo',0
	local str_data,str_size
	str_data dd str_size
	du 1,'040904E4',0
	macro vdata name,value&
		local vs_data,vs_size,value_data,value_size
		align 4
		vs_data dw vs_size,value_size/2
		du 1,name,0
		align 4
		value_data du value,0
		value_size = $ - value_data
		vs_size = $ - vs_data
	end macro
	iterate <name,value>, values
		vdata name,value
	end iterate
	purge vdata
	align 4
	str_size = $ - str_data
	sfi_size = $ - sfi_data
	local vfi_data,vfi_size,var_data,var_size
	vfi_data dd vfi_size
	du 1,'VarFileInfo',0,0
	var_data dw var_size,4
	du 0,'Translation',0,0
	dw lang,cp
	var_size = $ - var_data
	vfi_size = $ - vfi_data
	size = $ - data
end macro

macro resdata label
	local data,size
	label dd RVA data,size,0,0
	data = $
	macro endres
		size = $ - data
		align 4
		purge endres
	end macro
end macro
