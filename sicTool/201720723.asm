. prime factorization program 
. 201720723 소프트웨어학과 박수린

MAIN	JSUB	INPUT		. scan한 값이 register A에 저장되어 있음
	COMP	#1
	JLT	ERROR
	COMP	#10
	JGT	ERROR		. 입력값이 1~10 사이인지 확인
	MUL	#3
	STA	COUNT		. 입력값에 word단위를 곱해서 count에 저장
	LDX	#0
	LDS	#3
	LDT	COUNT		
SCAN	JSUB	INPUT		. 값 받아오기
	COMP	#2
	JLT	ERROR		
	COMP	#999
	JGT	ERROR		. 2 이상 999 이하인지 확인
	STA	ARRAY, X	. 올바른 값이면 array에 저장
	ADDR	S, X		. index 워드 단위로 증가
	COMPR	X, T		
	JLT	SCAN		. count 만큼 숫자 입력 받기
	LDX 	#0
	LDA	ENTER
	WD	STDOUT
RESULT	LDA	#2
	STA	FACTOR
	LDA	ARRAY, X
	STA	NUM		
	JSUB	FACT		. factorization 실행
	ADDR	S, X
	COMPR	X, T
	JLT	RESULT		. count 만큼 factorization
	LDA	ENTER
	WD	STDOUT
	J	STOP

INPUT	LDA	#0		
	STA	NUM		. 입력 받을 숫자 0으로 초기화
CHECK	TD	STDIN
	JEQ	CHECK
	LDA	#0		. 255보다 큰 숫자 입력 오류 해결
	RD	STDIN		. register A에 입력값 들어감 (1자리)
	COMP	ENTER
	JEQ	NUMTOA
	COMP	SPACE
	JEQ	NUMTOA		. enter와 space면 subroutine 종료
	COMP	ZERO
	JLT	ERROR
	COMP	NINE
	JGT	ERROR		. 입력값이 숫자인지 유효성 검사
	SUB	ZERO		
	STA	TEMP		. 입력값 임시로 temp에 저장
	LDA	NUM
	MUL	#10
	ADD	TEMP		. leftmost value 옆으로 밀고 temp 더하기
	STA	NUM		
	J	CHECK
	
NUMTOA	LDA	NUM
	RSUB	

FACT	STL	RETURN		. L register에 return address 저장
	STA	SELF		. 원래의 숫자 저장
FLOOP	DIV	FACTOR		
	STA	QUOTA
	MUL	FACTOR		. factor로 나눈 후 다시 곱했을 때
	COMP	NUM		. 원래 값과 같은 지 확인
	JEQ	PRINT		. 같으면 나머지가 0이므로 factor로 출력
	LDA	FACTOR
	ADD	#1
	STA	FACTOR		. 같지 않으면 factor 1 증가
	LDA	NUM
	J	FLOOP

PRINT	LDA	QUOTA
	COMP	#1
	JEQ	PRINTE		. 몫이 1이라면 factorization의 끝을 의미
	JSUB	OUTPUT		. FACTOR 출력
	LDA	SYMUL
	WD	STDOUT		. * 기호 출력
	LDA	QUOTA
	STA	NUM		. 몫을 계속해서 factorization
	J	FLOOP

PRINTE	LDA	FACTOR
	COMP	SELF
	JEQ	PRINTP		. 마지막 factor가 자기 자신이라면 소수
	JSUB	OUTPUT		. FACTOR 출력
	LDA	ENTER
	WD	STDOUT		. new line 출력
	LDL	RETURN		. L register에 return address load
	RSUB

PRINTP	LDA	PNUM
	WD	STDOUT		. P(소수) 기호 출력
	LDA	ENTER
	WD	STDOUT
	RSUB

OUTPUT	LDA	FACTOR
	STA	NUM
	DIV	#100
	COMP	#0
	JEQ	PASS
	STA	TEMP
	ADD	ZERO
	WD	STDOUT
	LDA	TEMP
	MUL	#100
	STA	TEMP
	LDA	NUM
	SUB	TEMP
	STA	NUM
	COMP	#9
	JGT	PASS
	LDA	#0
	ADD	ZERO
	WD	STDOUT
PASS	LDA	NUM
	DIV	#10
	COMP	#0
	JEQ	PASS2
	STA	TEMP
	ADD	ZERO
	WD	STDOUT
	LDA	TEMP
	MUL	#10
	STA	TEMP
	LDA	NUM
	SUB	TEMP
	STA	NUM
PASS2	LDA	NUM
	ADD	ZERO
	WD	STDOUT
	RSUB

ERROR	LDX	#0
ELOOP	LDCH	STR, X
	WD	STDOUT
	TIX	#22
	JLT	ELOOP
	LDA	ENTER
	WD	STDOUT
	J	STOP

STOP	J	STOP

STDIN	BYTE	0
STDOUT	BYTE	1

ENTER	WORD	0x0A
SPACE	WORD	0x20		
PNUM	WORD	0x50		. P
SYMUL	WORD	0x2A		. *
ZERO	WORD	0x30		. ascii code #0
NINE	WORD	0x39		. ascii code #9
STR	BYTE	C'ERROR! Please Restart.'

COUNT	RESW	1
TEMP	RESW	1
NUM	RESW	1
ARRAY	RESW	10
FACTOR	RESW	1
QUOTA	RESW	1
SELF	RESW	1
RETURN	RESW	1
