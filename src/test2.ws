c	Comment
asm     org     $0000	Origin Address
i	jp	next	Jump to next page
c	=======
asm	org	$30
c	=======
lb	next		aaa
i	nand	zero
i	nand	allone
c	=======
lb	loop0
i	add	one
i	wr	=DISP
i	jc	loop0
i	wr	=HALT
c	=======
db	one	$01
db	zero	$00
db	allone	$ff
c	=======
