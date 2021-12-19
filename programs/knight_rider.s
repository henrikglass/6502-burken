
  .org $8000    ; this is a good place for the entry point           

entry:
  lda #$01
rol_loop:
  rol A
  bcc rol_loop
ror_loop:
  ror A
  bcc ror_loop
  jmp rol_loop

  .org $fffc    ; reset vector
  .word entry   ; 
