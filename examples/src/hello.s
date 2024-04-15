
    LDA #$25
    STA $0200
    LDA #$30
    STA $0201
    LDA #$24
    STA $0202

    LDA #$c0  ;Load the hex value $c0 into the A register
    TAX       ;Transfer the value in the A register to X
    INX       ;Increment the value in the X register
    ADC #$c4  ;Add the hex value $c4 to the A register
    ADC #$00  ;Add the carry bit
    BPL next
    DEX
next:
    SED
    LDA #$17
    ADC #$14
    SEC
    SBC #$18
    CLD

    LDX #$08
decrement:
    DEX
    STX $0203
    CPX #$03
    BNE decrement
    STX $0204

    LDA #$04
    STA $02
    LDA #$02
    STA $03
    LDX #$02
    LDA ($00,X)
    LDA #$FF
    STA ($00,X)

    PHA
    PHP
    PLA
