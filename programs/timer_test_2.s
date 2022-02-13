  .org $8000        ; this is a good place for the entry point           

MASK = $2000

entry:
  lda #$ff
  sta $0501
  lda #$00
  sta $0502
  lda #$05
  sta $0500         ; Rembember to load control register after loading data registers
  lda #$ff
  sta MASK
  lda #%10101010

deadloop:
  nop
  jmp deadloop

swap:
  eor MASK
  jmp deadloop

  .org $fffc        ; reset vector
  .word entry       ; 
  .org $fffe        ; reset vector
  .word swap        ; 
