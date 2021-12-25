  .org $8000    ; this is a good place for the entry point           

entry:
  lda #$ff
  sta $0501
  lda #$00
  sta $0502
  lda #$05
  sta $0500       ; Rembember to load control register after loading data registers
  lda #$01
rol_loop:
  rol A
  bcc rol_loop
ror_loop:
  ror A
  bcc ror_loop
  jmp rol_loop

  .org $9000

irq_loop:
  nop
  nop
  nop
  jmp irq_loop

  .org $fffc       ; reset vector
  .word entry      ; 
  .org $fffe       ; reset vector
  .word irq_loop   ; 
