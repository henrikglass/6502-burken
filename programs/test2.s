
  .org $8000

entry:
  nop
  lda #$FA
  nop
  nop
  adc #25
  nop
  clc
  lda #$FB
  jmp entry
  nop
