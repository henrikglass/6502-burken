    .org $8000        ; this is a good place for the entry point           

MASK = $2000

entry:
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    lda #$D0
    sta $1601
    lda #$03
    sta $1602
    lda #$0B
    sta $1600         ; Rembember to load control register after loading data registers
    lda #$ff
    sta MASK
    lda #%10101010

deadloop:
    nop
    jmp deadloop

swap:
    eor MASK
    jmp deadloop
    
    .org $fffc        ; RESET vector
    .word entry       ; 
    .org $fffe        ; IRQ BRK vector
    .word swap        ; 
