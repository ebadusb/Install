page	60,132
;  Revision history:
; ----------------------------------------------------------------------------
; version	date	  by	comments
; -------- ------------- -----	----------------------------------------------
;   1.07   Oct  8, 1996   fsw	added support for LB/CAT486.
;   1.06   Nov 21, 1995   fsw	after a screw up, and a eprom is used in S0,
;				the video will install twice. patch it back
;				to C000
;   1.05   Sep 27, 1995   fsw	watching manufacturing update the board bios
;				with binary image designed to preprogram the
;				parts utility needs new option.
;   1.04   Aug 24, 1995   fsw	LB/486-i exbios read into incorrect buffer.
;				proto boards different.
;   1.03   Aug 21, 1995   fsw	LB/486-i messages changed to LB/486i.
;   1.02   Aug  9, 1995   fsw	changed some more for board id compatability
;				with SSD.
;   1.01   Aug  8, 1995   fsw	changed for board id compatability with SSD.
;   1.0    Jun 27, 1995   fsw	for the LB/486-i
;   1.0	   Jan 22, 1992   fsw	Code for lb486 flash bios programing
; ----------------------------------------------------------------------------
.186
ver 		equ	'1.07'

false		equ	0
true		equ	not false
cr		equ	0dh		; ascii carriage return
lf		equ	0ah		; ascii line feed
;lb486i		equ	300bh
lb486i		equ	301ah
lbcat		equ	301bh
cmdline		equ	128
; ----------------------------------------------------------------------------
cseg		segment public 'code'
	assume	cs:cseg,ds:cseg,es:nothing

		org	100h		; .com file
begin:		jmp	start
; ----------------------------------------------------------------------------
		align	4
filename	label	byte		; file name buffer
		db	128 dup (?)
u10sec		dw	0	; count for 10 usec loop time
file_handle	dw	0	; file handle from open file
buf_cnt		dw	8000h	; byte count returned from read file
buf_seg		dw	0
dbuf_seg	dw	0
spgm_flash	db	0,0
flash_seg	label	word
		dw	0d000h	; flash target address
bsize		dw	0	; number if 16k buffers
bseg		dw	0	; temp segment
boff		dw	0	; temp offset

signon		db	'LB/486i Flash BIOS Programing Utility ',ver,cr,lf,'$'
nofn		db	'No file name specified',cr,lf,'$'
nofile		db	'File not found',cr,lf,'$'
nomem		db	'Not enough Memory',cr,lf,'$'
frderr		db	'Error reading file',cr,lf,'$'
zlnfile		db	'Error unexpected end of file',cr,lf,'$'
not_ampro	db	'Not Ampro LB/486i',cr,lf,'$'
filename_err	db	'File name to long',cr,lf,'$'
bigfile		db	' is larger than allocated buffer size',cr,lf,'$'
erasing		db	'Erasing flash......$'
clearing	db	'Clearing Flash.....$'
pgming		db	'Programing flash...$'
done		db	'Done.',cr,lf,'$'
failed		db	'Failed.'
crlf		db	cr,lf,'$'
nomods		db	'Flash BIOS not changed, command unknown.',cr,lf,'$'

help	db	cr,lf
	db	'USE:   PGMIBIOS VIDEO=[fn] EXBIOS=[fn] ROMBIOS=[fn]',cr,lf
	db	'WHERE: VIDEO=[file name of Video BIOS]',cr,lf
	db	'       EXBIOS=[file name of Extended BIOS]',cr,lf
	db	'       ROMBIOS=[file name of ROM BIOS]',cr,lf
	db	'       UPDATE=[128k file name of Board BIOS]',cr,lf
	db	cr,lf,'$'

; ----------------------------------------------------------------------------
open_file:	mov	ax,3d00h	; open file read only
		mov	dx,offset filename
		int	21h
		ret

close_file:	mov	ah,3eh		; close the file
		mov	bx,file_handle
		int	21h
		ret

print_string:	mov	ah,09h		; print string function
		int	21h
		ret

rd_file:	push	ds
		mov	bx,file_handle
		mov	cx,4000h	; 16k at a time
		mov	dx,word ptr boff
		mov	ds,word ptr bseg
		mov	ah,03fh
		int	21h
		pop	ds
		ret

; dx=start of file name
; cx=buffers size * 16k
; es:bx=buffer address
fill_file:	mov	bsize,cx	; save size and pointers
		mov	bseg,es
		mov	boff,bx
		mov	ax,bx
		and	ax,000fh
		mov	boff,ax
		shr	bx,4
		mov	ax,es
		add	ax,bx
		mov	bseg,ax
		push	cs
		pop	es
		mov	di,offset filename
		mov	cx,128
		xor	al,al
		cld
		rep	stosb		; clear filename buffer
		mov	di,offset filename
		mov	si,dx
		mov	cx,128
fill0:		lodsb
		cmp	al,' '
		jbe	fill1
		stosb
		loop	fill0
		mov	dx,offset filename_err	; file name to long
		jmp	error_exit

fill1:		call	open_file
		jc	no_file		; file not found
		mov	file_handle,ax	; save handle

		call	rd_file
		jc	file_rerr
		cmp	ax,0		; see if zero length file
		je	zero_fl

; update pointers and keep reading into buffer
fill2:		mov	ax,bseg
		add	ax,400h
		mov	bseg,ax
		dec	word ptr bsize
		jz	fill3
		call	rd_file
		or	ax,ax
		jz	fill_done
		cmp	ax,4000h
		jb	fill_done
		jmp	fill2
fill3:		call	rd_file		; read one more time to make sure
		or	ax,ax		; the file was the correct size
		jnz	big_file	; if there was more = error
fill_done:	mov	spgm_flash,-1
		jmp	close_file

big_file:	call	close_file
		mov	si,offset filename	; display file name
bgf00:		lodsb
		or	al,al
		jz	bgf01
		mov	dl,al
		mov	ah,6
		int	21h
		jmp	bgf00
bgf01:		mov	dx,offset bigfile
		jmp	error_exit

no_file:	mov	dx,offset nofile
		jmp	error_exit

file_rerr:	mov	dx,offset frderr
		jmp	error_exit

zero_fl:	mov	dx,offset zlnfile
		jmp	error_exit

not_lb486:	mov	dx,offset not_ampro
		jmp	error_exit
no_mem:		mov	dx,offset nomem
		jmp	error_exit

show_help:	mov	dx,offset help
		jmp	error_exit

; --------------------------------------------------------------
start:		mov	sp,offset stack
		mov	dx,offset signon
		mov	ah,9
		int	21h
		push	ds
		xor	ax,ax
		mov	ds,ax
		mov	dx,0e000h		; patch video if
		in	al,80h
		cmp	word ptr ds:[42h],dx	; E000 seg
		jne	short sss0
		mov	ax,0c000h
		mov	word ptr ds:[42],ax
sss0:		pop	ds

		mov	bx,cmdline
		call	parse_string
		db	'/H',0
		jc	show_help
		call	parse_string
		db	'/?',0
		jc	show_help

		mov	bx,offset buffer
		mov	cl,4
		shr	bx,cl		; /16
		inc	bx
		mov	ah,4ah		; modify allocated memory
		int	21h

		mov	bx,(4096*2)	; request 128k
		mov	ah,48h		; alocate memory buffer
		int	21h
		jc	no_mem
		mov	buf_seg,ax
		call	prep_buffer
		mov	ax,0cd00h	; check for ampro board type
		mov	bx,-1
		mov	cx,bx
		mov	dx,cx
		int	13h
		jc	@f
		cmp	bx,lb486i
		je	ck_cmd
		cmp	dx,lb486i -15	; temp other way
		je	ck_cmd
		cmp	bx,lb486i -15	; temp other way
		je	ck_cmd
		cmp	bx,lbcat
		je	ck_cmd
@@:		jmp	not_lb486

ck_cmd:		mov	si,80h		; command line
		lodsb
		or	al,al
		jnz	get_fn
		mov	dx,offset nofn

error_exit:	call	print_string
bad_exit:	mov	ax,4c01h
		int	21h

; parse file name, use name passed in command line buffer
get_fn:		mov	bx,81
		call	parse_string
		db	'UPDATE=',0
		jnc	ck_vid

		mov	cx,8		; eight 16k buffers
		mov	ax,word ptr buf_seg
		mov	es,ax
		mov	bx,0		; es:bx=buffer address
		call	fill_file
		push	ds
		mov	ax,word ptr buf_seg
		mov	ds,ax
		add	ax,1000h
		mov	es,ax
		mov	cx,8000h
		xor	si,si
		xor	di,di
		cld
; swap upper buffer with lower buffer
bufswap:	mov	ax,word ptr es:[di]
		movsw
		mov	word ptr ds:[si-2],ax
		loop	bufswap
		pop	ds
		jmp	start_fp

ck_vid:		mov	bx,81h
		call	parse_string
		db	'VIDEO=',0
		jnc	novid
					; dx=start of file name
		mov	cx,3		; three 16k buffers
		
		;mov	es,word ptr buf_seg
		mov	ax,word ptr buf_seg
		add	ax,1000h
		mov	es,ax
		mov	bx,0		; es:bx=buffer address
		call	fill_file

novid:		mov	bx,81h
		call	parse_string
		db	'EXBIOS=',0
		jnc	noex
					; dx=start of file name
		mov	cx,1		; one 16k buffers
		;mov	es,word ptr buf_seg
		mov	ax,word ptr buf_seg
		add	ax,1000h
		mov	es,ax
		mov	bx,0c000h	; es:bx=buffer address
		call	fill_file

noex:		mov	bx,81h
		call	parse_string
		db	'ROMBIOS=',0
		jnc	start_fp
					; dx=start of file name
		mov	cx,4		; four 16k buffers
		mov	ax,word ptr buf_seg
		;add	ax,1000h
		mov	es,ax
		mov	bx,0		; es:bx=buffer address
		call	fill_file

start_fp:	cmp	byte ptr spgm_flash,-1
		jne	exit_tdos	; buffer has not been modified

		call	setspeed	; calculate timer loops
		call	flash_reset	; insure flash is in known state
		call	erase_flash	; erase the flash
		jnc	pgm_00
		jmp	bad_exit

pgm_00:		mov	dx,offset pgming
		call	print_string
		call	on_12v
		mov	ax,word ptr buf_seg
		mov	word ptr dbuf_seg,ax
		mov	es,flash_seg
		mov	cx,8192		; 131072/16 (128k/16)
pgm_01:		push	cx
		xor	di,di		; source/destination offset 
		mov	cx,16
pgm_02:		mov	es,word ptr dbuf_seg
		mov	al,es:[di]
		mov	es,word ptr flash_seg
		call	pgm_flash	; program the byte
		jc	pgm_03
		inc	di
		loop	pgm_02
		add	word ptr dbuf_seg,1
		add	word ptr flash_seg,1
		pop	cx
		loop	pgm_01

pgm_02a:	xor	al,al		; good status
		mov	dx,offset done

pgm_exit:	push	ax		; save ending status
		call	print_string
		call	off_12v
		pop	ax

exit_dos:	mov	ah,4ch
		int	21h

exit_tdos:	mov	dx,offset nomods
		call	print_string
		mov	ax,4c00h
		int	21h

; programing error occured
pgm_03:		mov	dx,offset failed
		mov	al,1
		jmp	short pgm_exit

; file read error occured
pgm_04:		mov	dx,offset frderr
		mov	al,2
		jmp	short pgm_exit

erase_flash:	mov	dx,offset clearing
		call	print_string
		call	on_12v
		call	blank_flash
		je	ef0d
		mov	ax,flash_seg
ef00:		mov	es,ax
		xor	di,di
		align	4
ef0:		mov	al,0		; fill flash with zero's before 
		call	pgm_flash	; erase.
		jc	era_fail
		inc	di
		jnz	ef0
		mov	ax,es
		add	ax,1000h	; next seg
		cmp	ax,0e000h
		jbe	ef00
ef0d:		mov	dx,offset done
		call	print_string

		mov	dx,offset erasing
		call	print_string
; start the timed erase cycle
		mov	cx,1000		; up to 1000 tries, 10 seconds of erase
		align	4
ef1:		cli
		mov	bx,cx		; save erase try counter
		mov	es,flash_seg
		xor	di,di
		mov	al,20h		; setup erase command
		mov	es:[di],al
		mov	es:[di],al	; start erase
		call	wait_10msec	; 10 msec timer
		mov	al,0a0h		; erase-verify
		mov	es:[di],al
		call	wait_6usec
		sti
		mov	ax,-1
		mov	cx,8000h
		repe	scasw
		je	ef2
		mov	cx,bx		; restore retry count
		loop	ef1
		jmp	short era_fail

ef2:		mov	ax,es
		add	ax,1000h
		mov	es,ax
		xor	di,di
		mov	ax,-1
		mov	cx,8000h
		repe	scasw
		je	era_done
		mov	cx,bx
		loop	ef1
		jmp	short era_fail

era_done:	call	off_12v
		mov	dx,offset done
		call	print_string
		clc
		ret

era_fail:	call	off_12v		; turn off 12v
		mov	dx,offset failed
		call	print_string
		stc
		ret

blank_flash:	mov	es,flash_seg
		xor	di,di
		mov	ax,-1
		mov	cx,8000h
		repe	scasw
		jne	not_blank
		mov	ax,es
		add	ax,1000h
		mov	es,ax
		xor	di,di
		mov	ax,-1
		mov	cx,8000h
		repe	scasw
not_blank:	ret

off_12v:	mov	ax,0cd09h
		mov	bx,0
		int	13h
		jmp	wait_10usec

on_12v:		mov	ax,0cd09h
		mov	bx,1
		int	13h
		mov	cx,500		; wait for vpp to stabilize
o12v0:		call	wait_10usec
		loop	o12v0
		ret

; es:di = address to program
;    al = byte to program
; 12v is on
pgm_flash:	push	cx
		push	ax
		mov	cx,25
		mov	ah,al		; save data
		align	4
pgm_f0:		cli
		mov	al,40h		; set-up program command
		mov	es:[di],al
		mov	al,ah
		mov	es:[di],al	; program
		call	wait_10usec
		mov	al,0c0h		; program verify
		mov	es:[di],al
		call	wait_6usec
		sti
		mov	al,es:[di]	; verify results
		cmp	ah,al
		je	pgm_fdone
		loop	pgm_f0		; hit it 25 up to times
		stc
pgm_fdone:	mov	byte ptr es:[di],0	; write read command
		pop	ax
		pop	cx
		ret

; reset flash device, it is assumed that 12v may be present and
; applied to vpp.
flash_reset:	push	es
		push	ax
		push	di
		mov	es,cs:flash_seg
		xor	di,di
		mov	al,-1
		mov	es:[di],al
		mov	es:[di],al
		call	wait_6usec
		inc	al
		mov	es:[di],al
		call	wait_10usec
		pop	di
		pop	ax
		pop	es
		ret

; ------------------------------------------------------------------------
; 10msec, 10usec, and 6usec timeout timers for flash
; ------------------------------------------------------------------------
		align	4
wait_10msec:	push	cx
		push	bx
		mov	cx,1000		; 1000 * 10usec
		cli
		align	4
w10m:		mov	bx,cx
		mov	cx,u10sec
		loop	$
		mov	cx,bx
		loop	w10m
		sti
		pop	bx
		pop	cx
		ret

		align	4
wait_10usec:	push	cx
		push	ax
		pushf
		mov	cx,u10sec
		cli
w10u:		in	al,80h
		loop	w10u
		popf
		pop	ax
		pop	cx
		ret

		align	4
wait_6usec:	push	cx
		push	ax
		pushf
		mov	cx,u10sec
		shr	cx,1
		cli
w6u:		in	al,80h
		loop	$
		popf
		pop	ax
		pop	cx
		ret

LoopDiv 	DW 	100
LoopTime 	DW 	5
WaitTime 	DW 	0
MsLpCnt 	DW 	10   ; 20 microsecond loop count for timer loops
;
; calculate cpu loop speed
LongTim:	SHL	WORD PTR LoopTime,1	; double loop time
		SHL	WORD PTR LoopDiv,1	; and double dividend
SetSpeed:	MOV	AH,0
		INT	1ah		; read system timer
		MOV	WaitTime,DX	; store the low order part
		MOV	CX,LoopTime
SpdW1:		PUSH	CX
		MOV	CX,55000
SpdW2:		in	al,80h
		LOOP	SpdW2		; wait and see
		POP	CX
		LOOP	SpdW1
		MOV	AH,0
		INT	1ah		; how long the loop took
		SUB	DX,WaitTime
		CMP	DX,5
		JC	LongTim		; if speed calibrate loop < 275 ms,
					; try again
		MOV	WaitTime,DX	; and store the result
		MOV	DX,0
		MOV	AX,LoopDiv
		DIV	WaitTime	; calculate the wait time loop count
		MOV	MsLpCnt,AX	; loop count for 20 microsecond loop
		SHR	AX,1
		MOV	u10sec,AX	; loop cout for 10 microsecond loop
		CLC
		RET
;
prep_buffer:	mov	ax,buf_seg
		mov	es,ax		; fill buffer with FF
		mov	cx,8000h
		xor	di,di
		mov	ax,-1
		cld
		rep	stosw
		mov	bx,es
		add	bx,1000h
		mov	es,bx
		mov	cx,8000h
		rep	stosw

; move shadowed bios into buffer encase it is not updated. if it
; is the updated data will overwrite.
		mov	ax,word ptr buf_seg
		mov	es,ax
		mov	ax,0f000h		; BIOS in bottom of buffer, 
		mov	ds,ax
		xor	si,si
		mov	cx,8000h
		rep	movsw
; move shadowed video bios into buffer encase it is not updated
		mov	ax,word ptr buf_seg
		add	ax,1000h
		mov	es,ax
		mov	ax,0c000h		; video in upper half
		mov	ds,ax
		xor	si,si
		xor	di,di
		cmp	word ptr ds:[si],0AA55h	; video bios present?
		je	mov_vid

		mov	ax,0e000h		; check to see ifmaybe video
		mov	ds,ax			; bios already present
		cmp	word ptr ds:[si],0AA55h	; video bios present here?
		jne	ck_exbios		; no video bios leave = ff
mov_vid:
		mov	cx,0c000h
		mov	ax,word ptr cs:buf_seg
		add	ax,1000h
		mov	es,ax
		rep	movsb	; put default video bios in buffer

ck_exbios:	push	es	; save buffer segment
		mov	ax,0e000h
		mov	es,ax
		mov	ds,ax
		mov	di,0c000h
		mov	ax,0AA55h
		mov	cx,4000h
		repne	scasw	; check for any 55aa in exbios area
		pop	es
		jne	ck_bdone
		mov	si,0c000h
		mov	di,0c000h
		mov	cx,4000h
		rep	movsb

; --------------------------------------------------------------
; default buffer setup completed
; --------------------------------------------------------------
ck_bdone:	mov	ax,cs	; restore default segment addresses
		mov	es,ax
		mov	ds,ax
		ret
; --------------------------------------------------------------
; parse string. bx points to string to be compared. the string to compare
; is contained inline with the code following the call and is terminated
; with a null. carry is set if match. string to match is terminated with cr.
; the inline string that is the basis for the compare must be upper case
; alpha numeric characters. commands must not duplicate as the whole
; command line is checked for match. carry set if sucessful match.
; dx points to last character of string that was matched
; --------------------------------------------------------------
parse_string:	pop	si		; get begining of compare string 
		push	bx		; save command start
		push	ax		; save ax
		mov	dx,si		; save the begining of string

; condition command line for comparasion, convert to upper case as needed

ps_fc:		push	si
		push	bx

; scan the command line and make alpha characters upper case

ps_fc0:		lodsb			; get char
		cmp	byte ptr[bx],cr	; end of command line ?
		je	ps_fc2		; yes
		cmp	byte ptr[bx],'z'; is lower case ?
		ja	ps_fc1		; more than 'z' don't convert
		cmp	byte ptr[bx],'a'; less than 'a' don't convert
		jb	ps_fc1		; 
		and	byte ptr[bx],5fh; make it upper case

ps_fc1:		inc	bx		; next char
		jmp	ps_fc0		; scan whole line for match

ps_fc2:		pop	bx		; end, restore pointers
		pop	si

; scan command line for first match

ps_loop:	lodsb			; get character
		or	al,al		; end of character string to match
		jz	ps_ext		; exit
ps_rtry:	cmp	al,[bx]		; scan for first match
		je	ps_fm		;
		cmp	byte ptr[bx],cr	; end of command line
		je	ps_te
ps_lp:		inc	bx		; next byte
		jmp	ps_rtry		; till first match

; we have first character match chech for next, if not match exit

ps_fm:		lodsb			; next byte
		inc	bx		; next char
		or	al,al		; sucessful string match, clear carry
		jz	ps_ext		; and exit
		cmp	al,[bx]		; equal ?
		je	ps_fm		; match

; if we have keyed on the first match the second or third may not match at
; this position in the command string. needs to start the scan at the current
; position again on the first character but further down the string, restore
; the begining of the compare pointer but do not reset command pointer (bx).

ps_sm:		mov	si,dx		; restore begining of pointer
		inc	bx		; point to next byte
		jmp	ps_loop		; start again

; scan inline string till end, so will return to right place

ps_te:		lodsb			; if not equal scan till end of string
		or	al,al
		jnz	ps_te		; loop till end if string
		stc			; set carry so exit will clear

; restore registers, and set/clear carry to indicate sucess or failure

ps_ext:		mov	dx,bx
		pop	ax		; restore ax
		pop	bx		; restore beging of string
		push	si		; restore return address
		cmc			; set or clear carry
		ret

; --------------------------------------------------------------
		align	16
stack		=	$+256
buffer		=	stack
; --------------------------------------------------------------
cseg		ends
		end	begin
