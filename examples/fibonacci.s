            .ORG $2000
INPUT       = $00
RESULT      = $01

            LDX INPUT
            JSR FIBON
            STX RESULT
            JMP EXIT

FIBON:      TXA
            BNE M01
            RTS
M01:        DEX
            BNE M02
            RTS
M02:        TXA
            PHA
            JSR FIBON
            TAY
            PLA
            TAX
            TYA
            PHA
            DEX
            JSR FIBON
            STA RESULT
            PLA
            CLC
            ADC RESULT
            TAX
            RTS

EXIT:       NOP
