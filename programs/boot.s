
; The memory layout of 6502-burken. Make sure this is the same
; as the listing found in `src/memory.h`.
ZERO_PAGE_LOW       = $0000
ZERO_PAGE_HIGH      = $00FF
STACK_PAGE_LOW      = $0100
STACK_PAGE_HIGH     = $01FF
VGA_TEXT_BUF_LOW    = $0200
VGA_TEXT_BUF_HIGH   = $119F
VGA_COLOR_BUF_LOW   = $11A0
VGA_COLOR_BUF_HIGH  = $11CF
VGA_CHAR_BUF_LOW    = $1200
VGA_CHAR_BUF_HIGH   = $15FF
VGA_SPRITE_BUF_LOW  = $1600
VGA_SPRITE1         = $1600
VGA_SPRITE2         = $1625
VGA_SPRITE3         = $164A
VGA_SPRITE4         = $166F
VGA_SPRITE5         = $1694
VGA_SPRITE_BUF_HIGH = $16FF
IO_PAGE_LOW         = $1700
TIMER1_CTRL         = $1700 ; 1 Byte
TIMER1_DATA         = $1701 ; 2 Bytes
TIMER2_CTRL         = $1703 ; 1 Byte
TIMER2_DATA         = $1704 ; 2 Bytes
VGA_CTRL            = $1706 ; 1 Bytes
KEYBOARD_IO_PORT    = $1708 ; 1 Bytes
IO_PAGE_HIGH        = $17FF
BOOT_SECTOR_LOW     = $1800
BOOT_SECTOR_HIGH    = $1FFF
FREE_RAM_LOW        = $2000
FREE_RAM_HIGH       = $6FFF
IO_MEM_LOW          = $7000
IO_MEM_HIGH         = $7FFF
FREE_ROM_LOW        = $8000
FREE_ROM_HIGH       = $FFF9
NMI_VECTOR          = $FFFA ; 2 bytes
RESET_VECTOR        = $FFFC ; 2 bytes
IRQ_BRK_VECTOR      = $FFFE ; 2 bytes

N_PAGES             = $FF
PAGE_SIZE           = $100
MEM_SIZE            = $10000

MASK = $2000
SOMETHING = $2001

;; to standardize subroutine argument passing and local variables
arg8_0 = $00
arg8_1 = $01
arg8_2 = $02
arg8_3 = $03
arg8_4 = $04
arg8_5 = $05
arg8_6 = $06
arg8_7 = $07

arg16_0 = $08
arg16_1 = $0a
arg16_2 = $0c
arg16_3 = $0e

tmp8_0 = $10
tmp8_1 = $11
tmp8_2 = $12
tmp8_3 = $13
tmp8_4 = $14
tmp8_5 = $15
tmp8_6 = $16
tmp8_7 = $17

tmp16_0 = $18
tmp16_1 = $1a
tmp16_2 = $1c
tmp16_3 = $1e

ret  = $A0

    .org $8000        ; this is a good place for the entry point           

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 
; VGA routines 
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; VGA_CURSOR_* holds the current cursor position. Address 0x11D0 is the first byte after 
; the color buffer. 2 bytes big.
VGA_CURSOR_POS = $11D0

; VGA_CURRENT_COLOR holds a color byte 
VGA_CURRENT_COLOR = $11D2

;---------------------------
;---------------------------
; init routine for VGA
;---------------------------
;---------------------------
vga_init:
    ; set cursor to top left of screen (i.e. first byte of text buffer:)
    jsr vga_cursor_mv_top_left
   
    ; set current color byte to some nice default 
    lda #$07
    sta VGA_CURRENT_COLOR

    ; set cursor character and blink bit of cursor
    lda VGA_CURSOR_POS      ; store cursor pos in zero page
    sta $00
    lda VGA_CURSOR_POS + 1
    sta $01
    ldy #0
    lda #127                ; load keycode for our cursor 
    sta ($00), Y            ; store acc using indirect addressing
    ldy #1
    lda VGA_CURRENT_COLOR
    ora #$80
    sta ($00), Y            ; store acc using indirect addressing
    
    ; return
    rts
    
;---------------------------
;---------------------------
; Sets the enable sprite 1 bit to 1 in the VGA_CTRL register
;---------------------------
;---------------------------
vga_enable_sprite1:
    lda VGA_CTRL
    ora #%00001000
    sta VGA_CTRL
    rts

;---------------------------
;---------------------------
; Sets the enable sprite 1 bit to 0 in the VGA_CTRL register
;---------------------------
;---------------------------
vga_disable_sprite1:
    lda VGA_CTRL
    and #%11110111
    sta VGA_CTRL
    rts

;---------------------------
;---------------------------
; Sets the blink bit to 1 in the VGA_CTRL register
;---------------------------
;---------------------------
vga_enable_blink:
    lda VGA_CTRL
    ora #%00000001
    sta VGA_CTRL
    rts

;---------------------------
;---------------------------
; Sets the inverse color bit to 1 in the VGA_CTRL register
;---------------------------
;---------------------------
vga_enable_inverse_color:
    lda VGA_CTRL
    ora #%00000010
    sta VGA_CTRL
    rts

;---------------------------
;---------------------------
; Sets the monochrome bit to 1 in the VGA_CTRL register
;---------------------------
;---------------------------
vga_enable_monochrome:
    lda VGA_CTRL
    ora #%00000100
    sta VGA_CTRL
    rts

;---------------------------
;---------------------------
; Sets the blink bit to 0 in the VGA_CTRL register
;---------------------------
;---------------------------
vga_disable_blink:
    lda VGA_CTRL
    and #%11111110
    sta VGA_CTRL
    rts

;---------------------------
;---------------------------
; Sets the inverse color bit to 0 in the VGA_CTRL register
;---------------------------
;---------------------------
vga_disable_inverse_color:
    lda VGA_CTRL
    and #%11111101
    sta VGA_CTRL
    rts

;---------------------------
;---------------------------
; Sets the monochrome bit to 0 in the VGA_CTRL register
;---------------------------
;---------------------------
vga_disable_monochrome:
    lda VGA_CTRL
    and #%11111011
    sta VGA_CTRL
    rts

;---------------------------
;---------------------------
; Moves the cursor to user defined position on screen
; 
; arg @ $00     cursor X pos
; arg @ $01     cursor Y pos
;---------------------------
;---------------------------
vga_cursor_mv_xy:

x = tmp8_4              ; mv x, y to temporary storage
y = tmp8_5

    lda arg8_0
    sta x
    lda arg8_1
    sta y

    lda #160                ; 160 * y
    sta arg8_0
    lda y
    sta arg8_1
    jsr mul_8_8
    
    lda ret                 ; load result into arg16_0
    sta arg16_0
    lda ret + 1 
    sta arg16_0 + 1
    
    lda x                   ; load result of x + VGA_TEXT_BUF_LOW into
    clc
    rol                     ; x *= 2; two bytes per tile
    sta arg16_1             ; arg16_1. This is ok, cause x i a single byte
    lda #$02                ; and VGA_TEXT_BUF_LOW starts at a new page. page 0x02.
    sta arg16_1 + 1

    jsr add_16_16           ; calculate VGA_TEXT_BUF_LOW + x + 160 * y to
                            ; figure out the cursor pos address

    lda ret                 ; store into VGA_CURSOR_POS
    sta VGA_CURSOR_POS
    lda ret + 1
    sta VGA_CURSOR_POS + 1

    rts


;---------------------------
;---------------------------
; Clears screen with character
; provided in arg8_0
;---------------------------
;---------------------------
vga_clear_screen:
    jsr vga_cursor_mv_top_left ; move to top left
loop:
    jsr vga_putc
    lda VGA_CURSOR_POS         ; compare current cursor position
    cmp #$A0                   ; with the start address of the next
    bne loop                   ; segment in memory (VGA_COLOR_BUF)
    lda VGA_CURSOR_POS + 1 
    cmp #$11
    bne loop
    rts

;---------------------------
;---------------------------
; Moves the cursor to the start 
; of the next line 
;---------------------------
;---------------------------
;vga_cursor_mv_next_line:
;    ;sec
;    lda VGA_CURSOR_POS      ; load low byte of cursor ptr
;    sec
;    sbc #160
;    cmp #0
;    beq return
;    jsr vga_cursor_mv_right
;    jmp vga_cursor_mv_next_line
;return:
;    rts

;---------------------------
;---------------------------
; Moves the cursor to start of text buffer 
;---------------------------
;---------------------------
vga_cursor_mv_top_left:
    lda #$00
    sta VGA_CURSOR_POS       ; LL
    lda #$02
    sta VGA_CURSOR_POS + 1   ; HH
    rts

;---------------------------
;---------------------------
; Moves the cursor 1 tile up 
;---------------------------
;---------------------------
vga_cursor_mv_up:
    sec
    lda VGA_CURSOR_POS
    sbc #160
    sta VGA_CURSOR_POS
    lda VGA_CURSOR_POS + 1
    sbc #0
    sta VGA_CURSOR_POS + 1
    rts

;---------------------------
;---------------------------
; Moves the cursor 1 tile down 
;---------------------------
;---------------------------
vga_cursor_mv_down:
    clc
    lda VGA_CURSOR_POS
    adc #160
    sta VGA_CURSOR_POS
    lda VGA_CURSOR_POS + 1
    adc #0
    sta VGA_CURSOR_POS + 1
    rts

;---------------------------
;---------------------------
; Moves the cursor 1 tile left 
;---------------------------
;---------------------------
vga_cursor_mv_left:
    sec
    lda VGA_CURSOR_POS
    sbc #2
    sta VGA_CURSOR_POS
    lda VGA_CURSOR_POS + 1
    sbc #0
    sta VGA_CURSOR_POS + 1
    rts

;---------------------------
;---------------------------
; Moves the cursor 1 tile right 
;---------------------------
;---------------------------
vga_cursor_mv_right:
    clc
    lda VGA_CURSOR_POS
    adc #2
    sta VGA_CURSOR_POS
    lda VGA_CURSOR_POS + 1
    adc #0
    sta VGA_CURSOR_POS + 1
    rts


;---------------------------
;---------------------------
; loads the memory at cursor with the contents of the ACC
; 
; arg @ $00    character to print
;---------------------------
;---------------------------
vga_putc:
    lda VGA_CURSOR_POS      ; store cursor pos in zero page
    sta $01
    lda VGA_CURSOR_POS + 1
    sta $02
    lda arg8_0              ; ACC <- temp
    ldy #0
    sta ($01), Y            ; store acc using indirect addressing
    ldy #1
    lda VGA_CURRENT_COLOR
    sta ($01), Y            ; store acc using indirect addressing
    jsr vga_cursor_mv_right
    rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 
; Math routines
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;---------------------------
;---------------------------
; adds two 16 bit numbers
;---------------------------
;---------------------------
add_16_16:
    clc
    lda arg16_0     ; add low bytes
    adc arg16_1
    sta ret
    lda arg16_0 + 1 ; add high bytes
    adc arg16_1 + 1
    sta ret + 1
    rts

;---------------------------
;---------------------------
; Multiplies two bytes using russian peasant algorithm
; by frantik
;
; arg @ $00     value1
; arg @ $01     value2
;
;---------------------------
;---------------------------
mul_8_8:

value1 = arg8_0
value2 = arg8_1
temp   = tmp8_0 

    lda #00         ; clear temporary variables
    sta ret
    sta ret+1
    sta temp
    lda value2
    cmp #0
    beq mul_8_8_end
    jmp mul_8_8_start

mul_8_8_loop:
    asl value1       ; double first value
    rol temp         ; using 16bit precision
    lsr value2       ; halve second vale
mul_8_8_start:
    lda value2       ;
    and #01          ; is new 2nd value an odd number?
    beq mul_8_8_loop ; 
    clc              ; if so, add new 1st value to running total
    lda ret          ;
    adc value1       ;
    sta ret          ;
    lda ret+1        ;
    adc temp         ;
    sta ret+1        ;
    lda value2       ;
    cmp #01          ; is 2nd value 1?  if so, we're done
    bne mul_8_8_loop ; otherwise, loop
mul_8_8_end:
    rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 
; Timer 1 routines 
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Set timer 1 to a ~1 sec pulse
timer1_set_1sec:
    lda #$D0
    sta TIMER1_DATA
    lda #$03
    sta TIMER1_DATA + 1
    lda #$0B
    sta TIMER1_CTRL         ; Remember to load control register after loading data registers
    rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 
; Boot program entry point.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
entry:
    jsr vga_init
    jsr vga_enable_blink
    jsr vga_enable_inverse_color
    jsr vga_enable_sprite1
    lda #39
    sta arg8_0 ; x
    lda #12
    sta arg8_1 ; y
    jsr vga_cursor_mv_xy
    nop
    nop
    nop
    nop
    jsr timer1_set_1sec
    lda #$ff
    sta MASK
    lda #%10101010
    sta SOMETHING
    nop
    nop
    nop
    lda #$31                ; set sprite color
    sta VGA_SPRITE1 + $20
    lda #100
    sta VGA_SPRITE1 + $23   ; set sprite y (ll)
    nop
    lda #0
    

deadloop:
    nop
    sta VGA_SPRITE1 + $21   ; set sprite x (ll)
    sta VGA_SPRITE1 + $23   ; set sprite y (ll)
    adc #1
    jmp deadloop


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 
; IRQ / NMI service routine(s)
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;---------------------------
;---------------------------
; ISR routine for timer
;---------------------------
;---------------------------
isr_timer:
    lda SOMETHING
    eor MASK
    sta SOMETHING
    sta arg8_0
    jsr vga_clear_screen
    rts

;---------------------------
;---------------------------
; ISR routine for keyboard
;
; return 1 if irq wasn't handled by this routine.
; return 0 if it was.
;
;---------------------------
;---------------------------
isr_keyboard:
    lda KEYBOARD_IO_PORT    ; check write/ack bit in KEYBOARD_IO_PORT
    and #$80
    bne handle_key_press    ; if 1 goto handle_key_press 
    lda #1
    sta ret
    rts                     ; if 0 do nothing

    handle_key_press:
        lda KEYBOARD_IO_PORT    ; acknowledge press by setting write/ack bit to 0
        and #$7F
        sta KEYBOARD_IO_PORT
        
        ; special keycodes
        lda #$01                ; BACKSPACE keycode
        cmp KEYBOARD_IO_PORT
        beq handle_backspace_key
    
        ;lda #$02                ; ENTER keycode
        ;cmp KEYBOARD_IO_PORT
        ;beq handle_enter_key
    
        lda #$03                ; UP ARROW keycode
        cmp KEYBOARD_IO_PORT
        beq handle_up_arrow
        
        lda #$04                ; DOWN ARROW keycode
        cmp KEYBOARD_IO_PORT
        beq handle_down_arrow
        
        lda #$05                ; LEFT ARROW keycode
        cmp KEYBOARD_IO_PORT
        beq handle_left_arrow
        
        lda #$06                ; RIGHT ARROW keycode
        cmp KEYBOARD_IO_PORT
        beq handle_right_arrow

        bne default
    
        handle_backspace_key:
            jsr vga_cursor_mv_left 
            lda #00
            sta arg8_0
            jsr vga_putc
            jsr vga_cursor_mv_left 
            jmp return
        
        ;handle_enter_key:
        ;    jsr vga_cursor_mv_next_line 
        ;    jmp return

        handle_up_arrow:
            jsr vga_cursor_mv_up 
            jmp return

        handle_down_arrow:
            jsr vga_cursor_mv_down  
            jmp return

        handle_left_arrow:
            jsr vga_cursor_mv_left 
            jmp return

        handle_right_arrow:
            jsr vga_cursor_mv_right  
            jmp return
        
        default: 
            lda KEYBOARD_IO_PORT
            sta arg8_0
            jsr vga_putc            ; ALPHANUMERIC keycode - print character to screen
            jmp return

    return:
        lda #0
        sta ret
        rts

;---------------------------
;---------------------------
; system ISR routine
;---------------------------
;---------------------------
isr_system:
    ; do isr_keyboard
    jsr isr_keyboard
    lda ret
    cmp #0
    beq isr_system_return
    
    ; do isr_timer
    jsr isr_timer
isr_system_return:
    cli                     ; clear interrupt disable bit
    rti                     ; return from isr

    .org $FFFA              ; NMI vector
    .word isr_system        ; 
    .org $FFFC              ; RESET vector
    .word entry             ; 
    .org $FFFE              ; IRQ BRK vector
    .word isr_system        ; 
