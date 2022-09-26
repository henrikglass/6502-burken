
; The memory layout of 6502-burken. Make sure this is the same
; as the listing found in `src/memory.h`.
ZERO_PAGE_LOW      = $0000
ZERO_PAGE_HIGH     = $00FF
STACK_PAGE_LOW     = $0100
STACK_PAGE_HIGH    = $01FF
VGA_TEXT_BUF_LOW   = $0200
VGA_TEXT_BUF_HIGH  = $119F
VGA_COLOR_BUF_LOW  = $11A0
VGA_COLOR_BUF_HIGH = $11CF
VGA_CHAR_BUF_LOW   = $1200
VGA_CHAR_BUF_HIGH  = $15FF
IO_PAGE_LOW        = $1600
TIMER1_CTRL        = $1600 ; 1 Byte
TIMER1_DATA        = $1601 ; 2 Bytes
TIMER2_CTRL        = $1603 ; 1 Byte
TIMER2_DATA        = $1604 ; 2 Bytes
VGA_CTRL           = $1606 ; 1 Bytes
KEYBOARD_IO_PORT   = $1608 ; 1 Bytes
IO_PAGE_HIGH       = $16FF
BOOT_SECTOR_LOW    = $1700
BOOT_SECTOR_HIGH   = $1FFF
FREE_RAM_LOW       = $2000
FREE_RAM_HIGH      = $6FFF
IO_MEM_LOW         = $7000
IO_MEM_HIGH        = $7FFF
FREE_ROM_LOW       = $8000
FREE_ROM_HIGH      = $FFF9
NMI_VECTOR         = $FFFA ; 2 bytes
RESET_VECTOR       = $FFFC ; 2 bytes
IRQ_BRK_VECTOR     = $FFFE ; 2 bytes

N_PAGES            = $FF
PAGE_SIZE          = $100
MEM_SIZE           = $10000

MASK = $2000
SOMETHING = $2001

    .org $8000        ; this is a good place for the entry point           

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 
; VGA routines 
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; VGA_CURSOR_* holds the current cursor position. Address 0x11D0 is the first byte after 
; the color buffer. 2 bytes big.
VGA_CURSOR_POS = $11D0

; VGA_CURRENT_COLOR holds a color byte 
VGA_CURRENT_COLOR = $11D2

vga_init:
    ; set cursor to top left of screen (i.e. first byte of text buffer:)
    lda #$00
    sta VGA_CURSOR_POS       ; LL
    lda #$02
    sta VGA_CURSOR_POS + 1   ; HH
   
    ; set current color byte to some nice default 
    lda #$07
    sta VGA_CURRENT_COLOR
    rts
    

; Sets the blink bit to 1 in the VGA_CTRL register
vga_enable_blink:
    lda VGA_CTRL
    ora #%00000001
    sta VGA_CTRL
    rts

; Sets the inverse color bit to 1 in the VGA_CTRL register
vga_enable_inverse_color:
    lda VGA_CTRL
    ora #%00000010
    sta VGA_CTRL
    rts

; Sets the monochrome bit to 1 in the VGA_CTRL register
vga_enable_monochrome:
    lda VGA_CTRL
    ora #%00000100
    sta VGA_CTRL
    rts

; Sets the blink bit to 0 in the VGA_CTRL register
vga_disable_blink:
    lda VGA_CTRL
    and #%11111110
    sta VGA_CTRL
    rts

; Sets the inverse color bit to 0 in the VGA_CTRL register
vga_disable_inverse_color:
    lda VGA_CTRL
    and #%11111101
    sta VGA_CTRL
    rts

; Sets the monochrome bit to 0 in the VGA_CTRL register
vga_disable_monochrome:
    lda VGA_CTRL
    and #%11111011
    sta VGA_CTRL
    rts

; Moves the cursor 1 tile (2 bytes) forward 
vga_inc_cursor_pos:
    clc
    lda VGA_CURSOR_POS
    adc #2
    sta VGA_CURSOR_POS
    lda VGA_CURSOR_POS + 1
    adc #0
    sta VGA_CURSOR_POS + 1
    rts

; loads the memory at cursor with the contents of the ACC
vga_putc:
    sta $00                 ; temp <- ACC
    lda VGA_CURSOR_POS      ; store cursor pos in zero page
    sta $01
    lda VGA_CURSOR_POS + 1
    sta $02
    lda $00                 ; ACC <- temp
    ldx #0
    sta ($01, X)            ; store acc using indirect addressing
    ldx #1
    lda VGA_CURRENT_COLOR
    sta ($01, X)            ; store acc using indirect addressing
    jsr vga_inc_cursor_pos
    rts


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 
; Timer 1 routines 
;
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
; 
; Boot program entry point.
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
entry:
    jsr timer1_set_1sec
    lda #$ff
    sta MASK
    lda #%10101010
    sta SOMETHING
    jsr vga_init
    jsr vga_enable_blink
    jsr vga_enable_inverse_color
    nop
    nop
    

deadloop:
    nop
    jmp deadloop


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 
; IRQ / NMI service routine(s)
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
isr_timer:
    lda SOMETHING
    eor MASK
    sta SOMETHING
    rts

isr_keyboard:
    lda KEYBOARD_IO_PORT    ; check write/ack bit in KEYBOARD_IO_PORT
    and #$80
    bne handle_key_press    ; if 1 goto handle_key_press 
    rts                     ; if 0 do nothing
handle_key_press:
    lda KEYBOARD_IO_PORT    ; acknowledge press by setting write/ack bit to 0
    and #$7F
    sta KEYBOARD_IO_PORT
    jsr vga_putc            ; print character to screen
    rts

isr_system:
    ;jsr isr_timer
    jsr isr_keyboard
    cli                     ; clear interrupt disable bit
    rti                     ; return from isr

    .org $FFFA              ; NMI vector
    .word isr_system        ; 
    .org $FFFC              ; RESET vector
    .word entry             ; 
    .org $FFFE              ; IRQ BRK vector
    .word isr_system        ; 
