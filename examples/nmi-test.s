            .ORG $2000
Begin:      LDX #$FF
Delay1:     LDY #$FF
Delay2:     DEY
            BNE Delay2
            DEX
            BNE Delay1
            JMP Begin
            BRK
            BRK
            BRK
            BRK
            
NMI_handler:
            PHA
            TXA
            PHA
            TYA
            PHA
            LDA #$10
            ASL A
            CLC
            ADC #$01
            STA $00
            PLA
            TAY
            PLA
            TAX
            PLA
            RTI
            
Exit:       BRK