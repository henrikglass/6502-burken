
  .org $8000    ; this is a good place for the entry point           

entry:
  lda #$01
loop:
  rol A
  jmp loop

  .org $fffc    ; reset vector
  .word entry   ; 
