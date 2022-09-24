  .org $8000        ; this is a good place for the entry point           

MASK = $2000

entry:
    sed             ; set decimal mode = ON
    clc             ; clear carry flag
    LDA #$90
    ADC #$90        ; overflow should be set
    nop
    nop
    nop
    clc
    lda #$99
    adc #$01
    cld             ; Z clear on 6502
    nop

deadloop:
  nop
  jmp deadloop

  .org $fffc        ; RESET vector
  .word entry       ; 
  .org $fffe        ; IRQ BRK vector
  .word entry       ; 
