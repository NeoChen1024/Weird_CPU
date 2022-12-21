c	Comment
asm     org     $0000	Origin Address
i	rd	one	Read from "one"
lb	loop0		Label "loop0"
i	add	one
i	wr	=DISP	Write to hex display
i	jc	loop0
i	wr	=HALT
db	one	$01
