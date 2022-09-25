
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

    .org $8000        ; this is a good place for the entry point           

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 
; VGA routines 
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

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
swap:
    eor MASK
    jmp deadloop

    .org $fffc        ; RESET vector
    .word entry       ; 
    .org $fffe        ; IRQ BRK vector
    .word swap        ; 
