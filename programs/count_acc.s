
  .org $8000    ; this is a good place for the entry point           

entry:
  lda #$00
loop:
  adc #1
  jmp loop

  .org $fffc    ; reset vector
  .word entry   ; 
