include char.inc
TEXT_MODE EQU 03H
STRT_NAME_PROMPT DB 'Please enter your name: (Must start with a letter)',13,10,'$'
STRT_POINT_PROMPT DB 'Enter your suggested initial points: (4-Digit max)',13,10,'$'
STRT_PRESS_ENTER_PROMPT DB 'Press Enter key to continue','$'
;=============================================
;              Screen MACROS
;=============================================

;Sets the video mode (3: TEXT MODE, 13: Graphics mode) 
;PARAMATER:     mode (1 Byte)
;REGISTERS CHANGED: AH  AL
SET_VIDEO MACRO MODE
    MOV AL, MODE
    MOV AH, 0
    INT 10H
ENDM SET_VIDEO

;GET a CHAR from STDIN (WITHOUT ECHO)
;OUTPUT:    AL (CHAR)
;REGISTERS CHANGED: AH  AL
GET_CHAR MACRO
    MOV AH, 08
    INT 21H
ENDM GET_CHAR

;Display a CHAR to STDOUT 
;PARAMATER:     CHAR
;REGISTERS CHANGED: AH  DL
DISPLAY_CHAR MACRO CHAR
    MOV DL, CHAR
    MOV AH, 02
    INT 21H
ENDM DISPLAY_CHAR

;Display a $ terminated string to STDOUT 
;PARAMATER:     MSG (1st byte of message)
;REGISTERS CHANGED: AH  DX
DISPLAY_MSG MACRO MSG
    MOV DX, OFFSET MSG
    MOV AH, 09
    INT 21H
ENDM DISPLAY_MSG

;Removes Current Character in STDOUT
;REGISTERS CHANGED: AH  DL
DISPLAY_NEWLINE MACRO
    DISPLAY_CHAR 13D ;CARIAGE RETURN
    DISPLAY_CHAR 10D ;LINEFEED
ENDM DISPLAY_NEWLINE

;Displays a new line in STDOUT
;REGISTERS CHANGED: AH  DL
DISPLAY_BACKSPACE MACRO
    DISPLAY_CHAR 08H ;BACKSPACE
    DISPLAY_CHAR 20H ;SPACE
    DISPLAY_CHAR 08H ;BACKSPACE
ENDM DISPLAY_BACKSPACE

;=============================================
;              Start Screen
;=============================================

;Gets Username from STDIN (With ECHO)
;First character must be a letter
;Min size = 1, Max size = Buff[0]
;Supports backspace
;Carriage return to confirm username
;PARAMETERS:  USERNAME BUFFER
GET_USERNAME MACRO USERNAME
    LOCAL GETFIRST, GETNEXT, DONEFIRST, BACKSPACE, RETURN
    MOV BX, 0000H       ;Current Input Size
    MOV CH, USERNAME[0] ;CH == Max Input Size

    GETFIRST:
    GET_CHAR
    ISALPHA AL, DONEFIRST  
    JMP GETFIRST

    DONEFIRST:
    DISPLAY_CHAR AL
    MOV USERNAME[BX][2], AL
    INC BL
    GETNEXT:
    GET_CHAR
    CMP AL, 13D   ;CARRIAGE RETURN
    JE RETURN

    CMP AL, 08H   ;BACKSPACE
    JE BACKSPACE

    CMP BL, CH   ;Max Size Reached
    JE GETNEXT 

    DISPLAY_CHAR AL
    MOV USERNAME[BX][2], AL ;Save current character to username buffer
    INC BL
    JMP GETNEXT

    BACKSPACE:
    DISPLAY_BACKSPACE
    MOV USERNAME[BX][2], '$'
    DEC BL
    CMP BL, 0
    JE GETFIRST
    JMP GETNEXT

    RETURN:
    MOV USERNAME[1], BL
ENDM GET_USERNAME

;Gets suggested from STDIN (With ECHO)
;No trailing zeros allowed
;Min size = 1, Max size = 9999
;Supports backspace
;Carriage return to confirm
;PARAMETERS:  POINTS (2-Bytes)
GET_POINTS MACRO POINTS
    LOCAL GETFIRST, GETNEXT, VALID, RETURN
    ;The number is stored in BX
    MOV BX, 0
    MOV CX, 0400H ;CL = 0 (current size), CH = 4 (Max Size)
    MOV SI, 10    ;Used in MUL, DIV

    GETFIRST:
    GET_CHAR
    ;No trailing zeros
    CMP AL, '0'  
    JE GETFIRST

    ISDIGIT AL, DONEFIRST
    JMP GETFIRST
    
    DONEFIRST:
    DISPLAY_CHAR AL
    TO_INT AL
    MOV BL, AL
    INC CL
    GETNEXT:
    GET_CHAR
    CMP AL, 13D   ;CARRIAGE RETURN
    JE RETURN

    CMP AL, 08H   ;BACKSPACE
    JE BACKSPACE

    CMP CL, CH   ;Max Size Reached
    JE GETNEXT 

    ISDIGIT AL, VALID
    JMP GETNEXT
    VALID:
    DISPLAY_CHAR AL
    TO_INT AL

    ;DECIMAL SHIFT LEFT WITH AL
    MOV AH, 0
    XCHG AX, BX
    MUL SI
    ADD BX, AX

    INC CL
    JMP GETNEXT

    BACKSPACE:
    DISPLAY_BACKSPACE
    ;DECIMAL SHIFT RIGHT
    MOV DX, 0  ;Ensure Word to Word Division
    MOV AX, BX
    DIV SI     ;AX = AX / 10
    MOV BX, AX

    DEC CL
    CMP CL, 0
    JE GETFIRST
    JMP GETNEXT

    RETURN:
    MOV POINTS, BX
ENDM GET_POINTS


RUN_STRT_SCREEN MACRO USERNAME, POINTS
    LOCAL GET_AGAIN

    SET_VIDEO TEXT_MODE
    DISPLAY_MSG STRT_NAME_PROMPT
    GET_USERNAME USERNAME
    DISPLAY_NEWLINE

    DISPLAY_MSG STRT_POINT_PROMPT
    GET_POINTS POINTS
    DISPLAY_NEWLINE

    DISPLAY_MSG STRT_PRESS_ENTER_PROMPT
    GET_AGAIN:
    GET_CHAR
    CMP AL, 13D ;Carriage return
    JNE GET_AGAIN
ENDM RUN_STRT_SCREEN