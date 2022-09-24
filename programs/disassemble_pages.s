  .org $8000        ; this is a good place for the entry point           

MASK = $2000

entry:
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
  jsr sr_far_away
  nop
  jmp deadloop

    .org $80f0
sr_far_away:
    nop
    nop
    lda #$40
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    sta $7ff0
    dec $7ff0
    dec $7ff0
    inc $7ff0
    dec $7ff0
    nop
    nop
    rts
    nop


  .org $fffc        ; RESET vector
  .word entry       ; 
  .org $fffe        ; IRQ BRK vector
  .word swap        ; 
