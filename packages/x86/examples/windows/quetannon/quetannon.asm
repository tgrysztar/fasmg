
format PE GUI 4.0
entry start

include 'win32a.inc'

include 'riched32.inc'

section '.text' code readable executable

  WM_SOCK = WM_USER + 100

  SOCK_IDLE   = 0
  SOCK_LISTEN = 1
  SOCK_SERVER = 2
  SOCK_CLIENT = 3

  start:
	mov	[initcomctl.dwSize],sizeof.INITCOMMONCONTROLSEX
	mov	[initcomctl.dwICC],ICC_INTERNET_CLASSES
	invoke	InitCommonControlsEx,initcomctl
	invoke	LoadLibrary,_riched
	or	eax,eax
	jz	exit
	invoke	WSAStartup,0101h,wsadata
	or	eax,eax
	jnz	exit
	invoke	GetModuleHandle,0
	invoke	DialogBoxParam,eax,IDR_DIALOG,HWND_DESKTOP,DialogProc,0
	invoke	WSACleanup
  exit:
	invoke	ExitProcess,0

  proc	DialogProc uses ebx esi edi, hwnddlg,msg,wparam,lparam
	cmp	[msg],WM_INITDIALOG
	je	wminitdialog
	cmp	[msg],WM_COMMAND
	je	wmcommand
	cmp	[msg],WM_SOCK
	je	wmsock
	xor	eax,eax
	jmp	finish
  wminitdialog:
	mov	[status],SOCK_IDLE
	jmp	processed
  wmcommand:
	mov	eax,[wparam]
	cmp	eax,IDCANCEL
	je	wmclose
	cmp	eax,IDOK
	je	.ok
	cmp	eax,ID_LISTEN
	je	.listen
	cmp	eax,ID_GETHOSTBYNAME
	je	.gethostbyname
	cmp	eax,ID_CONNECT
	je	.connect
	cmp	eax,ID_SEND
	je	.send
	shr	eax,16
	cmp	eax,EN_SETFOCUS
	je	.setfocus
	cmp	eax,EN_KILLFOCUS
	je	.killfocus
	cmp	eax,CBN_SETFOCUS
	je	.setfocus
	cmp	eax,CBN_KILLFOCUS
	je	.killfocus
	jmp	processed
  .ok:
	cmp	[focus],ID_HOSTNAME
	je	.gethostbyname
	cmp	[focus],ID_PORT
	je	.connect
	cmp	[focus],ID_SERVPORT
	je	.listen
	cmp	[focus],ID_COMMAND
	je	.send
	jmp	processed
  .setfocus:
	movzx	eax,word [wparam]
	mov	[focus],eax
	jmp	processed
  .killfocus:
	movzx	eax,word [wparam]
	cmp	[focus],eax
	jne	processed
	mov	[focus],0
	jmp	processed
  .listen:
	cmp	[status],SOCK_CLIENT
	je	processed
	cmp	[status],SOCK_IDLE
	jne	.stop
	invoke	socket,AF_INET,SOCK_STREAM,0
	cmp	eax,-1
	je	processed
	mov	[sock],eax
	mov	[saddr.sin_addr],0
	mov	[saddr.sin_family],AF_INET
	invoke	GetDlgItemInt,[hwnddlg],ID_SERVPORT,temp,FALSE
	cmp	[temp],0
	je	processed
	cmp	eax,0FFFFh
	ja	processed
	xchg	ah,al
	mov	[saddr.sin_port],ax
	invoke	bind,[sock],saddr,sizeof.sockaddr
	or	eax,eax
	jnz	.bind_failed
	invoke	listen,[sock],1
	invoke	WSAAsyncSelect,[sock],[hwnddlg],WM_SOCK,FD_ACCEPT
	mov	[status],SOCK_LISTEN
	invoke	GetDlgItem,[hwnddlg],ID_CONNECT
	invoke	EnableWindow,eax,FALSE
	invoke	SetDlgItemText,[hwnddlg],ID_LISTEN,_stop
	invoke	GetDlgItem,[hwnddlg],ID_LISTEN
	invoke	SendMessage,[hwnddlg],WM_NEXTDLGCTL,eax,TRUE
	jmp	processed
      .bind_failed:
	invoke	closesocket,[sock]
	jmp	processed
      .stop:
	invoke	closesocket,[sock]
	cmp	[status],SOCK_SERVER
	je	wmsock.disconnected
	mov	[status],SOCK_IDLE
	invoke	GetDlgItem,[hwnddlg],ID_CONNECT
	invoke	EnableWindow,eax,TRUE
	invoke	SetDlgItemText,[hwnddlg],ID_LISTEN,_listen
	jmp	processed
  .gethostbyname:
	invoke	GetDlgItemText,[hwnddlg],ID_HOSTNAME,buffer,8000h
	invoke	gethostbyname,buffer
	or	eax,eax
	jz	.bad_name
	virtual at eax
	.host	hostent
	end	virtual
	mov	eax,[.host.h_addr_list]
	mov	eax,[eax]
	mov	eax,[eax]
	bswap	eax
	invoke	SendDlgItemMessage,[hwnddlg],ID_IPADDR,IPM_SETADDRESS,0,eax
	invoke	SendDlgItemMessage,[hwnddlg],ID_HOSTNAME,CB_ADDSTRING,0,buffer
	invoke	GetDlgItem,[hwnddlg],ID_PORT
	invoke	SendMessage,[hwnddlg],WM_NEXTDLGCTL,eax,TRUE
	jmp	processed
      .bad_name:
	invoke	SendDlgItemMessage,[hwnddlg],ID_IPADDR,IPM_CLEARADDRESS,0,0
	invoke	GetDlgItem,[hwnddlg],ID_HOSTNAME
	invoke	SendMessage,[hwnddlg],WM_NEXTDLGCTL,eax,TRUE
	jmp	processed
  .connect:
	cmp	[status],SOCK_CLIENT
	je	.disconnect
	cmp	[status],SOCK_IDLE
	jne	processed
	invoke	SendDlgItemMessage,[hwnddlg],ID_IPADDR,IPM_GETADDRESS,0,temp
	mov	eax,[temp]
	bswap	eax
	mov	[saddr.sin_addr],eax
	mov	[saddr.sin_family],PF_INET
	invoke	GetDlgItemInt,[hwnddlg],ID_PORT,temp,FALSE
	cmp	[temp],0
	je	processed
	cmp	eax,0FFFFh
	ja	processed
	xchg	ah,al
	mov	[saddr.sin_port],ax
	invoke	closesocket,[sock]
	invoke	socket,AF_INET,SOCK_STREAM,0
	cmp	eax,-1
	je	processed
	mov	[sock],eax
	invoke	connect,[sock],saddr,sizeof.sockaddr_in
	or	eax,eax
	jnz	.refused
	mov	esi,_connected
	mov	eax,0x9F00
	call	write_status
	mov	[status],SOCK_CLIENT
	invoke	WSAAsyncSelect,[sock],[hwnddlg],WM_SOCK,FD_READ or FD_CLOSE
	invoke	SetDlgItemText,[hwnddlg],ID_CONNECT,_disconnect
	invoke	GetDlgItem,[hwnddlg],ID_SEND
	invoke	EnableWindow,eax,TRUE
	invoke	GetDlgItem,[hwnddlg],ID_LISTEN
	invoke	EnableWindow,eax,FALSE
	invoke	GetDlgItem,[hwnddlg],ID_COMMAND
	invoke	SendMessage,[hwnddlg],WM_NEXTDLGCTL,eax,TRUE
	jmp	processed
      .refused:
	mov	esi,_refused
	mov	eax,0xFF
	call	write_status
	jmp	processed
  .send:
	cmp	[status],SOCK_LISTEN
	jbe	processed
	invoke	GetDlgItemText,[hwnddlg],ID_COMMAND,buffer,8000h
	push	eax
	invoke	SendDlgItemMessage,[hwnddlg],ID_COMMAND,CB_ADDSTRING,0,buffer
	pop	eax
	mov	[buffer+eax],13
	inc	eax
	mov	[buffer+eax],10
	inc	eax
	mov	[buffer+eax],0
	invoke	send,[sock],buffer,eax,0
	mov	esi,buffer
	xor	eax,eax
	call	write_status
	mov	[buffer],0
	invoke	SetDlgItemText,[hwnddlg],ID_COMMAND,buffer
	jmp	processed
  .disconnect:
	invoke	closesocket,[sock]
	jmp	wmsock.disconnected
  write_status:
	mov	[charformat.cbSize],sizeof.CHARFORMAT
	mov	[charformat.dwMask],CFM_COLOR
	mov	[charformat.dwEffects],0
	mov	[charformat.crTextColor],eax
	invoke	GetDlgItem,[hwnddlg],ID_STATUS
	mov	ebx,eax
	invoke	SendMessage,ebx,EM_SETSEL,-1,-1
	invoke	SendMessage,ebx,EM_SCROLLCARET,0,0
	invoke	SendMessage,ebx,EM_SETCHARFORMAT,SCF_SELECTION,charformat
	invoke	SendMessage,ebx,EM_REPLACESEL,FALSE,esi
	retn
  wmsock:
	cmp	[status],SOCK_LISTEN
	je	.accept
	invoke	recv,[sock],buffer,8000h,0
	or	eax,eax
	jz	.disconnected
	cmp	eax,-1
	je	.no_response
	mov	[buffer+eax],0
	mov	esi,buffer
	mov	eax,0xFF0000
	call	write_status
      .no_response:
	jmp	processed
      .disconnected:
	mov	[status],SOCK_IDLE
	mov	esi,_disconnected
	mov	eax,0xFF
	call	write_status
	invoke	SetDlgItemText,[hwnddlg],ID_LISTEN,_listen
	invoke	SetDlgItemText,[hwnddlg],ID_CONNECT,_connect
	invoke	GetDlgItem,[hwnddlg],ID_SEND
	invoke	EnableWindow,eax,FALSE
	invoke	GetDlgItem,[hwnddlg],ID_LISTEN
	invoke	EnableWindow,eax,TRUE
	invoke	GetDlgItem,[hwnddlg],ID_CONNECT
	invoke	EnableWindow,eax,TRUE
	invoke	GetDlgItem,[hwnddlg],ID_HOSTNAME
	invoke	SendMessage,[hwnddlg],WM_NEXTDLGCTL,eax,TRUE
	jmp	processed
      .accept:
	invoke	accept,[sock],0,0
	cmp	eax,-1
	je	processed
	xchg	eax,[sock]
	invoke	closesocket,eax
	mov	esi,_accepted
	mov	eax,0x9F00
	call	write_status
	mov	[status],SOCK_SERVER
	invoke	WSAAsyncSelect,[sock],[hwnddlg],WM_SOCK,FD_READ or FD_CLOSE
	invoke	GetDlgItem,[hwnddlg],ID_SEND
	invoke	EnableWindow,eax,TRUE
	invoke	GetDlgItem,[hwnddlg],ID_CONNECT
	invoke	EnableWindow,eax,FALSE
	invoke	GetDlgItem,[hwnddlg],ID_COMMAND
	invoke	SendMessage,[hwnddlg],WM_NEXTDLGCTL,eax,TRUE
	jmp	processed
  wmclose:
	invoke	closesocket,[sock]
	invoke	EndDialog,[hwnddlg],0
  processed:
	mov	eax,1
  finish:
	ret
  endp

section '.data' data readable

  _riched db 'RICHED32.DLL',0

  _connect db '&Connect',0
  _disconnect db 'Dis&connect',0
  _listen db '&Listen',0
  _stop db '&Stop',0

  _refused db 'Connection refused.',13,10,0
  _connected db 'Connected.',13,10,0
  _disconnected db 'Disconnected.',13,10,0
  _accepted db 'Accepted incoming connection.',13,10,0

section '.bss' readable writeable

  sock dd ?
  temp dd ?
  focus dd ?

  initcomctl INITCOMMONCONTROLSEX
  charformat CHARFORMAT
  wsadata WSADATA
  saddr sockaddr_in

  buffer rb 8000h

  status db ?

section '.idata' import data readable

  library kernel,'KERNEL32.DLL',\
	  user,'USER32.DLL',\
	  comctl,'COMCTL32.DLL',\
	  winsock,'WSOCK32.DLL'

  import kernel,\
	 GetModuleHandle,'GetModuleHandleA',\
	 LoadLibrary,'LoadLibraryA',\
	 ExitProcess,'ExitProcess'

  import user,\
	 DialogBoxParam,'DialogBoxParamA',\
	 SendMessage,'SendMessageA',\
	 GetDlgItem,'GetDlgItem',\
	 GetDlgItemInt,'GetDlgItemInt',\
	 GetDlgItemText,'GetDlgItemTextA',\
	 SetDlgItemText,'SetDlgItemTextA',\
	 SendDlgItemMessage,'SendDlgItemMessageA',\
	 GetFocus,'GetFocus',\
	 EnableWindow,'EnableWindow',\
	 wsprintf,'wsprintfA',\
	 EndDialog,'EndDialog'

  import comctl,\
	 InitCommonControlsEx,'InitCommonControlsEx'

  import winsock,\
	 WSAStartup,'WSAStartup',\
	 WSACleanup,'WSACleanup',\
	 WSAAsyncSelect,'WSAAsyncSelect',\
	 gethostbyname,'gethostbyname',\
	 socket,'socket',\
	 bind,'bind',\
	 listen,'listen',\
	 accept,'accept',\
	 connect,'connect',\
	 recv,'recv',\
	 send,'send',\
	 closesocket,'closesocket'

section '.rsrc' resource data readable

  IDR_DIALOG = 37
  IDR_LOGO   = 7

  ID_SERVPORT	   = 0x101
  ID_LISTEN	   = 0x102
  ID_HOSTNAME	   = 0x103
  ID_GETHOSTBYNAME = 0x104
  ID_IPADDR	   = 0x105
  ID_PORT	   = 0x106
  ID_CONNECT	   = 0x107
  ID_STATUS	   = 0x108
  ID_COMMAND	   = 0x109
  ID_SEND	   = 0x10A

  directory RT_DIALOG,dialogs,\
	    RT_BITMAP,bitmaps

  resource dialogs,\
	   IDR_DIALOG,LANG_ENGLISH+SUBLANG_DEFAULT,main

  resource bitmaps,\
	   IDR_LOGO,LANG_ENGLISH+SUBLANG_DEFAULT,logo

  dialog main,'Quetannon',70,70,332,176,WS_CAPTION+WS_POPUP+WS_SYSMENU+WS_MINIMIZEBOX+DS_MODALFRAME
    dialogitem 'STATIC',IDR_LOGO,-1,4,4,248,20,WS_VISIBLE+SS_BITMAP
    dialogitem 'BUTTON','',IDOK,0,0,0,0,BS_DEFPUSHBUTTON
    dialogitem 'STATIC','&Host name:',-1,4,26,148,8,WS_VISIBLE
    dialogitem 'EDIT','',ID_HOSTNAME,4,36,148,12,WS_VISIBLE+WS_BORDER+ES_AUTOHSCROLL+WS_TABSTOP
    dialogitem 'BUTTON','>',ID_GETHOSTBYNAME,156,36,20,12,WS_VISIBLE+BS_PUSHBUTTON+WS_TABSTOP
    dialogitem 'STATIC','&IP address:',-1,180,26,72,8,WS_VISIBLE
    dialogitem 'SysIPAddress32','',ID_IPADDR,180,36,72,12,WS_VISIBLE+BS_PUSHBUTTON+WS_TABSTOP
    dialogitem 'STATIC','&Port:',-1,256,26,20,8,WS_VISIBLE
    dialogitem 'EDIT','',ID_PORT,256,36,24,12,WS_VISIBLE+WS_BORDER+ES_NUMBER+WS_TABSTOP
    dialogitem 'BUTTON','&Connect',ID_CONNECT,284,36,44,12,WS_VISIBLE+BS_PUSHBUTTON+WS_TABSTOP
    dialogitem 'STATIC','&Port:',-1,256,2,20,8,WS_VISIBLE
    dialogitem 'EDIT','',ID_SERVPORT,256,12,24,12,WS_VISIBLE+WS_BORDER+ES_NUMBER+WS_TABSTOP
    dialogitem 'BUTTON','&Listen',ID_LISTEN,284,12,44,12,WS_VISIBLE+BS_PUSHBUTTON+WS_TABSTOP
    dialogitem 'RichEdit','',ID_STATUS,4,52,324,104,WS_VISIBLE+WS_BORDER+WS_VSCROLL+ES_AUTOHSCROLL+ES_AUTOVSCROLL+ES_MULTILINE+ES_READONLY+WS_TABSTOP
    dialogitem 'EDIT','',ID_COMMAND,4,160,228,12,WS_VISIBLE+WS_BORDER+ES_AUTOHSCROLL+WS_TABSTOP
    dialogitem 'BUTTON','&Send',ID_SEND,236,160,44,12,WS_VISIBLE+WS_DISABLED+BS_PUSHBUTTON+WS_TABSTOP
    dialogitem 'BUTTON','E&xit',IDCANCEL,284,160,44,12,WS_VISIBLE+BS_PUSHBUTTON+WS_TABSTOP
  enddialog

  bitmap logo,'logo.bmp'
